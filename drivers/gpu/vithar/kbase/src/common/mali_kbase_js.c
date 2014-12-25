/*
 *
 * (C) COPYRIGHT 2011-2012 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 */



/*
 * Job Scheduler Implementation
 */
#include <kbase/src/common/mali_kbase.h>
#include <kbase/src/common/mali_kbase_js.h>
#include <kbase/src/common/mali_kbase_js_affinity.h>

#include "mali_kbase_jm.h"
#include <kbase/src/common/mali_kbase_defs.h>

/*
 * Private types
 */

/** Bitpattern indicating the result of releasing a context */
enum
{
	/** The context was descheduled - caller should try scheduling in a new one
	 * to keep the runpool full */
	KBASEP_JS_RELEASE_RESULT_WAS_DESCHEDULED = (1u << 0),

	/** The Runpool's context attributes changed. The scheduler might be able to
	 * submit more jobs than previously, and so the caller should call
	 * kbasep_js_try_run_next_job(). */
	KBASEP_JS_RELEASE_RESULT_CTX_ATTR_CHANGE = (1u << 1)

};

typedef u32 kbasep_js_release_result;

/*
 * Private function prototypes
 */
STATIC INLINE void kbasep_js_deref_permon_check_and_disable_cycle_counter( kbase_device *kbdev,
											kbase_jd_atom * katom );

STATIC INLINE void kbasep_js_ref_permon_check_and_enable_cycle_counter( kbase_device *kbdev,
											kbase_jd_atom * katom );

STATIC kbasep_js_release_result kbasep_js_runpool_release_ctx_internal( kbase_device *kbdev, kbase_context *kctx );

/** Helper for trace subcodes */
#if KBASE_TRACE_ENABLE != 0
STATIC int kbasep_js_trace_get_refcnt( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	int as_nr;
	int refcnt = 0;

	js_devdata = &kbdev->js_data;

	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );
	as_nr = kctx->as_nr;
	if ( as_nr != KBASEP_AS_NR_INVALID )
	{
		kbasep_js_per_as_data *js_per_as_data;
		js_per_as_data = &js_devdata->runpool_irq.per_as_data[as_nr];

		refcnt = js_per_as_data->as_busy_refcount;
	}
	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );

	return refcnt;
}
#else /* KBASE_TRACE_ENABLE != 0 */
STATIC int kbasep_js_trace_get_refcnt( kbase_device *kbdev, kbase_context *kctx )
{
	CSTD_UNUSED( kbdev );
	CSTD_UNUSED( kctx );
	return 0;
}
#endif /* KBASE_TRACE_ENABLE != 0 */



/*
 * Private types
 */
enum
{
	JS_DEVDATA_INIT_NONE            =0,
	JS_DEVDATA_INIT_CONSTANTS       =(1 << 0),
	JS_DEVDATA_INIT_RUNPOOL_MUTEX   =(1 << 1),
	JS_DEVDATA_INIT_QUEUE_MUTEX     =(1 << 2),
	JS_DEVDATA_INIT_RUNPOOL_IRQ_LOCK=(1 << 3),
	JS_DEVDATA_INIT_POLICY          =(1 << 4),
	JS_DEVDATA_INIT_ALL             =((1 << 5)-1)
};

enum
{
	JS_KCTX_INIT_NONE               =0,
	JS_KCTX_INIT_CONSTANTS          =(1 << 0),
	JS_KCTX_INIT_JSCTX_MUTEX        =(1 << 1),
	JS_KCTX_INIT_POLICY             =(1 << 2),
	JS_KCTX_INIT_JSCTX_WAITQ_SCHED  =(1 << 3),
	JS_KCTX_INIT_JSCTX_WAITQ_NSCHED =(1 << 4),
	JS_KCTX_INIT_ALL                =((1 << 5)-1)
};

/*
 * Private functions
 */

/**
 * Check if the job had performance monitoring enabled and decrement the count.  If no jobs require
 * performance monitoring, then the cycle counters will be disabled in the GPU.
 *
 * No locks need to be held - locking is handled further down
 *
 * This function does not sleep.
 */

STATIC INLINE void kbasep_js_deref_permon_check_and_disable_cycle_counter( kbase_device *kbdev, kbase_jd_atom * katom )
{
	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( katom != NULL );

	if ( katom->core_req & BASE_JD_REQ_PERMON )
	{
		kbase_pm_release_gpu_cycle_counter(kbdev);
	}
}

/**
 * Check if the job has performance monitoring enabled and keep a count of it.  If at least one
 * job requires performance monitoring, then the cycle counters will be enabled in the GPU.
 *
 * No locks need to be held - locking is handled further down
 *
 * This function does not sleep.
 */

STATIC INLINE void kbasep_js_ref_permon_check_and_enable_cycle_counter( kbase_device *kbdev, kbase_jd_atom * katom )
{
	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( katom != NULL );

	if ( katom->core_req & BASE_JD_REQ_PERMON )
	{
		kbase_pm_request_gpu_cycle_counter(kbdev);
	}
}

/*
 * The following locking conditions are made on the caller:
 * - The caller must hold the kbasep_js_kctx_info::ctx::jsctx_mutex.
 * - The caller must hold the kbasep_js_device_data::runpool_mutex
 */
STATIC INLINE void runpool_inc_context_count( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_kctx_info *js_kctx_info;
	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );

	js_devdata = &kbdev->js_data;
	js_kctx_info = &kctx->jctx.sched_info;

	/* Track total contexts */
	++(js_devdata->nr_all_contexts_running);

	if ( (js_kctx_info->ctx.flags & KBASE_CTX_FLAG_SUBMIT_DISABLED) == 0 )
	{
		/* Track contexts that can submit jobs */
		++(js_devdata->nr_user_contexts_running);
	}
}

/*
 * The following locking conditions are made on the caller:
 * - The caller must hold the kbasep_js_kctx_info::ctx::jsctx_mutex.
 * - The caller must hold the kbasep_js_device_data::runpool_mutex
 */
STATIC INLINE void runpool_dec_context_count( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_kctx_info *js_kctx_info;
	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );

	js_devdata = &kbdev->js_data;
	js_kctx_info = &kctx->jctx.sched_info;

	/* Track total contexts */
	--(js_devdata->nr_all_contexts_running);

	if ( (js_kctx_info->ctx.flags & KBASE_CTX_FLAG_SUBMIT_DISABLED) == 0 )
	{
		/* Track contexts that can submit jobs */
		--(js_devdata->nr_user_contexts_running);
	}
}

/**
 * @brief check whether the runpool is full for a specified context
 *
 * If kctx == NULL, then this makes the least restrictive check on the
 * runpool. A specific context that is supplied immediately after could fail
 * the check, even under the same conditions.
 *
 * Therefore, once a context is obtained you \b must re-check it with this
 * function, since the return value could change to MALI_FALSE.
 *
 * The following locking conditions are made on the caller:
 * - In all cases, the caller must hold kbasep_js_device_data::runpool_mutex
 * - When kctx != NULL the caller must hold the kbasep_js_kctx_info::ctx::jsctx_mutex.
 * - When kctx == NULL, then the caller need not hold any jsctx_mutex locks (but it doesn't do any harm to do so).
 */
STATIC mali_bool check_is_runpool_full( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	mali_bool is_runpool_full;
	OSK_ASSERT( kbdev != NULL );

	js_devdata = &kbdev->js_data;

	is_runpool_full = (mali_bool)(js_devdata->nr_all_contexts_running >= kbdev->nr_hw_address_spaces);

	if ( kctx != NULL && (kctx->jctx.sched_info.ctx.flags & KBASE_CTX_FLAG_SUBMIT_DISABLED) == 0 )
	{
		/* Contexts that don't submit might use less of the address spaces available, due to HW workarounds */
		is_runpool_full = (mali_bool)(js_devdata->nr_user_contexts_running >= kbdev->nr_user_address_spaces);
	}

	return is_runpool_full;
}



STATIC base_jd_core_req core_reqs_from_jsn_features( kbasep_jsn_feature features )
{
	base_jd_core_req core_req = 0u;

	if ( (features & KBASE_JSn_FEATURE_SET_VALUE_JOB) != 0 )
	{
		core_req |= BASE_JD_REQ_V;
	}
	if ( (features & KBASE_JSn_FEATURE_CACHE_FLUSH_JOB) != 0 )
	{
		core_req |= BASE_JD_REQ_CF;
	}
	if ( (features & KBASE_JSn_FEATURE_COMPUTE_JOB) != 0 )
	{
		core_req |= BASE_JD_REQ_CS;
	}
	if ( (features & KBASE_JSn_FEATURE_TILER_JOB) != 0 )
	{
		core_req |= BASE_JD_REQ_T;
	}
	if ( (features & KBASE_JSn_FEATURE_FRAGMENT_JOB) != 0 )
	{
		core_req |= BASE_JD_REQ_FS;
	}
	return core_req;
}

/**
 * Picks a free address space and add the context to the Policy. Then perform a
 * transaction on this AS and RunPool IRQ to:
 * - setup the runpool_irq structure and the context on that AS
 * - Activate the MMU on that AS
 * - Allow jobs to be submitted on that AS
 *
 * Locking conditions:
 * - Caller must hold the kbasep_js_kctx_info::jsctx_mutex
 * - Caller must hold the kbase_js_device_data::runpool_mutex
 * - AS transaction mutex will be obtained
 * - Runpool IRQ lock will be obtained
 */
STATIC void assign_and_activate_kctx_addr_space( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbase_as *current_as;
	kbasep_js_per_as_data *js_per_as_data;
	long ffs_result;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );

	js_devdata = &kbdev->js_data;

	/* Find the free address space */
	ffs_result = osk_find_first_set_bit( js_devdata->as_free );
	/* ASSERT that we should've found a free one */
	OSK_ASSERT( 0 <= ffs_result && ffs_result < kbdev->nr_hw_address_spaces );
	js_devdata->as_free &= ~((u16)(1u << ffs_result));

	/*
	 * Transaction on the AS and runpool_irq
	 */
	current_as = &kbdev->as[ffs_result];
	js_per_as_data = &js_devdata->runpool_irq.per_as_data[ffs_result];
	osk_mutex_lock( &current_as->transaction_mutex );
	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );

	/* Attribute handling */
	kbasep_js_ctx_attr_runpool_retain_ctx( kbdev, kctx );

	/* Assign addr space */
	kctx->as_nr = (int)ffs_result;

	/* Activate this address space on the MMU */
	kbase_mmu_update( kctx );

	/* Allow it to run jobs */
	kbasep_js_set_submit_allowed( js_devdata, kctx );

	/* Book-keeping */
	js_per_as_data->kctx = kctx;
	js_per_as_data->as_busy_refcount = 0;

	/* Lastly, add the context to the policy's runpool - this really allows it to run jobs */
	kbasep_js_policy_runpool_add_ctx( &js_devdata->policy, kctx );
	/*
	 * Transaction complete
	 */
	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );
	osk_mutex_unlock( &current_as->transaction_mutex );

}

void kbasep_js_try_run_next_job( kbase_device *kbdev )
{
	int js;

	OSK_ASSERT( kbdev != NULL );

	for ( js = 0; js < kbdev->nr_job_slots ; ++js )
	{
		kbasep_js_try_run_next_job_on_slot( kbdev, js );
	}
}

/** Hold the kbasep_js_device_data::runpool_irq::lock for this */
mali_bool kbasep_js_runpool_retain_ctx_nolock( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_per_as_data *js_per_as_data;
	mali_bool result = MALI_FALSE;
	int as_nr;
	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );
	js_devdata = &kbdev->js_data;

	as_nr = kctx->as_nr;
	if ( as_nr != KBASEP_AS_NR_INVALID )
	{
		int new_refcnt;

		OSK_ASSERT( as_nr >= 0 );
		js_per_as_data = &js_devdata->runpool_irq.per_as_data[as_nr];

		OSK_ASSERT( js_per_as_data->kctx != NULL );

		new_refcnt = ++(js_per_as_data->as_busy_refcount);
		KBASE_TRACE_ADD_REFCOUNT( kbdev, JS_RETAIN_CTX_NOLOCK, kctx, NULL, 0u,
								  new_refcnt );
		result = MALI_TRUE;
	}

	return result;
}

/*
 * Functions private to KBase ('Protected' functions)
 */
void kbase_js_try_run_jobs( kbase_device *kbdev )
{
	kbasep_js_device_data *js_devdata;

	OSK_ASSERT( kbdev != NULL );
	js_devdata = &kbdev->js_data;

	osk_mutex_lock( &js_devdata->runpool_mutex );

	kbasep_js_try_run_next_job( kbdev );

	osk_mutex_unlock( &js_devdata->runpool_mutex );
}

mali_error kbasep_js_devdata_init( kbase_device *kbdev )
{
	kbasep_js_device_data *js_devdata;
	mali_error err;
	int i;
	u16 as_present;
	osk_error osk_err;

	OSK_ASSERT( kbdev != NULL );

	js_devdata = &kbdev->js_data;

	OSK_ASSERT( js_devdata->init_status == JS_DEVDATA_INIT_NONE );

	/* These two must be recalculated if nr_hw_address_spaces changes (e.g. for HW workarounds) */
	as_present = (1U << kbdev->nr_hw_address_spaces) - 1;
	kbdev->nr_user_address_spaces = kbdev->nr_hw_address_spaces;
#if BASE_HW_ISSUE_8987 != 0
	{
		mali_bool use_workaround_for_security;
		use_workaround_for_security = (mali_bool)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_SECURE_BUT_LOSS_OF_PERFORMANCE );
		if ( use_workaround_for_security != MALI_FALSE )
		{
			OSK_PRINT(OSK_BASE_JM, "GPU has HW ISSUE 8987, and driver configured for security workaround: 1 address space only");
			kbdev->nr_user_address_spaces = 1;
		}
	}
#endif /* BASE_HW_ISSUE_8987 != 0 */

	js_devdata->nr_all_contexts_running = 0;
	js_devdata->nr_user_contexts_running = 0;
	js_devdata->as_free = as_present; /* All ASs initially free */
	js_devdata->runpool_irq.submit_allowed = 0u; /* No ctx allowed to submit */
	OSK_MEMSET( js_devdata->runpool_irq.ctx_attr_ref_count, 0, sizeof(js_devdata->runpool_irq.ctx_attr_ref_count) );
	OSK_MEMSET( js_devdata->runpool_irq.slot_affinities, 0, sizeof( js_devdata->runpool_irq.slot_affinities ) );
	js_devdata->runpool_irq.slots_blocked_on_affinity = 0u;
	OSK_MEMSET( js_devdata->runpool_irq.slot_affinity_refcount, 0, sizeof( js_devdata->runpool_irq.slot_affinity_refcount ) );

	/* Config attributes */
	js_devdata->scheduling_tick_ns = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_SCHEDULING_TICK_NS );
	js_devdata->soft_stop_ticks = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_SOFT_STOP_TICKS );
	js_devdata->hard_stop_ticks_ss = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_HARD_STOP_TICKS_SS );
	js_devdata->hard_stop_ticks_nss = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_HARD_STOP_TICKS_NSS );
	js_devdata->gpu_reset_ticks_ss = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_RESET_TICKS_SS );
	js_devdata->gpu_reset_ticks_nss = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_RESET_TICKS_NSS );
	js_devdata->ctx_timeslice_ns = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_CTX_TIMESLICE_NS );
	js_devdata->cfs_ctx_runtime_init_slices = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_CFS_CTX_RUNTIME_INIT_SLICES );
	js_devdata->cfs_ctx_runtime_min_slices = (u32)kbasep_get_config_value( kbdev->config_attributes, KBASE_CONFIG_ATTR_JS_CFS_CTX_RUNTIME_MIN_SLICES );

	OSK_PRINT_INFO( OSK_BASE_JM, "JS Config Attribs: " );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->scheduling_tick_ns:%u", js_devdata->scheduling_tick_ns );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->soft_stop_ticks:%u", js_devdata->soft_stop_ticks );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->hard_stop_ticks_ss:%u", js_devdata->hard_stop_ticks_ss );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->hard_stop_ticks_nss:%u", js_devdata->hard_stop_ticks_nss );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->gpu_reset_ticks_ss:%u", js_devdata->gpu_reset_ticks_ss );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->gpu_reset_ticks_nss:%u", js_devdata->gpu_reset_ticks_nss );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->ctx_timeslice_ns:%u", js_devdata->ctx_timeslice_ns );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->cfs_ctx_runtime_init_slices:%u", js_devdata->cfs_ctx_runtime_init_slices );
	OSK_PRINT_INFO( OSK_BASE_JM, "\tjs_devdata->cfs_ctx_runtime_min_slices:%u", js_devdata->cfs_ctx_runtime_min_slices );

#if MALI_BACKEND_KERNEL /* Only output on real kernel modules, otherwise it fills up multictx testing output */
#if KBASE_DISABLE_SCHEDULING_SOFT_STOPS != 0
	OSK_PRINT( OSK_BASE_JM,
			   "Job Scheduling Policy Soft-stops disabled, ignoring value for soft_stop_ticks==%u at %uns per tick. Other soft-stops may still occur.",
			   js_devdata->soft_stop_ticks,
			   js_devdata->scheduling_tick_ns );
#endif
#if KBASE_DISABLE_SCHEDULING_HARD_STOPS != 0
	OSK_PRINT( OSK_BASE_JM,
			   "Job Scheduling Policy Hard-stops disabled, ignoring values for hard_stop_ticks_ss==%d and hard_stop_ticks_nss==%u at %uns per tick. Other hard-stops may still occur.",
			   js_devdata->hard_stop_ticks_ss,
			   js_devdata->hard_stop_ticks_nss,
			   js_devdata->scheduling_tick_ns );
#endif
#if KBASE_DISABLE_SCHEDULING_SOFT_STOPS != 0 && KBASE_DISABLE_SCHEDULING_HARD_STOPS != 0
	OSK_PRINT( OSK_BASE_JM, "Note: The JS policy's tick timer (if coded) will still be run, but do nothing." );
#endif
#endif /* MALI_BACKEND_KERNEL */

	/* setup the number of irq throttle cycles base on given time */
	{
		u32 irq_throttle_time_us = kbdev->gpu_props.irq_throttle_time_us;
		u32 irq_throttle_cycles = kbasep_js_convert_us_to_gpu_ticks_max_freq(kbdev, irq_throttle_time_us);
		osk_atomic_set( &kbdev->irq_throttle_cycles, irq_throttle_cycles);
	}

	/* Clear the AS data, including setting NULL pointers */
	OSK_MEMSET( &js_devdata->runpool_irq.per_as_data[0], 0, sizeof(js_devdata->runpool_irq.per_as_data) );

	for ( i = 0; i < kbdev->nr_job_slots; ++i )
	{
		js_devdata->js_reqs[i] = core_reqs_from_jsn_features( kbdev->job_slot_features[i] );
	}
	js_devdata->init_status |= JS_DEVDATA_INIT_CONSTANTS;

	/* On error, we could continue on: providing none of the below resources
	 * rely on the ones above */

	osk_err = osk_mutex_init( &js_devdata->runpool_mutex, OSK_LOCK_ORDER_JS_RUNPOOL );
	if ( osk_err == OSK_ERR_NONE )
	{
		js_devdata->init_status |= JS_DEVDATA_INIT_RUNPOOL_MUTEX;
	}

	osk_err = osk_mutex_init( &js_devdata->queue_mutex, OSK_LOCK_ORDER_JS_QUEUE );
	if ( osk_err == OSK_ERR_NONE )
	{
		js_devdata->init_status |= JS_DEVDATA_INIT_QUEUE_MUTEX;
	}

	osk_err = osk_spinlock_irq_init( &js_devdata->runpool_irq.lock, OSK_LOCK_ORDER_JS_RUNPOOL_IRQ );
	if ( osk_err == OSK_ERR_NONE )
	{
		js_devdata->init_status |= JS_DEVDATA_INIT_RUNPOOL_IRQ_LOCK;
	}

	err = kbasep_js_policy_init( kbdev );
	if ( err == MALI_ERROR_NONE)
	{
		js_devdata->init_status |= JS_DEVDATA_INIT_POLICY;
	}

	/* On error, do no cleanup; this will be handled by the caller(s), since
	 * we've designed this resource to be safe to terminate on init-fail */
	if ( js_devdata->init_status != JS_DEVDATA_INIT_ALL)
	{
		return MALI_ERROR_FUNCTION_FAILED;
	}

	return MALI_ERROR_NONE;
}

void kbasep_js_devdata_halt( kbase_device *kbdev )
{
	CSTD_UNUSED(kbdev);
}

void kbasep_js_devdata_term( kbase_device *kbdev )
{
	kbasep_js_device_data *js_devdata;

	OSK_ASSERT( kbdev != NULL );

	js_devdata = &kbdev->js_data;

	if ( (js_devdata->init_status & JS_DEVDATA_INIT_CONSTANTS) )
	{
		s8 zero_ctx_attr_ref_count[KBASEP_JS_CTX_ATTR_COUNT] = { 0, };
		/* The caller must de-register all contexts before calling this */
		OSK_ASSERT( js_devdata->nr_all_contexts_running == 0 );
		OSK_ASSERT( OSK_MEMCMP( js_devdata->runpool_irq.ctx_attr_ref_count, zero_ctx_attr_ref_count, sizeof(js_devdata->runpool_irq.ctx_attr_ref_count)) == 0 );
		CSTD_UNUSED( zero_ctx_attr_ref_count );
	}
	if ( (js_devdata->init_status & JS_DEVDATA_INIT_POLICY) )
	{
		kbasep_js_policy_term( &js_devdata->policy );
	}
	if ( (js_devdata->init_status & JS_DEVDATA_INIT_RUNPOOL_IRQ_LOCK) )
	{
		osk_spinlock_irq_term( &js_devdata->runpool_irq.lock );
	}
	if ( (js_devdata->init_status & JS_DEVDATA_INIT_QUEUE_MUTEX) )
	{
		osk_mutex_term( &js_devdata->queue_mutex );
	}
	if ( (js_devdata->init_status & JS_DEVDATA_INIT_RUNPOOL_MUTEX) )
	{
		osk_mutex_term( &js_devdata->runpool_mutex );
	}

	js_devdata->init_status = JS_DEVDATA_INIT_NONE;
}


mali_error kbasep_js_kctx_init( kbase_context *kctx )
{
	kbase_device *kbdev;
	kbasep_js_kctx_info *js_kctx_info;
	mali_error err;
	osk_error osk_err;

	OSK_ASSERT( kctx != NULL );

	kbdev = kctx->kbdev;
	OSK_ASSERT( kbdev != NULL );

	js_kctx_info = &kctx->jctx.sched_info;
	OSK_ASSERT( js_kctx_info->init_status == JS_KCTX_INIT_NONE );

	js_kctx_info->ctx.nr_jobs = 0;
	js_kctx_info->ctx.is_scheduled = MALI_FALSE;
	js_kctx_info->ctx.is_dying = MALI_FALSE;
	OSK_MEMSET( js_kctx_info->ctx.ctx_attr_ref_count, 0, sizeof(js_kctx_info->ctx.ctx_attr_ref_count) );

	/* Initially, the context is disabled from submission until the create flags are set */
	js_kctx_info->ctx.flags = KBASE_CTX_FLAG_SUBMIT_DISABLED;

	js_kctx_info->init_status |= JS_KCTX_INIT_CONSTANTS;

	/* On error, we could continue on: providing none of the below resources
	 * rely on the ones above */
	osk_err = osk_mutex_init( &js_kctx_info->ctx.jsctx_mutex, OSK_LOCK_ORDER_JS_CTX );
	if ( osk_err == OSK_ERR_NONE )
	{
		js_kctx_info->init_status |= JS_KCTX_INIT_JSCTX_MUTEX;
	}

	osk_err = osk_waitq_init( &js_kctx_info->ctx.scheduled_waitq );
	if ( osk_err == OSK_ERR_NONE )
	{
		js_kctx_info->init_status |= JS_KCTX_INIT_JSCTX_WAITQ_SCHED;
	}

	osk_err = osk_waitq_init( &js_kctx_info->ctx.not_scheduled_waitq );
	if ( osk_err == OSK_ERR_NONE )
	{
		js_kctx_info->init_status |= JS_KCTX_INIT_JSCTX_WAITQ_NSCHED;
	}

	err = kbasep_js_policy_init_ctx( kbdev, kctx );
	if ( err == MALI_ERROR_NONE )
	{
		js_kctx_info->init_status |= JS_KCTX_INIT_POLICY;
	}

	/* On error, do no cleanup; this will be handled by the caller(s), since
	 * we've designed this resource to be safe to terminate on init-fail */
	if ( js_kctx_info->init_status != JS_KCTX_INIT_ALL)
	{
		return MALI_ERROR_FUNCTION_FAILED;
	}

	/* Initially, the context is not scheduled */
	osk_waitq_clear( &js_kctx_info->ctx.scheduled_waitq );
	osk_waitq_set( &js_kctx_info->ctx.not_scheduled_waitq );

	return MALI_ERROR_NONE;
}

void kbasep_js_kctx_term( kbase_context *kctx )
{
	kbase_device *kbdev;
	kbasep_js_kctx_info *js_kctx_info;
	kbasep_js_policy    *js_policy;

	OSK_ASSERT( kctx != NULL );

	kbdev = kctx->kbdev;
	OSK_ASSERT( kbdev != NULL );

	js_policy = &kbdev->js_data.policy;
	js_kctx_info = &kctx->jctx.sched_info;

	if ( (js_kctx_info->init_status & JS_KCTX_INIT_CONSTANTS) )
	{
		/* The caller must de-register all jobs before calling this */
		OSK_ASSERT( js_kctx_info->ctx.is_scheduled == MALI_FALSE );
		OSK_ASSERT( js_kctx_info->ctx.nr_jobs == 0 );
		/* Only certain Ctx Attributes will be zero (others can have a non-zero value for the life of the context) */
		OSK_ASSERT( kbasep_js_ctx_attr_count_on_runpool( kbdev, KBASEP_JS_CTX_ATTR_NSS ) == 0 );
	}

	if ( (js_kctx_info->init_status & JS_KCTX_INIT_JSCTX_WAITQ_SCHED) )
	{
		osk_waitq_term( &js_kctx_info->ctx.scheduled_waitq );
	}

	if ( (js_kctx_info->init_status & JS_KCTX_INIT_JSCTX_WAITQ_NSCHED) )
	{
		osk_waitq_term( &js_kctx_info->ctx.not_scheduled_waitq );
	}

	if ( (js_kctx_info->init_status & JS_KCTX_INIT_JSCTX_MUTEX) )
	{
		osk_mutex_term( &js_kctx_info->ctx.jsctx_mutex );
	}

	if ( (js_kctx_info->init_status & JS_KCTX_INIT_POLICY) )
	{
		kbasep_js_policy_term_ctx( js_policy, kctx );
	}

	js_kctx_info->init_status = JS_KCTX_INIT_NONE;
}

/* Evict jobs from the NEXT registers
 *
 * The caller must hold:
 * - kbasep_js_kctx_info::ctx::jsctx_mutex
 * - kbasep_js_device_data::runpool_mutex 
 */
STATIC void kbasep_js_runpool_evict_next_jobs( kbase_device *kbdev )
{
	int js;
	kbasep_js_device_data *js_devdata;
	u16 saved_submit_mask;

	js_devdata = &kbdev->js_data;

	/* Prevent contexts in the runpool from submitting jobs */
	osk_spinlock_irq_lock(&js_devdata->runpool_irq.lock);
	{
		saved_submit_mask = js_devdata->runpool_irq.submit_allowed;
		js_devdata->runpool_irq.submit_allowed = 0;
	}
	osk_spinlock_irq_unlock(&js_devdata->runpool_irq.lock);

	/* Evict jobs from the NEXT registers */
	for (js = 0; js < kbdev->nr_job_slots; js++)
	{
		kbase_jm_slot *slot;
		kbase_jd_atom *tail;

		kbase_job_slot_lock(kbdev, js);

		if (!kbase_reg_read(kbdev, JOB_SLOT_REG(js, JSn_COMMAND_NEXT), NULL))
		{
			/* No job in the NEXT register */
			kbase_job_slot_unlock(kbdev, js);
			continue;
		}

		slot = &kbdev->jm_slots[js];
		tail = kbasep_jm_peek_idx_submit_slot(slot, slot->submitted_nr-1);

		/* Clearing job from next registers */
		kbase_reg_write(kbdev, JOB_SLOT_REG(js, JSn_COMMAND_NEXT), JSn_COMMAND_NOP, NULL);

		/* Check to see if we did remove a job from the next registers */
		if (kbase_reg_read(kbdev, JOB_SLOT_REG(js, JSn_HEAD_NEXT_LO), NULL) != 0 ||
		    kbase_reg_read(kbdev, JOB_SLOT_REG(js, JSn_HEAD_NEXT_HI), NULL) != 0)
		{
			/* The job was successfully cleared from the next registers, requeue it */
			slot->submitted_nr--;

			/* Set the next registers to NULL */
			kbase_reg_write(kbdev, JOB_SLOT_REG(js, JSn_HEAD_NEXT_LO), 0, NULL);
			kbase_reg_write(kbdev, JOB_SLOT_REG(js, JSn_HEAD_NEXT_HI), 0, NULL);

			tail->event.event_code = BASE_JD_EVENT_REMOVED_FROM_NEXT;

			/* Complete the job, indicate that it took no time, and start_new_jobs==MALI_FALSE */
			kbase_jd_done(tail, js, NULL, MALI_FALSE);
		}

		kbase_job_slot_unlock(kbdev, js);
	}

	/* Allow contexts in the runpool to submit jobs again */
	osk_spinlock_irq_lock(&js_devdata->runpool_irq.lock);
	{
		js_devdata->runpool_irq.submit_allowed = saved_submit_mask;
	}
	osk_spinlock_irq_unlock(&js_devdata->runpool_irq.lock);
}

/**
 * Fast start a higher priority job
 * If the runpool is full, the lower priority contexts with no running jobs 
 * will be evicted from the runpool
 *
 * If \a kctx_new is NULL, the first context with no running jobs will be evicted
 *
 * The following locking conditions are made on the caller:
 * - The caller must \b not hold \a kctx_new's
 * kbasep_js_kctx_info::ctx::jsctx_mutex, or that mutex of any ctx in the
 * runpool. This is because \a kctx_new's jsctx_mutex and one of the other
 * scheduled ctx's jsctx_mutex will be obtained internally.
 * - it must \em not hold kbasep_js_device_data::runpool_irq::lock (as this will be
 * obtained internally)
 * - it must \em not hold kbasep_js_device_data::runpool_mutex (as this will be
 * obtained internally)
 * - it must \em not hold kbasep_jd_device_data::queue_mutex (again, it's used
 * internally).
 */
STATIC void kbasep_js_runpool_attempt_fast_start_ctx( kbase_device *kbdev, kbase_context *kctx_new )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_kctx_info   *js_kctx_new;
	kbasep_js_policy      *js_policy;
	kbasep_js_per_as_data *js_per_as_data;
	int evict_as_nr;

	OSK_ASSERT(kbdev != NULL);

	js_devdata = &kbdev->js_data;
	js_policy = &kbdev->js_data.policy;
	
	if (kctx_new != NULL)
	{
			js_kctx_new = &kctx_new->jctx.sched_info;
			osk_mutex_lock( &js_kctx_new->ctx.jsctx_mutex );
	}
	else
	{
		js_kctx_new = NULL;
		CSTD_UNUSED(js_kctx_new);
	}

	osk_mutex_lock( &js_devdata->runpool_mutex );

	/* If the runpool is full, attempt to fast start our context */
	if (check_is_runpool_full(kbdev, kctx_new) != MALI_FALSE)
	{
		/* No free address spaces - attempt to evict non-running lower priority context */
		osk_spinlock_irq_lock(&js_devdata->runpool_irq.lock);
		for(evict_as_nr = 0; evict_as_nr < kbdev->nr_hw_address_spaces; evict_as_nr++)
		{
			kbase_context *kctx_evict;
			js_per_as_data = &js_devdata->runpool_irq.per_as_data[evict_as_nr];
			kctx_evict = js_per_as_data->kctx;

			/* Look for the AS which is not currently running */
			if(0 == js_per_as_data->as_busy_refcount && kctx_evict != NULL)
			{
				/* Now compare the scheduled priority we are considering evicting with the new ctx priority
				 * and take into consideration if the scheduled priority is a realtime policy or not.
				 * Note that the lower the number, the higher the priority
				 */
				if((kctx_new == NULL) || kbasep_js_policy_ctx_has_priority(js_policy, kctx_evict, kctx_new))
				{
					mali_bool retain_result;
					kbasep_js_release_result release_result;
					KBASE_TRACE_ADD( kbdev, JS_FAST_START_EVICTS_CTX, kctx_evict, NULL, 0u, (u32)kctx_new );

					/* Retain the ctx to work on it - this shouldn't be able to fail */
					retain_result = kbasep_js_runpool_retain_ctx_nolock( kbdev, kctx_evict );
					OSK_ASSERT( retain_result != MALI_FALSE );
					CSTD_UNUSED( retain_result );

					/* This will cause the context to be scheduled out on the next runpool_release_ctx(),
					 * and also stop its refcount increasing */
					kbasep_js_clear_submit_allowed(js_devdata, kctx_evict);

					osk_spinlock_irq_unlock(&js_devdata->runpool_irq.lock);
					osk_mutex_unlock(&js_devdata->runpool_mutex);
					if (kctx_new != NULL)
					{
							osk_mutex_unlock( &js_kctx_new->ctx.jsctx_mutex );
					}

					/* Stop working on the target context, start working on the kctx_evict context */

					osk_mutex_lock( &kctx_evict->jctx.sched_info.ctx.jsctx_mutex );
					osk_mutex_lock( &js_devdata->runpool_mutex );
					release_result = kbasep_js_runpool_release_ctx_internal( kbdev, kctx_evict );
					osk_mutex_unlock( &js_devdata->runpool_mutex );
					/* Only requeue if actually descheduled, which is more robust in case
					 * something else retains it (e.g. two high priority contexts racing
					 * to evict the same lower priority context) */
					if ( (release_result & KBASEP_JS_RELEASE_RESULT_WAS_DESCHEDULED) != 0u )
					{
						kbasep_js_runpool_requeue_or_kill_ctx( kbdev, kctx_evict );
					}
					osk_mutex_unlock( &kctx_evict->jctx.sched_info.ctx.jsctx_mutex );

					/* release_result isn't propogated further:
					 * - the caller will be scheduling in a context anyway
					 * - which will also cause new jobs to run */

					/* ctx fast start has taken place */
					return;
				}
			}
		}
		osk_spinlock_irq_unlock(&js_devdata->runpool_irq.lock);
	}

	/* ctx fast start has not taken place */
	osk_mutex_unlock( &js_devdata->runpool_mutex );
	if (kctx_new != NULL)
	{
			osk_mutex_unlock( &js_kctx_new->ctx.jsctx_mutex );
	}
}

mali_bool kbasep_js_add_job( kbase_context *kctx, kbase_jd_atom *atom )
{
	kbasep_js_kctx_info *js_kctx_info;
	kbase_device *kbdev;
	kbasep_js_device_data *js_devdata;
	kbasep_js_policy    *js_policy;

	mali_bool policy_queue_updated = MALI_FALSE;

	OSK_ASSERT( kctx != NULL );
	OSK_ASSERT( atom != NULL );

	kbdev = kctx->kbdev;
	js_devdata = &kbdev->js_data;
	js_policy = &kbdev->js_data.policy;
	js_kctx_info = &kctx->jctx.sched_info;

	osk_mutex_lock( &js_kctx_info->ctx.jsctx_mutex );
	osk_mutex_lock( &js_devdata->runpool_mutex );
	{
		KBASE_TRACE_ADD_REFCOUNT( kbdev, JS_ADD_JOB, kctx, atom->user_atom, atom->jc,
								  kbasep_js_trace_get_refcnt(kbdev, kctx));
	}

	/* Refcount ctx.nr_jobs */
	OSK_ASSERT( js_kctx_info->ctx.nr_jobs < U32_MAX );
	++(js_kctx_info->ctx.nr_jobs);

	/* Setup any scheduling information */
	kbasep_js_clear_job_retry_submit( atom );

	/*
	 * Begin Runpool_irq transaction
	 */
	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );
	{
		/* Context Attribute Refcounting */
		kbasep_js_ctx_attr_ctx_retain_atom( kbdev, kctx, atom );

		/* Enqueue the job in the policy, causing it to be scheduled if the
		 * parent context gets scheduled */
		kbasep_js_policy_enqueue_job( js_policy, atom );
	}
	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );
	/* End runpool_irq transaction */

	if ( js_kctx_info->ctx.is_scheduled != MALI_FALSE )
	{
		/* Handle an already running context - try to run the new job, in case it
		 * matches requirements that aren't matched by any other job in the Run
		 * Pool */
		kbasep_js_try_run_next_job( kbdev );
	}
	osk_mutex_unlock( &js_devdata->runpool_mutex );

	if ( js_kctx_info->ctx.is_scheduled == MALI_FALSE && js_kctx_info->ctx.nr_jobs == 1 )
	{
		/* Handle Refcount going from 0 to 1: schedule the context on the Policy Queue */
		OSK_ASSERT( js_kctx_info->ctx.is_scheduled == MALI_FALSE );

		OSK_PRINT_INFO(OSK_BASE_JM, "JS: Enqueue Context %p", kctx );

		osk_mutex_lock( &js_devdata->queue_mutex );
		kbasep_js_policy_enqueue_ctx( js_policy, kctx );
		osk_mutex_unlock( &js_devdata->queue_mutex );
		/* If the runpool is full and this job has a higher priority than the non-running
		 * job in the runpool - evict it so this higher priority job starts faster */
		osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );

		/* Fast-starting requires the jsctx_mutex to be dropped, because it works on multiple ctxs */
		kbasep_js_runpool_attempt_fast_start_ctx( kbdev, kctx );

		/* This context is becoming active */
		kbase_pm_context_active(kctx->kbdev);

		/* NOTE: Potentially, we can make the scheduling of the head context
		 * happen in a work-queue if we need to wait for the PM to power
		 * up. Also need logic to submit nothing until PM really has completed
		 * powering up. */

		/* Policy Queue was updated - caller must try to schedule the head context */
		policy_queue_updated = MALI_TRUE;
	}
	else
	{
		osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );
	}

	return policy_queue_updated;
}

void kbasep_js_remove_job( kbase_context *kctx, kbase_jd_atom *atom )
{
	kbasep_js_kctx_info *js_kctx_info;
	kbase_device *kbdev;
	kbasep_js_device_data *js_devdata;
	kbasep_js_policy    *js_policy;
	mali_bool attr_state_changed;

	OSK_ASSERT( kctx != NULL );
	OSK_ASSERT( atom != NULL );

	kbdev = kctx->kbdev;
	js_devdata = &kbdev->js_data;
	js_policy = &kbdev->js_data.policy;
	js_kctx_info = &kctx->jctx.sched_info;

	{
		KBASE_TRACE_ADD_REFCOUNT( kbdev, JS_REMOVE_JOB, kctx, atom->user_atom, atom->jc,
								  kbasep_js_trace_get_refcnt(kbdev, kctx));
	}

	/* De-refcount ctx.nr_jobs */
	OSK_ASSERT( js_kctx_info->ctx.nr_jobs > 0 );
	--(js_kctx_info->ctx.nr_jobs);

	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );
	attr_state_changed = kbasep_js_ctx_attr_ctx_release_atom( kbdev, kctx, atom );
	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );

	/* De-register the job from the system */
	kbasep_js_policy_term_job( js_policy, atom );

	if ( attr_state_changed != MALI_FALSE )
	{
		/* A change in runpool ctx attributes might mean we can run more jobs
		 * than before. */
		osk_mutex_lock( &js_devdata->runpool_mutex );
		kbasep_js_try_run_next_job( kbdev );
		osk_mutex_unlock( &js_devdata->runpool_mutex );
	}

}


mali_bool kbasep_js_runpool_retain_ctx( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	mali_bool result;
	OSK_ASSERT( kbdev != NULL );
	js_devdata = &kbdev->js_data;

	/* KBASE_TRACE_ADD_REFCOUNT( kbdev, JS_RETAIN_CTX, kctx, NULL, 0,
	                             kbasep_js_trace_get_refcnt(kbdev, kctx)); */
	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );
	result = kbasep_js_runpool_retain_ctx_nolock( kbdev, kctx );
	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );

	return result;
}


kbase_context* kbasep_js_runpool_lookup_ctx( kbase_device *kbdev, int as_nr )
{
	kbasep_js_device_data *js_devdata;
	kbase_context *found_kctx = NULL;
	kbasep_js_per_as_data *js_per_as_data;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( 0 <= as_nr && as_nr < BASE_MAX_NR_AS );
	js_devdata = &kbdev->js_data;
	js_per_as_data = &js_devdata->runpool_irq.per_as_data[as_nr];

	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );

	found_kctx = js_per_as_data->kctx;

	if ( found_kctx != NULL )
	{
		++(js_per_as_data->as_busy_refcount);
	}

	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );

	return found_kctx;
}

/**
 * Internal function to release the reference on a ctx, only taking the runpool and as transaction mutexes
 *
 * This does none of the followup actions for scheduling:
 * - It does not schedule in a new context
 * - It does not start more jobs running in the case of an ctx-attribute state change
 * - It does not requeue or handling dying contexts
 *
 * For those tasks, just call kbasep_js_runpool_release_ctx() instead
 *
 * Requires:
 * - Context is scheduled in, and kctx->as_nr matches kctx_as_nr
 * - Context has a non-zero refcount
 * - Caller holds js_kctx_info->ctx.jsctx_mutex
 * - Caller holds js_devdata->runpool_mutex
 */
STATIC kbasep_js_release_result kbasep_js_runpool_release_ctx_internal( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_kctx_info   *js_kctx_info;
	kbasep_js_policy      *js_policy;
	kbasep_js_per_as_data *js_per_as_data;

	kbasep_js_release_result release_result = 0u;
	int kctx_as_nr;
	kbase_as *current_as;
	int new_ref_count;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );
	js_kctx_info = &kctx->jctx.sched_info;
	js_devdata = &kbdev->js_data;
	js_policy = &kbdev->js_data.policy;

	/* Ensure context really is scheduled in */
	OSK_ASSERT( js_kctx_info->ctx.is_scheduled != MALI_FALSE );

	/* kctx->as_nr and js_per_as_data are only read from here. The caller's
	 * js_ctx_mutex provides a barrier that ensures they are up-to-date.
	 *
	 * They will not change whilst we're reading them, because the refcount
	 * is non-zero (and we ASSERT on that last fact).
	 */
	kctx_as_nr = kctx->as_nr;
	OSK_ASSERT( kctx_as_nr != KBASEP_AS_NR_INVALID );
	js_per_as_data = &js_devdata->runpool_irq.per_as_data[kctx_as_nr];
	OSK_ASSERT( js_per_as_data->as_busy_refcount > 0 );

	/* 
	 * Transaction begins on AS and runpool_irq
	 *
	 * Assert about out calling contract
	 */
	current_as = &kbdev->as[kctx_as_nr];
	osk_mutex_lock( &current_as->transaction_mutex );
	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );
	OSK_ASSERT( kctx_as_nr == kctx->as_nr );
	OSK_ASSERT( js_per_as_data->as_busy_refcount > 0 );

	/* Update refcount */
	new_ref_count = --(js_per_as_data->as_busy_refcount);

	KBASE_TRACE_ADD_REFCOUNT( kbdev, JS_RELEASE_CTX, kctx, NULL, 0u,
							  new_ref_count);

	/* Make a set of checks to see if the context should be scheduled out */
	if ( new_ref_count == 0
		 && ( kctx->jctx.sched_info.ctx.nr_jobs == 0
			  || kbasep_js_is_submit_allowed( js_devdata, kctx ) == MALI_FALSE ) )
	{
		/* Last reference, and we've been told to remove this context from the Run Pool */
		OSK_PRINT_INFO(OSK_BASE_JM, "JS: RunPool Remove Context %p because as_busy_refcount=%d, jobs=%d, allowed=%d",
					   kctx,
					   new_ref_count,
					   js_kctx_info->ctx.nr_jobs,
					   kbasep_js_is_submit_allowed( js_devdata, kctx ) );

		kbasep_js_policy_runpool_remove_ctx( js_policy, kctx );

		/* Stop any more refcounts occuring on the context */
		js_per_as_data->kctx = NULL;

		/* Ensure we prevent the context from submitting any new jobs
		 * e.g. from kbasep_js_try_run_next_job_on_slot_irq_nolock()  */
		kbasep_js_clear_submit_allowed( js_devdata, kctx );

		/* Disable the MMU on the affected address space, and indicate it's invalid */
		kbase_mmu_disable( kctx );
		kctx->as_nr = KBASEP_AS_NR_INVALID;

		/* Ctx Attribute handling */
		if ( kbasep_js_ctx_attr_runpool_release_ctx( kbdev, kctx ) != MALI_FALSE )
		{
			release_result |= KBASEP_JS_RELEASE_RESULT_CTX_ATTR_CHANGE;
		}

		/*
		 * Transaction ends on AS and runpool_irq:
		 *
		 * By this point, the AS-related data is now clear and ready for re-use.
		 *
		 * Since releases only occur once for each previous successful retain, and no more
		 * retains are allowed on this context, no other thread will be operating in this
		 * code whilst we are
		 */
		osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );
		osk_mutex_unlock( &current_as->transaction_mutex );

		/* Free up the address space */
		js_devdata->as_free |= ((u16)(1u << kctx_as_nr));
		/* Note: Don't reuse kctx_as_nr now */

		/* update book-keeping info */
		runpool_dec_context_count( kbdev, kctx );
		js_kctx_info->ctx.is_scheduled = MALI_FALSE;
		/* Signal any waiter that the context is not scheduled, so is safe for
		 * termination - once the jsctx_mutex is also dropped, and jobs have
		 * finished. */
		osk_waitq_set( &js_kctx_info->ctx.not_scheduled_waitq );
		osk_waitq_clear( &js_kctx_info->ctx.scheduled_waitq );

		/* Queue an action to occur after we've dropped the lock */
		release_result |= KBASEP_JS_RELEASE_RESULT_WAS_DESCHEDULED;

	}
	else
	{
		osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );
		osk_mutex_unlock( &current_as->transaction_mutex );
	}

	return release_result;
}

void kbasep_js_runpool_requeue_or_kill_ctx( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_policy      *js_policy;
	kbasep_js_kctx_info   *js_kctx_info;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );
	js_kctx_info = &kctx->jctx.sched_info;
	js_policy = &kbdev->js_data.policy;
	js_devdata = &kbdev->js_data;

	/* This is called if and only if you've you've detached the context from
	 * the Runpool or the Policy Queue, and not added it back to the Runpool */
	OSK_ASSERT( js_kctx_info->ctx.is_scheduled == MALI_FALSE );

	if ( js_kctx_info->ctx.is_dying != MALI_FALSE )
	{
		/* Dying: kill and idle the context */

		/* Notify PM that a context has gone idle */
		OSK_PRINT_INFO(OSK_BASE_JM, "JS: Idling Context %p (not requeued)", kctx );
		kbase_pm_context_idle(kbdev);

		/* The killing happens asynchronously */
		OSK_PRINT_INFO(OSK_BASE_JM, "JS: ** Killing Context %p on RunPool Remove **", kctx );
		kbasep_js_policy_kill_all_ctx_jobs( js_policy, kctx );
	}
	else if ( js_kctx_info->ctx.nr_jobs > 0 )
	{
		/* Not dying, has jobs: add back to the queue */
		OSK_PRINT_INFO(OSK_BASE_JM, "JS: Requeue Context %p", kctx );
		osk_mutex_lock( &js_devdata->queue_mutex );
		kbasep_js_policy_enqueue_ctx( js_policy, kctx );
		osk_mutex_unlock( &js_devdata->queue_mutex );
	}
	else
	{
		/* Not dying, no jobs: PM-idle the context, don't add back to the queue */
		OSK_PRINT_INFO(OSK_BASE_JM, "JS: Idling Context %p (not requeued)", kctx );
		kbase_pm_context_idle(kbdev);
	}
}

void kbasep_js_runpool_release_ctx( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_device_data *js_devdata;
	kbasep_js_kctx_info   *js_kctx_info;

	kbasep_js_release_result release_result;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );
	js_kctx_info = &kctx->jctx.sched_info;
	js_devdata = &kbdev->js_data;

	osk_mutex_lock( &js_kctx_info->ctx.jsctx_mutex );

	osk_mutex_lock( &js_devdata->runpool_mutex );
	release_result = kbasep_js_runpool_release_ctx_internal( kbdev, kctx );
	osk_mutex_unlock( &js_devdata->runpool_mutex );

	/* Do we have an action queued whilst the lock was held? */
	if ( (release_result & KBASEP_JS_RELEASE_RESULT_WAS_DESCHEDULED) != 0u )
	{
		kbasep_js_runpool_requeue_or_kill_ctx( kbdev, kctx) ;
	}
	/* We've finished with this context for now, so drop the lock for it. */
	osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );

	if ( (release_result & KBASEP_JS_RELEASE_RESULT_WAS_DESCHEDULED) != 0u )
	{
		/* We've freed up an address space, so let's try to schedule in another
		 * context
		 *
		 * Note: if there's a context to schedule in, then it also tries to run
		 * another job, in case the new context has jobs satisfying requirements
		 * that no other context/job in the runpool does */
		kbasep_js_try_schedule_head_ctx( kbdev );
	}

	if ( (release_result & KBASEP_JS_RELEASE_RESULT_CTX_ATTR_CHANGE) != 0u )
	{
		/* A change in runpool ctx attributes might mean we can run more jobs
		 * than before - and this needs to be done when the above
		 * try_schedule_head_ctx() had no contexts to run */
		osk_mutex_lock( &js_devdata->runpool_mutex );
		kbasep_js_try_run_next_job( kbdev );
		osk_mutex_unlock( &js_devdata->runpool_mutex );
	}

}

/**
 * @brief Handle retaining cores for power management and affinity management,
 * ensuring that cores are powered up and won't violate affinity restrictions.
 *
 * This function enters at the following @ref kbase_atom_coreref_state states:
 *
 * - NO_CORES_REQUESTED,
 * - WAITING_FOR_REQUESTED_CORES,
 * - RECHECK_AFFINITY,
 *
 * The transitions are as folows:
 * - NO_CORES_REQUESTED -> WAITING_FOR_REQUESTED_CORES
 * - WAITING_FOR_REQUESTED_CORES -> ( WAITING_FOR_REQUESTED_CORES or RECHECK_AFFINITY )
 * - RECHECK_AFFINITY -> ( WAITING_FOR_REQUESTED_CORES or CHECK_AFFINITY_VIOLATIONS )
 * - CHECK_AFFINITY_VIOLATIONS -> ( RECHECK_AFFINITY or READY )
 *
 * The caller must hold:
 * - kbasep_js_device_data::runpool_irq::lock
 *
 * @return MALI_FALSE when the function makes a transition to the same or lower state, indicating
 * that the cores are not ready.
 * @return MALI_TRUE once READY state is reached, indicating that the cores are 'ready' and won't
 * violate affinity restrictions.
 *
 */
STATIC mali_bool kbasep_js_job_check_ref_cores(kbase_device *kbdev, int js, kbase_jd_atom *katom)
{
	u64 tiler_affinity = 0;
	/* The most recently checked affinity. Having this at this scope allows us
	 * to guarantee that we've checked the affinity in this function call. */
	u64 recently_chosen_affinity = 0;

	if (katom->core_req & BASE_JD_REQ_T)
	{
		tiler_affinity = kbdev->tiler_present_bitmap;
	}

	/* NOTE: The following uses a number of FALLTHROUGHs to optimize the
	 * calls to this function. Ending of the function is indicated by BREAK OUT */
	switch ( katom->coreref_state )
	{
		/* State when job is first attempted to be run */
		case KBASE_ATOM_COREREF_STATE_NO_CORES_REQUESTED:
			OSK_ASSERT( katom->affinity == 0 );
			/* Compute affinity */
			kbase_js_choose_affinity( &recently_chosen_affinity, kbdev, katom, js );

			/* Request the cores */
			if (MALI_ERROR_NONE != kbase_pm_request_cores( kbdev, recently_chosen_affinity, tiler_affinity ))
			{
				/* Failed to request cores, don't set the affinity so we try again and return */
				KBASE_TRACE_ADD_SLOT_INFO( kbdev, JS_CORE_REF_REQUEST_CORES_FAILED, katom->kctx, katom->user_atom, katom->jc, js, (u32)recently_chosen_affinity );
				/* *** BREAK OUT: No state transition *** */
				break;
			}

			katom->affinity = recently_chosen_affinity;
			/* Proceed to next state */
			katom->coreref_state = KBASE_ATOM_COREREF_STATE_WAITING_FOR_REQUESTED_CORES;

			/* ***FALLTHROUGH: TRANSITION TO HIGHER STATE*** */

		case KBASE_ATOM_COREREF_STATE_WAITING_FOR_REQUESTED_CORES:
			{
				mali_bool cores_ready;
				OSK_ASSERT( katom->affinity != 0 );

				cores_ready = kbase_pm_register_inuse_cores( kbdev, katom->affinity, tiler_affinity );
				if ( !cores_ready )
				{
					/* Stay in this state and return, to retry at this state later */
					KBASE_TRACE_ADD_SLOT_INFO( kbdev, JS_CORE_REF_REGISTER_INUSE_FAILED, katom->kctx, katom->user_atom, katom->jc, js, (u32)katom->affinity );
					/* *** BREAK OUT: No state transition *** */
					break;
				}
				/* Proceed to next state */
				katom->coreref_state = KBASE_ATOM_COREREF_STATE_RECHECK_AFFINITY;
			}

			/* ***FALLTHROUGH: TRANSITION TO HIGHER STATE*** */

		case KBASE_ATOM_COREREF_STATE_RECHECK_AFFINITY:
			OSK_ASSERT( katom->affinity != 0 );

			/* Optimize out choosing the affinity twice in the same function call */
			if ( recently_chosen_affinity == 0 )
			{
				/* See if the affinity changed since a previous call. */
				kbase_js_choose_affinity( &recently_chosen_affinity, kbdev, katom, js );
			}

			/* Now see if this requires a different set of cores */
			if ( recently_chosen_affinity != katom->affinity )
			{
				if (MALI_ERROR_NONE != kbase_pm_request_cores( kbdev, recently_chosen_affinity, tiler_affinity ))
				{
					/* Failed to request cores, rollback the previous gained set
					 * That also resets the state to NO_CORES_REQUESTED */
					kbasep_js_job_check_deref_cores( kbdev, katom );
					KBASE_TRACE_ADD_SLOT_INFO( kbdev, JS_CORE_REF_REQUEST_ON_RECHECK_FAILED, katom->kctx, katom->user_atom, katom->jc, js, (u32)recently_chosen_affinity );
					/* *** BREAK OUT: Transition to lower state *** */
					break;
				}
				else
				{
					mali_bool cores_ready;
					/* Register new cores whislt we still hold the old ones, to minimize power transitions */
					cores_ready = kbase_pm_register_inuse_cores( kbdev, recently_chosen_affinity, tiler_affinity );
					kbasep_js_job_check_deref_cores( kbdev, katom );

					/* Fixup the state that was reduced by deref_cores: */
					katom->coreref_state = KBASE_ATOM_COREREF_STATE_RECHECK_AFFINITY;
					katom->affinity = recently_chosen_affinity;
					/* Now might be waiting for powerup again, with a new affinity */
					if ( !cores_ready )
					{
						/* Return to previous state */
						katom->coreref_state = KBASE_ATOM_COREREF_STATE_WAITING_FOR_REQUESTED_CORES;
						KBASE_TRACE_ADD_SLOT_INFO( kbdev, JS_CORE_REF_REGISTER_ON_RECHECK_FAILED, katom->kctx, katom->user_atom, katom->jc, js, (u32)katom->affinity );
						/* *** BREAK OUT: Transition to lower state *** */
						break;
					}
				}
			}
			/* Proceed to next state */
			katom->coreref_state = KBASE_ATOM_COREREF_STATE_CHECK_AFFINITY_VIOLATIONS;

			/* ***FALLTHROUGH: TRANSITION TO HIGHER STATE*** */
		case KBASE_ATOM_COREREF_STATE_CHECK_AFFINITY_VIOLATIONS:
			OSK_ASSERT( katom->affinity != 0 );
			OSK_ASSERT( katom->affinity == recently_chosen_affinity );

			/* Note: this is where the caller must've taken the runpool_irq.lock */ 

			/* Check for affinity violations - if there are any, then we just ask
			 * the caller to requeue and try again later */
			if ( kbase_js_affinity_would_violate( kbdev, js, katom->affinity ) != MALI_FALSE )
			{
				/* Cause a re-attempt to submit from this slot on the next job complete */
				kbase_js_affinity_slot_blocked_an_atom( kbdev, js );
				/* Return to previous state */
				katom->coreref_state = KBASE_ATOM_COREREF_STATE_RECHECK_AFFINITY;
				/* *** BREAK OUT: Transition to lower state *** */
				KBASE_TRACE_ADD_SLOT_INFO( kbdev, JS_CORE_REF_AFFINITY_WOULD_VIOLATE, katom->kctx, katom->user_atom, katom->jc, js, (u32)katom->affinity );
				break;
			}

			/* No affinity violations would result, so the cores are ready */
			katom->coreref_state = KBASE_ATOM_COREREF_STATE_READY;
			/* *** BREAK OUT: Cores Ready *** */
			break;

		default:
			OSK_ASSERT_MSG( MALI_FALSE, "Unhandled kbase_atom_coreref_state %d", katom->coreref_state );
			break;
	}

	return (katom->coreref_state == KBASE_ATOM_COREREF_STATE_READY);
}

void kbasep_js_job_check_deref_cores(kbase_device *kbdev, struct kbase_jd_atom *katom)
{
	u64 tiler_affinity = 0;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( katom != NULL );

	if (katom->core_req & BASE_JD_REQ_T)
	{
		tiler_affinity = kbdev->tiler_present_bitmap;
	}

	switch ( katom->coreref_state )
	{
		case KBASE_ATOM_COREREF_STATE_READY:
			/* State where atom was submitted to the HW - just proceed to power-down */
			OSK_ASSERT( katom->affinity != 0 );

			/* *** FALLTHROUGH *** */

		case KBASE_ATOM_COREREF_STATE_RECHECK_AFFINITY:
			/* State where cores were registered */
			OSK_ASSERT( katom->affinity != 0 );
			kbase_pm_release_cores(kbdev, katom->affinity, tiler_affinity);

			/* Note: We do not clear the state for kbase_js_affinity_slot_blocked_an_atom().
			 * That is handled after finishing the job. This might be slightly
			 * suboptimal for some corner cases, but is otherwise not a problem
			 * (and resolves itself after the next job completes). */

			break;

		case KBASE_ATOM_COREREF_STATE_WAITING_FOR_REQUESTED_CORES:
			/* State where cores were requested, but not registered */
			OSK_ASSERT( katom->affinity != 0 );
			kbase_pm_unrequest_cores(kbdev, katom->affinity, tiler_affinity);
			break;

		case KBASE_ATOM_COREREF_STATE_NO_CORES_REQUESTED:
			/* Initial state - nothing required */
			OSK_ASSERT( katom->affinity == 0 );
			break;

		default:
			OSK_ASSERT_MSG( MALI_FALSE, "Unhandled coreref_state: %d", katom->coreref_state );
			break;
	}

	katom->affinity = 0;
	katom->coreref_state = KBASE_ATOM_COREREF_STATE_NO_CORES_REQUESTED;
}



/*
 * Note: this function is quite similar to kbasep_js_try_run_next_job_on_slot()
 */
mali_bool kbasep_js_try_run_next_job_on_slot_irq_nolock( kbase_device *kbdev, int js, s8 *submit_count )
{
	kbasep_js_device_data *js_devdata;
	mali_bool tried_to_dequeue_jobs_but_failed = MALI_FALSE;
	mali_bool cores_ready;

	OSK_ASSERT( kbdev != NULL );

	js_devdata = &kbdev->js_data;

	/* The caller of this function may not be aware of Ctx Attribute state changes so we
	 * must recheck if the given slot is still valid. Otherwise do not try to run.
	 */
	if (kbase_js_can_run_job_on_slot_no_lock( js_devdata, js))
	{
		/* Keep submitting while there's space to run a job on this job-slot,
		 * and there are jobs to get that match its requirements (see 'break'
		 * statement below) */
		while ( *submit_count < KBASE_JS_MAX_JOB_SUBMIT_PER_SLOT_PER_IRQ
				&& kbasep_jm_is_submit_slots_free( kbdev, js, NULL ) != MALI_FALSE )
		{
			kbase_jd_atom *dequeued_atom;
			mali_bool has_job = MALI_FALSE;
			
			/* Dequeue a job that matches the requirements */
			has_job = kbasep_js_policy_dequeue_job_irq( kbdev, js, &dequeued_atom );
			
			if ( has_job != MALI_FALSE )
			{
				/* NOTE: since the runpool_irq lock is currently held and acts across
				 * all address spaces, any context whose busy refcount has reached
				 * zero won't yet be scheduled out whilst we're trying to run jobs
				 * from it */
				kbase_context *parent_ctx = dequeued_atom->kctx;
				mali_bool retain_success;

				/* Retain/power up the cores it needs, check if cores are ready */
				cores_ready = kbasep_js_job_check_ref_cores( kbdev, js, dequeued_atom );

				if ( cores_ready != MALI_TRUE )
				{
					/* The job can't be submitted until the cores are ready, requeue the job */
					kbasep_js_policy_enqueue_job( &kbdev->js_data.policy, dequeued_atom );
					break;
				}

				/* ASSERT that the Policy picked a job from an allowed context */
				OSK_ASSERT( kbasep_js_is_submit_allowed( js_devdata, parent_ctx) );

				/* Retain the context to stop it from being scheduled out
				 * This is released when the job finishes */
				retain_success = kbasep_js_runpool_retain_ctx_nolock( kbdev, parent_ctx );
				OSK_ASSERT( retain_success != MALI_FALSE );
				CSTD_UNUSED( retain_success );

				/* Retain the affinity on the slot */
				kbase_js_affinity_retain_slot_cores( kbdev, js, dequeued_atom->affinity );

				/* Check if this job needs the cycle counter enabled before submission */
				kbasep_js_ref_permon_check_and_enable_cycle_counter( kbdev, dequeued_atom );

				/* Submit the job */
				kbase_job_submit_nolock( kbdev, dequeued_atom, js );

				++(*submit_count);
			}
			else
			{
				tried_to_dequeue_jobs_but_failed = MALI_TRUE;
				/* No more jobs - stop submitting for this slot */
				break;
			}
		}
	}

	/* Indicate whether a retry in submission should be tried on a different
	 * dequeue function. These are the reasons why it *must* happen:
	 *
	 * - kbasep_js_policy_dequeue_job_irq() couldn't get any jobs. In this case,
	 *   kbasep_js_policy_dequeue_job() might be able to get jobs (must be done
	 *   outside of IRQ)
	 * - kbasep_js_policy_dequeue_job_irq() got some jobs, but failed to get a
	 *   job in the last call to it. Again, kbasep_js_policy_dequeue_job()
	 *   might be able to get jobs.
	 * - the KBASE_JS_MAX_JOB_SUBMIT_PER_SLOT_PER_IRQ threshold was reached
	 *   and new scheduling must be performed outside of IRQ mode.
	 *
	 * Failure to indicate this correctly could stop further jobs being processed.
	 *
	 * However, we do not _need_ to indicate a retry for the following:
	 * - kbasep_jm_is_submit_slots_free() was MALI_FALSE, indicating jobs were
	 *   already running. When those jobs complete, that will still cause events
	 *   that cause us to resume job submission.
	 * - kbase_js_can_run_job_on_slot_no_lock() was MALI_FALSE - this is for
	 *   Ctx Attribute handling. That _can_ change outside of IRQ context, but
	 *   is handled explicitly by kbasep_js_remove_job() and
	 *   kbasep_js_runpool_release_ctx().
	 */
	return (mali_bool)(tried_to_dequeue_jobs_but_failed || *submit_count >= KBASE_JS_MAX_JOB_SUBMIT_PER_SLOT_PER_IRQ);
}

void kbasep_js_try_run_next_job_on_slot( kbase_device *kbdev, int js )
{
	kbasep_js_device_data *js_devdata;
	mali_bool has_job;
	mali_bool cores_ready;

	OSK_ASSERT( kbdev != NULL );

	js_devdata = &kbdev->js_data;
	
	if (js_devdata->nr_user_contexts_running == 0)
	{
		/* There are no contexts with jobs so return early */
		return;
	}

	kbase_job_slot_lock(kbdev, js);

	/* Keep submitting while there's space to run a job on this job-slot,
	 * and there are jobs to get that match its requirements (see 'break'
	 * statement below) */
	if (  kbasep_jm_is_submit_slots_free( kbdev, js, NULL ) != MALI_FALSE )
	{
		/* Only lock the Run Pool whilst there's work worth doing */
		osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );

		/* The caller of this function may not be aware of Ctx Attribute state changes so we
		 * must recheck if the given slot is still valid. Otherwise do not try to run.
		 */
		if (kbase_js_can_run_job_on_slot_no_lock( js_devdata, js))
		{
			do {
				kbase_jd_atom *dequeued_atom;

				/* Dequeue a job that matches the requirements */
				has_job = kbasep_js_policy_dequeue_job( kbdev, js, &dequeued_atom );

				if ( has_job != MALI_FALSE )
				{
					/* NOTE: since the runpool_irq lock is currently held and acts across
					 * all address spaces, any context whose busy refcount has reached
					 * zero won't yet be scheduled out whilst we're trying to run jobs
					 * from it */
					kbase_context *parent_ctx = dequeued_atom->kctx;
					mali_bool retain_success;

					/* Retain/power up the cores it needs, check if cores are ready */
					cores_ready = kbasep_js_job_check_ref_cores( kbdev, js, dequeued_atom );

					if ( cores_ready != MALI_TRUE )
					{
						/* The job can't be submitted until the cores are ready, requeue the job */
						kbasep_js_policy_enqueue_job( &kbdev->js_data.policy, dequeued_atom );
						break;
					}
					/* ASSERT that the Policy picked a job from an allowed context */
					OSK_ASSERT( kbasep_js_is_submit_allowed( js_devdata, parent_ctx) );

					/* Retain the context to stop it from being scheduled out
					 * This is released when the job finishes */
					retain_success = kbasep_js_runpool_retain_ctx_nolock( kbdev, parent_ctx );
					OSK_ASSERT( retain_success != MALI_FALSE );
					CSTD_UNUSED( retain_success );

					/* Retain the affinity on the slot */
					kbase_js_affinity_retain_slot_cores( kbdev, js, dequeued_atom->affinity );

					/* Check if this job needs the cycle counter enabled before submission */
					kbasep_js_ref_permon_check_and_enable_cycle_counter( kbdev, dequeued_atom );

					/* Submit the job */
					kbase_job_submit_nolock( kbdev, dequeued_atom, js );
				}

			} while ( kbasep_jm_is_submit_slots_free( kbdev, js, NULL ) != MALI_FALSE
				      && has_job != MALI_FALSE );
		}

		osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );
	}
	kbase_job_slot_unlock(kbdev, js);
}

void kbasep_js_try_schedule_head_ctx( kbase_device *kbdev )
{
	kbasep_js_device_data *js_devdata;
	mali_bool has_kctx;
	kbase_context *head_kctx;
	kbasep_js_kctx_info *js_kctx_info;
	mali_bool is_runpool_full;

	OSK_ASSERT( kbdev != NULL );

	js_devdata = &kbdev->js_data;

	/* Make a speculative check on the Run Pool - this MUST be repeated once
	 * we've obtained a context from the queue and reobtained the Run Pool
	 * lock */
	osk_mutex_lock( &js_devdata->runpool_mutex );
	is_runpool_full = check_is_runpool_full(kbdev, NULL);
	osk_mutex_unlock( &js_devdata->runpool_mutex );

	if ( is_runpool_full != MALI_FALSE )
	{
		/* No free address spaces - nothing to do */
		return;
	}

	/* Grab the context off head of queue - if there is one */
	osk_mutex_lock( &js_devdata->queue_mutex );
	has_kctx = kbasep_js_policy_dequeue_head_ctx( &js_devdata->policy, &head_kctx );
	osk_mutex_unlock( &js_devdata->queue_mutex );

	if ( has_kctx == MALI_FALSE )
	{
		/* No ctxs to run - nothing to do */
		return;
	}
	js_kctx_info = &head_kctx->jctx.sched_info;

	OSK_PRINT_INFO(OSK_BASE_JM, "JS: Dequeue Context %p", head_kctx );

	/*
	 * Atomic transaction on the Context and Run Pool begins
	 */
#ifdef CONFIG_VITHAR_RT_PM
	kbase_pm_context_active(kbdev);
	kbase_pm_request_gpu_cycle_counter(kbdev);
#endif
	osk_mutex_lock( &js_kctx_info->ctx.jsctx_mutex );
	osk_mutex_lock( &js_devdata->runpool_mutex );

	/* Re-check to see if the Run Pool is full
	 * Not just to preserve atomicity, but to check against a specific context
	 * (some contexts are allowed in whereas others may not, due to HW workarounds) */
	is_runpool_full = check_is_runpool_full(kbdev, head_kctx);
	if ( is_runpool_full != MALI_FALSE )
	{
		/* No free address spaces - roll back the transaction so far and return */
		osk_mutex_unlock( &js_devdata->runpool_mutex );

		kbasep_js_runpool_requeue_or_kill_ctx( kbdev, head_kctx );

		osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );
#ifdef CONFIG_VITHAR_RT_PM
		kbase_pm_release_gpu_cycle_counter(kbdev);
		kbase_pm_context_idle(kbdev);
#endif
		return;
	}

	KBASE_TRACE_ADD_REFCOUNT( kbdev, JS_TRY_SCHEDULE_HEAD_CTX, head_kctx, NULL, 0u,
							  kbasep_js_trace_get_refcnt(kbdev, head_kctx));


	/* update book-keeping info */
	js_kctx_info->ctx.is_scheduled = MALI_TRUE;

#if MALI_CUSTOMER_RELEASE == 0
	if ( js_devdata->nr_user_contexts_running == 0 )
	{
		/* Only when there are no other contexts submitting jobs:
		 * Latch in run-time job scheduler timeouts that were set through js_timeouts sysfs file */
		if (kbdev->js_soft_stop_ticks != 0)
		{
			js_devdata->soft_stop_ticks = kbdev->js_soft_stop_ticks;
		}
		if (kbdev->js_hard_stop_ticks_ss != 0)
		{
			js_devdata->hard_stop_ticks_ss = kbdev->js_hard_stop_ticks_ss;
		}
		if (kbdev->js_hard_stop_ticks_nss != 0)
		{
			js_devdata->hard_stop_ticks_nss = kbdev->js_hard_stop_ticks_nss;
		}
		if (kbdev->js_reset_ticks_ss != 0)
		{
			js_devdata->gpu_reset_ticks_ss = kbdev->js_reset_ticks_ss;
		}
		if (kbdev->js_reset_ticks_nss != 0)
		{
			js_devdata->gpu_reset_ticks_nss = kbdev->js_reset_ticks_nss;
		}
	}
#endif

	runpool_inc_context_count( kbdev, head_kctx );
	/* Cause any future waiter-on-termination to wait until the context is
	 * descheduled */
	osk_waitq_clear( &js_kctx_info->ctx.not_scheduled_waitq );
	osk_waitq_set( &js_kctx_info->ctx.scheduled_waitq );

	/* Do all the necessaries to pick the address space (inc. update book-keeping info)
	 * Add the context to the Run Pool, and allow it to run jobs */
	assign_and_activate_kctx_addr_space( kbdev, head_kctx );

	if ( (js_kctx_info->ctx.flags & KBASE_CTX_FLAG_PRIVILEGED) != 0 )
	{
		/* We need to retain it to keep the corresponding address space */
		kbasep_js_runpool_retain_ctx(kbdev, head_kctx);
	}

	/* Try to run the next job, in case this context has jobs that match the
	 * job slot requirements, but none of the other currently running contexts
	 * do */
	kbasep_js_try_run_next_job( kbdev );

	/* Transaction complete */
	osk_mutex_unlock( &js_devdata->runpool_mutex );
	osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );
	/* Note: after this point, the context could potentially get scheduled out immediately */
#ifdef CONFIG_VITHAR_RT_PM
	kbase_pm_release_gpu_cycle_counter(kbdev);
	kbase_pm_context_idle(kbdev);
#endif
}

void kbasep_js_schedule_privileged_ctx( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_kctx_info *js_kctx_info;
	kbasep_js_device_data *js_devdata;
	mali_bool is_scheduled;

	OSK_ASSERT( kbdev != NULL );
	OSK_ASSERT( kctx != NULL );

	js_devdata = &kbdev->js_data;
	js_kctx_info = &kctx->jctx.sched_info;

	osk_mutex_lock( &js_kctx_info->ctx.jsctx_mutex );
	/* Mark the context as privileged */
	js_kctx_info->ctx.flags |= KBASE_CTX_FLAG_PRIVILEGED;

	is_scheduled = js_kctx_info->ctx.is_scheduled;
	if ( is_scheduled == MALI_FALSE )
	{
		mali_bool is_runpool_full;

		/* Add the context to the runpool */
		osk_mutex_lock( &js_devdata->queue_mutex );
		kbasep_js_policy_enqueue_ctx( &js_devdata->policy, kctx );
		osk_mutex_unlock( &js_devdata->queue_mutex );

		osk_mutex_lock( &js_devdata->runpool_mutex );
		{
			is_runpool_full = check_is_runpool_full( kbdev, kctx);
			if ( is_runpool_full != MALI_FALSE )
			{
				/* Evict jobs from the NEXT registers to free an AS asap */
				kbasep_js_runpool_evict_next_jobs( kbdev );
			}
		}
		osk_mutex_unlock( &js_devdata->runpool_mutex );
		osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );
		/* Fast-starting requires the jsctx_mutex to be dropped, because it works on multiple ctxs */

		if ( is_runpool_full != MALI_FALSE )
		{
			/* Evict non-running contexts from the runpool */
			kbasep_js_runpool_attempt_fast_start_ctx( kbdev, NULL );
		}
		/* The context will be retained and marked as active when scheduled in */
	}
	else 
	{
		/* Already scheduled in - We need to retain it to keep the corresponding address space */
		kbasep_js_runpool_retain_ctx(kbdev, kctx);
		osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );
		
	}

	if (is_scheduled == MALI_FALSE)
	{
		/* Try to schedule the context in */
		kbasep_js_try_schedule_head_ctx( kbdev );

		/* Wait for the context to be scheduled in */
		osk_waitq_wait(&kctx->jctx.sched_info.ctx.scheduled_waitq);
	}
}

void kbasep_js_release_privileged_ctx( kbase_device *kbdev, kbase_context *kctx )
{
	kbasep_js_kctx_info *js_kctx_info;
	OSK_ASSERT( kctx != NULL );
	js_kctx_info = &kctx->jctx.sched_info;

	/* We don't need to use the address space anymore */
	osk_mutex_lock( &js_kctx_info->ctx.jsctx_mutex );
	js_kctx_info->ctx.flags &= (~KBASE_CTX_FLAG_PRIVILEGED);
	osk_mutex_unlock( &js_kctx_info->ctx.jsctx_mutex );

	/* Release the context - it wil lbe scheduled out if there is no pending job */
	kbasep_js_runpool_release_ctx(kbdev, kctx);
}


void kbasep_js_job_done_slot_irq( kbase_jd_atom *katom, int slot_nr, kbasep_js_tick *end_timestamp, mali_bool start_new_jobs )
{
	kbase_device *kbdev;
	kbasep_js_policy *js_policy;
	kbasep_js_device_data *js_devdata;
	mali_bool submit_retry_needed = MALI_TRUE; /* If we don't start jobs here, start them from the workqueue */
	kbasep_js_tick tick_diff;
	u32 microseconds_spent = 0u;
	kbase_context *parent_ctx;

	OSK_ASSERT(katom);
	parent_ctx = katom->kctx;
	OSK_ASSERT(parent_ctx);
	kbdev = parent_ctx->kbdev;
	OSK_ASSERT(kbdev);

	js_devdata = &kbdev->js_data;
	js_policy = &kbdev->js_data.policy;

	/*
	 * Release resources before submitting new jobs (bounds the refcount of
	 * the resource to BASE_JM_SUBMIT_SLOTS)
	 */
#if MALI_GATOR_SUPPORT
	kbase_trace_mali_job_slots_event(GATOR_MAKE_EVENT(GATOR_JOB_SLOTS_STOP, slot_nr));
#endif

#if BASE_HW_ISSUE_8316
	if (katom->poking)
	{
		OSK_ASSERT(parent_ctx->as_nr != KBASEP_AS_NR_INVALID);
		kbase_as_poking_timer_release(&kbdev->as[parent_ctx->as_nr]);
		katom->poking = 0;
	}
#endif /* BASE_HW_ISSUE_8316 */

	/* Check if submitted jobs no longer require the cycle counter to be enabled */
	kbasep_js_deref_permon_check_and_disable_cycle_counter( kbdev, katom );

	/* Release the affinity from the slot - must happen before next submission to this slot */
	kbase_js_affinity_release_slot_cores( kbdev, slot_nr, katom->affinity );
	kbase_js_debug_log_current_affinities( kbdev );
	/* Calculate the job's time used */
	if ( end_timestamp != NULL )
	{
		/* Only calculating it for jobs that really run on the HW (e.g. removed
		 * from next jobs never actually ran, so really did take zero time) */
		tick_diff = *end_timestamp - katom->start_timestamp;
		microseconds_spent = kbasep_js_convert_js_ticks_to_us( tick_diff );
		/* Round up time spent to the minimum timer resolution */
		if (microseconds_spent < KBASEP_JS_TICK_RESOLUTION_US)
		{
			microseconds_spent = KBASEP_JS_TICK_RESOLUTION_US;
		}
	}

	/* Lock the runpool_irq for modifying the runpool_irq data */
	osk_spinlock_irq_lock( &js_devdata->runpool_irq.lock );

	/* Log the result of the job (completion status, and time spent). */
	kbasep_js_policy_log_job_result( js_policy, katom, microseconds_spent );
	/* Determine whether the parent context's timeslice is up */
	if ( kbasep_js_policy_should_remove_ctx( js_policy, parent_ctx ) != MALI_FALSE )
	{
		kbasep_js_clear_submit_allowed( js_devdata, parent_ctx );
	}

	if ( start_new_jobs != MALI_FALSE )
	{
		/* Submit a new job (if there is one) to help keep the GPU's HEAD and NEXT registers full */
		KBASE_TRACE_ADD_SLOT( kbdev, JS_JOB_DONE_TRY_RUN_NEXT_JOB, parent_ctx, katom->user_atom, katom->jc, slot_nr);

		submit_retry_needed = kbasep_js_try_run_next_job_on_slot_irq_nolock(
			kbdev,
			slot_nr,
			&kbdev->slot_submit_count_irq[slot_nr] );
	}

	osk_spinlock_irq_unlock( &js_devdata->runpool_irq.lock );
	/* We've finished modifying runpool_irq data, so the lock is dropped */

	if ( submit_retry_needed != MALI_FALSE || katom->event.event_code == BASE_JD_EVENT_STOPPED )
	{
		/* The extra condition on STOPPED jobs is needed because they may be
		 * the only job present, but they won't get re-run until the JD work
		 * queue activates. Crucially, work queues can run items out of order
		 * e.g. on different CPUs, so being able to submit from the IRQ handler
		 * is not a good indication that we don't need to run jobss; the
		 * submitted job could be processed on the work-queue *before* the
		 * stopped job, even though it was submitted after.
		 *
		 * Therefore, we must try to run it, otherwise it might not get run at
		 * all after this. */

		KBASE_TRACE_ADD_SLOT( kbdev, JS_JOB_DONE_RETRY_NEEDED, parent_ctx, katom->user_atom, katom->jc, slot_nr);
		kbasep_js_set_job_retry_submit_slot( katom, slot_nr );
	}
}

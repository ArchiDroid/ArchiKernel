/*
 *
 * (C) COPYRIGHT 2010-2012 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 */



/**
 * @file mali_kbase_pm.c
 * Base kernel power management APIs
 */

#include <osk/mali_osk.h>
#include <kbase/src/common/mali_kbase.h>
#include <kbase/src/common/mali_midg_regmap.h>

#include <kbase/src/common/mali_kbase_pm.h>

/* Policy operation structures */
extern const kbase_pm_policy kbase_pm_always_on_policy_ops;
extern const kbase_pm_policy kbase_pm_demand_policy_ops;

/** A list of the power policies available in the system */
static const kbase_pm_policy * const policy_list[] =
{
#if MALI_NO_MALI
	&kbase_pm_always_on_policy_ops,
	&kbase_pm_demand_policy_ops
#else
#ifdef CONFIG_VITHAR_RT_PM
	&kbase_pm_demand_policy_ops,
	&kbase_pm_always_on_policy_ops
#else
	&kbase_pm_always_on_policy_ops,
	&kbase_pm_demand_policy_ops
#endif
#endif
};

/** The number of policies available in the system.
 * This is devired from the number of functions listed in policy_get_functions.
 */
#define POLICY_COUNT (sizeof(policy_list)/sizeof(*policy_list))

mali_error kbase_pm_init(kbase_device *kbdev)
{
	mali_error ret = MALI_ERROR_NONE;
	osk_error osk_err;
	kbase_pm_callback_conf *callbacks;

	OSK_ASSERT(kbdev != NULL);

	kbdev->pm.gpu_powered = MALI_FALSE;

	callbacks = (kbase_pm_callback_conf*) kbasep_get_config_value(kbdev->config_attributes,
	                                                              KBASE_CONFIG_ATTR_POWER_MANAGEMENT_CALLBACKS);
	if (callbacks)
	{
		kbdev->pm.callback_power_on = callbacks->power_on_callback;
		kbdev->pm.callback_power_off = callbacks->power_off_callback;
	}
	else
	{
		kbdev->pm.callback_power_on = NULL;
		kbdev->pm.callback_power_off = NULL;
	}

	/* Initialise the metrics subsystem */
	ret = kbasep_pm_metrics_init(kbdev);
	if (MALI_ERROR_NONE != ret)
	{
		return ret;
	}

	osk_err = osk_waitq_init(&kbdev->pm.power_up_waitqueue);
	if (OSK_ERR_NONE != osk_err)
	{
		goto power_up_waitq_fail;
	}

	osk_err = osk_waitq_init(&kbdev->pm.power_down_waitqueue);
	if (OSK_ERR_NONE != osk_err)
	{
		goto power_down_waitq_fail;
	}

	osk_err = osk_waitq_init(&kbdev->pm.policy_outstanding_event);
	if (OSK_ERR_NONE != osk_err)
	{
		goto policy_outstanding_event_waitq_fail;
	}
	osk_waitq_set(&kbdev->pm.policy_outstanding_event);


	osk_err = osk_workq_init(&kbdev->pm.workqueue, "kbase_pm", OSK_WORKQ_NON_REENTRANT);
	if (OSK_ERR_NONE != osk_err)
	{
		goto workq_fail;
	}

	osk_err = osk_spinlock_irq_init(&kbdev->pm.power_change_lock, OSK_LOCK_ORDER_POWER_MGMT);
	if (OSK_ERR_NONE != osk_err)
	{
		goto power_change_lock_fail;
	}

	osk_err = osk_spinlock_irq_init(&kbdev->pm.active_count_lock, OSK_LOCK_ORDER_POWER_MGMT_ACTIVE);
	if (OSK_ERR_NONE != osk_err)
	{
		goto active_count_lock_fail;
	}

	osk_err = osk_spinlock_irq_init(&kbdev->pm.gpu_cycle_counter_requests_lock, OSK_LOCK_ORDER_POWER_MGMT_GPU_CYCLE_COUNTER);
	if (OSK_ERR_NONE != osk_err)
	{
		goto gpu_cycle_counter_requests_lock_fail;
	}

#ifdef CONFIG_VITHAR_RT_PM
	osk_err = osk_spinlock_irq_init(&kbdev->pm.cmu_pmu_lock, OSK_LOCK_ORDER_CMU_PMU);
	if (OSK_ERR_NONE != osk_err)
	{
		goto cmu_pmu_lock_fail;
	}
#endif
	osk_err = osk_spinlock_irq_init(&kbdev->pm.gpu_powered_lock, OSK_LOCK_ORDER_POWER_MGMT);
	if (OSK_ERR_NONE != osk_err)
	{
		goto gpu_powered_lock_fail;
	}

	return MALI_ERROR_NONE;

#ifdef CONFIG_VITHAR_RT_PM
cmu_pmu_lock_fail:
    osk_spinlock_irq_term(&kbdev->pm.active_count_lock);
#endif
gpu_powered_lock_fail:
	osk_spinlock_irq_term(&kbdev->pm.gpu_cycle_counter_requests_lock);
gpu_cycle_counter_requests_lock_fail:
	osk_spinlock_irq_term(&kbdev->pm.active_count_lock);
active_count_lock_fail:
	osk_spinlock_irq_term(&kbdev->pm.power_change_lock);
power_change_lock_fail:
	osk_workq_term(&kbdev->pm.workqueue);
workq_fail:
	osk_waitq_term(&kbdev->pm.power_down_waitqueue);
policy_outstanding_event_waitq_fail:
	osk_waitq_term(&kbdev->pm.policy_outstanding_event);
power_down_waitq_fail:
	osk_waitq_term(&kbdev->pm.power_up_waitqueue);
power_up_waitq_fail:
	kbasep_pm_metrics_term(kbdev);
	return MALI_ERROR_FUNCTION_FAILED;
}
KBASE_EXPORT_TEST_API(kbase_pm_init)

mali_error kbase_pm_powerup(kbase_device *kbdev)
{
	mali_error ret;

	OSK_ASSERT(kbdev != NULL);

	ret = kbase_pm_init_hw(kbdev);
	if (ret != MALI_ERROR_NONE)
	{
		return ret;
	}

	kbase_pm_power_transitioning(kbdev);

	kbasep_pm_read_present_cores(kbdev);

	/* Pretend the GPU is active to prevent a power policy turning the GPU cores off */
	osk_spinlock_irq_lock(&kbdev->pm.active_count_lock);
	kbdev->pm.active_count = 1;
	osk_spinlock_irq_unlock(&kbdev->pm.active_count_lock);

	osk_spinlock_irq_lock(&kbdev->pm.gpu_cycle_counter_requests_lock);
	/* Ensure cycle counter is off */
	kbdev->pm.gpu_cycle_counter_requests = 0;
	kbase_reg_write(kbdev, GPU_CONTROL_REG(GPU_COMMAND), GPU_COMMAND_CYCLE_COUNT_STOP, NULL);
	osk_spinlock_irq_unlock(&kbdev->pm.gpu_cycle_counter_requests_lock);

	osk_atomic_set(&kbdev->pm.pending_events, 0);

	osk_atomic_set(&kbdev->pm.work_active,(u32)KBASE_PM_WORK_ACTIVE_STATE_INACTIVE);

	kbdev->pm.new_policy = NULL;
	kbdev->pm.current_policy = policy_list[0];
	kbdev->pm.current_policy->init(kbdev);

	kbase_pm_send_event(kbdev, KBASE_PM_EVENT_POLICY_INIT);

	/* Idle the GPU */
	kbase_pm_context_idle(kbdev);

	return MALI_ERROR_NONE;
}
KBASE_EXPORT_TEST_API(kbase_pm_powerup)

void kbase_pm_power_transitioning(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	/* Clear the wait queues that are used to detect successful power up or down */
	osk_waitq_clear(&kbdev->pm.power_up_waitqueue);
	osk_waitq_clear(&kbdev->pm.power_down_waitqueue);
}

KBASE_EXPORT_TEST_API(kbase_pm_power_transitioning)

void kbase_pm_power_up_done(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	osk_waitq_set(&kbdev->pm.power_up_waitqueue);
}
KBASE_EXPORT_TEST_API(kbase_pm_power_up_done)

void kbase_pm_reset_done(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	osk_waitq_set(&kbdev->pm.power_up_waitqueue);
}
KBASE_EXPORT_TEST_API(kbase_pm_reset_done)

void kbase_pm_power_down_done(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	osk_waitq_set(&kbdev->pm.power_down_waitqueue);
}
KBASE_EXPORT_TEST_API(kbase_pm_power_down_done)

static void kbase_pm_wait_for_no_outstanding_events(kbase_device *kbdev)
{
	osk_waitq_wait(&kbdev->pm.policy_outstanding_event);
}

void kbase_pm_context_active(kbase_device *kbdev)
{
	int c;

	OSK_ASSERT(kbdev != NULL);
	
	osk_spinlock_irq_lock(&kbdev->pm.active_count_lock);
	c = ++kbdev->pm.active_count;
	osk_spinlock_irq_unlock(&kbdev->pm.active_count_lock);

	KBASE_TRACE_ADD_REFCOUNT( kbdev, PM_CONTEXT_ACTIVE, NULL, NULL, 0u, c );

	if (c == 1)
	{
		/* First context active */
		kbase_pm_send_event(kbdev, KBASE_PM_EVENT_GPU_ACTIVE);

		kbasep_pm_record_gpu_active(kbdev);
	}
	/* Synchronise with the power policy to ensure that the event has been noticed */
	kbase_pm_wait_for_no_outstanding_events(kbdev);

	kbase_pm_wait_for_power_up(kbdev);
}
KBASE_EXPORT_TEST_API(kbase_pm_context_active)

void kbase_pm_context_idle(kbase_device *kbdev)
{
	int c;

	OSK_ASSERT(kbdev != NULL);

	osk_spinlock_irq_lock(&kbdev->pm.active_count_lock);

	c = --kbdev->pm.active_count;

	KBASE_TRACE_ADD_REFCOUNT( kbdev, PM_CONTEXT_IDLE, NULL, NULL, 0u, c );

	OSK_ASSERT(c >= 0);
	
	if (c == 0)
	{
		/* Last context has gone idle */
		kbase_pm_send_event(kbdev, KBASE_PM_EVENT_GPU_IDLE);

		kbasep_pm_record_gpu_idle(kbdev);
	}

	/* We must wait for the above functions to finish (in the case c==0) before releasing the lock otherwise there is
	 * a race with another thread calling kbase_pm_context_active - in this case the IDLE message could be sent
	 * *after* the ACTIVE message causing the policy and metrics systems to become confused
	 */
	osk_spinlock_irq_unlock(&kbdev->pm.active_count_lock);
}
KBASE_EXPORT_TEST_API(kbase_pm_context_idle)

void kbase_pm_halt(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	if (kbdev->pm.current_policy != NULL)
	{
		/* Turn the GPU off */
		kbase_pm_send_event(kbdev, KBASE_PM_EVENT_SYSTEM_SUSPEND);
		/* Wait for the policy to acknowledge */
		kbase_pm_wait_for_power_down(kbdev);
	}
}
KBASE_EXPORT_TEST_API(kbase_pm_halt)

void kbase_pm_term(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);
	OSK_ASSERT(kbdev->pm.active_count == 0);
	OSK_ASSERT(kbdev->pm.gpu_cycle_counter_requests == 0);
	/* Destroy the workqueue - this ensures that all messages have been processed */
	osk_workq_term(&kbdev->pm.workqueue);

	if (kbdev->pm.current_policy != NULL)
	{
		/* Free any resources the policy allocated */
		kbdev->pm.current_policy->term(kbdev);
	}

	/* Free the wait queues */
	osk_waitq_term(&kbdev->pm.power_up_waitqueue);
	osk_waitq_term(&kbdev->pm.power_down_waitqueue);
	osk_waitq_term(&kbdev->pm.policy_outstanding_event);
	
	/* Synchronise with other threads */
	osk_spinlock_irq_lock(&kbdev->pm.power_change_lock);
	osk_spinlock_irq_unlock(&kbdev->pm.power_change_lock);

	/* Free the spinlocks */
	osk_spinlock_irq_term(&kbdev->pm.power_change_lock);
	osk_spinlock_irq_term(&kbdev->pm.active_count_lock);
	osk_spinlock_irq_term(&kbdev->pm.gpu_cycle_counter_requests_lock);
	osk_spinlock_irq_term(&kbdev->pm.gpu_powered_lock);
	/* Shut down the metrics subsystem */
	kbasep_pm_metrics_term(kbdev);
}
KBASE_EXPORT_TEST_API(kbase_pm_term)

void kbase_pm_wait_for_power_up(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	osk_waitq_wait(&kbdev->pm.power_up_waitqueue);
}
KBASE_EXPORT_TEST_API(kbase_pm_wait_for_power_up)

void kbase_pm_wait_for_power_down(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	osk_waitq_wait(&kbdev->pm.power_down_waitqueue);
}
KBASE_EXPORT_TEST_API(kbase_pm_wait_for_power_down)

int kbase_pm_list_policies(const kbase_pm_policy * const **list)
{
	if (!list)
		return POLICY_COUNT;

	*list = policy_list;

	return POLICY_COUNT;
}
KBASE_EXPORT_TEST_API(kbase_pm_list_policies)

const kbase_pm_policy *kbase_pm_get_policy(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	return kbdev->pm.current_policy;
}
KBASE_EXPORT_TEST_API(kbase_pm_get_policy)

void kbase_pm_set_policy(kbase_device *kbdev, const kbase_pm_policy *new_policy)
{
	OSK_ASSERT(kbdev != NULL);
	OSK_ASSERT(new_policy != NULL);

	if (kbdev->pm.new_policy) {
		/* A policy change is already outstanding */
		return;
	}
	/* During a policy change we pretend the GPU is active */
	kbase_pm_context_active(kbdev);

	kbdev->pm.new_policy = new_policy;
	kbase_pm_send_event(kbdev, KBASE_PM_EVENT_POLICY_CHANGE);
}
KBASE_EXPORT_TEST_API(kbase_pm_set_policy)

void kbase_pm_change_policy(kbase_device *kbdev)
{
	OSK_ASSERT(kbdev != NULL);

	kbdev->pm.current_policy->term(kbdev);
	kbdev->pm.current_policy = kbdev->pm.new_policy;
	kbdev->pm.current_policy->init(kbdev);
	kbase_pm_send_event(kbdev, KBASE_PM_EVENT_POLICY_INIT);

	/* Now the policy change is finished, we release our fake context active reference */
	kbase_pm_context_idle(kbdev);
	
	kbdev->pm.new_policy = NULL;
}
KBASE_EXPORT_TEST_API(kbase_pm_change_policy)

/** Callback for the power management work queue.
 *
 * This function is called on the power management work queue and is responsible for delivering events to the active 
 * power policy. It manipulates the @ref kbase_pm_device_data.work_active field of @ref kbase_pm_device_data to track 
 * whether all events have been consumed.
 *
 * @param data      A pointer to the @c pm.work field of the @ref kbase_device struct
 */

STATIC void kbase_pm_worker(osk_workq_work *data)
{
	kbase_device *kbdev = CONTAINER_OF(data, kbase_device, pm.work);
	int pending_events;
	int old_value;
	int i;

	do
	{
	  osk_atomic_set(&kbdev->pm.work_active, (u32)KBASE_PM_WORK_ACTIVE_STATE_PROCESSING);

		/* Atomically read and clear the bit mask */
		pending_events = osk_atomic_get(&kbdev->pm.pending_events);

		do
		{
			old_value = pending_events;
			pending_events = osk_atomic_compare_and_swap(&kbdev->pm.pending_events, old_value, 0);
		} while (old_value != pending_events);

		for(i = 0; pending_events; i++)
		{
			if (pending_events & (1 << i))
			{
				kbdev->pm.current_policy->event(kbdev, (kbase_pm_event)i);

				pending_events &= ~(1 << i);
			}
		}
		i = osk_atomic_compare_and_swap(&kbdev->pm.work_active,
						(u32)KBASE_PM_WORK_ACTIVE_STATE_PROCESSING,
						(u32)KBASE_PM_WORK_ACTIVE_STATE_INACTIVE);
	} while (i == (u32)KBASE_PM_WORK_ACTIVE_STATE_PENDING_EVT);
	osk_waitq_set(&kbdev->pm.policy_outstanding_event);
}
KBASE_EXPORT_TEST_API(kbase_pm_worker)

/** Merge an event into the list of events to deliver.
 *
 * This ensures that if, for example, a GPU_IDLE is immediately followed by a GPU_ACTIVE then instead of delivering 
 * both messages to the policy the GPU_IDLE is simply discarded.
 *
 * In particular in the sequence GPU_IDLE, GPU_ACTIVE, GPU_IDLE the resultant message is GPU_IDLE and not (GPU_IDLE 
 * and GPU_ACTIVE).
 *
 * @param old_events    The bit mask of events that were previously pending
 * @param new_event     The event that should be merged into old_events
 *
 * @return The combination of old_events and the new event
 */
STATIC int kbasep_pm_merge_event(int old_events, kbase_pm_event new_event)
{
	switch(new_event) {
		case KBASE_PM_EVENT_POLICY_INIT:
			/* On policy initialisation, ignore any pending old_events. */
			return ( 1 << KBASE_PM_EVENT_POLICY_INIT);

		case KBASE_PM_EVENT_GPU_STATE_CHANGED:
		case KBASE_PM_EVENT_POLICY_CHANGE:
		case KBASE_PM_EVENT_CHANGE_GPU_STATE:
			/* Just merge these events into the list */
			return old_events | (1 << new_event);
		case KBASE_PM_EVENT_SYSTEM_SUSPEND:
			if (old_events & (1 << KBASE_PM_EVENT_SYSTEM_RESUME))
			{
				return old_events & ~(1 << KBASE_PM_EVENT_SYSTEM_RESUME);
			}
			return old_events | (1 << new_event);
		case KBASE_PM_EVENT_SYSTEM_RESUME:
			if (old_events & (1 << KBASE_PM_EVENT_SYSTEM_SUSPEND))
			{
				return old_events & ~(1 << KBASE_PM_EVENT_SYSTEM_SUSPEND);
			}
			return old_events | (1 << new_event);
		case KBASE_PM_EVENT_GPU_ACTIVE:
			if (old_events & (1 << KBASE_PM_EVENT_GPU_IDLE))
			{
				return old_events & ~(1 << KBASE_PM_EVENT_GPU_IDLE);
			}
			return old_events | (1 << new_event);
		case KBASE_PM_EVENT_GPU_IDLE:
			if (old_events & (1 << KBASE_PM_EVENT_GPU_ACTIVE))
			{
				return old_events & ~(1 << KBASE_PM_EVENT_GPU_ACTIVE);
			}
			return old_events | (1 << new_event);
		default:
			/* Unrecognised event - this should never happen */
			OSK_ASSERT(0);
			return old_events | (1 << new_event);
	}
}
KBASE_EXPORT_TEST_API(kbasep_pm_merge_event)

void kbase_pm_send_event(kbase_device *kbdev, kbase_pm_event event)
{
	int pending_events;
	int work_active;
	int old_value, new_value;

	OSK_ASSERT(kbdev != NULL);

	pending_events = osk_atomic_get(&kbdev->pm.pending_events);

	/* Atomically OR the new event into the pending_events bit mask */
	do
	{
		old_value = pending_events;
		new_value = kbasep_pm_merge_event(pending_events, event);
		if (old_value == new_value)
		{
			/* Event already pending */
			return;
		}
		pending_events = osk_atomic_compare_and_swap(&kbdev->pm.pending_events, old_value, new_value);
	} while (old_value != pending_events);

	work_active = osk_atomic_get(&kbdev->pm.work_active);
	do
	{
		old_value = work_active;
		switch(old_value)
		{
			case KBASE_PM_WORK_ACTIVE_STATE_INACTIVE:
				/* Need to enqueue an event */
				new_value = KBASE_PM_WORK_ACTIVE_STATE_ENQUEUED;
				break;
			case KBASE_PM_WORK_ACTIVE_STATE_ENQUEUED:
				/* Event already queued */
				return;
			case KBASE_PM_WORK_ACTIVE_STATE_PROCESSING:
				/* Event being processed, we need to ensure it checks for another event */
				new_value = KBASE_PM_WORK_ACTIVE_STATE_PENDING_EVT;
				break;
			case  KBASE_PM_WORK_ACTIVE_STATE_PENDING_EVT:
				/* Event being processed, but another check for events is going to happen */
				return;
			default:
				OSK_ASSERT(0);
		}
		work_active = osk_atomic_compare_and_swap(&kbdev->pm.work_active, old_value, new_value);
	} while (old_value != work_active);

	if (old_value == KBASE_PM_WORK_ACTIVE_STATE_INACTIVE)
	{
		osk_waitq_clear(&kbdev->pm.policy_outstanding_event);
		osk_workq_work_init(&kbdev->pm.work, kbase_pm_worker);
		osk_workq_submit(&kbdev->pm.workqueue, &kbdev->pm.work);
	}
}


KBASE_EXPORT_TEST_API(kbase_pm_send_event)

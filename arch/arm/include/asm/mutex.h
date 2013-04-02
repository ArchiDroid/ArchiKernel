/*
 * arch/arm/include/asm/mutex.h
 *
 * ARM optimized mutex locking primitives
 *
 * Please look into asm-generic/mutex-xchg.h for a formal definition.
 */
#ifndef _ASM_MUTEX_H
#define _ASM_MUTEX_H
//LGE_CHANGE_S[panchaxari.t@lge.com][patch from 3.4.26 kernel]
/*
 * On pre-ARMv6 hardware this results in a swp-based implementation,
 * which is the most efficient. For ARMv6+, we emit a pair of exclusive
 * accesses instead.
 */
#include <asm-generic/mutex-xchg.h>
#endif
//LGE_CHANGE_E[panchaxari.t@lge.com][patch from 3.4.26 kernel]

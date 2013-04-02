/*
 * as364x.h -  platform data structure for as364x led controller
 *
 * Copyright (C) 2010 Ulrich Herrmann <ulrich.herrmann@austriamicrosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#ifndef __LINUX_AS364X_H
#define __LINUX_AS364X_H

struct as364x_platform_data {
	bool use_tx_mask;
	u16 I_limit_mA; /* 2000, 2500, 3000, 3500 mA for the coil*/
	u16 txmasked_current_mA; /* 57,113,...847, roughly 56.47 mA steps */
	u16 vin_low_v_run_mV; /* 0=off, 3000, 3070, 3140, 3220, 3300, 3338,
				 3470 mV battery limit for dynamic flash
				 reduction */
	u16 vin_low_v_mV; /* 0=off, 3000, 3070, 3140, 3220, 3300, 3338, 3470 mV
			     battery limit for flash denial */
	u8 strobe_type; /* 0=edge, 1=level */
	bool freq_switch_on;
	bool led_off_when_vin_low; /* if 0 txmask current is used */
	/* LED configuration, two identical leds must be connected. */
	u16 max_peak_current_mA; /* This leds maximum peak current in mA */
	u16 max_peak_duration_ms; /* the maximum duration max_peak_current_mA
				     can be applied */
	u16 max_sustained_current_mA; /* This leds maximum sustained current
					 in mA */
	u16 min_current_mA; /* This leds minimum current in mA, desired
			       values smaller than this will be realised
			       using PWM. */
};
#endif /* __LINUX_as364x_H */

/*
 *  drivers/cpufreq/cpufreq_zzmoove.c
 *
 *  Copyright (C)  2001 Russell King
 *            (C)  2003 Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>.
 *                      Jun Nakajima <jun.nakajima@intel.com>
 *            (C)  2009 Alexander Clouter <alex@digriz.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------------
 * -  ZZMoove Governor by ZaneZam 2012/13 Changelog:													  -
 * --------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * Version 0.1 - first release
 *
 *	- codebase latest smoove governor version from midnight kernel (https://github.com/mialwe/mngb/)
 *	- modified frequency tables to match I9300 standard frequency range 200-1400 mhz
 *	- added cpu hotplug functionality with strictly cpu switching
 *	  (modifications partially taken from ktoonservative governor from
 *	  ktoonsez KT747-JB kernel https://github.com/ktoonsez/KT747-JB)
 *
 * Version 0.2 - improved
 *
 *	- added tuneables to be able to adjust values on early suspend (screen off) via sysfs instead
 *	  of using only hardcoded defaults
 *	- modified hotplug implementation to be able to tune threshold range per core indepentently
 *	  and to be able to manually turn cores offline
 *
 *	  for this functions following new tuneables were indroduced:
 *
 *	  sampling_rate_sleep_multiplier -> sampling rate multiplier on early suspend (possible values 1 or 2, default: 2)
 *	  up_threshold_sleep		 -> up threshold on early suspend (possible range from above "down_threshold_sleep" up to 100, default: 90)
 *	  down_threshold_sleep		 -> down threshold on early suspend (possible range from 11 to "under up_threshold_sleep", default: 44)
 *	  smooth_up_sleep		 -> smooth up scaling on early suspend (possible range from 1 to 100, default: 100)
 *	  up_threshold_hotplug1		 -> hotplug threshold for cpu1 (0 disable core1, possible range from "down_threshold" up to 100, default: 68)
 *	  up_threshold_hotplug2		 -> hotplug threshold for cpu2 (0 disable core2, possible range from "down_threshold" up to 100, default: 68)
 *	  up_threshold_hotplug3		 -> hotplug threshold for cpu3 (0 disable core3, possible range from "down_threshold" up to 100, default: 68)
 *	  down_threshold_hotplug1	 -> hotplug threshold for cpu1 (possible range from 11 to under "up_threshold", default: 55)
 *	  down_threshold_hotplug2	 -> hotplug threshold for cpu2 (possible range from 11 to under "up_threshold", default: 55)
 *	  down_threshold_hotplug3	 -> hotplug threshold for cpu3 (possible range from 11 to under "up_threshold", default: 55)
 *
 * Version 0.3 - more improvements
 *
 *	- added tuneable "hotplug_sleep" to be able to turn cores offline only on early suspend (screen off) via sysfs
 *	  possible values: 0 do not touch hotplug-settings on early suspend, values 1, 2 or 3 are equivalent to
 *	  cores which should be online at early suspend
 *	- modified scaling frequency table to match "overclock" freqencies to max 1600 mhz
 *	- fixed black screen of dead problem in hotplug logic due to missing mutexing on 3-core and 2-core settings
 *	- code cleaning and documentation
 *
 * Version 0.4 - limits
 *
 *	- added "soft"-freqency-limit. the term "soft" means here that this is unfortuneately not a hard limit. a hard limit is only possible with
 *	  cpufreq driver which is the freqency "giver" the governor is only the "consultant". So now the governor will scale up to only the given up
 *	  limit on higher system load but if the cpufreq driver "wants" to go above that limit the freqency will go up there. u can see this for
 *	  example with touchboost or wake up freqencies (1000 and 800 mhz) where the governor obviously will be "bypassed" by the cpufreq driver.
 *	  but nevertheless this soft-limit will now reduce the use of freqencies higer than given limit and therefore it will reduce power consumption.
 *
 *	  for this function following new tuneables were indroduced:
 *
 *	  freq_limit_sleep		 -> limit freqency on early suspend (possible values 0 disable limit, 200-1600, default: 0)
 *	  freq_limit			 -> limit freqency on awake (possible values 0 disable limit, 200-1600, default: 0)
 *
 *	- added scaling frequencies to frequency tables for a faster up/down scaling. This should bring more performance but on the other hand it
 *	  can be of course a little bit more power consumptive.
 *
 *	  for this function following new tuneables were indroduced:
 *
 *	  fast_scaling			 -> fast scaling on awake (possible values 0 disable or 1 enable, default: 0)
 *	  fast_scaling_sleep (sysfs)	 -> fast scaling on early suspend (possible values 0 disable or 1 enable, default: 0)
 *
 *	- added tuneable "freq_step_sleep" for setting the freq step at early suspend (possible values same as freq_step 0 to 100, default 5)
 *	- added DEF_FREQ_STEP and IGNORE_NICE macros
 *	- changed downscaling cpufreq relation to the "lower way"
 *	- code/documentation cleaning
 *
 * Version 0.5 - performance and fixes
 *
 *	- completely reworked fast scaling functionality. now using a "line jump" logic instead of fixed freq "colums".
 *	  fast scaling now in 4 steps and 2 modes possible (mode 1: only fast scaling up and mode2: fast scaling up/down)
 *	- added support for "Dynamic Screen Frequency Scaling" (original implementation into zzmoove governor highly improved by Yank555)
 *	  originated by AndreiLux more info: http://forum.xda-developers.com/showpost.php?p=38499071&postcount=3
 *	- re-enabled broken conservative sampling down factor functionality ("down skip" method).
 *	  originated by Stratosk - upstream kernel 3.10rc1:
 *	  https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/log/?id=refs%2Ftags%2Fv3.10-rc1&qt=author&q=Stratos+Ka
 *	- changed down threshold check to act like it should.
 *	  originated by Stratosk - upstream kernel 3.10rc1:
 *	  https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/log/?id=refs%2Ftags%2Fv3.10-rc1&qt=author&q=Stratos+Ka
 *	- implemented/ported "early demand" from ondemand governor.
 *	  originated by Stratosk - more info: http://www.semaphore.gr/80-latests/98-ondemand-early-demand
 *	- implemented/ported "sampling down momentum" from ondemand governor.
 *	  originated by Stratosk - more info: http://www.semaphore.gr/80-latests/80-sampling-down-momentum
 *	- modified some original conservative code parts regarding frequency scaling which should work better now.
 *	  originated by DerTeufel1980: https://github.com/DerTeufel/android_kernel_samsung_smdk4412/commit/6bab622344c548be853db19adf28c3917896f0a0
 *	- added the possibility to use sampling down momentum or conservative "down skip" method.
 *	- increased possible max sampling rate sleep multiplier to 4 and sampling down factor to 100000
 *	  accordingly to sampling down momentum implementation.
 *	- added frequency search limit for more efficient frequency searching in scaling "table" and for improving
 *	  frequency "hard" and "soft" limit handling.
 *	- added cpu idle exit time handling like it is in lulzactive
 *	  again work from ktoonsez : https://github.com/ktoonsez/KT747-JB/commit/a5931bee6ea9e69f386a340229745da6f2443b78
 *	  description in lulzactive governor:
 *	  https://github.com/ktoonsez/KT747-JB/blob/a5931bee6ea9e69f386a340229745da6f2443b78/drivers/cpufreq/cpufreq_lulzactive.c
 *	- fixed a little scaling step mistake and added overclocking frequencies up to 1800 mhz in scaling frequency "tables".
 *	- fixed possible freezes during start/stop/reload of governor and frequency limit change.
 *	- fixed hotplugging logic at online core 0+3 or 0+2 situations and improved hotplugging in general by
 *	  removing mutex locks and skipping hotplugging when it is not needed.
 *	- added possibility to disable hotplugging (that's a debugging relict but i thought maybe someone will find that usefull so i didn't remove it)
 *	- try to fix lags when coming from suspend if hotplug limitation at sleep was active by enabling all offline cores during resume.
 *	- code cleaning and documentation.
 *
 *	  for this functions following new tuneables were indroduced:
 *
 *	  Early Demand:
 *	  -------------
 *	  early_demand			-> switch to enable/disable early demand functionality (possible values 0 disable or 1 enable, default: 0)
 *	  grad_up_threshold		-> scale up frequency if the load goes up in one step of grad up value (possible range from 11 to 100, default 50)
 *	                                   little example for understanding: when the load rises up in one big 50% step then the
 *	                                   frequency will be scaled up immediately instead of wating till up_threshold is reached.
 *
 *	  Fast Scaling (improved):
 *	  ------------------------
 *	  Fast scaling has now 8 levels which at the same time have 2 modes included. Values from 1-4 equals to scaling jumps in the frequency table
 *	  and uses the Fast Scaling up but normal scaling down mode. Values from 5-8 equals to 1-4 scaling jumps but uses the fast scaling up and fast
 *	  scaling down mode.
 *
 *	  Hotplugging switch:
 *	  -------------------
 *	  disable_hotplug		-> switch to enable/disable hotplugging (possible values are any value above 0 to disable hotplugging and 0 to
 *	                                   enable it, default 0)
 *
 *	  Sampling Down Factor and Sampling Down Momentum:
 *	  ------------------------------------------------
 *	  Description: From the original author of ondemand_sampling_factor David Niemi:
 *	  "This improves performance by reducing the overhead of load evaluation and helping the CPU stay
 *	  at its top speed when truly busy, rather than shifting back and forth in speed."
 *
 *	  And that "Sampling Down Momentum" function from stratosk does this dynamicly now! ;)
 *
 *	  sampling_down_max_momentum		-> max sampling down factor which should be set by momentum (0 disable momentum, possible range from
 *	                                           sampling_down_factor up to MAX_SAMPLING_DOWN_FACTOR, default 0 disabled)
 *	  sampling_down_momentum_sensitivity	-> how fast the sampling down factor should be switched (possible values from 1 to 500, default 50)
 *	  sampling_down_factor			-> depending on which mode is active the factor for sampling rate multiplier which influences the whole
 *	                                           sampling rate or the value for stock "down skip" functionality which influences only the down scaling
 *	                                           mechanism (possible values are from 1 to MAX_SMPLING_DOWN_FACTOR, default 1 disabled)
 *
 *	  Original conservative "down skip" or "stock" method can be enabled by setting the momentum tuneable to 0. so if momentum is inactive there will
 *	  be a fallback to the stock method. as the name "down skip" says this method works "slightly" different from the ondemand stock sampling down method
 *	  (on which momentum was based on). It just skips the scaling down code for the given samples. if u want to completely disable the sampling down
 *	  functionality u can achieve this by setting sampling down factor to 1. so concluded: setting sampling_down_momentum = 0 and sampling_down_factor = 1
 *	  will disable sampling down completely (that is also the governor default setting)
 *
 *	  Dynamic Screen Frequency Scaling:
 *	  --------------------------------
 *
 *	  Dynamicly switches the screen frequency to 40hz or 60hz depending on cpu scaling and hotplug settings.
 *	  For compiling and enabling this functionality u have to do some more modification to the kernel sources, please take a look at AndreiLux Perseus
 *	  repository and there at following commit: https://github.com/AndreiLux/Perseus-S3/commit/3476799587d93189a091ba1db26a36603ee43519
 *	  After adding this patch u can enable the feature by setting "CPU_FREQ_LCD_FREQ_DFS=y" in your kernel config and if u want to check if it is
 *	  really working at runtime u can also enable the accounting which AndreiLux added by setting LCD_FREQ_SWITCH_ACCOUNTING=y in the kernel config.
 *	  If all goes well and u have the DFS up and running u can use following tuneables to do some screen magic:
 *	  (thx to Yank555 for highly extend and improving this!)
 *
 *	  lcdfreq_enable		-> to enable/disable LCDFreq scaling (possible values 0 disable or 1 enable, default: 0)
 *	  lcdfreq_kick_in_down_delay	-> the amount of samples to wait below the threshold frequency before entering low display frequency mode (40hz)
 *	  lcdfreq_kick_in_up_delay	-> the amount of samples to wait over the threshold frequency before entering high display frequency mode (60hz)
 *	  lcdfreq_kick_in_freq		-> the frequency threshold - below this cpu frequency the low display frequency will be active
 *	  lcdfreq_kick_in_cores		-> the number of cores which should be online before switching will be active. (also useable in combination
 *	                                   with kickin_freq)
 *
 *	  So this version is a kind of "featured by" release as i took (again *g*) some ideas and work from other projects and even some of that work
 *	  comes directly from other devs so i wanna thank and give credits:
 *
 *	  First of all to stratosk for his great work "sampling down momentum" and "early demand" and for all the code fixes which found their way into
 *	  the upstream kernel version of conservative governor! congrats and props on that stratos, happy to see such a nice and talented dev directly
 *	  contibuting to the upstream kernel, that is a real enrichment for all of us!
 *
 *	  Second to Yank555 for coming up with the idea and improving/completeing (leaves nothing to be desired now *g*) my first
 *	  rudimentary implementation of Dynamic Screen Frequency Scaling from AndreiLux (credits for the idea/work also to him at this point!).
 *
 *	  Third to DerTeufel1980 for his first implementation of stratosk's early demand functionality into version 0.3 of zzmoove governor
 *	  (even though i had to modify the original implementation a "little bit" to get it working properly ;)) and for some code optimizations/fixes
 *	  regarding scaling.
 *
 *	  Last but not least again to ktoonsez - I "cherry picked" again some code parts of his ktoonservative governor which should improve this governor
 *	  too.
 *
 * Version 0.5.1b - bugfixes and more optimisations (in cooperation with Yank555)
 *
 *	- highly optimised scaling logic (thx and credits to Yank555)
 *	- simplified some tuneables by using already available stuff instead of using redundant code (thx Yank555)
 *	- reduced/optimised hotplug logic and preperation for automatic detection of available cores
 *	  (maybe this fixes also the scaling/core stuck problems)
 *	- finally fixed the freezing issue on governor stop!
 *
 * Version 0.6 - flexibility (in cooperation with Yank555)
 *
 *	- removed fixed scaling lookup tables and use the system frequency table instead
 *	  changed scaling logic accordingly for this modification (thx and credits to Yank555)
 *	- reduced new hotplug logic loop to a minimum
 *	- again try to fix stuck issues by using seperate hotplug functions out of dbs_check_cpu (credits to ktoonesz)
 *	- added support for 2 and 8 core systems and added automatic detection of cores were it is needed
 *	  (for setting the different core modes you can use the macro 'MAX_CORES'. possible values are: 2,4 or 8, default are 4 cores)
 *	  reduced core threshold defaults to only one up/down default and use an array to hold all threshold values
 *	- fixed some mistakes in "frequency tuneables" (Yank555):
 *	  stop looping once the frequency has been found
 *	  return invalid error if new frequency is not found in the frequency table
 *
 * Version 0.6a - scaling logic flexibility (in cooperation with Yank555)
 *
 *	- added check if CPU freq. table is in ascending or descending order and scale accordingly
 *	  (compatibility for systems with 'inverted' frequency table like it is on OMAP4 platform)
 *	  thanks and credits to Yank555!
 *
 * Version 0.7 - slow down (in cooperation with Yank555)
 *
 *	- reindroduced the "old way" of hotplugging and scaling in form of the "Legacy Mode" (macros for enabling/disabling this done by Yank555, thx!)
 *	  NOTE: this mode can only handle 4 cores and a scaling max frequency up to 1800mhz.
 *	- added hotplug idle threshold for a balanced load at CPU idle to reduce possible higher idle temperatures when running on just one core.
 *        (inspired by JustArchi's observations, thx!)
 *	- added hotplug block cycles to reduce possible hotplugging overhead (credits to ktoonsez)
 *	- added possibility to disable hotplugging only at suspend (inspired by a request of STAticKY, thx for the idea)
 *	- introduced hotplug frequency thresholds (credits to Yank555)
 *	- hotplug tuneables handling optimized (credits to Yank555)
 *	- added version information tuneable (credits to Yank555)
 *
 *	  for this functions following new tuneables were indroduced:
 *
 *	  legacy_mode			-> for switching to the "old" method of scaling/hotplugging. possible values 0 to disable,
 *					   any values above 0 to enable (default is 0)
 *					   NOTE: the legacy mode has to be enabled by uncommenting the macro ENABLE_LEGACY_MODE below!
 *	  hotplug_idle_threshold	-> amount of load under which hotplugging should be disabled at idle times (respectively at scaling minimum).
 *					   possible values 0 disable, from 1 to 100 (default is 0)
 *	  hotplug_block_cycles		-> slow down hotplugging by waiting a given amount of cycles before plugging.
 *					   possible values 0 disbale, any values above 0 (default is 0)
 *	  disable_hotplug_sleep		-> same as disable_hotplug but will only take effect at suspend.
 *					   possible values 0 disable, any values above 0 to enable (default is 0)
 *	  up_threshold_hotplug_freq1	-> hotplug up frequency threshold for core1.
 *					   possible values 0 disable and range from over down_threshold_hotplug_freq1 to max scaling freqency (default is 0)
 *	  up_threshold_hotplug_freq2	-> hotplug up frequency threshold for core2.
 *					   possible values 0 disable and range from over down_threshold_hotplug_freq2 to max scaling freqency (default is 0)
 *	  up_threshold_hotplug_freq3	-> hotplug up frequency threshold for core3.
 *					   possible values 0 disable and range from over down_threshold_hotplug_freq3 to max scaling freqency (default is 0)
 *	  down_threshold_hotplug_freq1	-> hotplug down frequency threshold for core1.
 *					   possible values 0 disable and range from min saling to under up_threshold_hotplug_freq1 freqency (default is 0)
 *	  down_threshold_hotplug_freq2	-> hotplug down frequency threshold for core2.
 *					   possible values 0 disable and range from min saling to under up_threshold_hotplug_freq2 freqency (default is 0)
 *	  down_threshold_hotplug_freq3	-> hotplug down frequency threshold for core3.
 *					   possible values 0 disable and range from min saling to under up_threshold_hotplug_freq3 freqency (default is 0)
 *	  version			-> show the version of zzmoove governor
 *
 * Version 0.7a - little fix
 *
 *	- fixed a glitch in hotplug freq threshold tuneables which prevented setting of values in hotplug down freq thresholds when hotplug
 *	  up freq thresholds were set to 0
 *
 * Version 0.7b - compatibility improved and forgotten things
 *
 *	- fixed stuck at max scaling frequency when using stock kernel sources with unmodified cpufreq driver and without any oc capabilities.
 *	- readded forgotten frequency search optimisation in scaling logic (only effective when using governor soft or cpufreq frequency limit)
 *	- readded forgotten minor optimisation in dbs_check_cpu function
 *	- as forgotten to switch in last version Legacy Mode now again disabled by default
 *	- minor code format and comment fixes
 *
 * Version 0.7c - again compatibility and optimisations
 *
 *	- frequency search optimisation now fully compatible with ascending ordered system frequency tables (thx to psndna88 for testing!)
 *	- again minor optimisations at multiple points in dbs_check_cpu function
 *	- code cleaning - removed some unnecessary things and whitespaces nuked (sry for the bigger diff but from now on it will be clean ;))
 *	- corrected changelog for previous version regarding limits
 *
 * Version 0.7d - broken things
 *
 *	- fixed hotplug up threshold tuneables to be able again to disable cores manually via sysfs by setting them to 0
 *	- fixed the problem caused by a "wrong" tuneable apply order of non sticking values in hotplug down threshold tuneables when 
 *	  hotplug up values are lower than down values during apply.
 *	  NOTE: due to this change right after start of the governor the full validation of given values to these tuneables is disabled till 
 *	  all the tuneables were set for the first time. so if you set them for example with an init.d script or let them set automatically 
 *	  with any tuning app be aware that there are illogical value combinations possible then which might not work properly! 
 *	  simply be sure that all up values are higher than the down values and vice versa. after first set full validation checks are enabled 
 *	  again and setting of values manually will be checked again.
 *	- fixed a typo in hotplug threshold tuneable macros (would have been only a issue in 8-core mode)
 *	- fixed unwanted disabling of cores when setting hotplug threshold tuneables to lowest or highest possible value 
 *	  which would be a load of 100%/11% in up/down_hotplug_threshold and/or scaling frequency min/max in up/down_hotplug_threshold_freq
 *
 * Version 0.8 - dynamic sampling rate and profiles
 *
 *	- added scaling up block cycles for a adjustable slow down of upscaling (in normal and legacy mode)
 *	  this should avoid unwanted jumps to highest (how high depends on settings) frequency on idle when a load comes up just for a short peroid
 *	  of time and is hitting the up threshold because of that. reducing these jumps lowers CPU temperature on idle and maybe saves also a little bit of juice.
 *	  beside of that u can also influence the little bit odd scaling behaving when you a are running apps which are constandly "holding" the load in
 *	  some range and the freq seems to be unessesarly high for that range. In fact it just looks like so monitoring apps are mostly to slow to catch
 *	  realtime load - so you are watching "the past" actually. the app is stressing the system and holding it on a higer load level, due to this load level scaling 
 *	  up threshold is more often reached and the governor scales up and almost never scales down again. with the scaling up block cycles u can throttle 
 *	  the up scaling when the load is hanging around near up threshold. when the load is going over up threshold scaling up takes just a break for the 
 *	  amount of cycles that you have set up.
 *	- indroduced (D)ynamic (S)ampling (R)ate - thx to hellsgod for having the same idea at the same time and pointing me to an example even though 
 *	  at the end i did "my way" :) DSR switches between two sampling rates depending on the given load threshold from an idle one to a normal one.
 *	- indroduced build in profiles (credits to Yank555 for idea and prototype header file)
 *	  you can switch between multible build in profiles by just piping a number into the tuneable "profile_number". the settings of the profile will 
 *	  be applied then on the fly. when a profile is active and u set any value to a custom one profile switches to custom.
 *	- added enabling of offline cores on governor exit to avoid cores stucking in offline state when switching to a non-hotplugable governor
 *	  and by the way reduced reduntant code by using an inline function for switching cores on at other relevant points too (NOTE: switching
 *	  cores at governor stop might not work every time because doing such things on governor stop is a ittle bit tricky and also risky,
 *	  but that will maybe be improved in future versions
 *	- fixed setting to value 0 not possible in hotplug up threshold tunesables right after start of the governor because of apply order exception
 *	- changed value restriction from 11 to 1 in hotplug down threshold tuneables as the value 11 is only nessesary in scaling down tuneable
 *	- added missing fast scaling down / normal scaling up mode to fast scaling feature (value range 9-12 and only available in non-legacy mode)
 *	  thx OldBoy.Gr for pointing me to that missing mode
 *	- fixed stopping of up scaling at 100% load when up threshold tuneable is set to the max value of 100
 *	- fixed smooth up not active at 100% load when smooth up tuneable is set to the max value of 100
 *---------------------------------------------------------------------------------------------------------------------------------------------------------
 *-                                                                                                                                                       -
 *---------------------------------------------------------------------------------------------------------------------------------------------------------
 */

// Yank: Added a sysfs interface to display current zzmoove version
#define ZZMOOVE_VERSION "0.8-beta8"

// Yank: Allow to include or exclude legacy mode (support for SGS3/Note II only and max scaling freq 1800mhz!)
#define ENABLE_LEGACY_MODE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/earlysuspend.h>

// ZZ: include profiles header and set name for custom profile (tuneable mode or nothing applied)
#include "cpufreq_zzmoove_profiles.h"
static char custom_profile[20] = "custom"; // ZZ: name for custom profile

// cpu load trigger
#define DEF_SMOOTH_UP (75)

/*
 * dbs is used in this file as a shortform for demandbased switching
 * It helps to keep variable names smaller, simpler
 */

// ZZ: midnight and zzmoove default values
#define DEF_FREQUENCY_UP_THRESHOLD		  (70)
#define DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG	  (68)	// ZZ: default for hotplug up threshold for all cpus (cpu0 stays allways on)
#define DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ   (0)	// Yank: default for hotplug up threshold frequency for all cpus (0 = disabled)
#define DEF_FREQUENCY_DOWN_THRESHOLD		  (52)
#define DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG	  (55)	// ZZ: default for hotplug down threshold for all cpus (cpu0 stays allways on)
#define DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ (0)	// Yank: default for hotplug down threshold frequency for all cpus (0 = disabled)
#define DEF_IGNORE_NICE				  (0)	// ZZ: default for ignore nice load
#define DEF_FREQ_STEP				  (5)	// ZZ: default for freq step at awake
#define DEF_FREQ_STEP_SLEEP			  (5)	// ZZ: default for freq step at early suspend

// ZZ: LCDFreq Scaling default values
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
#define LCD_FREQ_KICK_IN_DOWN_DELAY		(20)	// ZZ: default for kick in down delay
#define LCD_FREQ_KICK_IN_UP_DELAY		(50)	// ZZ: default for kick in up delay
#define LCD_FREQ_KICK_IN_FREQ			(500000)// ZZ: default kick in frequency
#define LCD_FREQ_KICK_IN_CORES			(0)	// ZZ: number of cores which should be online before kicking in
extern int _lcdfreq_lock(int lock);			// ZZ: external lcdfreq lock function
#endif

/*
 * The polling frequency of this governor depends on the capability of
 * the processor. Default polling frequency is 1000 times the transition
 * latency of the processor. The governor will work on any processor with
 * transition latency <= 10mS, using appropriate sampling
 * rate.
 * For CPUs with transition latency > 10mS (mostly drivers with CPUFREQ_ETERNAL)
 * this governor will not work.
 * All times here are in uS.
 */

#define MIN_SAMPLING_RATE_RATIO			(2)

// ZZ: Sampling down momentum variables
static unsigned int min_sampling_rate;			// ZZ: minimal possible sampling rate
static unsigned int orig_sampling_down_factor;		// ZZ: for saving previously set sampling down factor
static unsigned int orig_sampling_down_max_mom;		// ZZ: for saving previously set smapling down max momentum

// ZZ: search limit for frequencies in scaling table, variables for scaling modes and state flags for deadlock fix/suspend detection
static unsigned int max_scaling_freq_soft = 0;		// ZZ: init value for "soft" scaling to 0 = full range
static unsigned int max_scaling_freq_hard = 0;		// ZZ: init value for "hard" scaling to 0 = full range
static unsigned int limit_table_end = CPUFREQ_TABLE_END;// ZZ: end limit for frequency table in descending table
static unsigned int limit_table_start = 0;		// ZZ: start limit for frequency table in decending table
static unsigned int suspend_flag = 0;			// ZZ: init value for suspend status. 1 = in early suspend
static unsigned int skip_hotplug_flag = 1;		// ZZ: initial start without hotplugging to fix lockup issues
static int scaling_mode_up;				// ZZ: fast scaling up mode holding up value during runtime
static int scaling_mode_down;				// ZZ: fast scaling down mode holding down value during runtime

// ZZ: added hotplug idle threshold, block and down force cycles
#define DEF_HOTPLUG_BLOCK_CYCLES		(0)
#define DEF_HOTPLUG_IDLE_THRESHOLD		(0)
#define DEF_SCALING_BLOCK_THRESHOLD		(0)
#define DEF_SCALING_BLOCK_CYCLES		(0)
#define DEF_SCALING_BLOCK_FREQ		(0)
static unsigned int hotplug_idle_flag = 0;
static unsigned int enable_cores_on_exit = 0;
static unsigned int hotplug_down_block_cycles = 0;
static unsigned int hotplug_up_block_cycles = 0;
static unsigned int scaling_block_cycles_count = 0;

// ZZ: sampling rate idle
#define DEF_SAMPLING_RATE_IDLE_THRESHOLD	(0)
#define DEF_SAMPLING_RATE_IDLE			(0)
#define DEF_SAMPLING_RATE_IDLE_DELAY		(0)
static unsigned int sampling_rate_step_up_delay = 0;
static unsigned int sampling_rate_step_down_delay = 0;

// ZZ: current load & freq. for hotplugging work
static int cur_load = 0;
static int cur_freq = 0;

// ZZ: hotplug threshold array
static int hotplug_thresholds[2][8]={
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
    };

// ZZ: hotplug threshold frequencies array
static int hotplug_thresholds_freq[2][8]={
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
    };

/*
* ZZ: hotplug tuneable init flags for making an exception in tuneable value rules
* which will give apply-order-independence directly after start of the governor
* and will switch back to full check after first apply of values in that tuneables
* keep in mind with this workaround odd values are possible when you are using
* init.d scripts or using saved profiles of any tuning app
*/
static int hotplug_thresholds_tuneable[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// ZZ: support for 2,4 or 8 cores (this will enable/disable hotplug threshold tuneables)
#define MAX_CORES		(4)

// raise sampling rate to SR*multiplier and adjust sampling rate/thresholds/hotplug/scaling/freq limit/freq step on blank screen

// ZZ: LCDFreq scaling
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
static int lcdfreq_lock_current = 0;			// ZZ: LCDFreq scaling lock switch
static int prev_lcdfreq_lock_current;			// ZZ: for saving previously set lock state
static int prev_lcdfreq_enable;				// ZZ: for saving previously set enabled state
#endif
static unsigned int sampling_rate_awake;
static unsigned int up_threshold_awake;
static unsigned int down_threshold_awake;
static unsigned int smooth_up_awake;
static unsigned int freq_limit_awake;			// ZZ: for saving freqency limit awake value
static unsigned int fast_scaling_awake;			// ZZ: for saving fast scaling awake value
static unsigned int freq_step_awake;			// ZZ: for saving frequency step awake value
static unsigned int disable_hotplug_awake;		// ZZ: for daving hotplug switch
static unsigned int hotplug1_awake;			// ZZ: for saving hotplug1 threshold awake value
#if (MAX_CORES == 4 || MAX_CORES == 8)
static unsigned int hotplug2_awake;			// ZZ: for saving hotplug2 threshold awake value
static unsigned int hotplug3_awake;			// ZZ: for saving hotplug3 threshold awake value
#endif
#if (MAX_CORES == 8)
static unsigned int hotplug4_awake;			// ZZ: for saving hotplug4 threshold awake value
static unsigned int hotplug5_awake;			// ZZ: for saving hotplug5 threshold awake value
static unsigned int hotplug6_awake;			// ZZ: for saving hotplug6 threshold awake value
static unsigned int hotplug7_awake;			// ZZ: for saving hotplug7 threshold awake value
#endif
static unsigned int sampling_rate_asleep;		// ZZ: for setting sampling rate value on early suspend
static unsigned int up_threshold_asleep;		// ZZ: for setting up threshold value on early suspend
static unsigned int down_threshold_asleep;		// ZZ: for setting down threshold value on early suspend
static unsigned int smooth_up_asleep;			// ZZ: for setting smooth scaling value on early suspend
static unsigned int freq_limit_asleep;			// ZZ: for setting frequency limit value on early suspend
static unsigned int fast_scaling_asleep;		// ZZ: for setting fast scaling value on early suspend
static unsigned int freq_step_asleep;			// ZZ: for setting freq step value on early suspend
static unsigned int disable_hotplug_asleep;		// ZZ: for setting hotplug on/off on early suspend

// ZZ: midnight and zzmoove momentum defaults
#define LATENCY_MULTIPLIER			(1000)
#define MIN_LATENCY_MULTIPLIER			(100)
#define DEF_SAMPLING_DOWN_FACTOR		(1)	// ZZ: default for sampling down factor (stratosk default = 4) here disabled by default
#define MAX_SAMPLING_DOWN_FACTOR		(100000)// ZZ: changed from 10 to 100000 for sampling down momentum implementation
#define TRANSITION_LATENCY_LIMIT		(10 * 1000 * 1000)

// ZZ: Sampling down momentum
#define DEF_SAMPLING_DOWN_MOMENTUM		(0)	// ZZ: sampling down momentum disabled by default
#define DEF_SAMPLING_DOWN_MAX_MOMENTUM		(0)	// ZZ: default for tuneable sampling_down_max_momentum stratosk default=16, here disabled by default
#define DEF_SAMPLING_DOWN_MOMENTUM_SENSITIVITY  (50)	// ZZ: default for tuneable sampling_down_momentum_sensitivity
#define MAX_SAMPLING_DOWN_MOMENTUM_SENSITIVITY  (1000)	// ZZ: max value for tuneable sampling_down_momentum_sensitivity

// ZZ: midnight and zzmoove defaults for suspend
#define DEF_SAMPLING_RATE_SLEEP_MULTIPLIER	(2)	// ZZ: default for tuneable sampling_rate_sleep_multiplier
#define MAX_SAMPLING_RATE_SLEEP_MULTIPLIER	(4)	// ZZ: maximum for tuneable sampling_rate_sleep_multiplier
#define DEF_UP_THRESHOLD_SLEEP			(90)	// ZZ: default for tuneable up_threshold_sleep
#define DEF_DOWN_THRESHOLD_SLEEP		(44)	// ZZ: default for tuneable down_threshold_sleep
#define DEF_SMOOTH_UP_SLEEP			(100)	// ZZ: default for tuneable smooth_up_sleep

/*
* ZZ: Hotplug Sleep: 0 do not touch hotplug settings on early suspend, so that all cores will be online
* the value is equivalent to the amount of cores which should be online on early suspend
*/

#define DEF_HOTPLUG_SLEEP			(0)	// ZZ: default for tuneable hotplug_sleep
#define DEF_GRAD_UP_THRESHOLD			(25)	// ZZ: default for grad up threshold

/*
* ZZ: Frequency Limit: 0 do not limit frequency and use the full range up to policy->max limit
* values policy->min to policy->max in khz
*/

#define DEF_FREQ_LIMIT				(0)	// ZZ: default for tuneable freq_limit
#define DEF_FREQ_LIMIT_SLEEP			(0)	// ZZ: default for tuneable freq_limit_sleep

/*
* ZZ: Fast Scaling: 0 do not activate fast scaling function
* values 1-4 to enable fast scaling with normal down scaling 5-8 to enable fast scaling with fast up and down scaling
*/

#define DEF_FAST_SCALING			(0)	// ZZ: default for tuneable fast_scaling
#define DEF_FAST_SCALING_SLEEP			(0)	// ZZ: default for tuneable fast_scaling_sleep

struct work_struct hotplug_offline_work;
struct work_struct hotplug_online_work;

static void do_dbs_timer(struct work_struct *work);

struct cpu_dbs_info_s {
	u64 time_in_idle;				// ZZ: added exit time handling
	u64 idle_exit_time;				// ZZ: added exit time handling
	cputime64_t prev_cpu_idle;
	cputime64_t prev_cpu_wall;
	cputime64_t prev_cpu_nice;
	struct cpufreq_policy *cur_policy;
	struct delayed_work work;
	unsigned int down_skip;				// ZZ: Smapling down reactivated
	unsigned int check_cpu_skip;			// ZZ: check_cpu skip counter (to avoid potential deadlock because of double locks from hotplugging)
	unsigned int requested_freq;
	unsigned int rate_mult;				// ZZ: Sampling down momentum sampling rate multiplier
	unsigned int momentum_adder;			// ZZ: Sampling down momentum adder
	int cpu;
	unsigned int enable:1;
	unsigned int prev_load;				// ZZ: Early demand var for previous load
	/*
	 * percpu mutex that serializes governor limit change with
	 * do_dbs_timer invocation. We do not want do_dbs_timer to run
	 * when user is changing the governor or limits.
	 */
	struct mutex timer_mutex;
};
static DEFINE_PER_CPU(struct cpu_dbs_info_s, cs_cpu_dbs_info);

static unsigned int dbs_enable;	/* number of CPUs using this policy */

/*
 * dbs_mutex protects dbs_enable in governor start/stop.
 */
static DEFINE_MUTEX(dbs_mutex);

static struct dbs_tuners {
	char profile[20];				// ZZ: added profile name tuneable
	unsigned int profile_number;			// ZZ: added profile number tuneable
	unsigned int sampling_rate;
	unsigned int sampling_rate_current;		// ZZ: currently active sampling rate
	unsigned int sampling_rate_idle_threshold;	// ZZ: switching threshold
	unsigned int sampling_rate_idle;		// ZZ: sampling rate at idle
	unsigned int sampling_rate_idle_delay;		// ZZ: switching delay
	unsigned int sampling_rate_sleep_multiplier;	// ZZ: added tuneable sampling_rate_sleep_multiplier
	unsigned int sampling_down_factor;		// ZZ: Sampling down factor (reactivated)
	unsigned int sampling_down_momentum;		// ZZ: Sampling down momentum tuneable
	unsigned int sampling_down_max_mom;		// ZZ: Sampling down momentum max tuneable
	unsigned int sampling_down_mom_sens;		// ZZ: Sampling down momentum sensitivity
	unsigned int up_threshold;
	unsigned int up_threshold_hotplug1;		// ZZ: added tuneable up_threshold_hotplug1 for core1
	unsigned int up_threshold_hotplug_freq1;	// Yank: added tuneable up_threshold_hotplug_freq1 for core1
#if (MAX_CORES == 4 || MAX_CORES == 8)
	unsigned int up_threshold_hotplug2;		// ZZ: added tuneable up_threshold_hotplug2 for core2
	unsigned int up_threshold_hotplug_freq2;	// Yank: added tuneable up_threshold_hotplug_freq1 for core2
	unsigned int up_threshold_hotplug3;		// ZZ: added tuneable up_threshold_hotplug3 for core3
	unsigned int up_threshold_hotplug_freq3;	// Yank: added tuneable up_threshold_hotplug_freq1 for core3
#endif
#if (MAX_CORES == 8)
	unsigned int up_threshold_hotplug4;		// ZZ: added tuneable up_threshold_hotplug4 for core4
	unsigned int up_threshold_hotplug_freq4;	// Yank: added tuneable up_threshold_hotplug_freq1 for core4
	unsigned int up_threshold_hotplug5;		// ZZ: added tuneable up_threshold_hotplug5 for core5
	unsigned int up_threshold_hotplug_freq5;	// Yank: added tuneable up_threshold_hotplug_freq1 for core5
	unsigned int up_threshold_hotplug6;		// ZZ: added tuneable up_threshold_hotplug6 for core6
	unsigned int up_threshold_hotplug_freq6;	// Yank: added tuneable up_threshold_hotplug_freq1 for core6
	unsigned int up_threshold_hotplug7;		// ZZ: added tuneable up_threshold_hotplug7 for core7
	unsigned int up_threshold_hotplug_freq7;	// Yank: added tuneable up_threshold_hotplug_freq1 for core7
#endif
	unsigned int up_threshold_sleep;		// ZZ: added tuneable up_threshold_sleep for early suspend
	unsigned int down_threshold;
	unsigned int down_threshold_hotplug1;		// ZZ: added tuneable down_threshold_hotplug1 for core1
	unsigned int down_threshold_hotplug_freq1;	// Yank: added tuneable down_threshold_hotplug_freq1 for core1
#if (MAX_CORES == 4 || MAX_CORES == 8)
	unsigned int down_threshold_hotplug2;		// ZZ: added tuneable down_threshold_hotplug2 for core2
	unsigned int down_threshold_hotplug_freq2;	// Yank: added tuneable down_threshold_hotplug_freq1 for core2
	unsigned int down_threshold_hotplug3;		// ZZ: added tuneable down_threshold_hotplug3 for core3
	unsigned int down_threshold_hotplug_freq3;	// Yank: added tuneable down_threshold_hotplug_freq1 for core3
#endif
#if (MAX_CORES == 8)
	unsigned int down_threshold_hotplug4;		// ZZ: added tuneable down_threshold_hotplug4 for core4
	unsigned int down_threshold_hotplug_freq4;	// Yank: added tuneable down_threshold_hotplug_freq1 for core4
	unsigned int down_threshold_hotplug5;		// ZZ: added tuneable down_threshold_hotplug5 for core5
	unsigned int down_threshold_hotplug_freq5;	// Yank: added tuneable down_threshold_hotplug_freq1 for core5
	unsigned int down_threshold_hotplug6;		// ZZ: added tuneable down_threshold_hotplug6 for core6
	unsigned int down_threshold_hotplug_freq6;	// Yank: added tuneable down_threshold_hotplug_freq1 for core6
	unsigned int down_threshold_hotplug7;		// ZZ: added tuneable down_threshold_hotplug7 for core7
	unsigned int down_threshold_hotplug_freq7;	// Yank: added tuneable down_threshold_hotplug_freq1 for core7
#endif
	unsigned int down_threshold_sleep;		// ZZ: added tuneable down_threshold_sleep for early suspend
	unsigned int ignore_nice;
	unsigned int freq_step;
	unsigned int freq_step_sleep;			// ZZ: added tuneable freq_step_sleep for early suspend
	unsigned int smooth_up;
	unsigned int smooth_up_sleep;			// ZZ: added tuneable smooth_up_sleep for early suspend
	unsigned int hotplug_sleep;			// ZZ: added tuneable hotplug_sleep for early suspend
	unsigned int freq_limit;			// ZZ: added tuneable freq_limit
	unsigned int freq_limit_sleep;			// ZZ: added tuneable freq_limit_sleep
	unsigned int fast_scaling;			// ZZ: added tuneable fast_scaling
	unsigned int fast_scaling_sleep;		// ZZ: added tuneable fast_scaling_sleep
	unsigned int grad_up_threshold;			// ZZ: Early demand grad up threshold tuneable
	unsigned int early_demand;			// ZZ: Early demand master switch
	unsigned int disable_hotplug;			// ZZ: Hotplug switch
	unsigned int disable_hotplug_sleep;		// ZZ: Hotplug switch for sleep
	unsigned int hotplug_block_cycles;		// ZZ: Hotplug block cycles
	unsigned int hotplug_idle_threshold;		// ZZ: Hotplug idle threshold
	unsigned int scaling_block_threshold;		// ZZ: Scaling block threshold
	unsigned int scaling_block_cycles;		// ZZ: Scaling block cycles
	unsigned int scaling_block_freq;		// ZZ: Scaling block freq
#ifdef ENABLE_LEGACY_MODE
	unsigned int legacy_mode;			// ZZ: Legacy Mode
#endif
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
	int lcdfreq_enable;				// ZZ: LCDFreq Scaling switch
	unsigned int lcdfreq_kick_in_down_delay;	// ZZ: LCDFreq Scaling kick in down delay counter
	unsigned int lcdfreq_kick_in_down_left;		// ZZ: LCDFreq Scaling kick in down left counter
	unsigned int lcdfreq_kick_in_up_delay;		// ZZ: LCDFreq Scaling kick in up delay counter
	unsigned int lcdfreq_kick_in_up_left;		// ZZ: LCDFreq Scaling kick in up left counter
	unsigned int lcdfreq_kick_in_freq;		// ZZ: LCDFreq Scaling kick in frequency
	unsigned int lcdfreq_kick_in_cores;		// ZZ: LCDFreq Scaling kick in cores
#endif

} dbs_tuners_ins = {
	.profile = "none",								// ZZ: set default value for new tuneable
	.profile_number = 0,								// ZZ: set default value for new tuneable
	.up_threshold = DEF_FREQUENCY_UP_THRESHOLD,
	.up_threshold_hotplug1 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq1 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
#if (MAX_CORES == 4 || MAX_CORES == 8)
	.up_threshold_hotplug2 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq2 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
	.up_threshold_hotplug3 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq3 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
#endif
#if (MAX_CORES == 8)
	.up_threshold_hotplug4 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq4 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
	.up_threshold_hotplug5 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq5 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
	.up_threshold_hotplug6 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq6 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
	.up_threshold_hotplug7 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG,			// ZZ: set default value for new tuneable
	.up_threshold_hotplug_freq7 = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG_FREQ,		// Yank: set default value for new tuneable
#endif
	.up_threshold_sleep = DEF_UP_THRESHOLD_SLEEP,					// ZZ: set default value for new tuneable
	.down_threshold = DEF_FREQUENCY_DOWN_THRESHOLD,
	.down_threshold_hotplug1 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq1 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
#if (MAX_CORES == 4 || MAX_CORES == 8)
	.down_threshold_hotplug2 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq2 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
	.down_threshold_hotplug3 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq3 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
#endif
#if (MAX_CORES == 8)
	.down_threshold_hotplug4 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq4 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
	.down_threshold_hotplug5 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq5 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
	.down_threshold_hotplug6 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq6 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
	.down_threshold_hotplug7 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG,		// ZZ: set default value for new tuneable
	.down_threshold_hotplug_freq7 = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG_FREQ,	// Yank: set default value for new tuneable
#endif
	.down_threshold_sleep = DEF_DOWN_THRESHOLD_SLEEP,				// ZZ: set default value for new tuneable
	.sampling_down_factor = DEF_SAMPLING_DOWN_FACTOR,				// ZZ: sampling down reactivated but disabled by default
	.sampling_down_momentum = DEF_SAMPLING_DOWN_MOMENTUM,				// ZZ: Sampling down momentum initial disabled
	.sampling_down_max_mom = DEF_SAMPLING_DOWN_MAX_MOMENTUM,			// ZZ: Sampling down momentum default for max momentum
	.sampling_down_mom_sens = DEF_SAMPLING_DOWN_MOMENTUM_SENSITIVITY,		// ZZ: Sampling down momentum default for sensitivity
	.sampling_rate_sleep_multiplier = DEF_SAMPLING_RATE_SLEEP_MULTIPLIER,		// ZZ: set default value for new tuneable
	.sampling_rate_idle_threshold = DEF_SAMPLING_RATE_IDLE_THRESHOLD,		// ZZ: set default value for new tuneable
	.sampling_rate_idle = DEF_SAMPLING_RATE_IDLE,					// ZZ: set default value for new tuneable
	.sampling_rate_idle_delay = DEF_SAMPLING_RATE_IDLE_DELAY,			// ZZ: set default value for new tuneable
	.ignore_nice = DEF_IGNORE_NICE,							// ZZ: set default value for tuneable
	.freq_step = DEF_FREQ_STEP,							// ZZ: set default value for new tuneable
	.freq_step_sleep = DEF_FREQ_STEP_SLEEP,						// ZZ: set default value for new tuneable
	.smooth_up = DEF_SMOOTH_UP,
	.smooth_up_sleep = DEF_SMOOTH_UP_SLEEP,						// ZZ: set default value for new tuneable
	.hotplug_sleep = DEF_HOTPLUG_SLEEP,						// ZZ: set default value for new tuneable
	.freq_limit = DEF_FREQ_LIMIT,							// ZZ: set default value for new tuneable
	.freq_limit_sleep = DEF_FREQ_LIMIT_SLEEP,					// ZZ: set default value for new tuneable
	.fast_scaling = DEF_FAST_SCALING,						// ZZ: set default value for new tuneable
	.fast_scaling_sleep = DEF_FAST_SCALING_SLEEP,					// ZZ: set default value for new tuneable
	.grad_up_threshold = DEF_GRAD_UP_THRESHOLD,					// ZZ: Early demand default for grad up threshold
	.early_demand = 0,								// ZZ: Early demand default off
	.disable_hotplug = false,							// ZZ: Hotplug switch default off (=hotplugging on)
	.disable_hotplug_sleep = false,							// ZZ: Hotplug switch for sleep default off (=hotplugging on)
	.hotplug_block_cycles = DEF_HOTPLUG_BLOCK_CYCLES,				// ZZ: Hotplug block cycles default
	.hotplug_idle_threshold = DEF_HOTPLUG_IDLE_THRESHOLD,				// ZZ: Hotplug idle threshold default
	.scaling_block_threshold = DEF_SCALING_BLOCK_THRESHOLD,				// ZZ: Scaling block threshold default
	.scaling_block_cycles = DEF_SCALING_BLOCK_CYCLES,				// ZZ: Scaling block cycles default
	.scaling_block_freq = DEF_SCALING_BLOCK_FREQ,					// ZZ: Scaling block freq default
#ifdef ENABLE_LEGACY_MODE
	.legacy_mode = false,								// ZZ: Legacy Mode default off
#endif
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
	.lcdfreq_enable = false,							// ZZ: LCDFreq Scaling default off
	.lcdfreq_kick_in_down_delay = LCD_FREQ_KICK_IN_DOWN_DELAY,			// ZZ: LCDFreq Scaling default for down delay
	.lcdfreq_kick_in_down_left = LCD_FREQ_KICK_IN_DOWN_DELAY,			// ZZ: LCDFreq Scaling default for down left
	.lcdfreq_kick_in_up_delay = LCD_FREQ_KICK_IN_UP_DELAY,				// ZZ: LCDFreq Scaling default for up delay
	.lcdfreq_kick_in_up_left = LCD_FREQ_KICK_IN_UP_DELAY,				// ZZ: LCDFreq Scaling default for up left
	.lcdfreq_kick_in_freq = LCD_FREQ_KICK_IN_FREQ,					// ZZ: LCDFreq Scaling default for kick in freq
	.lcdfreq_kick_in_cores = LCD_FREQ_KICK_IN_CORES,				// ZZ: LCDFreq Scaling default for kick in cores
#endif
	};

unsigned int freq_table_size = 0;							// Yank : upper index limit of freq. table
unsigned int min_scaling_freq = 0;							// Yank : lowest frequency index in global frequency table
int freq_table_order = 1;								// Yank : 1 for descending order, -1 for ascending order

/**
 * Smooth scaling conservative governor (by Michael Weingaertner)
 * -----------------------------------------------------------------------
 * -------------- since zzmoove v0.7 only in Legacy Mode -----------------
 * This modification makes the governor use two lookup tables holding
 * current, next and previous frequency to directly get a correct
 * target frequency instead of calculating target frequencies with
 * up_threshold and step_up %. The two scaling lookup tables used
 * contain different scaling steps/frequencies to achieve faster upscaling
 * on higher CPU load.
 * -----------------------------------------------------------------------
 * -------------- since zzmoove v0.7 only in Legacy Mode -----------------
 * CPU load triggering faster upscaling can be adjusted via SYSFS,
 * VALUE between 1 and 100 (% CPU load):
 * echo VALUE > /sys/devices/system/cpu/cpufreq/zzmoove/smooth_up
 *
 * improved by Zane Zaminsky and Yank555 2012/13
 */

#define SCALE_FREQ_UP 1
#define SCALE_FREQ_DOWN 2

#ifdef ENABLE_LEGACY_MODE
// ZZ: Legacy Mode
#define LEG_FREQ 0
#endif

/*
 * Table modified for use with Samsung I9300 by ZaneZam November 2012
 * zzmoove v0.3		- table modified to reach overclocking frequencies up to 1600mhz
 * zzmoove v0.4		- added fast scaling columns to frequency table
 * zzmoove v0.5		- removed fast scaling colums and use line jumps instead. 4 steps and 2 modes (with/without fast downscaling) possible now
 *			  table modified to reach overclocking frequencies up to 1800mhz
 *			  fixed wrong frequency stepping
 *			  added search limit for more efficent frequency searching and better hard/softlimit handling
 * zzmoove v0.5.1b	- combination of power and normal scaling table to only one array (idea by Yank555)
 *			- scaling logic reworked and optimized by Yank555
 * zzmoove v0.6		- completely removed lookup tables and use the system frequency table instead
 *                        modified scaling logic accordingly (credits to Yank555)
 * zzmoove v0.6a	- added check if CPU freq. table is in ascending or descending order and scale accordingly (credits to Yank555)
 * zzmoove v0.7		- reindroduced the "scaling lookup table way" in form of the "Legacy Mode"
 * zzmoove v0.7b	- readded forgotten frequency search optimisation
 * zzmoove v0.7c	- frequency search optimisation now fully compatible with ascending ordered system frequency tables
 * zzmoove v0.8		- added scaling block cycles for a adjustable slowdown of up scaling (in normal and legacy mode)
 *			- added auto fast scaling mode (aka insane scale mode)
 *			- fixed stopping of up scaling at 100% load when up threshold tuneable is set to the max value of 100
 *			- fixed smooth up not active at 100% load when smooth up tuneable is set to the max value of 100
 */

#ifdef ENABLE_LEGACY_MODE
/*
 * ZZ: Legacy Mode:
 *     Frequency table from version 0.5 extended with fast scaling frequencies
 *     Scaling logic base taken from version 0.4 enhanced with some optimizations
*/

static int leg_freqs[17][7]={
    {1800000,1800000,1700000,1800000,1700000,1800000,1500000},
    {1700000,1800000,1600000,1800000,1600000,1800000,1400000},
    {1600000,1700000,1500000,1800000,1500000,1800000,1300000},
    {1500000,1600000,1400000,1700000,1400000,1600000,1200000},
    {1400000,1500000,1300000,1600000,1300000,1600000,1100000},
    {1300000,1400000,1200000,1500000,1200000,1400000,1000000},
    {1200000,1300000,1100000,1400000,1100000,1400000, 900000},
    {1100000,1200000,1000000,1300000,1000000,1200000, 800000},
    {1000000,1100000, 900000,1200000, 900000,1200000, 700000},
    { 900000,1000000, 800000,1100000, 800000,1000000, 600000},
    { 800000, 900000, 700000,1000000, 700000,1000000, 500000},
    { 700000, 800000, 600000, 900000, 600000, 800000, 400000},
    { 600000, 700000, 400000, 800000, 500000, 800000, 300000},
    { 500000, 600000, 300000, 700000, 400000, 600000, 200000},
    { 400000, 500000, 200000, 600000, 300000, 600000, 200000},
    { 300000, 400000, 200000, 500000, 200000, 400000, 200000},
    { 200000, 300000, 200000, 400000, 200000, 400000, 200000}
};

// ZZ: Legacy Mode scaling
static int leg_get_next_freq(unsigned int curfreq, unsigned int updown, unsigned int load) {
    int i=0;

if (load <= dbs_tuners_ins.smooth_up)
    {
	for(i = 0; i < 17 ; i++)
	{
	    if(unlikely(curfreq == leg_freqs[i][LEG_FREQ])) {
	    if(dbs_tuners_ins.fast_scaling != 0)
		return leg_freqs[i][updown+4]; // updown 5|6 - fast scaling colums
	    else
		return leg_freqs[i][updown];   // updown 1|2 - normal colums
	    }
	}
    }
    else
    {
	for(i = 0; i < 17; i++)
	{
	    if(unlikely(curfreq == leg_freqs[i][LEG_FREQ])){
	    if(dbs_tuners_ins.fast_scaling != 0)
		return leg_freqs[i][updown+4]; // updown 5|6 - fast scaling colums
	    else
		return leg_freqs[i][updown+2]; // updown 3|4 - power colums
	    }
	}
    }
return (curfreq); // not found
}
#endif

// Yank : Return a valid value between min and max
static int validate_min_max(int val, int min, int max) {

	return min(max(val, min), max);

}

static int mn_get_next_freq(unsigned int curfreq, unsigned int updown, unsigned int load) {

	int i=0;
	int smooth_up_steps=0;			// Yank : smooth_up steps
	struct cpufreq_frequency_table *table;	// Yank : Use system frequency table

	table = cpufreq_frequency_get_table(0);	// Yank : Get system frequency table

	if (load <= dbs_tuners_ins.smooth_up)	// Yank : Consider smooth up
		smooth_up_steps=0;		//          load not reached, move by one step
	else
		smooth_up_steps=1;		//          load reached, move by two steps

	for(i = limit_table_start; (table[i].frequency != limit_table_end); i++) { // ZZ: added forgotten max scaling search optimization again

		if(unlikely(curfreq == table[i].frequency)) {

			// Yank : We found where we currently are (i)
			if(updown == SCALE_FREQ_UP)

				return	min(	// Yank : Scale up, but don't go above softlimit
						table[max_scaling_freq_soft                                                                                 ].frequency,
						table[validate_min_max((i - 1 - smooth_up_steps - scaling_mode_up  ) * freq_table_order, 0, freq_table_size)].frequency
					);

			else

				return	max(	// Yank : Scale down, but don't go below min. freq.
						table[min_scaling_freq                                                                                      ].frequency,
						table[validate_min_max((i + 1                   + scaling_mode_down) * freq_table_order, 0, freq_table_size)].frequency
					);

			return (curfreq);	// Yank : We should never get here...

		}

	}

	return (curfreq); // not found

}

// ZZ: function for enabling cores from offline state
static inline void enable_offline_cores(void)
{
	int i = 0;
	// ZZ: enable all offline cores
	for (i = 1; i < num_possible_cpus(); i++) {
		if (!cpu_online(i))
		cpu_up(i);
	}
}

static inline cputime64_t get_cpu_idle_time_jiffy(unsigned int cpu,
							cputime64_t *wall)
{
	cputime64_t idle_time;
	cputime64_t cur_wall_time;
	cputime64_t busy_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());
	busy_time = cputime64_add(kstat_cpu(cpu).cpustat.user,
			kstat_cpu(cpu).cpustat.system);

	busy_time = cputime64_add(busy_time, kstat_cpu(cpu).cpustat.irq);
	busy_time = cputime64_add(busy_time, kstat_cpu(cpu).cpustat.softirq);
	busy_time = cputime64_add(busy_time, kstat_cpu(cpu).cpustat.steal);
	busy_time = cputime64_add(busy_time, kstat_cpu(cpu).cpustat.nice);

	idle_time = cputime64_sub(cur_wall_time, busy_time);
	if (wall)
		*wall = (cputime64_t)jiffies_to_usecs(cur_wall_time);

	return (cputime64_t)jiffies_to_usecs(idle_time);
}

static inline cputime64_t get_cpu_idle_time(unsigned int cpu, cputime64_t *wall)
{
	u64 idle_time = get_cpu_idle_time_us(cpu, wall);

	if (idle_time == -1ULL)
		return get_cpu_idle_time_jiffy(cpu, wall);

	return idle_time;
}

/* keep track of frequency transitions */
static int
dbs_cpufreq_notifier(struct notifier_block *nb, unsigned long val,
		     void *data)
{
	struct cpufreq_freqs *freq = data;
	struct cpu_dbs_info_s *this_dbs_info = &per_cpu(cs_cpu_dbs_info,
							freq->cpu);

	struct cpufreq_policy *policy;

	if (!this_dbs_info->enable)
		return 0;

	policy = this_dbs_info->cur_policy;

	/*
	 * we only care if our internally tracked freq moves outside
	 * the 'valid' ranges of frequency available to us otherwise
	 * we do not change it
	*/
	if (this_dbs_info->requested_freq > policy->max
			|| this_dbs_info->requested_freq < policy->min)
		this_dbs_info->requested_freq = freq->new;

	return 0;
}

static struct notifier_block dbs_cpufreq_notifier_block = {
	.notifier_call = dbs_cpufreq_notifier
};

/************************** sysfs interface ************************/
static ssize_t show_sampling_rate_min(struct kobject *kobj,
				      struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", min_sampling_rate);
}

define_one_global_ro(sampling_rate_min);

/* cpufreq_zzmoove Governor Tunables */
#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return sprintf(buf, "%u\n", dbs_tuners_ins.object);		\
}
show_one(profile_number, profile_number);					// ZZ: added profile number tuneable
show_one(sampling_rate, sampling_rate);						// ZZ: normal sampling rate
show_one(sampling_rate_current, sampling_rate_current);				// ZZ: added tuneable for showing the active sampling rate
show_one(sampling_rate_idle_threshold, sampling_rate_idle_threshold);		// ZZ: added sampling rate idle threshold
show_one(sampling_rate_idle, sampling_rate_idle);				// ZZ: added sampling rate at idle
show_one(sampling_rate_idle_delay, sampling_rate_idle_delay);			// ZZ: added DSR switching delay
show_one(sampling_rate_sleep_multiplier, sampling_rate_sleep_multiplier);	// ZZ: added sampling_rate_sleep_multiplier tuneable for early suspend
show_one(sampling_down_factor, sampling_down_factor);				// ZZ: reactivated sampling down factor
show_one(sampling_down_max_momentum, sampling_down_max_mom);			// ZZ: added Sampling down momentum tuneable
show_one(sampling_down_momentum_sensitivity, sampling_down_mom_sens);		// ZZ: added Sampling down momentum tuneable
show_one(up_threshold, up_threshold);
show_one(up_threshold_sleep, up_threshold_sleep);				// ZZ: added up_threshold_sleep tuneable for early suspend
show_one(up_threshold_hotplug1, up_threshold_hotplug1);				// ZZ: added up_threshold_hotplug1 tuneable for cpu1
show_one(up_threshold_hotplug_freq1, up_threshold_hotplug_freq1);		// Yank: added up_threshold_hotplug_freq1 tuneable for cpu1
#if (MAX_CORES == 4 || MAX_CORES == 8)
show_one(up_threshold_hotplug2, up_threshold_hotplug2);				// ZZ: added up_threshold_hotplug2 tuneable for cpu2
show_one(up_threshold_hotplug_freq2, up_threshold_hotplug_freq2);		// Yank: added up_threshold_hotplug_freq2 tuneable for cpu2
show_one(up_threshold_hotplug3, up_threshold_hotplug3);				// ZZ: added up_threshold_hotplug3 tuneable for cpu3
show_one(up_threshold_hotplug_freq3, up_threshold_hotplug_freq3);		// Yank: added up_threshold_hotplug_freq3 tuneable for cpu3
#endif
#if (MAX_CORES == 8)
show_one(up_threshold_hotplug4, up_threshold_hotplug4);				// ZZ: added up_threshold_hotplug4 tuneable for cpu4
show_one(up_threshold_hotplug_freq4, up_threshold_hotplug_freq4);		// Yank: added up_threshold_hotplug_freq4 tuneable for cpu4
show_one(up_threshold_hotplug5, up_threshold_hotplug5);				// ZZ: added up_threshold_hotplug5 tuneable for cpu5
show_one(up_threshold_hotplug_freq5, up_threshold_hotplug_freq5);		// Yank: added up_threshold_hotplug_freq5 tuneable for cpu5
show_one(up_threshold_hotplug6, up_threshold_hotplug6);				// ZZ: added up_threshold_hotplug6 tuneable for cpu6
show_one(up_threshold_hotplug_freq6, up_threshold_hotplug_freq6);		// Yank: added up_threshold_hotplug_freq6 tuneable for cpu6
show_one(up_threshold_hotplug7, up_threshold_hotplug7);				// ZZ: added up_threshold_hotplug7 tuneable for cpu7
show_one(up_threshold_hotplug_freq7, up_threshold_hotplug_freq7);		// Yank: added up_threshold_hotplug_freq7 tuneable for cpu7
#endif
show_one(down_threshold, down_threshold);
show_one(down_threshold_sleep, down_threshold_sleep);				// ZZ: added down_threshold_sleep tuneable for early suspend
show_one(down_threshold_hotplug1, down_threshold_hotplug1);			// ZZ: added down_threshold_hotplug1 tuneable for cpu1
show_one(down_threshold_hotplug_freq1, down_threshold_hotplug_freq1);		// Yank: added down_threshold_hotplug_freq1 tuneable for cpu1
#if (MAX_CORES == 4 || MAX_CORES == 8)
show_one(down_threshold_hotplug2, down_threshold_hotplug2);			// ZZ: added down_threshold_hotplug2 tuneable for cpu2
show_one(down_threshold_hotplug_freq2, down_threshold_hotplug_freq2);		// Yank: added down_threshold_hotplug_freq2 tuneable for cpu2
show_one(down_threshold_hotplug3, down_threshold_hotplug3);			// ZZ: added down_threshold_hotplug3 tuneable for cpu3
show_one(down_threshold_hotplug_freq3, down_threshold_hotplug_freq3);		// Yank: added down_threshold_hotplug_freq3 tuneable for cpu3
#endif
#if (MAX_CORES == 8)
show_one(down_threshold_hotplug4, down_threshold_hotplug4);			// ZZ: added down_threshold_hotplug4 tuneable for cpu4
show_one(down_threshold_hotplug_freq4, down_threshold_hotplug_freq4);		// Yank: added down_threshold_hotplug_freq4 tuneable for cpu4
show_one(down_threshold_hotplug5, down_threshold_hotplug5);			// ZZ: added down_threshold_hotplug5 tuneable for cpu5
show_one(down_threshold_hotplug_freq5, down_threshold_hotplug_freq5);		// Yank: added down_threshold_hotplug_freq5 tuneable for cpu5
show_one(down_threshold_hotplug6, down_threshold_hotplug6);			// ZZ: added down_threshold_hotplug6 tuneable for cpu6
show_one(down_threshold_hotplug_freq6, down_threshold_hotplug_freq6);		// Yank: added down_threshold_hotplug_freq6 tuneable for cpu6
show_one(down_threshold_hotplug7, down_threshold_hotplug7);			// ZZ: added down_threshold_hotplug7 tuneable for cpu7
show_one(down_threshold_hotplug_freq7, down_threshold_hotplug_freq7);		// Yank: added down_threshold_hotplug_freq7 tuneable for cpu7
#endif
show_one(ignore_nice_load, ignore_nice);
show_one(freq_step, freq_step);
show_one(freq_step_sleep, freq_step_sleep);					// ZZ: added freq_step_sleep tuneable for early suspend
show_one(smooth_up, smooth_up);
show_one(smooth_up_sleep, smooth_up_sleep);					// ZZ: added smooth_up_sleep tuneable for early suspend
show_one(hotplug_sleep, hotplug_sleep);						// ZZ: added hotplug_sleep tuneable for early suspend
show_one(freq_limit, freq_limit);						// ZZ: added freq_limit tuneable
show_one(freq_limit_sleep, freq_limit_sleep);					// ZZ: added freq_limit_sleep tuneable for early suspend
show_one(fast_scaling, fast_scaling);						// ZZ: added fast_scaling tuneable
show_one(fast_scaling_sleep, fast_scaling_sleep);				// ZZ: added fast_scaling_sleep tuneable for early suspend
show_one(grad_up_threshold, grad_up_threshold);					// ZZ: added Early demand tuneable grad up threshold
show_one(early_demand, early_demand);						// ZZ: added Early demand tuneable master switch
show_one(disable_hotplug, disable_hotplug);					// ZZ: added Hotplug switch
show_one(disable_hotplug_sleep, disable_hotplug_sleep);				// ZZ: added Hotplug switch for sleep
show_one(hotplug_block_cycles, hotplug_block_cycles);				// ZZ: added Hotplug block cycles
show_one(hotplug_idle_threshold, hotplug_idle_threshold);			// ZZ: added Hotplug idle threshold
show_one(scaling_block_threshold, scaling_block_threshold);			// ZZ: added Scaling block threshold
show_one(scaling_block_cycles, scaling_block_cycles);				// ZZ: added Scaling block cycles
show_one(scaling_block_freq, scaling_block_freq);				// ZZ: added Scaling block freq
#ifdef ENABLE_LEGACY_MODE
show_one(legacy_mode, legacy_mode);						// ZZ: Legacy Mode switch
#endif
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
show_one(lcdfreq_enable, lcdfreq_enable);					// ZZ: added LCDFreq Scaling tuneable master switch
show_one(lcdfreq_kick_in_down_delay, lcdfreq_kick_in_down_delay);		// ZZ: added LCDFreq Scaling tuneable kick in down delay
show_one(lcdfreq_kick_in_up_delay, lcdfreq_kick_in_up_delay);			// ZZ: added LCDFreq Scaling tuneable kick in up delay
show_one(lcdfreq_kick_in_freq, lcdfreq_kick_in_freq);				// ZZ: added LCDFreq Scaling tuneable kick in freq
show_one(lcdfreq_kick_in_cores, lcdfreq_kick_in_cores);				// ZZ: added LCDFreq Scaling tuneable kick in cores
#endif

// ZZ:  added tuneable for showing the active profile
static ssize_t show_profile(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", dbs_tuners_ins.profile);
}

// ZZ: added tuneable for Sampling down momentum -> possible values: 0 (disable) to MAX_SAMPLING_DOWN_FACTOR, if not set default is 0
static ssize_t store_sampling_down_max_momentum(struct kobject *a,
		struct attribute *b, const char *buf, size_t count)
{
	unsigned int input, j;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_DOWN_FACTOR -
	dbs_tuners_ins.sampling_down_factor || input < 0)
	return -EINVAL;

	dbs_tuners_ins.sampling_down_max_mom = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	orig_sampling_down_max_mom = dbs_tuners_ins.sampling_down_max_mom;

	/* ZZ: Reset sampling down factor to default if momentum was disabled */
	if (dbs_tuners_ins.sampling_down_max_mom == 0)
	dbs_tuners_ins.sampling_down_factor = DEF_SAMPLING_DOWN_FACTOR;

	/* Reset momentum_adder and reset down sampling multiplier in case momentum was disabled */
	for_each_online_cpu(j) {
	    struct cpu_dbs_info_s *dbs_info;
	    dbs_info = &per_cpu(cs_cpu_dbs_info, j);
	    dbs_info->momentum_adder = 0;
	    if (dbs_tuners_ins.sampling_down_max_mom == 0)
	    dbs_info->rate_mult = 1;
	}

return count;
}

// ZZ: added tuneable for Sampling down momentum -> possible values: 1 to MAX_SAMPLING_DOWN_SENSITIVITY, if not set default is 50
static ssize_t store_sampling_down_momentum_sensitivity(struct kobject *a,
			struct attribute *b, const char *buf, size_t count)
{
	unsigned int input, j;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_DOWN_MOMENTUM_SENSITIVITY || input < 1)
	return -EINVAL;

	dbs_tuners_ins.sampling_down_mom_sens = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	/* Reset momentum_adder */
	for_each_online_cpu(j) {
	    struct cpu_dbs_info_s *dbs_info;
	    dbs_info = &per_cpu(cs_cpu_dbs_info, j);
	    dbs_info->momentum_adder = 0;
	}

return count;
}

// ZZ: Sampling down factor (reactivated) added reset loop for momentum functionality -> possible values: 1 (disabled) to MAX_SAMPLING_DOWN_FACTOR, if not set default is 1
static ssize_t store_sampling_down_factor(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input, j;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_DOWN_FACTOR || input < 1)
		return -EINVAL;

	dbs_tuners_ins.sampling_down_factor = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	/* ZZ: Reset down sampling multiplier in case it was active */
	for_each_online_cpu(j) {
	    struct cpu_dbs_info_s *dbs_info;
	    dbs_info = &per_cpu(cs_cpu_dbs_info, j);
	    dbs_info->rate_mult = 1;
	}

	return count;
}

static ssize_t store_sampling_rate(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	unsigned int tempsave_idle_rate;
	int ret;
	ret = sscanf(buf, "%u", &input);
	
	if (ret != 1)
		return -EINVAL;

	if(dbs_tuners_ins.sampling_rate_idle != 0) {
	    tempsave_idle_rate = dbs_tuners_ins.sampling_rate_idle; // ZZ: temporary save idle rate
	    dbs_tuners_ins.sampling_rate_idle = 0; // ZZ: disable dynamic sample rate during setting normal rate
	    dbs_tuners_ins.sampling_rate = dbs_tuners_ins.sampling_rate_current = max(input, min_sampling_rate); // ZZ: set it to new value

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

	    dbs_tuners_ins.sampling_rate_idle = tempsave_idle_rate; // ZZ: restore idle rate
	} else {
	    dbs_tuners_ins.sampling_rate = dbs_tuners_ins.sampling_rate_current = max(input, min_sampling_rate); // ZZ: set it to new value
	}
	return count;
}

// ZZ: sampling rate for idle times
static ssize_t store_sampling_rate_idle(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input == 0) {
	    dbs_tuners_ins.sampling_rate_current = dbs_tuners_ins.sampling_rate; // ZZ: reset normal rate in case it was on idle rate
	    dbs_tuners_ins.sampling_rate_idle = input; // ZZ: set it to 0

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

	} else {
	    dbs_tuners_ins.sampling_rate_idle = max(input, min_sampling_rate); // ZZ: or validate the input as normal
		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}
	}
	return count;
}

static ssize_t store_sampling_rate_idle_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 100)
		return -EINVAL;

	dbs_tuners_ins.sampling_rate_idle_threshold = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable sampling rate idle delay -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_sampling_rate_idle_delay(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (input < 0)
	return -EINVAL;

	if (input == 0)
	sampling_rate_step_up_delay = 0;
	sampling_rate_step_down_delay = 0;

	dbs_tuners_ins.sampling_rate_idle_delay = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable -> possible values: 1 to 4, if not set default is 2
static ssize_t store_sampling_rate_sleep_multiplier(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_RATE_SLEEP_MULTIPLIER || input < 1)
		return -EINVAL;

	dbs_tuners_ins.sampling_rate_sleep_multiplier = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

static ssize_t store_up_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 100 ||
			input <= dbs_tuners_ins.down_threshold)
		return -EINVAL;

	dbs_tuners_ins.up_threshold = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneble -> possible values: range from above down_threshold_sleep value up to 100, if not set default is 90
static ssize_t store_up_threshold_sleep(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 100 ||
			input <= dbs_tuners_ins.down_threshold_sleep)
		return -EINVAL;

	dbs_tuners_ins.up_threshold_sleep = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// Yank : also use definitions for other hotplug tunables
#define store_up_threshold_hotplug(name,core)							\
static ssize_t store_up_threshold_hotplug##name							\
(struct kobject *a, struct attribute *b, const char *buf, size_t count)				\
{												\
	unsigned int input;									\
	int ret;										\
	ret = sscanf(buf, "%u", &input);							\
												\
	if (hotplug_thresholds_tuneable[core] == 0) {						\
												\
	if (ret != 1 || input > 100								\
	|| (input <= dbs_tuners_ins.down_threshold_hotplug##name && input != 0))		\
		return -EINVAL;									\
												\
	    dbs_tuners_ins.up_threshold_hotplug##name = input;					\
	    hotplug_thresholds[0][core] = input;						\
												\
	    if (dbs_tuners_ins.profile_number != 0) {						\
		dbs_tuners_ins.profile_number = 0;						\
		strncpy(dbs_tuners_ins.profile, custom_profile, 				\
		sizeof(dbs_tuners_ins.profile));						\
	    }											\
												\
	} else {										\
												\
	    if (ret != 1 || input < 0 || input > 100)						\
		return -EINVAL;									\
												\
	    dbs_tuners_ins.up_threshold_hotplug##name = input;					\
	    hotplug_thresholds[0][core] = input;						\
												\
	    if (dbs_tuners_ins.profile_number != 0) {						\
		dbs_tuners_ins.profile_number = 0;						\
		strncpy(dbs_tuners_ins.profile, custom_profile, 				\
		sizeof(dbs_tuners_ins.profile));						\
	    }											\
												\
	    hotplug_thresholds_tuneable[core] = 0;						\
	}											\
	return count;										\
}

#define store_down_threshold_hotplug(name,core)							\
static ssize_t store_down_threshold_hotplug##name						\
(struct kobject *a, struct attribute *b, const char *buf, size_t count)				\
{												\
	unsigned int input;									\
	int ret;										\
	ret = sscanf(buf, "%u", &input);							\
												\
	if (hotplug_thresholds_tuneable[core] == 0) {						\
												\
	if (ret != 1 || input < 1 || input > 100						\
	|| input >= dbs_tuners_ins.up_threshold_hotplug##name)					\
		return -EINVAL;									\
												\
	    dbs_tuners_ins.down_threshold_hotplug##name = input;				\
	    hotplug_thresholds[1][core] = input;						\
												\
	    if (dbs_tuners_ins.profile_number != 0) {						\
		dbs_tuners_ins.profile_number = 0;						\
		strncpy(dbs_tuners_ins.profile, custom_profile, 				\
		sizeof(dbs_tuners_ins.profile));						\
	    }											\
												\
	} else {										\
												\
	    if (ret != 1 || input < 1 || input > 100)						\
		return -EINVAL;									\
	    dbs_tuners_ins.down_threshold_hotplug##name = input;				\
	    hotplug_thresholds[1][core] = input;						\
												\
	    if (dbs_tuners_ins.profile_number != 0) {						\
		dbs_tuners_ins.profile_number = 0;						\
		strncpy(dbs_tuners_ins.profile, custom_profile, 				\
		sizeof(dbs_tuners_ins.profile));						\
	    }											\
												\
	    hotplug_thresholds_tuneable[core] = 0;						\
	}											\
	return count;										\
}

// ZZ: added tuneable -> possible values: 0 to disable core, range from down_threshold up to 100, if not set default is 68
store_up_threshold_hotplug(1,0);
store_down_threshold_hotplug(1,0);
#if (MAX_CORES == 4 || MAX_CORES == 8)
store_up_threshold_hotplug(2,1);
store_down_threshold_hotplug(2,1);
store_up_threshold_hotplug(3,2);
store_down_threshold_hotplug(3,2);
#endif
#if (MAX_CORES == 8)
store_up_threshold_hotplug(4,3);
store_down_threshold_hotplug(4,3);
store_up_threshold_hotplug(5,4);
store_down_threshold_hotplug(5,4);
store_up_threshold_hotplug(6,5);
store_down_threshold_hotplug(6,5);
store_up_threshold_hotplug(7,6);
store_down_threshold_hotplug(7,6);
#endif

static ssize_t store_down_threshold(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	/* cannot be lower than 11 otherwise freq will not fall */
	if (ret != 1 || input < 11 || input > 100 ||
			input >= dbs_tuners_ins.up_threshold)
		return -EINVAL;

	dbs_tuners_ins.down_threshold = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable -> possible values: range from 11 to up_threshold_sleep but not up_threshold_sleep, if not set default is 44
static ssize_t store_down_threshold_sleep(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	/* cannot be lower than 11 otherwise freq will not fall */
	if (ret != 1 || input < 11 || input > 100 ||
			input >= dbs_tuners_ins.up_threshold_sleep)
		return -EINVAL;

	dbs_tuners_ins.down_threshold_sleep = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

static ssize_t store_ignore_nice_load(struct kobject *a, struct attribute *b,
				      const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	unsigned int j;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	if (input > 1)
		input = 1;

	if (input == dbs_tuners_ins.ignore_nice) { /* nothing to do */
		return count;
	}

	dbs_tuners_ins.ignore_nice = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	/* we need to re-evaluate prev_cpu_idle */
	for_each_online_cpu(j) {
		struct cpu_dbs_info_s *dbs_info;
		dbs_info = &per_cpu(cs_cpu_dbs_info, j);
		dbs_info->prev_cpu_idle = get_cpu_idle_time(j,
		&dbs_info->prev_cpu_wall);
		if (dbs_tuners_ins.ignore_nice)
			dbs_info->prev_cpu_nice = kstat_cpu(j).cpustat.nice;

	}
	return count;
}

static ssize_t store_freq_step(struct kobject *a, struct attribute *b,
			       const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input > 100)
		input = 100;

	/* no need to test here if freq_step is zero as the user might actually
	 * want this, they would be crazy though :) */
	dbs_tuners_ins.freq_step = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

/*
 * ZZ: added tuneable -> possible values: range from 0 to 100, if not set default is 5 -> value 0 will stop freq scaling and hold on actual freq
 * value 100 will directly jump up/down to limits like ondemand governor
 */
static ssize_t store_freq_step_sleep(struct kobject *a, struct attribute *b,
			       const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input > 100)
		input = 100;

	/* no need to test here if freq_step is zero as the user might actually
	 * want this, they would be crazy though :) */
	dbs_tuners_ins.freq_step_sleep = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

static ssize_t store_smooth_up(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 100 || input < 1)
		return -EINVAL;

	dbs_tuners_ins.smooth_up = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable -> possible values: range from 1 to 100, if not set default is 100
static ssize_t store_smooth_up_sleep(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 100 || input < 1)
		return -EINVAL;

	dbs_tuners_ins.smooth_up_sleep = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

/*
 * ZZ: added tuneable -> possible values: 0 do not touch the hotplug values on early suspend,
 * input value 1 to MAX_CORES - 1 equals cores to run at early suspend, if not set default is 0 (=all cores enabled)
 */
static ssize_t store_hotplug_sleep(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input >= num_possible_cpus() || (input < 0 && input != 0))
	return -EINVAL;

	dbs_tuners_ins.hotplug_sleep = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable -> possible values: 0 disable, system table freq->min to freq->max in khz -> freqency soft-limit, if not set default is 0
// Yank: updated : possible values now depend on the system frequency table only
static ssize_t store_freq_limit(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	struct cpufreq_frequency_table *table;	// Yank : Use system frequency table
	int ret;
	int i=0;

	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	table = cpufreq_frequency_get_table(0);	// Yank : Get system frequency table

	if (!table)
		return -EINVAL;

	if (input == 0) {
	     max_scaling_freq_soft = max_scaling_freq_hard;
		if (freq_table_order == 1)			// ZZ: if descending ordered table is used
		    limit_table_start = max_scaling_freq_soft;	// ZZ: we should use the actual scaling soft limit value as search start point
		else
		    limit_table_end = table[freq_table_size].frequency; // ZZ: set search end point to max freq when using ascending table
	    	    dbs_tuners_ins.freq_limit = input;

		    // ZZ: set profile number to custom mode
		    if (dbs_tuners_ins.profile_number != 0) {
			dbs_tuners_ins.profile_number = 0;
			strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		    }

	     return count;
	}

	if (input > table[max_scaling_freq_hard].frequency) {	 // Yank : Allow only frequencies below or equal to hard max limit
		return -EINVAL;
	} else {
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
			if (table[i].frequency == input) {
				max_scaling_freq_soft = i;
				if (freq_table_order == 1)			// ZZ: if descending ordered table is used
				    limit_table_start = max_scaling_freq_soft;	// ZZ: we should use the actual scaling soft limit value as search start point
				else
				    limit_table_end = table[i].frequency;	// ZZ: set search end point to max soft freq limit when using ascenting table
				    dbs_tuners_ins.freq_limit = input;

				    // ZZ: set profile number to custom mode
				    if (dbs_tuners_ins.profile_number != 0) {
					dbs_tuners_ins.profile_number = 0;
					strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
				    }

				return count;
			}
	}

	return -EINVAL;
}

// ZZ: added tuneable -> possible values: 0 disable, system table freq->min to freq->max in khz -> freqency soft-limit, if not set default is 0
// Yank: updated : possible values now depend on the system frequency table only
static ssize_t store_freq_limit_sleep(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	struct cpufreq_frequency_table *table;	// Yank : Use system frequency table
	int ret;
	int i=0;

	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input == 0) {
	     dbs_tuners_ins.freq_limit_sleep = input;

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

	     return count;
	}

	table = cpufreq_frequency_get_table(0);	// Yank : Get system frequency table

	if (!table) {
		return -EINVAL;
	} else if (input > table[max_scaling_freq_hard].frequency) {	 // Yank : Allow only frequencies below or equal to hard max
		return -EINVAL;
	} else {
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
			if (table[i].frequency == input) {
			    dbs_tuners_ins.freq_limit_sleep = input;

			    // ZZ: set profile number to custom mode
			    if (dbs_tuners_ins.profile_number != 0) {
				dbs_tuners_ins.profile_number = 0;
			        strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
			    }

			    return count;
			}
	}

	return -EINVAL;
}

// ZZ: added tuneable -> possible values: 0 disable, 1-4 number of scaling jumps only for up scaling, 5-8 is equivalent to 1-4 for up and down scaling, 
//     9-12 is equivalent to 1-4 for fast down scaling only and normal up scaling, if not set default is 0
static ssize_t store_fast_scaling(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 13 || input < 0)
		return -EINVAL;

	dbs_tuners_ins.fast_scaling = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}
	
	if (input > 12) // ZZ: auto fast scaling mode
	    return count;

	if (input > 8) {
	    scaling_mode_up   = 0;
	    scaling_mode_down = input - 8;	// ZZ: fast scaling down only

	} else if (input > 4) {
	    scaling_mode_up   = input - 4;	// Yank : fast scaling up
	    scaling_mode_down = input - 4;	// Yank : fast scaling down
	
	} else {
	    scaling_mode_up   = input;		// Yank : fast scaling up only
	    scaling_mode_down = 0;
	}
	
	return count;
}

// ZZ: added tuneable -> possible values: 0 disable, 1-4 number of scaling jumps only for up scaling, 5-8 is equivalent to 1-4 for up and down scaling, 
//     9-12 is equivalent to 1-4 for fast down scaling only and normal up scaling, if not set default is 0
static ssize_t store_fast_scaling_sleep(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 12 || input < 0)
		return -EINVAL;

	dbs_tuners_ins.fast_scaling_sleep = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: LCDFreq Scaling - added tuneable (Master Switch) -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
static ssize_t store_lcdfreq_enable(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
	return -EINVAL;

	if (input > 0) {
		dbs_tuners_ins.lcdfreq_enable = true;

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

	} else {
		dbs_tuners_ins.lcdfreq_enable = false;

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

		// Set screen to 60Hz when stopping to switch
		lcdfreq_lock_current = 0;
		_lcdfreq_lock(lcdfreq_lock_current);
	}
	return count;
}

// ZZ: LCDFreq Scaling - added tuneable (Down Delay) -> number of samples to wait till switching to 40hz, possible range not specified, if not set default is 20
static ssize_t store_lcdfreq_kick_in_down_delay(struct kobject *a, struct attribute *b,
							const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1 && input < 0)
	return -EINVAL;

	dbs_tuners_ins.lcdfreq_kick_in_down_delay = input;
	dbs_tuners_ins.lcdfreq_kick_in_down_left =
	dbs_tuners_ins.lcdfreq_kick_in_down_delay;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: LCDFreq Scaling - added tuneable (Up Delay) -> number of samples to wait till switching to 40hz, possible range not specified, if not set default is 50
static ssize_t store_lcdfreq_kick_in_up_delay(struct kobject *a, struct attribute *b,
							const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1 && input < 0)
	return -EINVAL;

	dbs_tuners_ins.lcdfreq_kick_in_up_delay = input;
	dbs_tuners_ins.lcdfreq_kick_in_up_left =
	dbs_tuners_ins.lcdfreq_kick_in_up_delay;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: LCDFreq Scaling - added tuneable (Frequency Threshold) -> frequency from where to start switching LCDFreq, possible values are all valid frequencies up to actual scaling limit, if not set default is 500000
// Yank: updated : possible values now depend on the system frequency table only
static ssize_t store_lcdfreq_kick_in_freq(struct kobject *a, struct attribute *b,
						    const char *buf, size_t count)
{
	unsigned int input;
	struct cpufreq_frequency_table *table;	// Yank : Use system frequency table
	int ret;
	int i=0;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	table = cpufreq_frequency_get_table(0);	// Yank : Get system frequency table

	if (!table) {
		return -EINVAL;
	} else if (input > table[max_scaling_freq_hard].frequency) {	 // Yank : Allow only frequencies below or equal to hard max
		return -EINVAL;
	} else {
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
			if (table[i].frequency == input) {
				dbs_tuners_ins.lcdfreq_kick_in_freq = input;

				// ZZ: set profile number to custom mode
				    if (dbs_tuners_ins.profile_number != 0) {
					dbs_tuners_ins.profile_number = 0;
					strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
				    }

				return count;
			}
	}

	return -EINVAL;
}

// ZZ: LCDFreq Scaling - added tuneable (Online Cores Threshold) -> amount of cores which have to be online before switching LCDFreq (useable in combination with Freq Threshold), possible values 0-4, if not set default is 0
static ssize_t store_lcdfreq_kick_in_cores(struct kobject *a, struct attribute *b,
						    const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > num_possible_cpus() || (input < 0 && input != 0))
	return -EINVAL;

	dbs_tuners_ins.lcdfreq_kick_in_cores = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}
#endif

// ZZ: Early demand - added tuneable grad up threshold -> possible values: from 11 to 100, if not set default is 50
static ssize_t store_grad_up_threshold(struct kobject *a, struct attribute *b,
						const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 100 || input < 11)
	return -EINVAL;

	dbs_tuners_ins.grad_up_threshold = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: Early demand - added tuneable master switch -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_early_demand(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
	return -EINVAL;

	dbs_tuners_ins.early_demand = !!input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable hotplug switch -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_disable_hotplug(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
	return -EINVAL;

	if (input > 0) {
		dbs_tuners_ins.disable_hotplug = true;

		    // ZZ: set profile number to custom mode
		    if (dbs_tuners_ins.profile_number != 0) {
			dbs_tuners_ins.profile_number = 0;
			strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		    }

			enable_offline_cores();
	} else {
		dbs_tuners_ins.disable_hotplug = false;

		    // ZZ: set profile number to custom mode
		    if (dbs_tuners_ins.profile_number != 0) {
			dbs_tuners_ins.profile_number = 0;
			strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		    }

	}
	return count;
}

// ZZ: added tuneable hotplug switch for early supend -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_disable_hotplug_sleep(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
	return -EINVAL;

	if (input > 0) {
		dbs_tuners_ins.disable_hotplug_sleep = true;

		    // ZZ: set profile number to custom mode
		    if (dbs_tuners_ins.profile_number != 0) {
			dbs_tuners_ins.profile_number = 0;
			strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		    }
	} else {
		dbs_tuners_ins.disable_hotplug_sleep = false;

		    // ZZ: set profile number to custom mode
		    if (dbs_tuners_ins.profile_number != 0) {
			dbs_tuners_ins.profile_number = 0;
			strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		    }

	}
	return count;
}

// ZZ: added tuneable hotplug block cycles -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_hotplug_block_cycles(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (input < 0)
	return -EINVAL;

	if (input == 0)
	hotplug_up_block_cycles = 0;

	dbs_tuners_ins.hotplug_block_cycles = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

#ifdef ENABLE_LEGACY_MODE
// ZZ: added tuneable for Legacy Mode -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_legacy_mode(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
	return -EINVAL;

	if (input > 0) {
		dbs_tuners_ins.legacy_mode = true;

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

	} else {
		dbs_tuners_ins.legacy_mode = false;
	}
	return count;
}
#endif

// ZZ: added tuneable hotplug idle threshold -> possible values: range from 0 disabled to 100, if not set default is 0
static ssize_t store_hotplug_idle_threshold(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if ((ret != 1 || input < 0 || input > 100) && input != 0)
		return -EINVAL;

	dbs_tuners_ins.hotplug_idle_threshold = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable scaling idle threshold -> possible values: range from 0 disabled to 100, if not set default is 0
static ssize_t store_scaling_block_threshold(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if ((ret != 1 || input < 0 || input > 100) && input != 0)
		return -EINVAL;
	
	dbs_tuners_ins.scaling_block_threshold = input;
	
	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}
	return count;
}

// ZZ: added tuneable scaling block cycles -> possible values: 0 to disable, any value above 0 to enable, if not set default is 0
static ssize_t store_scaling_block_cycles(struct kobject *a, struct attribute *b,
					    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (input < 0)
	return -EINVAL;

	if (input == 0)
	scaling_block_cycles_count = 0;

	dbs_tuners_ins.scaling_block_cycles = input;

	// ZZ: set profile number to custom mode
	if (dbs_tuners_ins.profile_number != 0) {
	    dbs_tuners_ins.profile_number = 0;
	    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
	}

	return count;
}

// ZZ: added tuneable scaling up idle frequency -> frequency from where the scaling up idle should begin. possible values all valid system frequenies
static ssize_t store_scaling_block_freq(struct kobject *a,
					  struct attribute *b,
					  const char *buf, size_t count)
{
	unsigned int input;
	struct cpufreq_frequency_table *table;	// Yank : Use system frequency table
	int ret;
	int i=0;

	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input == 0) {
	     dbs_tuners_ins.scaling_block_freq = input;

		// ZZ: set profile number to custom mode
		if (dbs_tuners_ins.profile_number != 0) {
		    dbs_tuners_ins.profile_number = 0;
		    strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
		}

	     return count;
	}

	table = cpufreq_frequency_get_table(0);	// Yank : Get system frequency table

	if (!table) {
		return -EINVAL;
	} else if (input > table[max_scaling_freq_hard].frequency) {	 // Yank : Allow only frequencies below or equal to hard max
		return -EINVAL;
	} else {
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
			if (table[i].frequency == input) {
			    dbs_tuners_ins.scaling_block_freq = input;

			    // ZZ: set profile number to custom mode
			    if (dbs_tuners_ins.profile_number != 0) {
				dbs_tuners_ins.profile_number = 0;
				strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));
			    }

			    return count;
			}
	}

	return -EINVAL;
}

static ssize_t store_profile_number(struct kobject *a, struct attribute *b,
					const char *buf, size_t count)
{
	struct cpufreq_frequency_table *table;	// ZZ: for tuneables using system table
	unsigned int input;			// ZZ: regular input handling of this tuneable
	int ret;				// ZZ: regular input handling of this tuneable
	int i = 0;				// ZZ: for main profile loop
	int t = 0;				// ZZ: for tuneables sub-loops
	unsigned int j;				// ZZ: for tuneables update routines
	unsigned int tempsave_idle_rate;	// ZZ: for sampling rate idle rate tuneable

	table = cpufreq_frequency_get_table(0);	// ZZ: for tuneables using system table

	ret = sscanf(buf, "%u", &input);	// ZZ: regular input handling of this tuneable

	if (ret != 1)
		return -EINVAL;

	if (input == 0) {
	dbs_tuners_ins.profile_number = input;
	strncpy(dbs_tuners_ins.profile, custom_profile, sizeof(dbs_tuners_ins.profile));	// ZZ: set profile to custom mode
	return count;
	}

	for (i = 0; (unlikely(zzmoove_profiles[i].profile_number != PROFILE_TABLE_END)); i++) {
		if (unlikely(zzmoove_profiles[i].profile_number == input)) {
			
		// ZZ: set disable_hotplug value
		if (zzmoove_profiles[i].disable_hotplug > 0) {
		dbs_tuners_ins.disable_hotplug = true;
			enable_offline_cores();
		} else {
			dbs_tuners_ins.disable_hotplug = false;
		}

		// ZZ: set disable_hotplug_sleep value
		if (zzmoove_profiles[i].disable_hotplug_sleep > 0)
		    dbs_tuners_ins.disable_hotplug_sleep = true;
		else
		    dbs_tuners_ins.disable_hotplug_sleep = false;

		// ZZ: set down_threshold value
		if (zzmoove_profiles[i].down_threshold > 11 && zzmoove_profiles[i].down_threshold <= 100 &&
		    zzmoove_profiles[i].down_threshold < zzmoove_profiles[i].up_threshold)
		    dbs_tuners_ins.down_threshold = zzmoove_profiles[i].down_threshold;

		// ZZ: set down_threshold_hotplug1 value
		if ((zzmoove_profiles[i].down_threshold_hotplug1 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug1 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug1 < zzmoove_profiles[i].up_threshold_hotplug1)
		|| zzmoove_profiles[i].down_threshold_hotplug1 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug1 = zzmoove_profiles[i].down_threshold_hotplug1;
		    hotplug_thresholds[0][0] = zzmoove_profiles[i].down_threshold_hotplug1;
		}
#if (MAX_CORES == 4 || MAX_CORES == 8)
		// ZZ: set down_threshold_hotplug2 value
		if ((zzmoove_profiles[i].down_threshold_hotplug2 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug2 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug2 < zzmoove_profiles[i].up_threshold_hotplug2)
		|| zzmoove_profiles[i].down_threshold_hotplug2 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug2 = zzmoove_profiles[i].down_threshold_hotplug2;
		    hotplug_thresholds[0][1] = zzmoove_profiles[i].down_threshold_hotplug2;
		}

		// ZZ: set down_threshold_hotplug3 value
		if ((zzmoove_profiles[i].down_threshold_hotplug3 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug3 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug3 < zzmoove_profiles[i].up_threshold_hotplug3)
		|| zzmoove_profiles[i].down_threshold_hotplug3 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug3 = zzmoove_profiles[i].down_threshold_hotplug3;
		    hotplug_thresholds[0][2] = zzmoove_profiles[i].down_threshold_hotplug3;
		}
#endif
#if (MAX_CORES == 8)
		// ZZ: set down_threshold_hotplug4 value
		if ((zzmoove_profiles[i].down_threshold_hotplug4 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug4 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug4 < zzmoove_profiles[i].up_threshold_hotplug4)
		|| zzmoove_profiles[i].down_threshold_hotplug4 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug4 = zzmoove_profiles[i].down_threshold_hotplug4;
		    hotplug_thresholds[0][3] = zzmoove_profiles[i].down_threshold_hotplug4;
		}

		// ZZ: set down_threshold_hotplug5 value
		if ((zzmoove_profiles[i].down_threshold_hotplug5 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug5 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug5 < zzmoove_profiles[i].up_threshold_hotplug5)
		|| zzmoove_profiles[i].down_threshold_hotplug5 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug5 = zzmoove_profiles[i].down_threshold_hotplug5;
		    hotplug_thresholds[0][4] = zzmoove_profiles[i].down_threshold_hotplug5;
		}

		// ZZ: set down_threshold_hotplug6 value
		if ((zzmoove_profiles[i].down_threshold_hotplug6 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug6 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug6 < zzmoove_profiles[i].up_threshold_hotplug6)
		|| zzmoove_profiles[i].down_threshold_hotplug6 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug6 = zzmoove_profiles[i].down_threshold_hotplug6;
		    hotplug_thresholds[0][5] = zzmoove_profiles[i].down_threshold_hotplug6;
		}

		// ZZ: set down_threshold_hotplug7 value
		if ((zzmoove_profiles[i].down_threshold_hotplug7 <= 100
		&& zzmoove_profiles[i].down_threshold_hotplug7 >= 1
		&& zzmoove_profiles[i].down_threshold_hotplug7 < zzmoove_profiles[i].up_threshold_hotplug7)
		|| zzmoove_profiles[i].down_threshold_hotplug7 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug7 = zzmoove_profiles[i].down_threshold_hotplug7;
		    hotplug_thresholds[0][6] = zzmoove_profiles[i].down_threshold_hotplug7;
		}
#endif
		// ZZ: set down_threshold_hotplug_freq1 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq1 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq1 = zzmoove_profiles[i].down_threshold_hotplug_freq1;
		    hotplug_thresholds_freq[1][0] = zzmoove_profiles[i].down_threshold_hotplug_freq1;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq1 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].down_threshold_hotplug_freq1 < zzmoove_profiles[i].up_threshold_hotplug_freq1) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq1) {
				dbs_tuners_ins.down_threshold_hotplug_freq1 = zzmoove_profiles[i].down_threshold_hotplug_freq1;
				hotplug_thresholds_freq[1][0] = zzmoove_profiles[i].down_threshold_hotplug_freq1;
			}
		    }
		}
#if (MAX_CORES == 4 || MAX_CORES == 8)
		// ZZ: set down_threshold_hotplug_freq2 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq2 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq2 = zzmoove_profiles[i].down_threshold_hotplug_freq2;
		    hotplug_thresholds_freq[1][1] = zzmoove_profiles[i].down_threshold_hotplug_freq2;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq2 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].down_threshold_hotplug_freq2 < zzmoove_profiles[i].up_threshold_hotplug_freq2) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq2) {
				dbs_tuners_ins.down_threshold_hotplug_freq2 = zzmoove_profiles[i].down_threshold_hotplug_freq2;
				hotplug_thresholds_freq[1][1] = zzmoove_profiles[i].down_threshold_hotplug_freq2;
			}
		    }
		}

		// ZZ: set down_threshold_hotplug_freq3 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq3 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq3 = zzmoove_profiles[i].down_threshold_hotplug_freq3;
		    hotplug_thresholds_freq[1][2] = zzmoove_profiles[i].down_threshold_hotplug_freq3;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq3 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].down_threshold_hotplug_freq3 < zzmoove_profiles[i].up_threshold_hotplug_freq3) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq3) {
				dbs_tuners_ins.down_threshold_hotplug_freq3 = zzmoove_profiles[i].down_threshold_hotplug_freq3;
				hotplug_thresholds_freq[1][2] = zzmoove_profiles[i].down_threshold_hotplug_freq3;
			}
		    }
		}
#endif
#if (MAX_CORES == 8)
		// ZZ: set down_threshold_hotplug_freq4 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq4 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq4 = zzmoove_profiles[i].down_threshold_hotplug_freq4;
		    hotplug_thresholds_freq[1][3] = zzmoove_profiles[i].down_threshold_hotplug_freq4;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq4 <= table[max_scaling_freq_hard].frequency  &&
		zzmoove_profiles[i].down_threshold_hotplug_freq4 < zzmoove_profiles[i].up_threshold_hotplug_freq4) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq4) {
				dbs_tuners_ins.down_threshold_hotplug_freq4 = zzmoove_profiles[i].down_threshold_hotplug_freq4;
				hotplug_thresholds_freq[1][3] = zzmoove_profiles[i].down_threshold_hotplug_freq4;
			}
		    }
		}

		// ZZ: set down_threshold_hotplug_freq5 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq5 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq5 = zzmoove_profiles[i].down_threshold_hotplug_freq5;
		    hotplug_thresholds_freq[1][4] = zzmoove_profiles[i].down_threshold_hotplug_freq5;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq5 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].down_threshold_hotplug_freq5 < zzmoove_profiles[i].up_threshold_hotplug_freq5) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq5) {
				dbs_tuners_ins.down_threshold_hotplug_freq5 = zzmoove_profiles[i].down_threshold_hotplug_freq5;
				hotplug_thresholds_freq[1][4] = zzmoove_profiles[i].down_threshold_hotplug_freq5;
			}
		    }
		}

		// ZZ: set down_threshold_hotplug_freq6 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq6 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq6 = zzmoove_profiles[i].down_threshold_hotplug_freq6;
		    hotplug_thresholds_freq[1][5] = zzmoove_profiles[i].down_threshold_hotplug_freq6;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq6 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].down_threshold_hotplug_freq6 < zzmoove_profiles[i].up_threshold_hotplug_freq6) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq6) {
				dbs_tuners_ins.down_threshold_hotplug_freq6 = zzmoove_profiles[i].down_threshold_hotplug_freq6;
				hotplug_thresholds_freq[1][5] = zzmoove_profiles[i].down_threshold_hotplug_freq6;
			}
		    }
		}

		// ZZ: set down_threshold_hotplug_freq7 value
		if (zzmoove_profiles[i].down_threshold_hotplug_freq7 == 0) {
		    dbs_tuners_ins.down_threshold_hotplug_freq7 = zzmoove_profiles[i].down_threshold_hotplug_freq7;
		    hotplug_thresholds_freq[1][6] = zzmoove_profiles[i].down_threshold_hotplug_freq7;
		}

		if (table && zzmoove_profiles[i].down_threshold_hotplug_freq7 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].down_threshold_hotplug_freq7 < zzmoove_profiles[i].up_threshold_hotplug_freq7) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].down_threshold_hotplug_freq7) {
				dbs_tuners_ins.down_threshold_hotplug_freq7 = zzmoove_profiles[i].down_threshold_hotplug_freq7;
				hotplug_thresholds_freq[1][6] = zzmoove_profiles[i].down_threshold_hotplug_freq7;
			}
		    }
		}
#endif
		// ZZ: set down_threshold_sleep value
		if (zzmoove_profiles[i].down_threshold_sleep > 11 && zzmoove_profiles[i].down_threshold_sleep <= 100 &&
			zzmoove_profiles[i].down_threshold_sleep < dbs_tuners_ins.up_threshold_sleep)
			dbs_tuners_ins.down_threshold_sleep = zzmoove_profiles[i].down_threshold_sleep;

		// ZZ: set early_demand value
		dbs_tuners_ins.early_demand = !!zzmoove_profiles[i].early_demand;

		// ZZ: set fast_scaling value
		if (zzmoove_profiles[i].fast_scaling <= 13 && zzmoove_profiles[i].fast_scaling >= 0)
		    dbs_tuners_ins.fast_scaling = zzmoove_profiles[i].fast_scaling;

		if (zzmoove_profiles[i].fast_scaling > 12) {
		    scaling_mode_up   = 0;			// ZZ: auto fast scaling
		    scaling_mode_down = 0;			// ZZ: auto fast scaling
		
		} else if (zzmoove_profiles[i].fast_scaling > 8) {
		    scaling_mode_up   = 0;
		    scaling_mode_down = zzmoove_profiles[i].fast_scaling - 8;	// ZZ: fast scaling down only
		
		} else if (zzmoove_profiles[i].fast_scaling > 4) {
		    scaling_mode_up   = zzmoove_profiles[i].fast_scaling - 4;	// Yank : fast scaling up
		    scaling_mode_down = zzmoove_profiles[i].fast_scaling - 4;	// Yank : fast scaling down
		
		} else {
		    scaling_mode_up   = zzmoove_profiles[i].fast_scaling;	// Yank : fast scaling up only
		    scaling_mode_down = 0;
		}

		// ZZ: set fast_scaling_sleep value
		if (zzmoove_profiles[i].fast_scaling_sleep <= 8 && zzmoove_profiles[i].fast_scaling_sleep >= 0)
		    dbs_tuners_ins.fast_scaling_sleep = zzmoove_profiles[i].fast_scaling_sleep;

		// ZZ: set freq_limit value
		if (table && zzmoove_profiles[i].freq_limit == 0) {
	    	    max_scaling_freq_soft = max_scaling_freq_hard;
		
		if (freq_table_order == 1)					// ZZ: if descending ordered table is used
		    limit_table_start = max_scaling_freq_soft;			// ZZ: we should use the actual scaling soft limit value as search start point
		else
		    limit_table_end = table[freq_table_size].frequency;		// ZZ: set search end point to max freq when using ascending table

	    	dbs_tuners_ins.freq_limit = zzmoove_profiles[i].freq_limit;

		} else if (table && zzmoove_profiles[i].freq_limit <= table[max_scaling_freq_hard].frequency) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].freq_limit) {
				max_scaling_freq_soft = t;
				if (freq_table_order == 1)			// ZZ: if descending ordered table is used
				    limit_table_start = max_scaling_freq_soft;	// ZZ: we should use the actual scaling soft limit value as search start point
				else
				    limit_table_end = table[t].frequency;	// ZZ: set search end point to max soft freq limit when using ascenting table
				dbs_tuners_ins.freq_limit = zzmoove_profiles[i].freq_limit;
			}
		    }
		}
		
		// ZZ: set freq_limit_sleep value
		if (table && zzmoove_profiles[i].freq_limit_sleep == 0) {
		    dbs_tuners_ins.freq_limit_sleep = zzmoove_profiles[i].freq_limit_sleep;

		} else if (table && zzmoove_profiles[i].freq_limit_sleep <= table[max_scaling_freq_hard].frequency) { // Yank : Allow only frequencies below or equal to hard max
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].freq_limit_sleep)
				dbs_tuners_ins.freq_limit_sleep = zzmoove_profiles[i].freq_limit_sleep;
		    }
		}

		// ZZ: set freq_step value
		if (zzmoove_profiles[i].freq_step > 100)
		    dbs_tuners_ins.freq_step = 100;
		else
		    dbs_tuners_ins.freq_step = zzmoove_profiles[i].freq_step;

		// ZZ: set freq_step_sleep value
		if (zzmoove_profiles[i].freq_step_sleep > 100)
		    dbs_tuners_ins.freq_step_sleep = 100;
		else
		    dbs_tuners_ins.freq_step_sleep = zzmoove_profiles[i].freq_step_sleep;
		
		// ZZ: set grad_up_threshold value
		if (zzmoove_profiles[i].grad_up_threshold < 100 && zzmoove_profiles[i].grad_up_threshold > 11)
		    dbs_tuners_ins.grad_up_threshold = zzmoove_profiles[i].grad_up_threshold;

		// ZZ: set hotplug_block_cycles value
		if (zzmoove_profiles[i].hotplug_block_cycles >= 0)
		    dbs_tuners_ins.hotplug_block_cycles = zzmoove_profiles[i].hotplug_block_cycles;

		// ZZ: set hotplug_idle_threshold value
		if (zzmoove_profiles[i].hotplug_idle_threshold >= 0 && zzmoove_profiles[i].hotplug_idle_threshold < 100)
		    dbs_tuners_ins.hotplug_idle_threshold = zzmoove_profiles[i].hotplug_idle_threshold;

		// ZZ: set ignore_nice_load value
		if (zzmoove_profiles[i].ignore_nice_load > 1)
		    zzmoove_profiles[i].ignore_nice_load = 1;

		dbs_tuners_ins.ignore_nice = zzmoove_profiles[i].ignore_nice_load;

		/* we need to re-evaluate prev_cpu_idle */
		for_each_online_cpu(j) {
		    struct cpu_dbs_info_s *dbs_info;
		    dbs_info = &per_cpu(cs_cpu_dbs_info, j);
		    dbs_info->prev_cpu_idle = get_cpu_idle_time(j,
						&dbs_info->prev_cpu_wall);
		if (dbs_tuners_ins.ignore_nice)
		    dbs_info->prev_cpu_nice = kstat_cpu(j).cpustat.nice;
		}
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
		// ZZ: set lcdfreq_enable value
		if (zzmoove_profiles[i].lcdfreq_enable > 0) {
		    dbs_tuners_ins.lcdfreq_enable = true;
		} else {
		    dbs_tuners_ins.lcdfreq_enable = false;
		    // Set screen to 60Hz when stopping to switch
		    lcdfreq_lock_current = 0;
		    _lcdfreq_lock(lcdfreq_lock_current);
		}
		
		// ZZ: set lcdfreq_kick_in_cores value
		if (zzmoove_profiles[i].lcdfreq_kick_in_cores <= num_possible_cpus()
		|| zzmoove_profiles[i].lcdfreq_kick_in_cores == 0)
		    dbs_tuners_ins.lcdfreq_kick_in_cores = zzmoove_profiles[i].lcdfreq_kick_in_cores;
		
		// ZZ: set lcdfreq_kick_in_down_delay value
		if (zzmoove_profiles[i].lcdfreq_kick_in_down_delay >= 0) {
		    dbs_tuners_ins.lcdfreq_kick_in_down_delay = zzmoove_profiles[i].lcdfreq_kick_in_down_delay;
		    dbs_tuners_ins.lcdfreq_kick_in_down_left = dbs_tuners_ins.lcdfreq_kick_in_down_delay;
		}

		// ZZ: set lcdfreq_kick_in_freq value
		if (table && zzmoove_profiles[i].lcdfreq_kick_in_freq <= table[max_scaling_freq_hard].frequency) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].lcdfreq_kick_in_freq) {
				dbs_tuners_ins.lcdfreq_kick_in_freq = zzmoove_profiles[i].lcdfreq_kick_in_freq;
			}
		    }
		}
		
		// ZZ: set lcdfreq_kick_in_up_delay value
		if (zzmoove_profiles[i].lcdfreq_kick_in_up_delay >= 0) {
		    dbs_tuners_ins.lcdfreq_kick_in_up_delay = zzmoove_profiles[i].lcdfreq_kick_in_up_delay;
		    dbs_tuners_ins.lcdfreq_kick_in_up_left = dbs_tuners_ins.lcdfreq_kick_in_up_delay;
		}
#endif

		// ZZ: set sampling_down_factor value
		if (zzmoove_profiles[i].sampling_down_factor <= MAX_SAMPLING_DOWN_FACTOR && zzmoove_profiles[i].sampling_down_factor >= 1)
		    dbs_tuners_ins.sampling_down_factor = zzmoove_profiles[i].sampling_down_factor;

		    // ZZ: Reset down sampling multiplier in case it was active
		    for_each_online_cpu(j) {
			struct cpu_dbs_info_s *dbs_info;
			dbs_info = &per_cpu(cs_cpu_dbs_info, j);
			dbs_info->rate_mult = 1;
		    }

		// ZZ: set sampling_down_max_momentum value
		if (zzmoove_profiles[i].sampling_down_max_momentum <= MAX_SAMPLING_DOWN_FACTOR - dbs_tuners_ins.sampling_down_factor 
		&& zzmoove_profiles[i].sampling_down_max_momentum >= 0) {
		    dbs_tuners_ins.sampling_down_max_mom = zzmoove_profiles[i].sampling_down_max_momentum;
		    orig_sampling_down_max_mom = dbs_tuners_ins.sampling_down_max_mom;
		}
	
		// ZZ: Reset sampling down factor to default if momentum was disabled
		if (dbs_tuners_ins.sampling_down_max_mom == 0)
		    dbs_tuners_ins.sampling_down_factor = DEF_SAMPLING_DOWN_FACTOR;

		    // ZZ: Reset momentum_adder and reset down sampling multiplier in case momentum was disabled
		    for_each_online_cpu(j) {
			struct cpu_dbs_info_s *dbs_info;
			dbs_info = &per_cpu(cs_cpu_dbs_info, j);
			dbs_info->momentum_adder = 0;
			if (dbs_tuners_ins.sampling_down_max_mom == 0)
			dbs_info->rate_mult = 1;
		    }

		// ZZ: set sampling_down_momentum_sensitivity value
		if (zzmoove_profiles[i].sampling_down_momentum_sensitivity <= MAX_SAMPLING_DOWN_MOMENTUM_SENSITIVITY && zzmoove_profiles[i].sampling_down_momentum_sensitivity >= 1) {
		    dbs_tuners_ins.sampling_down_mom_sens = zzmoove_profiles[i].sampling_down_momentum_sensitivity;

		    // ZZ: Reset momentum_adder
		    for_each_online_cpu(j) {
			struct cpu_dbs_info_s *dbs_info;
			dbs_info = &per_cpu(cs_cpu_dbs_info, j);
			dbs_info->momentum_adder = 0;
		    }

		// ZZ: set sampling_rate value
		if(dbs_tuners_ins.sampling_rate_idle != 0) {
		    tempsave_idle_rate = dbs_tuners_ins.sampling_rate_idle;		// ZZ: temporary save idle rate
		    dbs_tuners_ins.sampling_rate_idle = 0;				// ZZ: disable dynamic sample rate during setting normal rate
		    dbs_tuners_ins.sampling_rate = dbs_tuners_ins.sampling_rate_current = max(zzmoove_profiles[i].sampling_rate, min_sampling_rate); // ZZ: set it to new value
		    dbs_tuners_ins.sampling_rate_idle = tempsave_idle_rate;				// ZZ: restore idle rate
		} else {
		    dbs_tuners_ins.sampling_rate = dbs_tuners_ins.sampling_rate_current = max(zzmoove_profiles[i].sampling_rate, min_sampling_rate); // ZZ: set it to new value
		}

		// ZZ: set sampling_rate_idle value
		if (zzmoove_profiles[i].sampling_rate_idle == 0) {
		    dbs_tuners_ins.sampling_rate_current = dbs_tuners_ins.sampling_rate;	// ZZ: reset normal rate in case it was on idle rate
		    dbs_tuners_ins.sampling_rate_idle = zzmoove_profiles[i].sampling_rate_idle;	// ZZ: set it to 0
		} else {
		    dbs_tuners_ins.sampling_rate_idle = max(zzmoove_profiles[i].sampling_rate_idle, min_sampling_rate);	// ZZ: or validate the input as normal
		}
		
		// ZZ: set sampling_rate_idle_delay value
		if (zzmoove_profiles[i].sampling_rate_idle_delay >= 0) {
		    sampling_rate_step_up_delay = 0;
		    sampling_rate_step_down_delay = 0;
		    dbs_tuners_ins.sampling_rate_idle_delay = zzmoove_profiles[i].sampling_rate_idle_delay;
		}

		// ZZ: set sampling_rate_idle_threshold value
		if (zzmoove_profiles[i].sampling_rate_idle_threshold <= 100)
		    dbs_tuners_ins.sampling_rate_idle_threshold = zzmoove_profiles[i].sampling_rate_idle_threshold;

		// ZZ: set sampling_rate_sleep_multiplier value
		if (zzmoove_profiles[i].sampling_rate_sleep_multiplier <= MAX_SAMPLING_RATE_SLEEP_MULTIPLIER && zzmoove_profiles[i].sampling_rate_sleep_multiplier >= 1)
		    dbs_tuners_ins.sampling_rate_sleep_multiplier = zzmoove_profiles[i].sampling_rate_sleep_multiplier;

		// ZZ: set scaling_block_cycles value
		if (zzmoove_profiles[i].scaling_block_cycles >= 0) {
		    dbs_tuners_ins.scaling_block_cycles = zzmoove_profiles[i].scaling_block_cycles;
		    if (zzmoove_profiles[i].scaling_block_cycles == 0)
			scaling_block_cycles_count = 0;
		}

		// ZZ: set scaling_block_freq value
		if (zzmoove_profiles[i].scaling_block_freq == 0) {
	    	    dbs_tuners_ins.scaling_block_freq = zzmoove_profiles[i].scaling_block_freq;

		} else if (table && zzmoove_profiles[i].scaling_block_freq <= table[max_scaling_freq_hard].frequency) {	 // Yank : Allow only frequencies below or equal to hard max
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].scaling_block_freq) {
			    dbs_tuners_ins.scaling_block_freq = zzmoove_profiles[i].scaling_block_freq;
			}
		    }
		}

		// ZZ: set scaling_block_threshold value
		if (zzmoove_profiles[i].scaling_block_threshold >= 0 && zzmoove_profiles[i].scaling_block_threshold <= 100)
		    dbs_tuners_ins.scaling_block_threshold = zzmoove_profiles[i].scaling_block_threshold;

		// ZZ: set smooth_up value
		if (zzmoove_profiles[i].smooth_up <= 100 && zzmoove_profiles[i].smooth_up >= 1)
		    dbs_tuners_ins.smooth_up = zzmoove_profiles[i].smooth_up;

		// ZZ: set smooth_up_sleep value
		if (zzmoove_profiles[i].smooth_up_sleep <= 100 && zzmoove_profiles[i].smooth_up_sleep >= 1)
		    dbs_tuners_ins.smooth_up_sleep = zzmoove_profiles[i].smooth_up_sleep;

		// ZZ: set up_threshold value
		if (zzmoove_profiles[i].up_threshold <= 100 && zzmoove_profiles[i].up_threshold >= zzmoove_profiles[i].down_threshold)
		    dbs_tuners_ins.up_threshold = zzmoove_profiles[i].up_threshold;

		// ZZ: set up_threshold_hotplug1 value
		if (zzmoove_profiles[i].up_threshold_hotplug1 >= 0 && zzmoove_profiles[i].up_threshold_hotplug1 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug1 > zzmoove_profiles[i].down_threshold_hotplug1) {
		    dbs_tuners_ins.up_threshold_hotplug1 = zzmoove_profiles[i].up_threshold_hotplug1;
		    hotplug_thresholds[0][0] = zzmoove_profiles[i].up_threshold_hotplug1;
		}
#if (MAX_CORES == 4 || MAX_CORES == 8)
		// ZZ: set up_threshold_hotplug2 value
		if (zzmoove_profiles[i].up_threshold_hotplug2 >= 0 && zzmoove_profiles[i].up_threshold_hotplug2 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug2 > zzmoove_profiles[i].down_threshold_hotplug2) {
		    dbs_tuners_ins.up_threshold_hotplug2 = zzmoove_profiles[i].up_threshold_hotplug2;
		    hotplug_thresholds[0][1] = zzmoove_profiles[i].up_threshold_hotplug2;
		}

		// ZZ: set up_threshold_hotplug3 value
		if (zzmoove_profiles[i].up_threshold_hotplug3 >= 0 && zzmoove_profiles[i].up_threshold_hotplug3 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug3 > zzmoove_profiles[i].down_threshold_hotplug3) {
		    dbs_tuners_ins.up_threshold_hotplug3 = zzmoove_profiles[i].up_threshold_hotplug3;
		    hotplug_thresholds[0][2] = zzmoove_profiles[i].up_threshold_hotplug3;
		}
#endif
#if (MAX_CORES == 8)
		// ZZ: set up_threshold_hotplug4 value
		if (zzmoove_profiles[i].up_threshold_hotplug4 >= 0 && zzmoove_profiles[i].up_threshold_hotplug4 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug4 > zzmoove_profiles[i].down_threshold_hotplug4) {
		    dbs_tuners_ins.up_threshold_hotplug4 = zzmoove_profiles[i].up_threshold_hotplug4;
		    hotplug_thresholds[0][3] = zzmoove_profiles[i].up_threshold_hotplug4;
		}
		
		// ZZ: set up_threshold_hotplug5 value
		if (zzmoove_profiles[i].up_threshold_hotplug5 >= 0 && zzmoove_profiles[i].up_threshold_hotplug5 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug5 > zzmoove_profiles[i].down_threshold_hotplug5) {
		    dbs_tuners_ins.up_threshold_hotplug5 = zzmoove_profiles[i].up_threshold_hotplug5;
		    hotplug_thresholds[0][4] = zzmoove_profiles[i].up_threshold_hotplug5;
		}
		
		// ZZ: set up_threshold_hotplug6 value
		if (zzmoove_profiles[i].up_threshold_hotplug6 >= 0 && zzmoove_profiles[i].up_threshold_hotplug6 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug6 > zzmoove_profiles[i].down_threshold_hotplug6) {
		    dbs_tuners_ins.up_threshold_hotplug6 = zzmoove_profiles[i].up_threshold_hotplug6;
		    hotplug_thresholds[0][5] = zzmoove_profiles[i].up_threshold_hotplug6;
		}
		
		// ZZ: set up_threshold_hotplug7 value
		if (zzmoove_profiles[i].up_threshold_hotplug7 >= 0 && zzmoove_profiles[i].up_threshold_hotplug7 <= 100 &&
		    zzmoove_profiles[i].up_threshold_hotplug7 > zzmoove_profiles[i].down_threshold_hotplug7) {
		    dbs_tuners_ins.up_threshold_hotplug7 = zzmoove_profiles[i].up_threshold_hotplug7;
		    hotplug_thresholds[0][6] = zzmoove_profiles[i].up_threshold_hotplug7;
		}
#endif
		// ZZ: set up_threshold_hotplug_freq1 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq1 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq1 = zzmoove_profiles[i].up_threshold_hotplug_freq1;
		    hotplug_thresholds_freq[0][0] = zzmoove_profiles[i].up_threshold_hotplug_freq1;
		}

		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq1 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq1 > zzmoove_profiles[i].down_threshold_hotplug_freq1) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
		        if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq1) {
				dbs_tuners_ins.up_threshold_hotplug_freq1 = zzmoove_profiles[i].up_threshold_hotplug_freq1;
				hotplug_thresholds_freq[0][0] = zzmoove_profiles[i].up_threshold_hotplug_freq1;
		        }
		    }
		}

#if (MAX_CORES == 4 || MAX_CORES == 8)
		// ZZ: set up_threshold_hotplug_freq2 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq2 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq2 = zzmoove_profiles[i].up_threshold_hotplug_freq2;
		    hotplug_thresholds_freq[0][1] = zzmoove_profiles[i].up_threshold_hotplug_freq2;
		}
		
		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq2 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq2 > zzmoove_profiles[i].down_threshold_hotplug_freq2) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq2) {
				dbs_tuners_ins.up_threshold_hotplug_freq2 = zzmoove_profiles[i].up_threshold_hotplug_freq2;
				hotplug_thresholds_freq[0][1] = zzmoove_profiles[i].up_threshold_hotplug_freq2;
			}
		    }
		}

		// ZZ: set up_threshold_hotplug_freq3 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq3 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq3 = zzmoove_profiles[i].up_threshold_hotplug_freq3;
		    hotplug_thresholds_freq[0][2] = zzmoove_profiles[i].up_threshold_hotplug_freq3;
		}
		
		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq3 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq3 > zzmoove_profiles[i].down_threshold_hotplug_freq3) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq3) {
				dbs_tuners_ins.up_threshold_hotplug_freq3 = zzmoove_profiles[i].up_threshold_hotplug_freq3;
				hotplug_thresholds_freq[0][2] = zzmoove_profiles[i].up_threshold_hotplug_freq3;
			}
		    }
		}
#endif
#if (MAX_CORES == 8)
		// ZZ: set up_threshold_hotplug_freq4 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq4 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq4 = zzmoove_profiles[i].up_threshold_hotplug_freq4;
		    hotplug_thresholds_freq[0][3] = zzmoove_profiles[i].up_threshold_hotplug_freq4;
		}
	
		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq4 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq4 > zzmoove_profiles[i].down_threshold_hotplug_freq4) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq4) {
				dbs_tuners_ins.up_threshold_hotplug_freq4 = zzmoove_profiles[i].up_threshold_hotplug_freq4;
				hotplug_thresholds_freq[0][3] = zzmoove_profiles[i].up_threshold_hotplug_freq4;
			}
		    }
		}
		
		// ZZ: set up_threshold_hotplug_freq5 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq5 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq5 = zzmoove_profiles[i].up_threshold_hotplug_freq5;
		    hotplug_thresholds_freq[0][4] = zzmoove_profiles[i].up_threshold_hotplug_freq5;
		}
		
		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq5 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq5 > zzmoove_profiles[i].down_threshold_hotplug_freq5) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq5) {
				dbs_tuners_ins.up_threshold_hotplug_freq5 = zzmoove_profiles[i].up_threshold_hotplug_freq5;
				hotplug_thresholds_freq[0][4] = zzmoove_profiles[i].up_threshold_hotplug_freq5;
			}
		    }
		}
	
		// ZZ: set up_threshold_hotplug_freq6 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq6 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq6 = zzmoove_profiles[i].up_threshold_hotplug_freq6;
		    hotplug_thresholds_freq[0][5] = zzmoove_profiles[i].up_threshold_hotplug_freq6;
		}
		
		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq6 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq6 > zzmoove_profiles[i].down_threshold_hotplug_freq6) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq6) {
				dbs_tuners_ins.up_threshold_hotplug_freq6 = zzmoove_profiles[i].up_threshold_hotplug_freq6;
				hotplug_thresholds_freq[0][5] = zzmoove_profiles[i].up_threshold_hotplug_freq6;
			}
		    }
		}
		
		// ZZ: set up_threshold_hotplug_freq7 value
		if (zzmoove_profiles[i].up_threshold_hotplug_freq7 == 0) {
		    dbs_tuners_ins.up_threshold_hotplug_freq7 = zzmoove_profiles[i].up_threshold_hotplug_freq7;
		    hotplug_thresholds_freq[0][6] = zzmoove_profiles[i].up_threshold_hotplug_freq7;
		}
		
		if (table && zzmoove_profiles[i].up_threshold_hotplug_freq7 <= table[max_scaling_freq_hard].frequency &&
		zzmoove_profiles[i].up_threshold_hotplug_freq7 > zzmoove_profiles[i].down_threshold_hotplug_freq7) {
		    for (t = 0; (table[t].frequency != CPUFREQ_TABLE_END); t++) {
			if (table[t].frequency == zzmoove_profiles[i].up_threshold_hotplug_freq7) {
				dbs_tuners_ins.up_threshold_hotplug_freq7 = zzmoove_profiles[i].up_threshold_hotplug_freq7;
				hotplug_thresholds_freq[0][6] = zzmoove_profiles[i].up_threshold_hotplug_freq7;
			}
		    }
		}
#endif
		// ZZ: set up_threshold_sleep value
		if (zzmoove_profiles[i].up_threshold_sleep <= 100 && zzmoove_profiles[i].up_threshold_sleep > dbs_tuners_ins.down_threshold_sleep)
		    dbs_tuners_ins.up_threshold_sleep = zzmoove_profiles[i].up_threshold_sleep;
#ifdef ENABLE_LEGACY_MODE
		// ZZ: set legacy_mode value
		if (zzmoove_profiles[i].legacy_mode > 0)
		dbs_tuners_ins.legacy_mode = true;
		else
		dbs_tuners_ins.legacy_mode = false;
#endif
		dbs_tuners_ins.profile_number = input;
		strncpy(dbs_tuners_ins.profile, zzmoove_profiles[i].profile_name, sizeof(dbs_tuners_ins.profile)); // ZZ: set profile to custom mode
		return count;
	     }
	   }
	}
return -EINVAL;
}

// Yank: add hotplug up/down threshold sysfs store interface

#define store_up_threshold_hotplug_freq(name,core)						\
static ssize_t store_up_threshold_hotplug_freq##name						\
(struct kobject *a, struct attribute *b, const char *buf, size_t count)				\
{												\
	unsigned int input;									\
	struct cpufreq_frequency_table *table;							\
	int ret;										\
	int i=0;										\
												\
	ret = sscanf(buf, "%u", &input);							\
	if (ret != 1)										\
		return -EINVAL;									\
												\
	if (input == 0) {									\
		dbs_tuners_ins.up_threshold_hotplug_freq##name = input;				\
		hotplug_thresholds_freq[0][core] = input;					\
		if (dbs_tuners_ins.profile_number != 0) {					\
		    dbs_tuners_ins.profile_number = 0;						\
		    strncpy(dbs_tuners_ins.profile, custom_profile, 				\
		    sizeof(dbs_tuners_ins.profile));						\
		}										\
		return count;									\
	}											\
												\
	if (input <= dbs_tuners_ins.down_threshold_hotplug_freq##name				\
		&& dbs_tuners_ins.down_threshold_hotplug_freq##name != 0)			\
		return -EINVAL;									\
												\
	table = cpufreq_frequency_get_table(0);							\
												\
	if (!table) {										\
		return -EINVAL;									\
	} else if (input > table[max_scaling_freq_hard].frequency) {				\
		return -EINVAL;									\
	} else {										\
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)			\
			if (table[i].frequency == input) {					\
				dbs_tuners_ins.up_threshold_hotplug_freq##name = input;		\
				hotplug_thresholds_freq[0][core] = input;			\
				if (dbs_tuners_ins.profile_number != 0) {			\
				    dbs_tuners_ins.profile_number = 0;				\
				    strncpy(dbs_tuners_ins.profile, custom_profile, 		\
				    sizeof(dbs_tuners_ins.profile));				\
				}								\
				return count;							\
			}									\
	}											\
	return -EINVAL;										\
}

#define store_down_threshold_hotplug_freq(name,core)						\
static ssize_t store_down_threshold_hotplug_freq##name						\
(struct kobject *a, struct attribute *b, const char *buf, size_t count)				\
{												\
	unsigned int input;									\
	struct cpufreq_frequency_table *table;							\
	int ret;										\
	int i=0;										\
												\
	ret = sscanf(buf, "%u", &input);							\
	if (ret != 1)										\
		return -EINVAL;									\
												\
	if (input == 0) {									\
		dbs_tuners_ins.down_threshold_hotplug_freq##name = input;			\
		hotplug_thresholds_freq[1][core] = input;					\
		if (dbs_tuners_ins.profile_number != 0) {					\
		    dbs_tuners_ins.profile_number = 0;						\
		    strncpy(dbs_tuners_ins.profile, custom_profile, 				\
		    sizeof(dbs_tuners_ins.profile));						\
		}										\
		return count;									\
	}											\
												\
	if (input >= dbs_tuners_ins.up_threshold_hotplug_freq##name				\
		&& dbs_tuners_ins.up_threshold_hotplug_freq##name != 0)				\
		return -EINVAL;									\
												\
	table = cpufreq_frequency_get_table(0);							\
												\
	if (!table) {										\
		return -EINVAL;									\
	} else if (input > table[max_scaling_freq_hard].frequency) {				\
		return -EINVAL;									\
	} else {										\
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)			\
			if (table[i].frequency == input) {					\
				dbs_tuners_ins.down_threshold_hotplug_freq##name = input;	\
				hotplug_thresholds_freq[1][core] = input;			\
				if (dbs_tuners_ins.profile_number != 0) {			\
				    dbs_tuners_ins.profile_number = 0;				\
				    strncpy(dbs_tuners_ins.profile, custom_profile, 		\
				    sizeof(dbs_tuners_ins.profile));				\
				}								\
				return count;							\
			}									\
	}											\
	return -EINVAL;										\
}

store_up_threshold_hotplug_freq(1,0);
store_down_threshold_hotplug_freq(1,0);
#if (MAX_CORES == 4 || MAX_CORES == 8)
store_up_threshold_hotplug_freq(2,1);
store_down_threshold_hotplug_freq(2,1);
store_up_threshold_hotplug_freq(3,2);
store_down_threshold_hotplug_freq(3,2);
#endif
#if (MAX_CORES == 8)
store_up_threshold_hotplug_freq(4,3);
store_down_threshold_hotplug_freq(4,3);
store_up_threshold_hotplug_freq(5,4);
store_down_threshold_hotplug_freq(5,4);
store_up_threshold_hotplug_freq(6,5);
store_down_threshold_hotplug_freq(6,5);
store_up_threshold_hotplug_freq(7,6);
store_down_threshold_hotplug_freq(7,6);
#endif
define_one_global_rw(profile_number);				// ZZ: added profile number tunable
define_one_global_ro(profile);					// ZZ: added profile name tuneable
define_one_global_ro(sampling_rate_current);			// ZZ: currently active sampling rate
define_one_global_rw(sampling_rate);
define_one_global_rw(sampling_rate_idle_threshold);
define_one_global_rw(sampling_rate_idle);
define_one_global_rw(sampling_rate_idle_delay);
define_one_global_rw(sampling_rate_sleep_multiplier);		// ZZ: added tuneable
define_one_global_rw(sampling_down_factor);			// ZZ: Sampling down factor (reactived)
define_one_global_rw(sampling_down_max_momentum);		// ZZ: Sampling down momentum tuneable
define_one_global_rw(sampling_down_momentum_sensitivity);	// ZZ: Sampling down momentum tuneable
define_one_global_rw(up_threshold);
define_one_global_rw(up_threshold_sleep);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug1);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq1);		// Yank: added tuneable
#if (MAX_CORES == 4 || MAX_CORES == 8)
define_one_global_rw(up_threshold_hotplug2);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq2);		// Yank: added tuneable
define_one_global_rw(up_threshold_hotplug3);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq3);		// Yank: added tuneable
#endif
#if (MAX_CORES == 8)
define_one_global_rw(up_threshold_hotplug4);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq4);		// Yank: added tuneable
define_one_global_rw(up_threshold_hotplug5);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq5);		// Yank: added tuneable
define_one_global_rw(up_threshold_hotplug6);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq6);		// Yank: added tuneable
define_one_global_rw(up_threshold_hotplug7);			// ZZ: added tuneable
define_one_global_rw(up_threshold_hotplug_freq7);		// Yank: added tuneable
#endif
define_one_global_rw(down_threshold);
define_one_global_rw(down_threshold_sleep);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug1);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq1);		// Yank: added tuneable
#if (MAX_CORES == 4 || MAX_CORES == 8)
define_one_global_rw(down_threshold_hotplug2);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq2);		// Yank: added tuneable
define_one_global_rw(down_threshold_hotplug3);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq3);		// Yank: added tuneable
#endif
#if (MAX_CORES == 8)
define_one_global_rw(down_threshold_hotplug4);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq4);		// Yank: added tuneable
define_one_global_rw(down_threshold_hotplug5);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq5);		// Yank: added tuneable
define_one_global_rw(down_threshold_hotplug6);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq6);		// Yank: added tuneable
define_one_global_rw(down_threshold_hotplug7);			// ZZ: added tuneable
define_one_global_rw(down_threshold_hotplug_freq7);		// Yank: added tuneable
#endif
define_one_global_rw(ignore_nice_load);
define_one_global_rw(freq_step);
define_one_global_rw(freq_step_sleep);				// ZZ: added tuneable
define_one_global_rw(smooth_up);
define_one_global_rw(smooth_up_sleep);				// ZZ: added tuneable
define_one_global_rw(hotplug_sleep);				// ZZ: added tuneable
define_one_global_rw(freq_limit);				// ZZ: added tuneable
define_one_global_rw(freq_limit_sleep);				// ZZ: added tuneable
define_one_global_rw(fast_scaling);				// ZZ: added tuneable
define_one_global_rw(fast_scaling_sleep);			// ZZ: added tuneable
define_one_global_rw(grad_up_threshold);			// ZZ: Early demand tuneable
define_one_global_rw(early_demand);				// ZZ: Early demand tuneable
define_one_global_rw(disable_hotplug);				// ZZ: Hotplug switch
define_one_global_rw(disable_hotplug_sleep);			// ZZ: Hotplug switch for sleep
define_one_global_rw(hotplug_block_cycles);			// ZZ: Hotplug block cycles
define_one_global_rw(hotplug_idle_threshold);			// ZZ: Hotplug idle threshold
define_one_global_rw(scaling_block_threshold);			// ZZ: Scaling block threshold
define_one_global_rw(scaling_block_cycles);			// ZZ: Scaling block cycles
define_one_global_rw(scaling_block_freq);			// ZZ: Scaling block freq
#ifdef ENABLE_LEGACY_MODE
define_one_global_rw(legacy_mode);				// ZZ: Legacy Mode switch
#endif
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
define_one_global_rw(lcdfreq_enable);				// ZZ: LCDFreq Scaling tuneable
define_one_global_rw(lcdfreq_kick_in_down_delay);		// ZZ: LCDFreq Scaling tuneable
define_one_global_rw(lcdfreq_kick_in_up_delay);			// ZZ: LCDFreq Scaling tuneable
define_one_global_rw(lcdfreq_kick_in_freq);			// ZZ: LCDFreq Scaling tuneable
define_one_global_rw(lcdfreq_kick_in_cores);			// ZZ: LCDFreq Scaling tuneable
#endif

// Yank: add version info tunable
static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf) {

    return sprintf(buf, "%s\n", ZZMOOVE_VERSION);

    }

    static DEVICE_ATTR(version, S_IRUGO , show_version, NULL);

// ZZ: profiles version info tunable
static ssize_t show_version_profiles(struct device *dev, struct device_attribute *attr, char *buf) {

    return sprintf(buf, "%s\n", profiles_file_version);

    }

    static DEVICE_ATTR(version_profiles, S_IRUGO , show_version_profiles, NULL);


static struct attribute *dbs_attributes[] = {
	&sampling_rate_min.attr,
	&sampling_rate.attr,
	&sampling_rate_current.attr,				// ZZ: currently active sampling rate
	&sampling_rate_idle_threshold.attr,
	&sampling_rate_idle.attr,
	&sampling_rate_idle_delay.attr,
	&sampling_rate_sleep_multiplier.attr,			// ZZ: added tuneable
	&sampling_down_factor.attr,
	&sampling_down_max_momentum.attr,			// ZZ: Sampling down momentum tuneable
	&sampling_down_momentum_sensitivity.attr,		// ZZ: Sampling down momentum tuneable
	&up_threshold_hotplug1.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq1.attr,			// Yank: added tuneable
#if (MAX_CORES == 4 || MAX_CORES == 8)
	&up_threshold_hotplug2.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq2.attr,			// Yank: added tuneable
	&up_threshold_hotplug3.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq3.attr,			// Yank: added tuneable
#endif
#if (MAX_CORES == 8)
	&up_threshold_hotplug4.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq4.attr,			// Yank: added tuneable
	&up_threshold_hotplug5.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq5.attr,			// Yank: added tuneable
	&up_threshold_hotplug6.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq6.attr,			// Yank: added tuneable
	&up_threshold_hotplug7.attr,				// ZZ: added tuneable
	&up_threshold_hotplug_freq7.attr,			// Yank: added tuneable
#endif
	&down_threshold.attr,
	&down_threshold_sleep.attr,				// ZZ: added tuneable
	&down_threshold_hotplug1.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq1.attr,			// Yank: added tuneable
#if (MAX_CORES == 4 || MAX_CORES == 8)
	&down_threshold_hotplug2.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq2.attr,			// Yank: added tuneable
	&down_threshold_hotplug3.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq3.attr,			// Yank: added tuneable
#endif
#if (MAX_CORES == 8)
	&down_threshold_hotplug4.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq4.attr,			// Yank: added tuneable
	&down_threshold_hotplug5.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq5.attr,			// Yank: added tuneable
	&down_threshold_hotplug6.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq6.attr,			// Yank: added tuneable
	&down_threshold_hotplug7.attr,				// ZZ: added tuneable
	&down_threshold_hotplug_freq7.attr,			// Yank: added tuneable
#endif
	&ignore_nice_load.attr,
	&freq_step.attr,
	&freq_step_sleep.attr,					// ZZ: added tuneable
	&smooth_up.attr,
	&smooth_up_sleep.attr,					// ZZ: added tuneable
	&up_threshold.attr,
	&up_threshold_sleep.attr,				// ZZ: added tuneable
	&hotplug_sleep.attr,					// ZZ: added tuneable
	&freq_limit.attr,					// ZZ: added tuneable
	&freq_limit_sleep.attr,					// ZZ: added tuneable
	&fast_scaling.attr,					// ZZ: added tuneable
	&fast_scaling_sleep.attr,				// ZZ: added tuneable
	&grad_up_threshold.attr,				// ZZ: Early demand tuneable
	&early_demand.attr,					// ZZ: Early demand tuneable
	&disable_hotplug.attr,					// ZZ: Hotplug switch
	&disable_hotplug_sleep.attr,				// ZZ: Hotplug switch sleep
	&hotplug_block_cycles.attr,				// ZZ: Hotplug block cycles
	&hotplug_idle_threshold.attr,				// ZZ: Hotplug idle threshold
	&scaling_block_threshold.attr,				// ZZ: Scaling block threshold
	&scaling_block_cycles.attr,				// ZZ: Scaling block cycles
	&scaling_block_freq.attr,				// ZZ: Scaling block freq
#ifdef ENABLE_LEGACY_MODE
	&legacy_mode.attr,					// ZZ: Legacy Mode switch
#endif
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
	&lcdfreq_enable.attr,					// ZZ: LCD Freq Scaling tuneable
	&lcdfreq_kick_in_down_delay.attr,			// ZZ: LCD Freq Scaling tuneable
	&lcdfreq_kick_in_up_delay.attr,				// ZZ: LCD Freq Scaling tuneable
	&lcdfreq_kick_in_freq.attr,				// ZZ: LCD Freq Scaling tuneable
	&lcdfreq_kick_in_cores.attr,				// ZZ: LCD Freq Scaling tuneable
#endif
	&dev_attr_version.attr,					// Yank: zzmoove version
	&dev_attr_version_profiles.attr,			// ZZ: profiles file version
	&profile.attr,						// ZZ: added tuneable
	&profile_number.attr,					// ZZ: added tuneable
	NULL
};

static struct attribute_group dbs_attr_group = {
	.attrs = dbs_attributes,
	.name = "zzmoove",
};

/************************** sysfs end ************************/

static void dbs_check_cpu(struct cpu_dbs_info_s *this_dbs_info)
{
	unsigned int load = 0;
	unsigned int max_load = 0;
	int boost_freq = 0;					// ZZ: Early demand boost freq flag
	int force_down_scaling = 0;				// ZZ: force down scaling flag
	int cancel_up_scaling = 0;				// ZZ: cancel up scaling flag
	struct cpufreq_policy *policy;
	unsigned int j;

	policy = this_dbs_info->cur_policy;

	/*
	 * ZZ: Frequency Limit: we try here at a verly early stage to limit freqencies above limit by setting the current target_freq to freq_limit.
	 * This could be for example wakeup or touchboost freqencies which could be above the limit and are out of governors control.
	 * This can not avoid the incrementation to these frequencies but should bring it down again earlier. Not sure if that is
	 * a good way to do that or if its realy working. Just an idea - maybe a remove-candidate!
	 */
	if (dbs_tuners_ins.freq_limit != 0 && policy->cur > dbs_tuners_ins.freq_limit)
	__cpufreq_driver_target(policy, dbs_tuners_ins.freq_limit, CPUFREQ_RELATION_L);

	/*
	 * Every sampling_rate, we check, if current idle time is less than 20%
	 * (default), then we try to increase frequency. Every sampling_rate *
	 * sampling_down_factor, we check, if current idle time is more than 80%
	 * (default), then we try to decrease frequency.
	 *
	 * Any frequency increase takes it to the maximum frequency.
	 * Frequency reduction happens at minimum steps of
	 * 5% (default) of maximum frequency
	 */

	/* Get Absolute Load */
	for_each_cpu(j, policy->cpus) {
		struct cpu_dbs_info_s *j_dbs_info;
		cputime64_t cur_wall_time, cur_idle_time;
		unsigned int idle_time, wall_time;

		j_dbs_info = &per_cpu(cs_cpu_dbs_info, j);

		cur_idle_time = get_cpu_idle_time(j, &cur_wall_time);

		wall_time = (unsigned int) cputime64_sub(cur_wall_time,
				j_dbs_info->prev_cpu_wall);
		j_dbs_info->prev_cpu_wall = cur_wall_time;

		idle_time = (unsigned int) cputime64_sub(cur_idle_time,
				j_dbs_info->prev_cpu_idle);
		j_dbs_info->prev_cpu_idle = cur_idle_time;

		if (dbs_tuners_ins.ignore_nice) {
			cputime64_t cur_nice;
			unsigned long cur_nice_jiffies;

			cur_nice = cputime64_sub(kstat_cpu(j).cpustat.nice,
					 j_dbs_info->prev_cpu_nice);
			/*
			 * Assumption: nice time between sampling periods will
			 * be less than 2^32 jiffies for 32 bit sys
			 */
			cur_nice_jiffies = (unsigned long)
					cputime64_to_jiffies64(cur_nice);

			j_dbs_info->prev_cpu_nice = kstat_cpu(j).cpustat.nice;
			idle_time += jiffies_to_usecs(cur_nice_jiffies);
		}

		if (unlikely(!wall_time || wall_time < idle_time))
			continue;

		load = 100 * (wall_time - idle_time) / wall_time;

		if (load > max_load) {
			max_load = load;
			cur_load = load; // ZZ: current load for hotplugging functions
		}

		cur_freq = policy->cur;  // Yank: store current frequency for hotplugging frequency thresholds

	    /*
	     * ZZ: Early demand by stratosk
	     * Calculate the gradient of load. If it is too steep we assume
	     * that the load will go over up_threshold in next iteration(s) and
	     * we increase the frequency immediately
	     */
	    if (dbs_tuners_ins.early_demand) {
               if (max_load > this_dbs_info->prev_load &&
               (max_load - this_dbs_info->prev_load >
               dbs_tuners_ins.grad_up_threshold))
                  boost_freq = 1;
            }

	    /*
	     * ZZ: auto fast scaling mode
	     * switch to all 4 fast scaling modes depending on load gradient
	     * mode will switch on every over-25% load change in both directions
	     */
	    if (dbs_tuners_ins.fast_scaling > 12) {
		if (max_load > this_dbs_info->prev_load && (max_load - this_dbs_info->prev_load < 25)) {
            	    scaling_mode_up = 1;
		} else if (max_load < this_dbs_info->prev_load && (this_dbs_info->prev_load - max_load < 25)) {
            	    scaling_mode_down = 1;
        	} else if (max_load > this_dbs_info->prev_load && (max_load - this_dbs_info->prev_load > 25 && max_load - this_dbs_info->prev_load < 50)) {
            	    scaling_mode_up = 2;
        	} else if (max_load < this_dbs_info->prev_load && (this_dbs_info->prev_load - max_load > 25 && this_dbs_info->prev_load - max_load < 50)) {
            	    scaling_mode_down = 2;
            	} else if (max_load > this_dbs_info->prev_load && (max_load - this_dbs_info->prev_load > 50 && max_load - this_dbs_info->prev_load < 75)) {
            	    scaling_mode_up = 3;
            	} else if (max_load < this_dbs_info->prev_load && (this_dbs_info->prev_load - max_load > 50 && this_dbs_info->prev_load - max_load < 75)) {
            	    scaling_mode_down = 3;
            	} else if (max_load > this_dbs_info->prev_load && (max_load - this_dbs_info->prev_load > 75)) {
            	    scaling_mode_up = 4;
            	} else if (max_load < this_dbs_info->prev_load && (this_dbs_info->prev_load - max_load > 75)) {
            	    scaling_mode_down = 4;
		}
	    }
	
	    /*
	     * ZZ: scaling block for reducing "itchiness"
	     * if the given freq threshold is reached do following:
	     * calculate the gradient of load in both directions count them every time they are under the load threshold,
	     * and block up scaling during that time. if max count of cycles (and therefore threshold hits) is reached switch to "force down mode" which
	     * lowers the freq the next given block cycles. by all that we can avoid "sticking" on max or relatively high frequency (caused by the
	     * very fast scaling behaving of zzmoove) when load is constantly on mid to higher load during a longer peroid.
	     */
	    
	    // ZZ: start blocking if activated and freq threshold is reached
	    if (dbs_tuners_ins.scaling_block_cycles != 0 && policy->cur >= dbs_tuners_ins.scaling_block_freq && suspend_flag == 0 && max_load != 100) {
		
		// ZZ: depending on load threshold count the gradients and block up scaling till max cycles are reached
               if ((scaling_block_cycles_count <= dbs_tuners_ins.scaling_block_cycles && max_load > this_dbs_info->prev_load && max_load - this_dbs_info->prev_load <= dbs_tuners_ins.scaling_block_threshold) ||
               (scaling_block_cycles_count <= dbs_tuners_ins.scaling_block_cycles && max_load < this_dbs_info->prev_load && this_dbs_info->prev_load - max_load <= dbs_tuners_ins.scaling_block_threshold) ||
               dbs_tuners_ins.scaling_block_threshold == 0) {
                    scaling_block_cycles_count++;		// ZZ: count gradients
                    cancel_up_scaling = 1;			// ZZ: block scaling up at the same time
		}
		
		// ZZ: then switch to "force down mode"
                if (scaling_block_cycles_count == dbs_tuners_ins.scaling_block_cycles)			// ZZ: amount of cycles is reached
                    scaling_block_cycles_count = dbs_tuners_ins.scaling_block_cycles * 2;		// ZZ: switch to force down mode

		// ZZ: and force down scaling during next given bock cycles
		if (scaling_block_cycles_count > dbs_tuners_ins.scaling_block_cycles) {
                	if (unlikely(--scaling_block_cycles_count > dbs_tuners_ins.scaling_block_cycles))
                	    force_down_scaling = 1;		// ZZ: force down scaling
                	    else
			    scaling_block_cycles_count = 0;	// ZZ: done - reset counter
		}
		
	    }
	
	// ZZ: used for gradient load calculation in scaling block and early demand
	if (dbs_tuners_ins.early_demand || dbs_tuners_ins.scaling_block_cycles != 0 || dbs_tuners_ins.fast_scaling > 12)
	this_dbs_info->prev_load = max_load;
	}
	
	/*
	 * ZZ: reduction of possible deadlocks - we try here to avoid deadlocks due to double locking from hotplugging and timer mutex
	 * during start/stop/limit events. to be "sure" we skip here 15 times till the locks hopefully are unlocked again. yeah that's dirty
	 * but no better way found yet! ;)
	 */
	if (unlikely(this_dbs_info->check_cpu_skip != 0)) {
		if (++this_dbs_info->check_cpu_skip >= 15)
		    this_dbs_info->check_cpu_skip = 0;
		return;
	}

	/*
	 * break out if we 'cannot' reduce the speed as the user might
	 * want freq_step to be zero
	 */
	if (unlikely(dbs_tuners_ins.freq_step == 0))
		return;

	/*
	 * zzmoove v0.1		- Modification by ZaneZam November 2012
	 *			  Check for frequency increase is greater than hotplug up threshold value and wake up cores accordingly
	 *			  Following will bring up 3 cores in a row (cpu0 stays always on!)
	 *
	 * zzmoove v0.2		- changed hotplug logic to be able to tune up threshold per core and to be able to set
	 *			  cores offline manually via sysfs
	 *
	 * zzmoove v0.5		- fixed non switching cores at 0+2 and 0+3 situations
	 *			- optimized hotplug logic by removing locks and skipping hotplugging if not needed
	 *			- try to avoid deadlocks at critical events by using a flag if we are in the middle of hotplug decision
	 *
	 * zzmoove v0.5.1b	- optimised hotplug logic by reducing code and concentrating only on essential parts
	 *			- preperation for automatic core detection
	 *
	 * zzmoove v0.6		- reduced hotplug loop to a minimum and use seperate functions out of dbs_check_cpu for hotplug work (credits to ktoonsez)
	 *
	 * zzmoove v0.7		- added legacy mode for enabling the "old way of hotplugging" from versions 0.4/0.5
	 *                      - added hotplug idle threshold for automatic disabling of hotplugging when CPU idles
	 *                        (balanced cpu load might bring cooler cpu at that state - inspired by JustArchis observations, thx!)
	 *                      - added hotplug block cycles to reduce hotplug overhead (credits to ktoonesz)
	 *                      - added hotplug frequency thresholds (credits to Yank555)
	 * 
	 * zzmoove v0.7a	- fixed a glitch in hotplug freq threshold tuneables
	 *
	 * zzmoove v0.7d	- fixed hotplug up threshold tuneables to be able again to disable cores manually via sysfs by setting them to 0
	 * 			- fixed the problem caused by a "wrong" tuneable apply order of non sticking values in hotplug down threshold tuneables
	 *			- fixed a typo in hotplug threshold tuneable macros (would have been only a issue in 8-core mode)
	 *			- fixed unwanted disabling of cores when setting hotplug threshold tuneables to lowest or highest possible value 
	 *
	 */

	// ZZ: if hotplug idle threshold is reached and cpu frequency is at its minimum disable hotplug
	if (policy->cur == policy->min && max_load < dbs_tuners_ins.hotplug_idle_threshold && dbs_tuners_ins.hotplug_idle_threshold != 0 && suspend_flag == 0)
	    hotplug_idle_flag = 1;
	else
	    hotplug_idle_flag = 0;

	// ZZ: added block cycles to be able slow down hotplugging
	if ((!dbs_tuners_ins.disable_hotplug && skip_hotplug_flag == 0 && num_online_cpus() != num_possible_cpus() && policy->cur != policy->min) || hotplug_idle_flag == 1) {
	    if (hotplug_up_block_cycles > dbs_tuners_ins.hotplug_block_cycles || dbs_tuners_ins.hotplug_block_cycles == 0) {
		    schedule_work_on(0, &hotplug_online_work);
		    if (dbs_tuners_ins.hotplug_block_cycles != 0)
		    hotplug_up_block_cycles = 0;
	    }
	if (dbs_tuners_ins.hotplug_block_cycles != 0)
	hotplug_up_block_cycles++;
	}

	/* Check for frequency increase */
	if ((max_load >= dbs_tuners_ins.up_threshold && force_down_scaling == 0 && cancel_up_scaling == 0) || boost_freq) { // ZZ: added boost for Early demand and scaling block switch

		// ZZ: Sampling rate idle
		if (dbs_tuners_ins.sampling_rate_idle != 0 && max_load > dbs_tuners_ins.sampling_rate_idle_threshold && suspend_flag == 0 && dbs_tuners_ins.sampling_rate_current != dbs_tuners_ins.sampling_rate) {
		    if (sampling_rate_step_up_delay >= dbs_tuners_ins.sampling_rate_idle_delay) {
		    	dbs_tuners_ins.sampling_rate_current = dbs_tuners_ins.sampling_rate;
			    if (dbs_tuners_ins.sampling_rate_idle_delay != 0)
				    sampling_rate_step_up_delay = 0;
		    }
    		    if (dbs_tuners_ins.sampling_rate_idle_delay != 0)
    			sampling_rate_step_up_delay++;
		}

	    /* ZZ: Sampling down momentum - if momentum is inactive switch to "down_skip" method */
	    if (dbs_tuners_ins.sampling_down_max_mom == 0 && dbs_tuners_ins.sampling_down_factor > 1)
		this_dbs_info->down_skip = 0;

	    /* if we are already at full speed then break out early */
	    if (policy->cur == policy->max) // ZZ: changed check from reqested_freq to current freq (DerTeufel1980)
		return;

	    /* ZZ: Sampling down momentum - if momentum is active and we are switching to max speed, apply sampling_down_factor */
	    if (dbs_tuners_ins.sampling_down_max_mom != 0 && policy->cur < policy->max)
		this_dbs_info->rate_mult = dbs_tuners_ins.sampling_down_factor;

	    /* ZZ: Frequency Limit: if we are at freq_limit break out early */
	    if (dbs_tuners_ins.freq_limit != 0 && policy->cur == dbs_tuners_ins.freq_limit)
		return;

	    /* ZZ: Frequency Limit: try to strictly hold down freqency at freq_limit by spoofing requested freq */
	    if (dbs_tuners_ins.freq_limit != 0 && policy->cur > dbs_tuners_ins.freq_limit) {
		this_dbs_info->requested_freq = dbs_tuners_ins.freq_limit;

#ifdef ENABLE_LEGACY_MODE
		/* ZZ: check if requested freq is higher than max freq if so bring it down to max freq (DerTeufel1980) */
		if (unlikely(this_dbs_info->requested_freq > policy->max) && dbs_tuners_ins.legacy_mode != 0)
		    this_dbs_info->requested_freq = policy->max;
#endif
		__cpufreq_driver_target(policy, this_dbs_info->requested_freq,
				CPUFREQ_RELATION_H);

		    /* ZZ: Sampling down momentum - calculate momentum and update sampling down factor */
		    if (dbs_tuners_ins.sampling_down_max_mom != 0 && this_dbs_info->momentum_adder < dbs_tuners_ins.sampling_down_mom_sens) {
			this_dbs_info->momentum_adder++;
			dbs_tuners_ins.sampling_down_momentum = (this_dbs_info->momentum_adder * dbs_tuners_ins.sampling_down_max_mom) / dbs_tuners_ins.sampling_down_mom_sens;
			dbs_tuners_ins.sampling_down_factor = orig_sampling_down_factor + dbs_tuners_ins.sampling_down_momentum;
		    }
		    return;

	    /* ZZ: Frequency Limit: but let it scale up as normal if the freqencies are lower freq_limit */
	    } else if (dbs_tuners_ins.freq_limit != 0 && policy->cur < dbs_tuners_ins.freq_limit) {

#ifdef ENABLE_LEGACY_MODE
			// ZZ: Legacy Mode
			if (unlikely(dbs_tuners_ins.legacy_mode == true))
				this_dbs_info->requested_freq = leg_get_next_freq(policy->cur, SCALE_FREQ_UP, max_load);
			else
#endif
				this_dbs_info->requested_freq = mn_get_next_freq(policy->cur, SCALE_FREQ_UP, max_load);
#ifdef ENABLE_LEGACY_MODE
		    /* ZZ: check again if we are above limit because of fast scaling */
		    if (dbs_tuners_ins.freq_limit != 0 && this_dbs_info->requested_freq > dbs_tuners_ins.freq_limit && dbs_tuners_ins.legacy_mode != 0)
			this_dbs_info->requested_freq = dbs_tuners_ins.freq_limit;

		    /* ZZ: check if requested freq is higher than max freq if so bring it down to max freq (DerTeufel1980) */
		    if (unlikely(this_dbs_info->requested_freq > policy->max) && dbs_tuners_ins.legacy_mode != 0)
		          this_dbs_info->requested_freq = policy->max;
#endif
    		        __cpufreq_driver_target(policy, this_dbs_info->requested_freq,
				    CPUFREQ_RELATION_H);

		    /* ZZ: Sampling down momentum - calculate momentum and update sampling down factor */
		    if (dbs_tuners_ins.sampling_down_max_mom != 0 && this_dbs_info->momentum_adder < dbs_tuners_ins.sampling_down_mom_sens) {
			this_dbs_info->momentum_adder++;
			dbs_tuners_ins.sampling_down_momentum = (this_dbs_info->momentum_adder * dbs_tuners_ins.sampling_down_max_mom) / dbs_tuners_ins.sampling_down_mom_sens;
			dbs_tuners_ins.sampling_down_factor = orig_sampling_down_factor + dbs_tuners_ins.sampling_down_momentum;
		    }
		return;
	    }

#ifdef ENABLE_LEGACY_MODE
	    // ZZ: Legacy Mode
	    if (unlikely(dbs_tuners_ins.legacy_mode == true))
		this_dbs_info->requested_freq = leg_get_next_freq(policy->cur, SCALE_FREQ_UP, max_load);
	     else
#endif
		this_dbs_info->requested_freq = mn_get_next_freq(policy->cur, SCALE_FREQ_UP, max_load);

#ifdef ENABLE_LEGACY_MODE
	    /* ZZ: check if requested freq is higher than max freq if so bring it down to max freq (DerTeufel1980) */
	    if (unlikely(this_dbs_info->requested_freq > policy->max) && dbs_tuners_ins.legacy_mode != 0)
		 this_dbs_info->requested_freq = policy->max;
#endif
		        __cpufreq_driver_target(policy, this_dbs_info->requested_freq,
				    CPUFREQ_RELATION_H);

	    /* ZZ: Sampling down momentum - calculate momentum and update sampling down factor */
	    if (dbs_tuners_ins.sampling_down_max_mom != 0 && this_dbs_info->momentum_adder < dbs_tuners_ins.sampling_down_mom_sens) {
		this_dbs_info->momentum_adder++;
		dbs_tuners_ins.sampling_down_momentum = (this_dbs_info->momentum_adder * dbs_tuners_ins.sampling_down_max_mom) / dbs_tuners_ins.sampling_down_mom_sens;
		dbs_tuners_ins.sampling_down_factor = orig_sampling_down_factor + dbs_tuners_ins.sampling_down_momentum;
	    }
	return;
	}

#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
	if(dbs_tuners_ins.lcdfreq_enable) {

		// ZZ: LCDFreq Scaling delays
		if( (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // No core threshold, only check freq. threshold
		     dbs_tuners_ins.lcdfreq_kick_in_cores == 0                                ) ||

		    (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // Core threshold reached, check freq. threshold
		     dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&
		     dbs_tuners_ins.lcdfreq_kick_in_cores == num_online_cpus()                ) ||

		    (dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&      // Core threshold passed, no need to check freq. threshold
		     dbs_tuners_ins.lcdfreq_kick_in_cores <  num_online_cpus()                )
		                                                                                ) {

			// We are above threshold, reset down delay, decrement up delay
			if(dbs_tuners_ins.lcdfreq_kick_in_down_left != dbs_tuners_ins.lcdfreq_kick_in_down_delay)
				dbs_tuners_ins.lcdfreq_kick_in_down_left = dbs_tuners_ins.lcdfreq_kick_in_down_delay;
			dbs_tuners_ins.lcdfreq_kick_in_up_left--;

		} else {

			// We are below threshold, reset up delay, decrement down delay
			if(dbs_tuners_ins.lcdfreq_kick_in_up_left != dbs_tuners_ins.lcdfreq_kick_in_up_delay)
				dbs_tuners_ins.lcdfreq_kick_in_up_left = dbs_tuners_ins.lcdfreq_kick_in_up_delay;
			dbs_tuners_ins.lcdfreq_kick_in_down_left--;

		}

		// ZZ: LCDFreq Scaling set frequency if needed
		if(dbs_tuners_ins.lcdfreq_kick_in_up_left <= 0 && lcdfreq_lock_current != 0) {

			// We reached up delay, set frequency to 60Hz
			lcdfreq_lock_current = 0;
			_lcdfreq_lock(lcdfreq_lock_current);

		} else if(dbs_tuners_ins.lcdfreq_kick_in_down_left <= 0 && lcdfreq_lock_current != 1) {

			// We reached down delay, set frequency to 40Hz
			lcdfreq_lock_current = 1;
			_lcdfreq_lock(lcdfreq_lock_current);

		}
	}
#endif

	/*
	 * zzmoove v0.1		- Modification by ZaneZam November 2012
	 *			  Check for frequency decrease is lower than hotplug value and put cores to sleep accordingly
	 *			  Following will disable 3 cores in a row (cpu0 is always on!)
	 *
	 * zzmoove v0.2		- changed logic to be able to tune down threshold per core via sysfs
	 *
	 * zzmoove v0.5		- fixed non switching cores at 0+2 and 0+3 situations
	 *			- optimized hotplug logic by removing locks and skipping hotplugging if not needed
	 *			- try to avoid deadlocks at critical events by using a flag if we are in the middle of hotplug decision
	 *
	 * zzmoove 0.5.1b	- optimised hotplug logic by reducing code and concentrating only on essential parts
	 *			- preperation for automatic core detection
	 *
	 * zzmoove v0.6		- reduced hotplug loop to a minimum and use seperate functions out of dbs_check_cpu for hotplug work (credits to ktoonsez)
	 *
	 * zzmoove v0.7		- added legacy mode for enabling the "old way of hotplugging" from versions 0.4/0.5
	 *                      - added hotplug idle threshold for automatic disabling of hotplugging when CPU idles
	 *                        (balanced cpu load might bring cooler cpu at that state)
	 *                      - added hotplug block cycles to reduce hotplug overhead (credits to ktoonesz)
	 *                      - added hotplug frequency thresholds (credits to Yank555)
	 *
	 * zzmoove v0.7a	- fixed a glitch in hotplug freq threshold tuneables
	 *
	 * zzmoove v0.7d	- fixed hotplug up threshold tuneables to be able again to disable cores manually via sysfs by setting them to 0
	 * 			- fixed the problem caused by a "wrong" tuneable apply order of non sticking values in hotplug down threshold tuneables
	 *			- fixed a typo in hotplug threshold tuneable macros (would have been only a issue in 8-core mode)
	 *			- fixed unwanted disabling of cores when setting hotplug threshold tuneables to lowest or highest possible value
	 */

	// ZZ: added block cycles to be able to slow down hotplugging
	if (!dbs_tuners_ins.disable_hotplug && skip_hotplug_flag == 0 && num_online_cpus() != 1 && hotplug_idle_flag == 0) {
	    if (unlikely(hotplug_down_block_cycles > dbs_tuners_ins.hotplug_block_cycles) || dbs_tuners_ins.hotplug_block_cycles == 0) {
		schedule_work_on(0, &hotplug_offline_work);
		if (dbs_tuners_ins.hotplug_block_cycles != 0)
		hotplug_down_block_cycles = 0;
	    }
	if (dbs_tuners_ins.hotplug_block_cycles != 0)
	hotplug_down_block_cycles++;
	}

	/* ZZ: Sampling down momentum - if momentum is inactive switch to down skip method and if sampling_down_factor is active break out early */
	if (dbs_tuners_ins.sampling_down_max_mom == 0 && dbs_tuners_ins.sampling_down_factor > 1) {
	    if (++this_dbs_info->down_skip < dbs_tuners_ins.sampling_down_factor)
		return;
	this_dbs_info->down_skip = 0;
	}

	/* ZZ: Sampling down momentum - calculate momentum and update sampling down factor */
	if (dbs_tuners_ins.sampling_down_max_mom != 0 && this_dbs_info->momentum_adder > 1) {
	this_dbs_info->momentum_adder -= 2;
	dbs_tuners_ins.sampling_down_momentum = (this_dbs_info->momentum_adder * dbs_tuners_ins.sampling_down_max_mom) / dbs_tuners_ins.sampling_down_mom_sens;
	dbs_tuners_ins.sampling_down_factor = orig_sampling_down_factor + dbs_tuners_ins.sampling_down_momentum;
	}

	 /* Check for frequency decrease */
	if (max_load < dbs_tuners_ins.down_threshold || (force_down_scaling == 1 && boost_freq == 0)) { // ZZ: added force down switch

		// ZZ: Sampling rate idle
		if (dbs_tuners_ins.sampling_rate_idle != 0 && max_load < dbs_tuners_ins.sampling_rate_idle_threshold && suspend_flag == 0 && dbs_tuners_ins.sampling_rate_current != dbs_tuners_ins.sampling_rate_idle) {
		    if (sampling_rate_step_down_delay >= dbs_tuners_ins.sampling_rate_idle_delay) {
        		dbs_tuners_ins.sampling_rate_current = dbs_tuners_ins.sampling_rate_idle;
			if (dbs_tuners_ins.sampling_rate_idle_delay != 0)
			sampling_rate_step_down_delay = 0;
		    }
		if (dbs_tuners_ins.sampling_rate_idle_delay != 0)
		sampling_rate_step_down_delay++;
		}		

	    /* ZZ: Sampling down momentum - No longer fully busy, reset rate_mult */
	    this_dbs_info->rate_mult = 1;

		/* if we cannot reduce the frequency anymore, break out early */
		if (policy->cur == policy->min)
			return;

	/* ZZ: Frequency Limit: this should bring down freqency faster if we are coming from above limit (eg. touchboost/wakeup freqencies) */
	if (dbs_tuners_ins.freq_limit != 0 && policy->cur > dbs_tuners_ins.freq_limit) {
		this_dbs_info->requested_freq = dbs_tuners_ins.freq_limit;

#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
		if(dbs_tuners_ins.lcdfreq_enable) {

			// ZZ: LCDFreq Scaling delays
			if( (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // No core threshold, only check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores == 0                                ) ||

			    (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // Core threshold reached, check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&
			     dbs_tuners_ins.lcdfreq_kick_in_cores == num_online_cpus()                ) ||

			    (dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&      // Core threshold passed, no need to check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores <  num_online_cpus()                )
			                                                                                   ) {

				// We are above threshold, reset down delay, decrement up delay
				if(dbs_tuners_ins.lcdfreq_kick_in_down_left != dbs_tuners_ins.lcdfreq_kick_in_down_delay)
					dbs_tuners_ins.lcdfreq_kick_in_down_left = dbs_tuners_ins.lcdfreq_kick_in_down_delay;
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left > 0)
				dbs_tuners_ins.lcdfreq_kick_in_up_left--;

			} else {

				// We are below threshold, reset up delay, decrement down delay
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left != dbs_tuners_ins.lcdfreq_kick_in_up_delay)
					dbs_tuners_ins.lcdfreq_kick_in_up_left = dbs_tuners_ins.lcdfreq_kick_in_up_delay;
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left > 0)
				dbs_tuners_ins.lcdfreq_kick_in_down_left--;

			}

			// ZZ: LCDFreq Scaling set frequency if needed
			if(dbs_tuners_ins.lcdfreq_kick_in_up_left <= 0 && lcdfreq_lock_current != 0) {

				// We reached up delay, set frequency to 60Hz
				lcdfreq_lock_current = 0;
				_lcdfreq_lock(lcdfreq_lock_current);

			} else if(dbs_tuners_ins.lcdfreq_kick_in_down_left <= 0 && lcdfreq_lock_current != 1) {

				// We reached down delay, set frequency to 40Hz
				lcdfreq_lock_current = 1;
				_lcdfreq_lock(lcdfreq_lock_current);

			}
		}
#endif
		__cpufreq_driver_target(policy, this_dbs_info->requested_freq,
					CPUFREQ_RELATION_L);
		return;

	/* ZZ: Frequency Limit: else we scale down as usual */
	} else if (dbs_tuners_ins.freq_limit != 0 && policy->cur <= dbs_tuners_ins.freq_limit) {

#ifdef ENABLE_LEGACY_MODE
		    // ZZ: Legacy Mode
		    if (unlikely(dbs_tuners_ins.legacy_mode == true))
			this_dbs_info->requested_freq = leg_get_next_freq(policy->cur, SCALE_FREQ_DOWN, max_load);
		    else
#endif
			this_dbs_info->requested_freq = mn_get_next_freq(policy->cur, SCALE_FREQ_DOWN, max_load);

#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
		if(dbs_tuners_ins.lcdfreq_enable) {

			// ZZ: LCDFreq Scaling delays
			if( (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // No core threshold, only check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores == 0                                ) ||

			    (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // Core threshold reached, check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&
			     dbs_tuners_ins.lcdfreq_kick_in_cores == num_online_cpus()                ) ||

			    (dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&      // Core threshold passed, no need to check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores <  num_online_cpus()                )
			                                                                                   ) {

				// We are above threshold, reset down delay, decrement up delay
				if(dbs_tuners_ins.lcdfreq_kick_in_down_left != dbs_tuners_ins.lcdfreq_kick_in_down_delay)
					dbs_tuners_ins.lcdfreq_kick_in_down_left = dbs_tuners_ins.lcdfreq_kick_in_down_delay;
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left > 0)
				dbs_tuners_ins.lcdfreq_kick_in_up_left--;

			} else {

				// We are below threshold, reset up delay, decrement down delay
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left != dbs_tuners_ins.lcdfreq_kick_in_up_delay)
					dbs_tuners_ins.lcdfreq_kick_in_up_left = dbs_tuners_ins.lcdfreq_kick_in_up_delay;
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left > 0)
				dbs_tuners_ins.lcdfreq_kick_in_down_left--;

			}

			// ZZ: LCDFreq Scaling set frequency if needed
			if(dbs_tuners_ins.lcdfreq_kick_in_up_left <= 0 && lcdfreq_lock_current != 0) {

				// We reached up delay, set frequency to 60Hz
				lcdfreq_lock_current = 0;
				_lcdfreq_lock(lcdfreq_lock_current);

			} else if(dbs_tuners_ins.lcdfreq_kick_in_down_left <= 0 && lcdfreq_lock_current != 1) {

				// We reached down delay, set frequency to 40Hz
				lcdfreq_lock_current = 1;
				_lcdfreq_lock(lcdfreq_lock_current);

			}
		}
#endif
		__cpufreq_driver_target(policy, this_dbs_info->requested_freq,
					CPUFREQ_RELATION_L); // ZZ: changed to relation low
		return;
	}

#ifdef ENABLE_LEGACY_MODE
	    // ZZ: Legacy Mode
	    if (unlikely(dbs_tuners_ins.legacy_mode == true))
		this_dbs_info->requested_freq = leg_get_next_freq(policy->cur, SCALE_FREQ_DOWN, max_load);
	    else
#endif
		this_dbs_info->requested_freq = mn_get_next_freq(policy->cur, SCALE_FREQ_DOWN, max_load);

#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
		if(dbs_tuners_ins.lcdfreq_enable) {

			// ZZ: LCDFreq Scaling delays
			if( (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // No core threshold, only check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores == 0                                ) ||

			    (dbs_tuners_ins.lcdfreq_kick_in_freq  <= this_dbs_info->requested_freq &&      // Core threshold reached, check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&
			     dbs_tuners_ins.lcdfreq_kick_in_cores == num_online_cpus()                ) ||

			    (dbs_tuners_ins.lcdfreq_kick_in_cores != 0                             &&      // Core threshold passed, no need to check freq. threshold
			     dbs_tuners_ins.lcdfreq_kick_in_cores <  num_online_cpus()                )
			                                                                                   ) {

				// We are above threshold, reset down delay, decrement up delay
				if(dbs_tuners_ins.lcdfreq_kick_in_down_left != dbs_tuners_ins.lcdfreq_kick_in_down_delay)
					dbs_tuners_ins.lcdfreq_kick_in_down_left = dbs_tuners_ins.lcdfreq_kick_in_down_delay;
				dbs_tuners_ins.lcdfreq_kick_in_up_left--;

			} else {

				// We are below threshold, reset up delay, decrement down delay
				if(dbs_tuners_ins.lcdfreq_kick_in_up_left != dbs_tuners_ins.lcdfreq_kick_in_up_delay)
					dbs_tuners_ins.lcdfreq_kick_in_up_left = dbs_tuners_ins.lcdfreq_kick_in_up_delay;
				dbs_tuners_ins.lcdfreq_kick_in_down_left--;

			}

			// ZZ: LCDFreq Scaling set frequency if needed
			if(dbs_tuners_ins.lcdfreq_kick_in_up_left <= 0 && lcdfreq_lock_current != 0) {

				// We reached up delay, set frequency to 60Hz
				lcdfreq_lock_current = 0;
				_lcdfreq_lock(lcdfreq_lock_current);

			} else if(dbs_tuners_ins.lcdfreq_kick_in_down_left <= 0 && lcdfreq_lock_current != 1) {

				// We reached down delay, set frequency to 40Hz
				lcdfreq_lock_current = 1;
				_lcdfreq_lock(lcdfreq_lock_current);

			}
		}
#endif
		__cpufreq_driver_target(policy, this_dbs_info->requested_freq,
					CPUFREQ_RELATION_L); // ZZ: changed to relation low
		return;
	}
}

// ZZ: function for hotplug down work
static void __cpuinit hotplug_offline_work_fn(struct work_struct *work)
{
	int i=0;

#ifdef ENABLE_LEGACY_MODE
// ZZ: Legacy hotplugging
	if (unlikely(dbs_tuners_ins.legacy_mode == true && num_possible_cpus() == 4)) {
	    if (num_online_cpus() > 3) {
		if (cur_load < hotplug_thresholds[1][2] && skip_hotplug_flag == 0 && cpu_online(3))
		    cpu_down(3);
		if (cur_load < hotplug_thresholds[1][1] && skip_hotplug_flag == 0 && cpu_online(2))
		    cpu_down(2);
		if (cur_load < hotplug_thresholds[1][0] && skip_hotplug_flag == 0 && cpu_online(1))
		    cpu_down(1);
	    } else if (num_online_cpus() > 2) {
		if (cur_load < hotplug_thresholds[1][1] && skip_hotplug_flag == 0 && cpu_online(2))
		    cpu_down(2);
		if (cur_load < hotplug_thresholds[1][0] && skip_hotplug_flag == 0 && cpu_online(1))
		    cpu_down(1);
	    } else if (num_online_cpus() > 1 && cpu_online(2)) {
		if (cur_load < hotplug_thresholds[1][1] && skip_hotplug_flag == 0)
		    cpu_down(2);
	    } else if (num_online_cpus() > 1 && cpu_online(3)) {
		if (cur_load < hotplug_thresholds[1][2] && skip_hotplug_flag == 0)
		cpu_down(3);
	    } else if (num_online_cpus() > 1) {
		if (cur_load < hotplug_thresholds[1][0] && skip_hotplug_flag == 0 && cpu_online(1))
		    cpu_down(1);
	    }

	} else {
#endif
// Yank: added frequency thresholds
	    for (i = num_possible_cpus() - 1; i >= 1; i--) {
		    if( cpu_online(i)										&&
			skip_hotplug_flag == 0									&&
			cur_load <= hotplug_thresholds[1][i-1]							&&
			(hotplug_thresholds_freq[1][i-1] == 0 || cur_freq <= hotplug_thresholds_freq[1][i-1])       )
			    cpu_down(i);
	    }
#ifdef ENABLE_LEGACY_MODE
    }
#endif
}

// ZZ: function for hotplug up work
static void __cpuinit hotplug_online_work_fn(struct work_struct *work)
{
int i = 0;
	// ZZ: enable offline cores to avoid higher / achieve balanced cpu load on idle
	if (unlikely(hotplug_idle_flag == 1 || enable_cores_on_exit == 1)){
	enable_offline_cores();
	return;
	}

#ifdef ENABLE_LEGACY_MODE
	// ZZ: Legacy hotplugging
	if (unlikely(dbs_tuners_ins.legacy_mode == true && num_possible_cpus() == 4)) {
		if (num_online_cpus() < 2) {
		    if (hotplug_thresholds[0][0] != 0 && cur_load > hotplug_thresholds[0][0] && skip_hotplug_flag == 0 && !cpu_online(1))
			cpu_up(1);
		    if (hotplug_thresholds[0][1] != 0 && cur_load > hotplug_thresholds[0][1] && skip_hotplug_flag == 0 && !cpu_online(2))
			cpu_up(2);
		    if (hotplug_thresholds[0][2] != 0 && cur_load > hotplug_thresholds[0][2] && skip_hotplug_flag == 0 && !cpu_online(3))
			cpu_up(3);
		} else if (num_online_cpus() < 3 && cpu_online(3)) {
		    if (hotplug_thresholds[0][0] != 0 && cur_load > hotplug_thresholds[0][0] && skip_hotplug_flag == 0 && !cpu_online(1))
			cpu_up(1);
		    if (hotplug_thresholds[0][1] != 0 && cur_load > hotplug_thresholds[0][1] && skip_hotplug_flag == 0 && !cpu_online(2))
			cpu_up(2);
		} else if (num_online_cpus() < 3 && cpu_online(2)) {
		    if (hotplug_thresholds[0][0] != 0 && cur_load > hotplug_thresholds[0][0] && skip_hotplug_flag == 0 && !cpu_online(1))
			cpu_up(1);
		    if (hotplug_thresholds[0][2] != 0 && cur_load > hotplug_thresholds[0][2] && skip_hotplug_flag == 0 && !cpu_online(3))
			cpu_up(3);
		} else if (num_online_cpus() < 3) {
		    if (hotplug_thresholds[0][1] != 0 && cur_load > hotplug_thresholds[0][1] && skip_hotplug_flag == 0 && !cpu_online(2))
			cpu_up(2);
		    if (hotplug_thresholds[0][2] != 0 && cur_load > hotplug_thresholds[0][2] && skip_hotplug_flag == 0 && !cpu_online(3))
			cpu_up(3);
		} else if (num_online_cpus() < 4) {
		    if (hotplug_thresholds[0][2] != 0 && cur_load > hotplug_thresholds[0][2] && skip_hotplug_flag == 0 && !cpu_online(3))
			cpu_up(3);
		}

	} else {
#endif
	    // Yank: added frequency thresholds
	    for (i = 1; i < num_possible_cpus(); i++) {
		    if( !cpu_online(i)										&&
			skip_hotplug_flag == 0									&&
			hotplug_thresholds[0][i-1] != 0								&&
			cur_load >= hotplug_thresholds[0][i-1]							&&
			(hotplug_thresholds_freq[0][i-1] == 0 || cur_freq >= hotplug_thresholds_freq[0][i-1])       )
			    cpu_up(i);
	    }
#ifdef ENABLE_LEGACY_MODE
	}
#endif
}

static void do_dbs_timer(struct work_struct *work)
{
	struct cpu_dbs_info_s *dbs_info =
		container_of(work, struct cpu_dbs_info_s, work.work);
	unsigned int cpu = dbs_info->cpu;

	/* We want all CPUs to do sampling nearly on same jiffy */
	int delay = usecs_to_jiffies(dbs_tuners_ins.sampling_rate_current * dbs_info->rate_mult); // ZZ: Sampling down momentum - added multiplier

	delay -= jiffies % delay;

	mutex_lock(&dbs_info->timer_mutex);

	dbs_check_cpu(dbs_info);

	schedule_delayed_work_on(cpu, &dbs_info->work, delay);
	mutex_unlock(&dbs_info->timer_mutex);
}

static inline void dbs_timer_init(struct cpu_dbs_info_s *dbs_info)
{
	/* We want all CPUs to do sampling nearly on same jiffy */
	int delay = usecs_to_jiffies(dbs_tuners_ins.sampling_rate_current);
	delay -= jiffies % delay;

	dbs_info->enable = 1;
	INIT_DELAYED_WORK_DEFERRABLE(&dbs_info->work, do_dbs_timer);
	schedule_delayed_work_on(dbs_info->cpu, &dbs_info->work, delay);
}

static inline void dbs_timer_exit(struct cpu_dbs_info_s *dbs_info)
{
	dbs_info->enable = 0;
	cancel_delayed_work(&dbs_info->work); // ZZ: Use asyncronous mode to avoid freezes / reboots when leaving zzmoove
}

static void powersave_early_suspend(struct early_suspend *handler)
{
  int i=0;
  struct cpufreq_frequency_table *table;				// Yank : Use system frequency table
  skip_hotplug_flag = 1;						// ZZ: try to avoid deadlock by disabling hotplugging if we are in the middle of hotplugging logic
  suspend_flag = 1;							// ZZ: we want to know if we are at suspend because of things that shouldn't be executed at suspend
  for (i = 0; i < 1000; i++);						// ZZ: wait a few samples to be sure hotplugging is off (never be sure so this is dirty)

  table = cpufreq_frequency_get_table(0);				// Yank : Get system frequency table

  mutex_lock(&dbs_mutex);
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
	prev_lcdfreq_enable = dbs_tuners_ins.lcdfreq_enable;		// ZZ: LCDFreq Scaling - store state
	prev_lcdfreq_lock_current = lcdfreq_lock_current;		// ZZ: LCDFreq Scaling - store lock current
	if(dbs_tuners_ins.lcdfreq_enable) {				// ZZ: LCDFreq Scaling - reset display freq. to 60Hz only if it was enabled
		dbs_tuners_ins.lcdfreq_enable = false;
		lcdfreq_lock_current = 0;
		_lcdfreq_lock(lcdfreq_lock_current);
	}
#endif
  sampling_rate_awake = dbs_tuners_ins.sampling_rate_current;
  up_threshold_awake = dbs_tuners_ins.up_threshold;
  down_threshold_awake = dbs_tuners_ins.down_threshold;
  dbs_tuners_ins.sampling_down_max_mom = 0;				// ZZ: Sampling down momentum - disabled at suspend
  smooth_up_awake = dbs_tuners_ins.smooth_up;
  freq_step_awake = dbs_tuners_ins.freq_step;				// ZZ: save freq step
  freq_limit_awake = dbs_tuners_ins.freq_limit;				// ZZ: save freq limit
  fast_scaling_awake = dbs_tuners_ins.fast_scaling;			// ZZ: save scaling setting
  disable_hotplug_awake = dbs_tuners_ins.disable_hotplug;		// ZZ: save hotplug switch state

  if (dbs_tuners_ins.hotplug_sleep != 0) {				// ZZ: if set to 0 do not touch hotplugging values
	hotplug1_awake = dbs_tuners_ins.up_threshold_hotplug1;		// ZZ: save hotplug1 value for restore on awake
#if (MAX_CORES == 4 || MAX_CORES == 8)
	hotplug2_awake = dbs_tuners_ins.up_threshold_hotplug2;		// ZZ: save hotplug2 value for restore on awake
	hotplug3_awake = dbs_tuners_ins.up_threshold_hotplug3;		// ZZ: save hotplug3 value for restore on awake
#endif
#if (MAX_CORES == 8)
	hotplug4_awake = dbs_tuners_ins.up_threshold_hotplug4;		// ZZ: save hotplug4 value for restore on awake
	hotplug5_awake = dbs_tuners_ins.up_threshold_hotplug5;		// ZZ: save hotplug5 value for restore on awake
	hotplug6_awake = dbs_tuners_ins.up_threshold_hotplug6;		// ZZ: save hotplug6 value for restore on awake
	hotplug7_awake = dbs_tuners_ins.up_threshold_hotplug7;		// ZZ: save hotplug7 value for restore on awake
#endif
  }

  sampling_rate_asleep = dbs_tuners_ins.sampling_rate_sleep_multiplier; // ZZ: save sleep multiplier
  up_threshold_asleep = dbs_tuners_ins.up_threshold_sleep;		// ZZ: save up threshold
  down_threshold_asleep = dbs_tuners_ins.down_threshold_sleep;		// ZZ: save down threshold
  smooth_up_asleep = dbs_tuners_ins.smooth_up_sleep;			// ZZ: save smooth up
  freq_step_asleep = dbs_tuners_ins.freq_step_sleep;			// ZZ: save frequency step
  freq_limit_asleep = dbs_tuners_ins.freq_limit_sleep;			// ZZ: save frequency limit
  fast_scaling_asleep = dbs_tuners_ins.fast_scaling_sleep;		// ZZ: save fast scaling
  disable_hotplug_asleep = dbs_tuners_ins.disable_hotplug_sleep;	// ZZ: save disable hotplug switch
  dbs_tuners_ins.sampling_rate_current *= sampling_rate_asleep;		// ZZ: set sampling rate
  dbs_tuners_ins.up_threshold = up_threshold_asleep;			// ZZ: set up threshold
  dbs_tuners_ins.down_threshold = down_threshold_asleep;		// ZZ: set down threshold
  dbs_tuners_ins.smooth_up = smooth_up_asleep;				// ZZ: set smooth up
  dbs_tuners_ins.freq_step = freq_step_asleep;				// ZZ: set freqency step
  dbs_tuners_ins.freq_limit = freq_limit_asleep;			// ZZ: set freqency limit
  dbs_tuners_ins.fast_scaling = fast_scaling_asleep;			// ZZ: set fast scaling
  dbs_tuners_ins.disable_hotplug = disable_hotplug_asleep;		// ZZ: set hotplug switch

  if (dbs_tuners_ins.disable_hotplug_sleep) {				// ZZ: enable all cores at suspend if disable hotplug sleep is set
      enable_offline_cores();
  }

	if (dbs_tuners_ins.fast_scaling > 12) {				// ZZ: set scaling mode
	    scaling_mode_up   = 0;					// ZZ: auto fast scaling
	    scaling_mode_down = 0;					// ZZ: auto fast scaling

	} else if (dbs_tuners_ins.fast_scaling > 8) {				// ZZ: set scaling mode
	    scaling_mode_up   = 0;					// ZZ: normal up scaling
	    scaling_mode_down = dbs_tuners_ins.fast_scaling - 8;	// ZZ: fast scaling down only
	
	} else if (dbs_tuners_ins.fast_scaling > 4) {
	    scaling_mode_up   = dbs_tuners_ins.fast_scaling - 4;	// Yank : fast scaling up
	    scaling_mode_down = dbs_tuners_ins.fast_scaling - 4;	// Yank : fast scaling down
	
	} else {
	    scaling_mode_up   = dbs_tuners_ins.fast_scaling;		// Yank : fast scaling up only
	    scaling_mode_down = 0;					// Yank : normal scaling down
	}

  if (freq_limit_asleep == 0 ||						// Yank : if there is no sleep freq. limit
      freq_limit_asleep > table[max_scaling_freq_hard].frequency) {	// Yank : or it is higher than hard max freq.
	max_scaling_freq_soft = max_scaling_freq_hard;			// Yank : use hard max freq.
	if (freq_table_order == 1)					// ZZ: if descending ordered table is used
	    limit_table_start = max_scaling_freq_soft;			// ZZ: we should use the actual scaling soft limit value as search start point
	else
	    limit_table_end = table[freq_table_size].frequency;		// ZZ: set search end point to max freq when using ascending table
  } else {
	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++) {
		if (freq_limit_asleep == table[i].frequency) {		// Yank : else lookup sleep max. frequency index
			max_scaling_freq_soft = i;
			if (freq_table_order == 1)			// ZZ: if descending ordered table is used
			    limit_table_start = max_scaling_freq_soft;	// ZZ: we should use the actual scaling soft limit value as search start point
			else
			    limit_table_end = table[i].frequency;	// ZZ: set search end point to max freq when using ascending table
			break;
		}
	}
  }

  if (dbs_tuners_ins.hotplug_sleep != 0) {				// ZZ: if set to 0 do not touch hotplugging values
	if (dbs_tuners_ins.hotplug_sleep == 1) {
	    dbs_tuners_ins.up_threshold_hotplug1 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][0] = 0;
#if (MAX_CORES == 4 || MAX_CORES == 8)
	    dbs_tuners_ins.up_threshold_hotplug2 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][1] = 0;
	    dbs_tuners_ins.up_threshold_hotplug3 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][2] = 0;
#endif
#if (MAX_CORES == 8)
	    dbs_tuners_ins.up_threshold_hotplug4 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][3] = 0;
	    dbs_tuners_ins.up_threshold_hotplug5 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][4] = 0;
	    dbs_tuners_ins.up_threshold_hotplug6 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][5] = 0;
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to one core
	    hotplug_thresholds[0][6] = 0;
#endif
	}
#if (MAX_CORES == 4 || MAX_CORES == 8)
	if (dbs_tuners_ins.hotplug_sleep == 2) {
	    dbs_tuners_ins.up_threshold_hotplug2 = 0;			// ZZ: set to two cores
	    hotplug_thresholds[0][1] = 0;
	    dbs_tuners_ins.up_threshold_hotplug3 = 0;			// ZZ: set to two cores
	    hotplug_thresholds[0][2] = 0;
#endif
#if (MAX_CORES == 8)
	    dbs_tuners_ins.up_threshold_hotplug4 = 0;			// ZZ: set to two cores
	    hotplug_thresholds[0][3] = 0;
	    dbs_tuners_ins.up_threshold_hotplug5 = 0;			// ZZ: set to two cores
	    hotplug_thresholds[0][4] = 0;
	    dbs_tuners_ins.up_threshold_hotplug6 = 0;			// ZZ: set to two cores
	    hotplug_thresholds[0][5] = 0;
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to two cores
	    hotplug_thresholds[0][6] = 0;
#endif
#if (MAX_CORES == 4 || MAX_CORES == 8)
	}
	if (dbs_tuners_ins.hotplug_sleep == 3) {
	    dbs_tuners_ins.up_threshold_hotplug3 = 0;			// ZZ: set to three cores
	    hotplug_thresholds[0][2] = 0;
#endif
#if (MAX_CORES == 8)
	    dbs_tuners_ins.up_threshold_hotplug4 = 0;			// ZZ: set to three cores
	    hotplug_thresholds[0][3] = 0;
	    dbs_tuners_ins.up_threshold_hotplug5 = 0;			// ZZ: set to three cores
	    hotplug_thresholds[0][4] = 0;
	    dbs_tuners_ins.up_threshold_hotplug6 = 0;			// ZZ: set to three cores
	    hotplug_thresholds[0][5] = 0;
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to three cores
	    hotplug_thresholds[0][6] = 0;
#endif
#if (MAX_CORES == 4 || MAX_CORES == 8)
	}
#endif
#if (MAX_CORES == 8)
	if (dbs_tuners_ins.hotplug_sleep == 4) {
	    dbs_tuners_ins.up_threshold_hotplug4 = 0;			// ZZ: set to four cores
	    hotplug_thresholds[0][3] = 0;
	    dbs_tuners_ins.up_threshold_hotplug5 = 0;			// ZZ: set to four cores
	    hotplug_thresholds[0][4] = 0;
	    dbs_tuners_ins.up_threshold_hotplug6 = 0;			// ZZ: set to four cores
	    hotplug_thresholds[0][5] = 0;
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to four cores
	    hotplug_thresholds[0][6] = 0;
	}
	if (dbs_tuners_ins.hotplug_sleep == 5) {
	    dbs_tuners_ins.up_threshold_hotplug5 = 0;			// ZZ: set to five cores
	    hotplug_thresholds[0][4] = 0;
	    dbs_tuners_ins.up_threshold_hotplug6 = 0;			// ZZ: set to five cores
	    hotplug_thresholds[0][5] = 0;
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to five cores
	    hotplug_thresholds[0][6] = 0;
	}
	if (dbs_tuners_ins.hotplug_sleep == 6) {
	    dbs_tuners_ins.up_threshold_hotplug6 = 0;			// ZZ: set to six cores
	    hotplug_thresholds[0][5] = 0;
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to six cores
	    hotplug_thresholds[0][6] = 0;
	}
	if (dbs_tuners_ins.hotplug_sleep == 7) {
	    dbs_tuners_ins.up_threshold_hotplug7 = 0;			// ZZ: set to seven cores
	    hotplug_thresholds[0][6] = 0;
	}
#endif
  }

  mutex_unlock(&dbs_mutex);
  for (i = 0; i < 1000; i++);						// ZZ: wait a few samples to be sure hotplugging is off (never be sure so this is dirty)
  skip_hotplug_flag = 0;						// ZZ: enable hotplugging again

}

static void powersave_late_resume(struct early_suspend *handler)
{
  int i=0;
  struct cpufreq_frequency_table *table;				// Yank : Use system frequency table
  skip_hotplug_flag = 1;						// ZZ: same as above skip hotplugging to avoid deadlocks
  suspend_flag = 0;							// ZZ: we are resuming so reset supend flag

  if (!dbs_tuners_ins.disable_hotplug_sleep) {
    enable_offline_cores();						// ZZ: enable offline cores to avoid stuttering after resume if hotplugging limit was active
  }

  for (i = 0; i < 1000; i++);						// ZZ: wait a few samples to be sure hotplugging is off (never be sure so this is dirty)

  table = cpufreq_frequency_get_table(0);				// Yank : Get system frequency table

 mutex_lock(&dbs_mutex);
#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
	dbs_tuners_ins.lcdfreq_enable = prev_lcdfreq_enable;		// ZZ: LCDFreq Scaling - enable it again if it was enabled
	if(dbs_tuners_ins.lcdfreq_enable) {				// ZZ: LCDFreq Scaling - restore display freq. only if it was enabled before suspend
		lcdfreq_lock_current = prev_lcdfreq_lock_current;
		_lcdfreq_lock(lcdfreq_lock_current);
	}
#endif

   if (dbs_tuners_ins.hotplug_sleep != 0) {
	dbs_tuners_ins.up_threshold_hotplug1 = hotplug1_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][0] = hotplug1_awake;
#if (MAX_CORES == 4 || MAX_CORES == 8)
	dbs_tuners_ins.up_threshold_hotplug2 = hotplug2_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][1] = hotplug2_awake;
	dbs_tuners_ins.up_threshold_hotplug3 = hotplug3_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][2] = hotplug3_awake;
#endif
#if (MAX_CORES == 8)
	dbs_tuners_ins.up_threshold_hotplug4 = hotplug4_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][3] = hotplug4_awake;
	dbs_tuners_ins.up_threshold_hotplug5 = hotplug5_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][4] = hotplug5_awake;
	dbs_tuners_ins.up_threshold_hotplug6 = hotplug6_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][5] = hotplug6_awake;
	dbs_tuners_ins.up_threshold_hotplug7 = hotplug7_awake;		// ZZ: restore previous settings
	hotplug_thresholds[0][6] = hotplug7_awake;
#endif
  }

  dbs_tuners_ins.sampling_down_max_mom = orig_sampling_down_max_mom;	// ZZ: Sampling down momentum - restore max value
  dbs_tuners_ins.sampling_rate_current = sampling_rate_awake;		// ZZ: restore previous settings
  dbs_tuners_ins.up_threshold = up_threshold_awake;			// ZZ: restore previous settings
  dbs_tuners_ins.down_threshold = down_threshold_awake;			// ZZ: restore previous settings
  dbs_tuners_ins.smooth_up = smooth_up_awake;				// ZZ: restore previous settings
  dbs_tuners_ins.freq_step = freq_step_awake;				// ZZ: restore previous settings
  dbs_tuners_ins.freq_limit = freq_limit_awake;				// ZZ: restore previous settings
  dbs_tuners_ins.fast_scaling = fast_scaling_awake;			// ZZ: restore previous settings
  dbs_tuners_ins.disable_hotplug = disable_hotplug_awake;		// ZZ: restore previous settings

	if (dbs_tuners_ins.fast_scaling > 12) {				// ZZ: set scaling mode
	    scaling_mode_up   = 0;					// ZZ: auto fast scaling
	    scaling_mode_down = 0;					// ZZ: auto fast scaling

	} else if (dbs_tuners_ins.fast_scaling > 8) {				// ZZ: set scaling mode
	    scaling_mode_up   = 0;					// ZZ: normal up scaling
	    scaling_mode_down = dbs_tuners_ins.fast_scaling - 8;	// ZZ: fast scaling down only
	
	} else if (dbs_tuners_ins.fast_scaling > 4) {
	    scaling_mode_up   = dbs_tuners_ins.fast_scaling - 4;	// Yank : fast scaling up
	    scaling_mode_down = dbs_tuners_ins.fast_scaling - 4;	// Yank : fast scaling down
	
	} else {
	    scaling_mode_up   = dbs_tuners_ins.fast_scaling;		// Yank : fast scaling up only
	    scaling_mode_down = 0;					// Yank : normal scaling down
	}

  if (freq_limit_awake == 0 ||						// Yank : if there is no awake freq. limit
      freq_limit_awake > table[max_scaling_freq_hard].frequency) {	// Yank : or it is higher than hard max freq.
	max_scaling_freq_soft = max_scaling_freq_hard;			// Yank : use hard max freq.
	    if (freq_table_order == 1)					// ZZ: if descending ordered table is used
		limit_table_start = max_scaling_freq_soft;		// ZZ: we should use the actual scaling soft limit value as search start point
	    else
		limit_table_end = table[freq_table_size].frequency;	// ZZ: set search end point to max freq when using ascending table
  } else {
	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++) {
		if (freq_limit_awake == table[i].frequency) {		// Yank : else lookup awake max. frequency index
			max_scaling_freq_soft = i;
			if (freq_table_order == 1)			// ZZ: if descending ordered table is used
			    limit_table_start = max_scaling_freq_soft;	// ZZ: we should use the actual scaling soft limit value as search start point
			else
			    limit_table_end = table[i].frequency;	// ZZ: set search end point to soft freq limit when using ascending table
			break;
		}
	}
  }
  mutex_unlock(&dbs_mutex);
  for (i = 0; i < 1000; i++);						// ZZ: wait a few samples to be sure hotplugging is off (never be sure so this is dirty)
  skip_hotplug_flag = 0;						// ZZ: enable hotplugging again
}

static struct early_suspend _powersave_early_suspend = {
  .suspend = powersave_early_suspend,
  .resume = powersave_late_resume,
  .level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
};

static int cpufreq_governor_dbs(struct cpufreq_policy *policy,
				   unsigned int event)
{
	unsigned int cpu = policy->cpu;
	struct cpu_dbs_info_s *this_dbs_info;
	struct cpufreq_frequency_table *table; // Yank : Use system frequency table
	unsigned int j;
	int rc;
	int i=0;
	int calc_index=0;

	this_dbs_info = &per_cpu(cs_cpu_dbs_info, cpu);

	table = cpufreq_frequency_get_table(0); // Yank : Get system frequency table

	switch (event) {
	case CPUFREQ_GOV_START:
		if ((!cpu_online(cpu)) || (!policy->cur))
			return -EINVAL;

		mutex_lock(&dbs_mutex);

		for_each_cpu(j, policy->cpus) {
			struct cpu_dbs_info_s *j_dbs_info;
			j_dbs_info = &per_cpu(cs_cpu_dbs_info, j);
			j_dbs_info->cur_policy = policy;

			j_dbs_info->prev_cpu_idle = get_cpu_idle_time(j,
						&j_dbs_info->prev_cpu_wall);
			if (dbs_tuners_ins.ignore_nice) {
				j_dbs_info->prev_cpu_nice =
						kstat_cpu(j).cpustat.nice;
			}
			j_dbs_info->time_in_idle = get_cpu_idle_time_us(cpu, &j_dbs_info->idle_exit_time); // ZZ: added idle exit time handling
		}
		this_dbs_info->cpu = cpu;		// ZZ: Initialise the cpu field during conservative governor start (https://github.com/ktoonsez/KT747-JB/commit/298dd04a610a6a655d7b77f320198d9f6c7565b6)
		this_dbs_info->rate_mult = 1;		// ZZ: Sampling down momentum - reset multiplier
		this_dbs_info->momentum_adder = 0;	// ZZ: Sampling down momentum - reset momentum adder
		this_dbs_info->down_skip = 0;		// ZZ: Sampling down - reset down_skip
		this_dbs_info->check_cpu_skip = 1;	// ZZ: we do not want to crash because of hotplugging so we start without it by skipping check_cpu
		this_dbs_info->requested_freq = policy->cur;
		enable_cores_on_exit = 0;		// ZZ: reset enable cores flag - this fixes value persistence problems after governor reload
		
		// ZZ: save default values in threshold array
		for (i = 0; i < num_possible_cpus(); i++) {
		    hotplug_thresholds[0][i] = DEF_FREQUENCY_UP_THRESHOLD_HOTPLUG;
		    hotplug_thresholds[1][i] = DEF_FREQUENCY_DOWN_THRESHOLD_HOTPLUG;
		    hotplug_thresholds_tuneable[i] = 1;
		}

		// ZZ: initialisation of freq search in scaling table
		for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++) {
			if (policy->max == table[i].frequency) {
				max_scaling_freq_hard = max_scaling_freq_soft = i; // ZZ: init soft and hard value
				// Yank : Continue looping until table end is reached, we need this to set the table size limit below
			}
		}

		freq_table_size = i - 1; // Yank : upper index limit of freq. table

		/*
		 * ZZ: we have to take care about where we are in the frequency table. when using kernel sources without OC capability
		 * it might be that index 0 and 1 contains no frequencies so a save index start point is needed.
		 */
		calc_index = freq_table_size - max_scaling_freq_hard;	// ZZ: calculate the difference and use it as start point
		if (calc_index == freq_table_size)			// ZZ: if we are at the end of the table
		    calc_index = calc_index - 1;			// ZZ: shift in range for order calculation below

		// Yank : assert if CPU freq. table is in ascending or descending order
		if (table[calc_index].frequency > table[calc_index+1].frequency) {
			freq_table_order = +1;				// Yank : table is in descending order as expected, lowest freq at the bottom of the table
			min_scaling_freq = i - 1;			// Yank : last valid frequency step (lowest frequency)
			limit_table_start = max_scaling_freq_soft;	// ZZ: we should use the actual scaling soft limit value as search start point
		} else {
			freq_table_order = -1;				// Yank : table is in ascending order, lowest freq at the top of the table
			min_scaling_freq = 0;				// Yank : first valid frequency step (lowest frequency)
			limit_table_start = 0;				// ZZ: start searching at lowest freq
			limit_table_end = table[freq_table_size].frequency; // ZZ: end searching at highest freq limit
		}

		mutex_init(&this_dbs_info->timer_mutex);
		dbs_enable++;

		/*
		 * Start the timerschedule work, when this governor
		 * is used for first time
		 */
		if (dbs_enable == 1) {
			unsigned int latency;
			/* policy latency is in nS. Convert it to uS first */
			latency = policy->cpuinfo.transition_latency / 1000;
			if (latency == 0)
				latency = 1;

			rc = sysfs_create_group(cpufreq_global_kobject,
						&dbs_attr_group);
			if (rc) {
				mutex_unlock(&dbs_mutex);
				return rc;
			}

			/*
			 * conservative does not implement micro like ondemand
			 * governor, thus we are bound to jiffes/HZ
			 */
			min_sampling_rate =
				MIN_SAMPLING_RATE_RATIO * jiffies_to_usecs(3);
			/* Bring kernel and HW constraints together */
			min_sampling_rate = max(min_sampling_rate,
					MIN_LATENCY_MULTIPLIER * latency);
			dbs_tuners_ins.sampling_rate_current =
				max(min_sampling_rate,
				    latency * LATENCY_MULTIPLIER);
			orig_sampling_down_factor = dbs_tuners_ins.sampling_down_factor;	// ZZ: Sampling down momentum - set down factor
			orig_sampling_down_max_mom = dbs_tuners_ins.sampling_down_max_mom;	// ZZ: Sampling down momentum - set max momentum
			sampling_rate_awake = dbs_tuners_ins.sampling_rate = dbs_tuners_ins.sampling_rate_current;
			up_threshold_awake = dbs_tuners_ins.up_threshold;
			down_threshold_awake = dbs_tuners_ins.down_threshold;
			smooth_up_awake = dbs_tuners_ins.smooth_up;
			cpufreq_register_notifier(
					&dbs_cpufreq_notifier_block,
					CPUFREQ_TRANSITION_NOTIFIER);
		}
		mutex_unlock(&dbs_mutex);
		dbs_timer_init(this_dbs_info);
	        register_early_suspend(&_powersave_early_suspend);
		break;

	case CPUFREQ_GOV_STOP:
		enable_cores_on_exit = 1;		// ZZ: enable all cores to avoid staying in offline state when changing to a non-hotplugging aware governor
		skip_hotplug_flag = 1;			// ZZ: disable hotplugging during stop to avoid deadlocks if we are in the hotplugging logic
		this_dbs_info->check_cpu_skip = 1;	// ZZ: and we disable cpu_check also on next 15 samples

		mutex_lock(&dbs_mutex);			// ZZ: added for deadlock fix on governor stop
		dbs_timer_exit(this_dbs_info);
		mutex_unlock(&dbs_mutex);		// ZZ: added for deadlock fix on governor stop

		this_dbs_info->idle_exit_time = 0;	// ZZ: added idle exit time handling

		mutex_lock(&dbs_mutex);
		dbs_enable--;
		mutex_destroy(&this_dbs_info->timer_mutex);
		/*
		 * Stop the timerschedule work, when this governor
		 * is used for first time
		 */
		if (dbs_enable == 0)
			cpufreq_unregister_notifier(
					&dbs_cpufreq_notifier_block,
					CPUFREQ_TRANSITION_NOTIFIER);

		mutex_unlock(&dbs_mutex);
		if (!dbs_enable)
			sysfs_remove_group(cpufreq_global_kobject,
					   &dbs_attr_group);

		unregister_early_suspend(&_powersave_early_suspend);

#ifdef CONFIG_CPU_FREQ_LCD_FREQ_DFS
		if (dbs_tuners_ins.lcdfreq_enable == true) {
			lcdfreq_lock_current = 0;		// ZZ: LCDFreq Scaling disable at stop
			_lcdfreq_lock(lcdfreq_lock_current);	// ZZ: LCDFreq Scaling disable at stop
		}
#endif
		break;

	case CPUFREQ_GOV_LIMITS:
		skip_hotplug_flag = 1;			// ZZ: disable hotplugging during limit change
		this_dbs_info->check_cpu_skip = 1;	// ZZ: to avoid deadlocks skip check_cpu next 25 samples
		for (i = 0; i < 1000; i++);		// ZZ: wait a few samples to be sure hotplugging is off (never be sure so this is dirty)
		/*
		 * ZZ: we really want to do this limit update but here are deadlocks possible if hotplugging locks are active, so if we are about
		 * to crash skip the whole freq limit change attempt by using mutex_trylock instead of mutex_lock.
		 * so now this is a real fix but on the other hand it could also avoid limit changes so we keep all the other workarounds
		 * to reduce the chance of such situations!
		 */
		if (mutex_trylock(&this_dbs_info->timer_mutex)) {
		    if (policy->max < this_dbs_info->cur_policy->cur)
			__cpufreq_driver_target(
					this_dbs_info->cur_policy,
					policy->max, CPUFREQ_RELATION_H);
		    else if (policy->min > this_dbs_info->cur_policy->cur)
			__cpufreq_driver_target(
					this_dbs_info->cur_policy,
					policy->min, CPUFREQ_RELATION_L);
		    mutex_unlock(&this_dbs_info->timer_mutex);
		} else {
		    return 0;
		}
		/*
		* ZZ: obviously this "limit case" will be executed multiple times at suspend (not sure why!?)
		* but we have already a early suspend code to handle scaling search limits so we have to use a flag to avoid double execution at suspend!
		*/

		if (unlikely(suspend_flag == 0 && policy->max != table[max_scaling_freq_hard].frequency)) {		// Yank : if policy->max has changed and we are not sleeping
		    for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++) {
			if (policy->max == table[i].frequency) {
				max_scaling_freq_hard = i;								// ZZ   : set new freq scaling number
				break;
			}
		    }

		    if (unlikely(table[max_scaling_freq_soft].frequency > table[max_scaling_freq_hard].frequency)) {	// Yank : if we would go above hard limits reset them
			    max_scaling_freq_soft = max_scaling_freq_hard;						// Yank : if soft freq. is higher than hard max limit then set it to hard max limit value
				if (freq_table_order == 1)								// ZZ: if descending ordered table is used
				    limit_table_start = max_scaling_freq_soft;						// ZZ: we should use the actual scaling soft limit value as search starting point
				else
				    limit_table_end = policy->max;							// ZZ: prepare max search range for ascending ordered table
			    if (policy->max <= dbs_tuners_ins.freq_limit)						// ZZ   : check limit
				dbs_tuners_ins.freq_limit = 0;								// Yank : and delete active limit if it is above hard limit
			    if (policy->max <= dbs_tuners_ins.freq_limit_sleep)						// ZZ   : check sleep limit
				dbs_tuners_ins.freq_limit_sleep = 0;							// Yank : if we would go also above this hard limit delete soft limit also
		    } else if (unlikely(table[max_scaling_freq_soft].frequency < table[max_scaling_freq_hard].frequency
										 && dbs_tuners_ins.freq_limit == 0)) {
			    max_scaling_freq_soft = max_scaling_freq_hard;						// ZZ: if no limit is set and soft freq lower than limit then set back to hard max limit value
				if (freq_table_order == 1)								// ZZ: if descending ordered table is used
				    limit_table_start = max_scaling_freq_soft;						// ZZ: we should use the actual scaling soft limit value as search starting point
				else
				    limit_table_end = policy->max;							// ZZ: prepare max search range for ascending ordered table
		    }
		}

		skip_hotplug_flag = 0;											// ZZ: enable hotplugging again
		this_dbs_info->time_in_idle = get_cpu_idle_time_us(cpu, &this_dbs_info->idle_exit_time);		// ZZ: added idle exit time handling
		break;
	}
	return 0;
}

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_ZZMOOVE
static
#endif
struct cpufreq_governor cpufreq_gov_zzmoove = {
	.name			= "zzmoove",
	.governor		= cpufreq_governor_dbs,
	.max_transition_latency	= TRANSITION_LATENCY_LIMIT,
	.owner			= THIS_MODULE,
};

static int __init cpufreq_gov_dbs_init(void) // ZZ: added idle exit time handling
{
    unsigned int i;
    struct cpu_dbs_info_s *this_dbs_info;
    /* Initalize per-cpu data: */
    for_each_possible_cpu(i) {
	this_dbs_info = &per_cpu(cs_cpu_dbs_info, i);
	this_dbs_info->time_in_idle = 0;
	this_dbs_info->idle_exit_time = 0;
    }

    INIT_WORK(&hotplug_offline_work, hotplug_offline_work_fn); // ZZ: init hotplug work
    INIT_WORK(&hotplug_online_work, hotplug_online_work_fn); // ZZ: init hotplug work

	return cpufreq_register_governor(&cpufreq_gov_zzmoove);
}

static void __exit cpufreq_gov_dbs_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_zzmoove);
}

/*
 * zzmoove governor is based on the modified conservative (original author
 * Alexander Clouter <alex@digriz.org.uk>) smoove governor from Michael
 * Weingaertner <mialwe@googlemail.com> (source: https://github.com/mialwe/mngb/)
 * Modified by Zane Zaminsky November 2012 to be hotplug-able and optimzed for use
 * with Samsung I9300. CPU Hotplug modifications partially taken from ktoonservative
 * governor from ktoonsez KT747-JB kernel (https://github.com/ktoonsez/KT747-JB)
 */

MODULE_AUTHOR("Zane Zaminsky <cyxman@yahoo.com>");
MODULE_DESCRIPTION("'cpufreq_zzmoove' - A dynamic cpufreq governor based "
		"on smoove governor from Michael Weingaertner which was originally based on "
		"cpufreq_conservative from Alexander Clouter. Optimized for use with Samsung I9300 "
		"using a fast scaling and CPU hotplug logic - ported/modified for I9300 "
		"by ZaneZam November 2012/13");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_ZZMOOVE
fs_initcall(cpufreq_gov_dbs_init);
#else
module_init(cpufreq_gov_dbs_init);
#endif
module_exit(cpufreq_gov_dbs_exit);

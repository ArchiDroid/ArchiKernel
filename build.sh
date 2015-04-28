#!/bin/bash

#     _             _     _ _  __                    _
#    / \   _ __ ___| |__ (_) |/ /___ _ __ _ __   ___| |
#   / _ \ | '__/ __| '_ \| | ' // _ \ '__| '_ \ / _ \ |
#  / ___ \| | | (__| | | | | . \  __/ |  | | | |  __/ |
# /_/   \_\_|  \___|_| |_|_|_|\_\___|_|  |_| |_|\___|_|
#
# Copyright 2014 Łukasz "JustArchi" Domeradzki
# Contact: JustArchi@JustArchi.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Usage:
# build.sh <option1> <option2> <option3> yourconfig
# NOTICE: yourconfig must exist in arch/arm/configs

# Examples:
# ./build.sh aosp_ak_defconfig -> Build ArchiKernel for default AOSP variant
# ./build.sh samsung_ak_defconfig -> Build ArchiKernel for Samsung variant
# ./build.sh aosp_ak_defconfig samsung_ak_defconfig -> Build ArchiKernel for both AOSP and Samsung variant
# ./build.sh --all -> Build ArchiKernel for all currently supported variants. Same as ./build.sh every possible variant
# ./build.sh --version=2.0 aosp_ak_defconfig -> Build ArchiKernel for default AOSP variant and specify that version is V2.0
# ./build.sh --configtest -> Use currently available .config instead of predefined configs. Useful for config tests
# ./build.sh --dirty -> Don't clean, use currentnly available .config and build. Perfect for testing if new commit compiles
# source build.sh --source -> Append proper variables to current shell, so you can use i.e. make clean/menuconfig/all standalone

# Release:
# ./build.sh --all --version=X.Y.Z

SOURCE=0 # --source -> This will prepare environment for cross compiling (only). Use only in conjuction with "source" command, i.e. source build.sh --source
REGEN=0 # --regen/--regenerate -> This will regenerate ArchiKernel configs according to current Kconfig
CLEAN=0 # --clean -> This will clean build directory, same as make clean && make mrproper
DIRTY=0 # --dirty -> This will not call make clean && make mrproper. Implies --configtest
CONFIGTEST=0 # --configtest -> This will call only make clean (without make mrproper) and use .config file instead of $TARGETCONFIG. Useful for config tests
BARE=0 # --bare -> This will finish the script as soon as the kernel is compiled, so no modules stripping or copying files will be done
ALL=0 # --all -> This will build every currently supported ArchiKernel variant, from arch/arm/configs/*_ak_defconfig

# Detect HOME properly
# This workaround is required because arm-eabi-nm has problems following ~. Don't change it
if [[ "$(dirname ~)" = "/" ]]; then
	HOME="$(dirname ~)$(basename ~)" # Root
else
	HOME="$(dirname ~)/$(basename ~)" # User
fi

# You may need to change these variables
TOOLCHAIN="$HOME/TC" # This is where your toolchain is located. Current default ArchiKernel toolchain: https://github.com/ArchiDroid/Toolchain/tree/linaro-4.9-arm-linux-gnueabihf
TARGETZIPDIR="$HOME/shared/kernel/m0" # If valid, output zip will be moved there from default TARGETDIR location
BEEP=1 # This will beep three times on finish to wake me up :). Works even through SSH!

# Above settings should be enough, changing below things shouldn't be required
TARGETDIR="archikernel/flasher" # This is the general output path. Don't change it, change TARGETZIPDIR above instead
TARGETDIRKERNEL="$TARGETDIR/prebuilt" # This is where zImage is put, this shouldn't be changed
TARGETDIRMODULES="$TARGETDIRKERNEL/system/lib/modules" # Similar to above, but for modules
TARGETZIPNAME="ArchiKernel_$(date '+%y%m%d_%H%M%S')" # Name of output zip. By keeping a date here we can be sure that all zips will be unique
JOBS="$(grep -c "processor" "/proc/cpuinfo")" # Maximum number of jobs, can be declared statically if needed, default to number of threads of the CPU

# This is where magic starts
for ARG in "$@"; do
	case "$ARG" in
		--source|source) SOURCE=1 ;;
		--regen|regen|--regenerate|regenerate) REGEN=1 ;;
		--clean|clean) CLEAN=1 ;;
		--dirty|dirty) DIRTY=1 ;;
		--configtest|configtest) CONFIGTEST=1 ;;
		--bare|bare) BARE=1 ;;
		--all|all) ALL=1 ;;
		--version=*) TARGETZIPNAME="ArchiKernel_V$(echo "$ARG" | cut -d '=' -f2)" ;;
		*) TARGETCONFIGS+=("$ARG") ;;
	esac
done

PREFIXES="arm-architoolchain-linux-gnueabihf arm-linux-gnueabihf arm-eabi" # Valid prefixes used by ArchiToolchain, Linaro and Google
for PREFIX in $PREFIXES; do
	if [[ -x "$TOOLCHAIN/bin/${PREFIX}-gcc" ]]; then
		CROSS_COMPILE="$TOOLCHAIN/bin/${PREFIX}-"
		echo "Found ${PREFIX} toolchain!"
		echo
		break
	fi
done

if [[ -z "$CROSS_COMPILE" ]]; then
	echo "ERROR: Could not find any valid toolchain prefix!"
	echo "Make sure that $TOOLCHAIN/bin/PREFIX-gcc exists"
	echo "Where PREFIX must be one of the following: $PREFIXES"
	exit 1
fi

export ARCH=arm
export CROSS_COMPILE="$CROSS_COMPILE"

if [[ "$SOURCE" -eq 1 ]]; then
	return
	exit 0 # If we're in fact sourcing this file, this won't execute
fi

set -e

SCRIPTDIR="$(dirname "$0")"

# We may have build.sh symlinked e.g. to /bin/adkernel
# In such case, navigating to script's dir is not a way
if [[ -f "$SCRIPTDIR/Kconfig" ]]; then
	cd "$SCRIPTDIR" # Navigate to proper dir
elif [[ ! -f "Kconfig" ]]; then
	echo "ERROR: Couldn't find Kconfig"
	echo "Make sure you're in proper dir!"
	exit 1
fi

if [[ "$REGEN" -eq 1 ]]; then
	find "arch/$ARCH/configs" -type f -iname "*_ak_defconfig" | while read TOREGEN; do
		TOREGENSHORT="$(basename "$TOREGEN")"
		echo "Regenerating $TOREGENSHORT"
		make -j "$JOBS" "$TOREGENSHORT"
		mv .config "$TOREGEN"
	done
	make -j "$JOBS" clean
	make -j "$JOBS" mrproper
	echo "All configs are regenerated!"
	exit 0
fi

if [[ "$ALL" -eq 1 ]]; then
	while read TARGETCONFIG; do
		TARGETCONFIGS+=("$(basename "$TARGETCONFIG")")
	done < <(find "arch/$ARCH/configs" -type f -iname "*_ak_defconfig")
fi

if [[ -z "$TARGETCONFIGS" ]]; then
	if [[ "$CLEAN" -eq 0 && "$CONFIGTEST" -eq 0 && "$DIRTY" -eq 0 ]]; then
		echo "ERROR: You didn't specify any config!"
		echo
		echo "If you want to use locally generated .config, try --configtest"
		echo "If you want to build AK for all currently supported variants, try --all"
		echo
		echo "Otherwise, try invoking this script with argument which matches one of these:"
		echo "-=-=-=-=-=-="
		find "arch/$ARCH/configs" -type f -iname "*_ak_defconfig" | while read line; do
			basename "$line"
		done
		echo "-=-=-=-=-=-="
		echo
		exit 1
	else
		TARGETCONFIGS+=("NULLCONFIG");
	fi
fi

for TARGETCONFIG in "${TARGETCONFIGS[@]}"; do
	(
		if [[ "$DIRTY" -eq 0 ]]; then
			make -j "$JOBS" clean
			if [[ "$CONFIGTEST" -eq 1 && -f ".config" ]]; then
				mv ".config" ".configBackup"
			else
				rm -f ".configBackup"
			fi
			make -j "$JOBS" mrproper
			if [[ "$CLEAN" -eq 1 ]]; then
				rm -f ".configBackup" # Just in case if somebody would call configtest with clean...
				exit 0
			fi
			if [[ "$CONFIGTEST" -eq 1 && -f ".configBackup" ]]; then
				mv ".configBackup" ".config"
			elif [[ -f "arch/$ARCH/configs/$TARGETCONFIG" ]]; then
				make -j "$JOBS" "$TARGETCONFIG"
				APPLIEDCONFIG="$(echo $TARGETCONFIG | rev | cut -d'_' -f3- | rev)"
				APPLIEDCONFIG="${APPLIEDCONFIG^^}" # To uppercase
			else
				echo "ERROR: Could not find specified config: arch/$ARCH/configs/$TARGETCONFIG"
				continue
			fi
		fi

		# Try to detect applied config if no config has been specified (e.g. --dirty)
		if [[ -z "$APPLIEDCONFIG" ]]; then
			APPLIEDCONFIG="Unknown"
			CONFIGMD5="$(md5sum .config | awk '{print $1}')"
			while read TOCHECK; do
				if [[ "$(md5sum "$TOCHECK" | awk '{print $1}')" = "$CONFIGMD5" ]]; then
					APPLIEDCONFIG="$(basename "$TOCHECK" | rev | cut -d'_' -f3- | rev)"
					APPLIEDCONFIG="${APPLIEDCONFIG^^}" # To uppercase
					break
				fi
			done < <(find "arch/$ARCH/configs" -type f -iname "*_ak_defconfig")
		fi

		make -j "$JOBS" all

		if [[ "$BARE" -eq 1 ]]; then
			continue
		elif [[ -z "$(which zip)" ]]; then
			echo "ERROR: Zip binary could not be found!"
			echo "You can either install missing zip binary or build in --bare mode"
			echo "For debian try: apt-get install zip"
			exit 1
		fi

		mkdir -p "$TARGETDIRKERNEL" "$TARGETDIRMODULES"
		cp "arch/$ARCH/boot/zImage" "$TARGETDIRKERNEL"

		find "$TARGETDIRMODULES" -type f -iname "*.ko" | while read KO; do
			rm -f "$KO"
		done

		find . -type f -iname "*.ko" | while read KO; do
			echo "Including module: $(basename "$KO")"
			${CROSS_COMPILE}strip --strip-unneeded "$KO"
			cp "$KO" "$TARGETDIRMODULES"
		done

		THISZIPNAME="$TARGETZIPNAME-$APPLIEDCONFIG"
		cd "$TARGETDIR"
		zip -qry -9 "$THISZIPNAME.zip" . -x "*.zip"

		if [[ ! -z "$TARGETZIPDIR" && -d "$TARGETZIPDIR" ]]; then
			mv "$THISZIPNAME.zip" "$TARGETZIPDIR"
		fi

		echo "Done! Output zip: $THISZIPNAME.zip"
	)
done

if [[ "$BEEP" -eq 1 ]]; then
	echo -e "\a"
	sleep 0.2
	echo -e "\a"
	sleep 0.2
	echo -e "\a"
fi

echo "All tasks done! :)"

exit 0

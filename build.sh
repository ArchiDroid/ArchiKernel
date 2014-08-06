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
# build.sh <option1> <option2> <option3> defconfig
SOURCE=0 # --source -> This will prepare environment for cross compiling (only). Use only in conjuction with "source" command, i.e. source build.sh --source
CLEAN=0 # --clean -> This will clean build directory, same as make clean && make mrproper
DIRTY=0 # --dirty -> This will not call make clean && make mrproper
CONFIGTEST=0 # --configtest -> This will call only make clean (without make mrproper) and use .config file instead of $TARGETCONFIG. Useful for config tests
BARE=0 # --bare -> This will finish the script as soon as the kernel is compiled, so no modules stripping or copying files will be done

# You may need to change these variables
HOME="$(dirname ~)$(basename ~)" # This workaround is required because arm-eabi-nm has problems following ~. Don't change it
TOOLCHAIN="$HOME/TC/bin" # This is where your toolchain is located. This path must contain arm-eabi-* binaries, typically Toolchaindir/bin
TARGETDIR="archikernel/flasher" # This is the general output path. You should unpack AK in zip format somewhere, then put proper path here
TARGETZIPDIR="$HOME/shared/kernel/m0" # If not empty, output zip will be moved here
TARGETCONFIG="aosp_ak_defconfig" # This is default config, which is overrided if extra argument is given
BEEP=1 # This will beep three times on finish to wake me up :). Works even through SSH!

# This should be enough, changing below things shouldn't be required
TARGETDIRKERNEL="$TARGETDIR/prebuilt" # This is where zImage is put. If you keep AK directory structure, you don't need to edit that
TARGETDIRMODULES="$TARGETDIRKERNEL/system/lib/modules" # Similar to above, but for modules
TARGETZIPNAME="ArchiKernel_$(date '+%d%m%y_%H%M%S')" # Name of output zip. If you keep date here, zips will be unique, if you declare this as static, we'll overwrite specific one
JOBS="$(grep -c "processor" "/proc/cpuinfo")" # Maximum number of jobs, can be declared statically if needed, default to number of threads of the CPU

# This is where magic starts
for ARG in "$@"; do
	case "$ARG" in
		--source|source) SOURCE=1 ;;
		--clean|clean) CLEAN=1 ;;
		--dirty|dirty) DIRTY=1 ;;
		--configtest|configtest) CONFIGTEST=1 ;;
		--bare|bare) BARE=1 ;;
		*) TARGETCONFIG="$ARG" ;;
	esac
done

PREFIXES="arm-linux-gnueabihf arm-eabi" # Linaro uses gnueabihf, while Google uses eabi
for PREFIX in $PREFIXES; do
	if [[ -x "$TOOLCHAIN/${PREFIX}-gcc" ]]; then
		CROSS_COMPILE="$TOOLCHAIN/${PREFIX}-"
		echo "Found ${PREFIX} toolchain!"
		echo
		break
	fi
done

if [[ -z "$CROSS_COMPILE" ]]; then
	echo "ERROR: Could not find any valid toolchain prefix!"
	echo "Make sure that $TOOLCHAIN/PREFIX-gcc exists"
	echo "Where PREFIX must be one of the following: $PREFIXES"
	exit 1
fi

export ARCH=arm
export CROSS_COMPILE="$CROSS_COMPILE"

if [[ "$SOURCE" -eq 1 ]]; then
        return
fi

set -e
cd "$(dirname "$0")" # Navigate to proper dir

if [[ ! -f "arch/$ARCH/configs/$TARGETCONFIG" ]]; then
	echo "ERROR: Could not find specified config: arch/$ARCH/configs/$TARGETCONFIG"
	exit 1
fi
if [[ "$DIRTY" -eq 0 ]]; then
	make -j"$JOBS" clean
	if [[ "$CONFIGTEST" -eq 0 ]]; then
		make -j "$JOBS" mrproper
		if [[ "$CLEAN" -eq 1 ]]; then
			exit 0
		fi
		make -j"$JOBS" "$TARGETCONFIG"
	fi
fi

make -j "$JOBS" all

if [[ "$BARE" -eq 1 ]]; then
	exit 0
fi

mkdir -p "$TARGETDIRKERNEL" "$TARGETDIRMODULES"
cp arch/arm/boot/zImage "$TARGETDIRKERNEL"

find "$TARGETDIRMODULES" -iname "*.ko" | while read line; do
	rm -f "$line"
done

find . -type f -iname "*.ko" | while read line; do
	${CROSS_COMPILE}strip --strip-unneeded "$line"
		cp "$line" "$TARGETDIRMODULES"
done

cd "$TARGETDIR"
zip -ry -9 "$TARGETZIPNAME.zip" . -x "*.zip"

if [[ ! -z "$TARGETZIPDIR" && -d "$TARGETZIPDIR" ]]; then
	mv "$TARGETZIPNAME.zip" "$TARGETZIPDIR"
fi

if [[ "$BEEP" -eq 1 ]]; then
	echo -e "\a"
	sleep 0.2
	echo -e "\a"
	sleep 0.2
	echo -e "\a"
fi

echo "Done! Output zip: $TARGETZIPNAME.zip"
exit 0

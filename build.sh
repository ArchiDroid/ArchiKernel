#!/bin/bash
# Base by cybojenix <anthonydking@gmail.com>
# ReWriten by Caio Oliveira aka Caio99BR <caiooliveirafarias0@gmail.com>
# Rashed for the base of zip making
# And the internet for filling in else where

# You need to download https://github.com/TeamVee/android_prebuilt_toolchains
# Clone in the same folder as the kernel to choose a toolchain and not specify a location

# Prepare output customization commands - Start

customoutput() {
if [ "$coloroutput" == "" ]; then
	coloroutput="ON"
fi
if [ "$coloroutput" == "ON" ]; then
	# Stock Color
	txtrst=$(tput sgr0)
	# Bold Colors
	txtbld=$(tput bold) # Bold
	bldred=${txtbld}$(tput setaf 1) # red
	bldgrn=${txtbld}$(tput setaf 2) # green
	bldyel=${txtbld}$(tput setaf 3) # yellow
	bldblu=${txtbld}$(tput setaf 4) # blue
	bldmag=${txtbld}$(tput setaf 5) # magenta
	bldcya=${txtbld}$(tput setaf 6) # cyan
	bldwhi=${txtbld}$(tput setaf 7) # white
	coloroutputcheck="${bldcya}ON${txtrst}"
	coloroutputzip="--color=auto"
else
	unset txtbld bldred bldgrn bldyel bldblu bldmag bldcya bldwhi
	coloroutputcheck="OFF"
	coloroutputzip="--color=never"
fi
}

setcustomoutput() {
if [ "$coloroutput" == "ON" ]; then
	coloroutput="OFF"
else
	coloroutput="ON"
fi
}

# Prepare output customization commands - End

# Clean - Start

cleanzip() {
rm -rf zip-creator/*.zip zip-creator/zImage zip-creator/system/lib/modules/*.ko
cleanzipcheck="Done"
unset zippackagecheck adbcopycheck
}

cleankernel() {
make clean mrproper &> /dev/null
cleankernelcheck="Done"
unset buildprocesscheck target serie variant maindevicecheck BUILDTIME
}

# Clean - End

# Main Process - Start

maindevice() {
echo "${bldred}First Gen${txtrst}"
echo "0) L5 NFC"
echo "1) L5 NoNFC"
echo "2) L7 NFC - 8m"
echo "3) L7 NFC"
echo "4) L7 NoNFC"
echo "${bldblu}Second Gen${txtrst}"
echo "5) L1 II Single/Dual"
echo "6) L3 II Single/Dual"
echo "7) L7 II NFC"
echo "8) L7 II NoNFC"
echo "9) L7 II Dual"
read -p "Choice: " -n 1 -s choice
case "$choice" in
	0 ) target="L5-"; variant="NFC"; echo "$choice - $target$variant"; make cyanogenmod_m4_defconfig &> /dev/null; maindevicecheck="On";;
	1 ) target="L5-"; variant="NoNFC"; echo "$choice - $target$variant"; make cyanogenmod_m4_nonfc_defconfig &> /dev/null; maindevicecheck="On";;
	2 ) target="L7-"; variant="NFC-8m"; echo "$choice - $target$variant"; make cyanogenmod_u0_8m_defconfig &> /dev/null; maindevicecheck="On";;
	3 ) target="L7-"; variant="NFC"; echo "$choice - $target$variant"; make cyanogenmod_u0_defconfig &> /dev/null; maindevicecheck="On";;
	4 ) target="L7-"; variant="NoNFC"; echo "$choice - $target$variant"; make cyanogenmod_u0_nonfc_defconfig &> /dev/null; maindevicecheck="On";;
	5 ) target="L1II-"; variant="SD"; echo "$choice - $target$variant"; make cyanogenmod_v1_defconfig &> /dev/null; maindevicecheck="On";;
	6 ) target="L3II-"; variant="SD"; echo "$choice - $target$variant"; make cyanogenmod_vee3_defconfig &> /dev/null; maindevicecheck="On";;
	7 ) target="L7II-"; variant="NFC"; echo "$choice - $target$variant"; make cyanogenmod_vee7_defconfig &> /dev/null; maindevicecheck="On";;
	8 ) target="L7II-"; variant="NoNFC"; echo "$choice - $target$variant"; make cyanogenmod_vee7_nonfc_defconfig &> /dev/null; maindevicecheck="On";;
	9 ) target="L7II-"; variant="Dual"; echo "$choice - $target$variant"; make cyanogenmod_vee7ds_defconfig &> /dev/null; maindevicecheck="On";;
	* ) echo "$choice - This option is not valid"; sleep 2;;
esac
}

maintoolchain() {
if [ -d ../android_prebuilt_toolchains ]; then
	echo "1) 4.7 Google GCC"
	echo "2) 4.8 Google GCC"
	echo "3) 4.6.4 Linaro GCC"
	echo "4) 4.7.4 Linaro GCC"
	echo "5) 4.8.4 Linaro GCC"
	echo "6) 4.9.3 Linaro GCC"
	read -p "Choice: " -n 1 -s toolchain
	case "$toolchain" in
		1 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-eabi-4.7/bin/arm-eabi-";;
		2 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-eabi-4.8/bin/arm-eabi-";;
		3 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-unknown-linux-gnueabi-linaro_4.6.4-2013.05/bin/arm-unknown-linux-gnueabi-";;
		4 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-unknown-linux-gnueabi-linaro_4.7.4-2013.12/bin/arm-unknown-linux-gnueabi-";;
		5 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-linux-gnueabi-linaro_4.8.4-2014.11/bin/arm-linux-gnueabi-";;
		6 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-cortex-linux-gnueabi-linaro_4.9.3-2015.03/bin/arm-cortex-linux-gnueabi-";;
		* ) echo "$toolchain - This option is not valid"; sleep 2;;
	esac
else
	echo "Script says: You don't have TeamVee Prebuilt Toolchains"
	echo ""
	echo "Script says: Please specify a location"
	echo "Script says: and the prefix of the chosen toolchain at the end"
	echo "Caio99BR says: GCC 4.6 ex. ../arm-eabi-4.6/bin/arm-eabi-"
	read -p "Place: " CROSS_COMPILE
fi
}

# Main Process - End

# Build Process - Start

buildprocess() {
START=$(date +"%s")
# Check cpu's
NR_CPUS=$(grep -c ^processor /proc/cpuinfo)
if [ "$NR_CPUS" -le "2" ]; then
	NR_CPUS=4
fi
echo "${bldblu}Building $customkernel with $NR_CPUS jobs at once${txtrst}"
make -j ${NR_CPUS}
END=$(date +"%s")
BUILDTIME=$(($END - $START))
if [ -f arch/arm/boot/zImage ]; then
	buildprocesscheck="Done"
	unset cleankernelcheck
else
	buildprocesscheck="Something goes wrong"
fi
}

zippackage() {
if [ "$target" == "L1II-" ]; then
	tol1ii
elif [ "$target" == "L3II-" ]; then
	tol3ii
elif [ "$target" == "L7II-" ]; then
	tol7ii
elif [ "$target" == "L7-" ]; then
	tol7
fi

if [ "$variant" == "NoNFC" ]; then
	tononfc
elif [ "$variant" == "Dual" ]; then
	todual
fi

cp arch/arm/boot/zImage zip-creator
find . -name *.ko | xargs cp -a --target-directory=zip-creator/system/lib/modules/ &> /dev/null

cd zip-creator
zip -r $zipfile * -x */.gitignore *.zip &> /dev/null
cd ..

if [ "$variant" == "Dual" ]; then
	ofdual
elif [ "$variant" == "NoNFC" ]; then
	ofnonfc
fi

if [ "$target" == "L7-" ]; then
	ofl7
elif [ "$target" == "L7II-" ]; then
	ofl7ii
elif [ "$target" == "L3II-" ]; then
	ofl3ii
elif [ "$target" == "L1II-" ]; then
	ofl1ii
fi

zippackagecheck="Done"
unset cleanzipcheck
}

tol1ii() {
sed 's/m4/vee3/; s/14/15/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5 NFC/L1 II Single|Dual/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofl1ii() {
sed 's/vee3/m4/; s/15/14/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L1 II Single|Dual/L5 NFC/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tol3ii() {
sed 's/m4/vee3/; s/14/15/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5 NFC/L3 II Single and Dual/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofl3ii() {
sed 's/vee3/m4/; s/15/14/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L3 II Single and Dual/L5 NFC/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tol7ii() {
sed 's/m4/vee7/; s/14/15/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5/L7 II/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofl7ii() {
sed 's/vee7/m4/; s/15/14/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L7 II/L5/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tol7() {
sed 's/m4/u0/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5/L7/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofl7() {
sed 's/u0/m4/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L7/L5/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tononfc() {
sed 's/NFC/NoNFC/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofnonfc() {
sed 's/NoNFC/NFC/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

todual() {
sed 's/NFC/Dual/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofdual() {
sed 's/Dual/NFC/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

# Build Process - End

# ADB - Start

adbcopy() {
echo "Script says: You want to copy to Internal or External Card?"
echo "i) For Internal"
echo "e) For External"
read -p "Choice: " -n 1 -s adbcoping
case "$adbcoping" in
	i ) echo "Coping to Internal Card..."; adb shell rm -rf /storage/sdcard0/$zipfile &> /dev/null; adb push zip-creator/$zipfile /storage/sdcard0/$zipfile &> /dev/null; adbcopycheck="Done";;
	e ) echo "Coping to External Card..."; adb shell rm -rf /storage/sdcard1/$zipfile &> /dev/null; adb push zip-creator/$zipfile /storage/sdcard1/$zipfile &> /dev/null; adbcopycheck="Done";;
	* ) echo "$adbcoping - This option is not valid"; sleep 2;;
esac
}

# ADB - End

# Menu - Start

customkernel=CAFKernel
export ARCH=arm

buildsh() {
kernelversion=`cat Makefile | grep VERSION | cut -c 11- | head -1`
kernelpatchlevel=`cat Makefile | grep PATCHLEVEL | cut -c 14- | head -1`
kernelsublevel=`cat Makefile | grep SUBLEVEL | cut -c 12- | head -1`
kernelname=`cat Makefile | grep NAME | cut -c 8- | head -1`
zipfile="$customkernel-$target$variant.zip"
lszip=`ls zip-creator/*.zip 2>/dev/null | wc -l`
customoutput
clear
echo "Caio99BR says: Simple Kernel Build Script."
echo "This is an open source script, feel free to use, edit and share it."
echo "Linux Kernel $kernelversion.$kernelpatchlevel.$kernelsublevel - $kernelname"
echo
echo "${bldred}Clean:${txtrst}"
echo "1) Last Zip Package (${bldred}$cleanzipcheck${txtrst})"
echo "2) Kernel (${bldred}$cleankernelcheck${txtrst})"
echo "${bldgrn}Main Process:${txtrst}"
echo "3) Device Choice (${bldgrn}$target$variant${txtrst})"
echo "4) Toolchain Choice (${bldgrn}$CROSS_COMPILE${txtrst})"
echo "${bldyel}Build Process:${txtrst}"
if ! [ "$maindevicecheck" == "" ]; then
	if ! [ "$CROSS_COMPILE" == "" ]; then
		echo "5) Build $customkernel (${bldyel}$buildprocesscheck${txtrst})"
	else
		echo "Use "4" first."
	fi
else
	echo "Use "3" first."
fi
if [ -f arch/arm/boot/zImage ]; then
	echo "6) Build Zip Package (${bldyel}$zippackagecheck${txtrst})"
fi
if [ -f zip-creator/$zipfile ]; then
	echo "${bldblu}Test Process:${txtrst}"
	echo "7) Copy to device - Via Adb (${bldblu}$adbcopycheck${txtrst})"
	if [ "$adbcopycheck" == "Done" ]; then
		echo "8) Reboot device to recovery"
	fi
fi
echo "${bldmag}Status:${txtrst}"
if ! [ "$BUILDTIME" == "" ]; then
	echo "${bldgrn}Build Time: $(($BUILDTIME / 60)) minutes and $(($BUILDTIME % 60)) seconds.${txtrst}"
fi
if [ "$maindevicecheck" == "" ]; then
	if [ -f arch/arm/boot/zImage ]; then
		echo "${bldblu}You have old Kernel build!${txtrst}"
		buildprocesscheck="Old build"
	fi
elif [ "$CROSS_COMPILE" == "" ]; then
	if [ -f arch/arm/boot/zImage ]; then
		echo "${bldblu}You have old Kernel build!${txtrst}"
		buildprocesscheck="Old build"
	fi
fi
if [ -f zip-creator/$zipfile ]; then
	echo "${bldyel}Zip Saved to zip-creator/$zipfile ${txtrst}"
elif [ "$lszip" -ge 2 ]; then
	echo "${bldblu}You have old Zips Saved on zip-creator folder!${txtrst}"
	ls zip-creator/*.zip ${coloroutputzip}
elif [ "$lszip" -ge 1 ]; then
	echo "${bldblu}You have old Zip Saved on zip-creator folder!${txtrst}"
	ls zip-creator/*.zip ${coloroutputzip}
fi
echo "${bldcya}Menu:${txtrst}"
echo "c) Color ($coloroutputcheck)"
echo "q) Quit"
read -n 1 -p "${txtbld}Choice: ${txtrst}" -s x
case $x in
	1) echo "$x - Cleaning Zips..."; cleanzip; buildsh;;
	2) echo "$x - Cleaning $customkernel..."; cleankernel; buildsh;;
	3) echo "$x - Device choice"; maindevice; buildsh;;
	4) echo "$x - Toolchain choice"; maintoolchain; buildsh;;
	5) if [ -f .config ]; then
		echo "$x - Building Kernel..."; buildprocess; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	6) if [ -f arch/arm/boot/zImage ]; then
		echo "$x - Ziping $customkernel..."; zippackage; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	7) if [ -f zip-creator/*.zip ]; then
		echo "$x - Coping $customkernel..."; adbcopy; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	8) if [ "$adbcopycheck" == "Done" ]; then
		echo "$x - Rebooting to Recovery..."; adb reboot recovery; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	c) setcustomoutput; buildsh;;
	q) echo "Ok, Bye!"; unset zippackagecheck;;
	*) echo "$x - This option is not valid"; sleep 2; buildsh;;
esac
}

# Menu - End

# The core of script is here!

if ! [ -e build.sh ]; then
	echo
	echo "Ensure you run this file from the SAME folder as where it was,"
	echo "otherwise the script will have problems running the commands."
	echo "After you 'cd' to the correct folder, start the build script"
	echo "with the ./build.sh command, NOT with any other command!"
	echo; sleep 3
else
	if [ -f zip-creator/*.zip ]; then
		unset cleanzipcheck
	else
		cleanzipcheck="Done"
	fi

	if [ -f .config ]; then
		unset cleankernelcheck
	else
		cleankernelcheck="Done"
	fi

	buildsh
fi

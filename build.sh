#!/bin/bash
# Base by cybojenix <anthonydking@gmail.com>
# ReWriten by Caio Oliveira aka Caio99BR <caiooliveirafarias0@gmail.com>
# Rashed for the base of zip making
# And the internet for filling in else where

# You need to download https://github.com/TeamVee/android_prebuilt_toolchains
# Clone in the same folder as the kernel

# Clean - Start

cleanzip() {
rm -rf zip-creator/*.zip
rm -rf zip-creator/zImage
rm -rf zip-creator/system/lib/modules/*.ko
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
echo "1) L1 II Single/Dual"
echo "2) L3 II Single"
echo "3) L3 II Dual"
echo "4) L5 NFC"
echo "5) L5 NoNFC"
echo "6) L7 NFC"
echo "7) L7 NoNFC"
read -p "Choice: " -n 1 -s choice
case "$choice" in
	1 ) target="L1II-"; variant=""; echo "$choice - $target$variant"; make cyanogenmod_v1_defconfig &> /dev/null; maindevicecheck="On";;
	2 ) target="L3II-"; variant="Single"; echo "$choice - $target$variant"; make cyanogenmod_vee3_defconfig &> /dev/null; maindevicecheck="On";;
	3 ) target="L3II-"; variant="Dual"; echo "$choice - $target$variant"; make cyanogenmod_vee3ds_defconfig &> /dev/null; maindevicecheck="On";;
	4 ) target="L5-"; variant="NFC"; echo "$choice - $target$variant"; make cyanogenmod_m4_defconfig &> /dev/null; maindevicecheck="On";;
	5 ) target="L5-"; variant="NoNFC"; echo "$choice - $target$variant"; make cyanogenmod_m4_nonfc_defconfig &> /dev/null; maindevicecheck="On";;
	6 ) target="L7-"; variant="NFC"; echo "$choice - $target$variant"; make cyanogenmod_u0_defconfig &> /dev/null; maindevicecheck="On";;
	7 ) target="L7-"; variant="NoNFC"; echo "$choice - $target$variant"; make cyanogenmod_u0_nonfc_defconfig &> /dev/null; maindevicecheck="On";;
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
make -j4
END=$(date +"%s")
BUILDTIME=$(($END - $START))
if [ -f arch/arm/boot/zImage ]; then
	buildprocesscheck="Done"
	unset cleankernelcheck
else
	buildprocesscheck="Something is wrong, contact Dev!"
fi
}

zippackage() {
if [ "$variant" == "Dual" ]; then
	todual
fi

if [ "$target" == "L1II-" ]; then
	tol1ii
fi

if [ "$target" == "L3II-" ]; then
	tol3ii
fi

if [ "$target" == "L7-" ]; then
	tol7
fi

cp arch/arm/boot/zImage zip-creator
find . -name *.ko | xargs cp -a --target-directory=zip-creator/system/lib/modules/ &> /dev/null

zipfile="$customkernel-$target$variant.zip"

cd zip-creator
zip -r $zipfile * -x */.gitignore* &> /dev/null
cd ..

if [ "$target" == "L7-" ]; then
	ofl7
fi

if [ "$target" == "L3II-" ]; then
	ofl3ii
fi

if [ "$target" == "L1II-" ]; then
	ofl1ii
fi

if [ "$variant" == "Dual" ]; then
	tosingle
fi
zippackagecheck="Done"
unset cleanzipcheck
}

todual() {
sed 's/Single/Dual/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tosingle() {
sed 's/Dual/Single/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tol1ii() {
sed 's/m4/vee3/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/14/15/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5 Single/L1 II/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script

}

ofl1ii() {
sed 's/vee3/m4/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/15/14/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L1 II/L5 Single/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tol3ii() {
sed 's/m4/vee3/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/14/15/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5/L3 II/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script

}

ofl3ii() {
sed 's/vee3/m4/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/15/14/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L3 II/L5/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

tol7() {
sed 's/m4/u0/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L5/L7/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
mv zip-creator/META-INF/com/google/android/updater-script-temp zip-creator/META-INF/com/google/android/updater-script
}

ofl7() {
sed 's/u0/m4/' zip-creator/tools/kernel_flash.sh > zip-creator/tools/kernel_flash-temp.sh
rm zip-creator/tools/kernel_flash.sh
mv zip-creator/tools/kernel_flash-temp.sh zip-creator/tools/kernel_flash.sh
sed 's/L7/L5/' zip-creator/META-INF/com/google/android/updater-script > zip-creator/META-INF/com/google/android/updater-script-temp
rm zip-creator/META-INF/com/google/android/updater-script
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
clear
echo "Caio99BR says: Simple $customkernel Build Script."
echo "This is an open source script, feel free to use, edit and share it."
echo "Linux Kernel $kernelversion.$kernelpatchlevel.$kernelsublevel - $kernelname"
echo
echo "Clean:"
echo "1) Last Zip Package ($cleanzipcheck)"
echo "2) Kernel ($cleankernelcheck)"
echo
echo "Main Process:"
echo "3) Device Choice ($target$variant)"
echo "4) Toolchain Choice ($CROSS_COMPILE)"
echo
echo "Build Process:"
if ! [ "$maindevicecheck" == "" ]; then
	if ! [ "$CROSS_COMPILE" == "" ]; then
		echo "5) Build Kernel ($buildprocesscheck)"
	else
		echo "Use "4" first."
	fi
else
	echo "Use "3" first."
fi
if [ -f arch/arm/boot/zImage ]; then
	echo "6) Build Zip Package ($zippackagecheck)"
fi
if [ -f zip-creator/*.zip ]; then
	echo
	echo "Test Process:"
	echo "7) Copy to device - Via Adb ($adbcopycheck)"
fi
if [ "$adbcopycheck" == "Done" ]; then
	echo
	echo "8) Reboot device to recovery"
fi
echo
if ! [ "$BUILDTIME" == "" ]; then
	echo -e "\033[32mBuild Time: $(($BUILDTIME / 60)) minutes and $(($BUILDTIME % 60)) seconds.\033[0m"
	echo
fi
echo "q) Quit"
read -n 1 -p "Choice: " -s x
case $x in
	1) echo "$x - Cleaning Zips..."; cleanzip; buildsh;;
	2) echo "$x - Cleaning Kernel..."; cleankernel; buildsh;;
	3) echo "$x - Device choice"; maindevice; buildsh;;
	4) echo "$x - Toolchain choice"; maintoolchain; buildsh;;
	5) if [ -f .config ]; then
		echo "$x - Building Kernel..."; buildprocess; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	6) if [ -f arch/arm/boot/zImage ]; then
		echo "$x - Ziping Kernel..."; zippackage; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	7) if [ -f zip-creator/*.zip ]; then
		echo "$x - Coping Kernel..."; adbcopy; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
	8) if [ "$adbcopycheck" == "Done" ]; then
		echo "$x - Rebooting to Recovery..."; adb reboot recovery; buildsh
	else
		echo "$x - This option is not valid"; sleep 2; buildsh
	fi;;
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

	if [ -f arch/arm/boot/zImage ]; then
		buildprocesscheck="Done"
	else
		unset buildprocesscheck
	fi

	buildsh
fi

#!/bin/bash
# Original Live by cybojenix <anthonydking@gmail.com>
# New Live/Menu by Caio Oliveira aka Caio99BR <caiooliveirafarias0@gmail.com>
# Colors by Aidas Lukošius aka aidasaidas75 <aidaslukosius75@yahoo.com>
# Toolchains by Suhail aka skyinfo <sh.skyinfo@gmail.com>
# Rashed for the base of zip making
# And the internet for filling in else where

# You need to download https://github.com/TeamVee/android_prebuilt_toolchains
# Clone in the same folder as the kernel to choose a toolchain and not specify a location

# Prepare output customization commands - Start

customcoloroutput() {
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
	coloroutputzip="--color=auto"
fi
if [ "$coloroutput" == "OFF" ]; then
	unset txtbld bldred bldgrn bldyel bldblu bldmag bldcya bldwhi
	coloroutputzip="--color=never"
fi
}

setcustomcoloroutput() {
if [ "$coloroutput" == "ON" ]; then
	coloroutput="OFF"
else
	coloroutput="ON"
fi
}

setcustombuildoutput() {
if [ "$buildoutput" == "ON" ]; then
	buildoutput="OFF"
else
	buildoutput="ON"
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
make clean mrproper &> /dev/null | echo "Cleaning..."
cleankernelcheck="Done"
unset buildprocesscheck target serie variant maindevicecheck BUILDTIME
}

# Clean - End

# Main Process - Start

maindevice() {
echo "-${bldred}First Gen${txtrst}-"
echo "0) L5 NFC            (E610)"
echo "1) L5 NoNFC          (E612/E617)"
echo "2) L7 NFC            (P700)"
echo "3) L7 NoNFC          (P705)"
echo "4) L7 NFC - 8m       (P708)"
echo "-${bldblu}Second Gen${txtrst}-"
echo "5) L1 II Single/Dual (E410/E411/E415/E420)"
echo "6) L3 II Single/Dual (E425/E430/E431/E435)"
echo "7) L7 II NFC         (P710/P712)"
echo "8) L7 II NoNFC       (P713/P714)"
echo "9) L7 II Dual        (P715/P716)"
unset errorchoice
read -p "Choice: " -n 1 -s choice
case "$choice" in
	0 ) target="L5-"; variant="NFC"; defconfig="cyanogenmod_m4_defconfig";;
	1 ) target="L5-"; variant="NoNFC"; defconfig="cyanogenmod_m4_nonfc_defconfig";;
	2 ) target="L7-"; variant="NFC"; defconfig="cyanogenmod_u0_defconfig";;
	3 ) target="L7-"; variant="NoNFC"; defconfig="cyanogenmod_u0_nonfc_defconfig";;
	4 ) target="L7-"; variant="NFC-8m"; defconfig="cyanogenmod_u0_8m_defconfig";;
	5 ) target="L1II-"; variant="SD"; defconfig="cyanogenmod_v1_defconfig";;
	6 ) target="L3II-"; variant="SD"; defconfig="cyanogenmod_vee3_defconfig";;
	7 ) target="L7II-"; variant="NFC"; defconfig="cyanogenmod_vee7_defconfig";;
	8 ) target="L7II-"; variant="NoNFC"; defconfig="cyanogenmod_vee7_nonfc_defconfig";;
	9 ) target="L7II-"; variant="Dual"; defconfig="cyanogenmod_vee7ds_defconfig";;
	* ) echo "$choice - This option is not valid"; sleep .5; errorchoice="ON";;
esac
if ! [ "$errorchoice" == "ON" ]; then
	echo "$choice - $target$variant"; make $defconfig &> /dev/null | echo "Setting..."; maindevicecheck="ON"
	zipfile="$customkernel-$target$variant-$daytime.zip"
fi
}

maintoolchain() {
if [ -d ../android_prebuilt_toolchains ]; then
	echo "1) 4.7 Google GCC"
	echo "2) 4.8 Google GCC"
	echo "3) 4.6.4 Linaro GCC"
	echo "4) 4.7.4 Linaro GCC"
	echo "5) 4.8.4 Linaro GCC"
	echo "6) 4.9.3 Linaro GCC"
	unset errortoolchain
	read -p "Choice: " -n 1 -s toolchain
	case "$toolchain" in
		1 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-eabi-4.7/bin/arm-eabi-"; ToolchainCompile="GCC 4.7";;
		2 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-eabi-4.8/bin/arm-eabi-"; ToolchainCompile="GCC 4.8";;
		3 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-unknown-linux-gnueabi-linaro_4.6.4-2013.05/bin/arm-unknown-linux-gnueabi-"; ToolchainCompile="Linaro 4.6.4";;
		4 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-unknown-linux-gnueabi-linaro_4.7.4-2013.12/bin/arm-unknown-linux-gnueabi-"; ToolchainCompile="Linaro 4.7.4";;
		5 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-linux-gnueabi-linaro_4.8.4-2014.11/bin/arm-linux-gnueabi-"; ToolchainCompile="Linaro 4.8.4";;
		6 ) export CROSS_COMPILE="../android_prebuilt_toolchains/arm-cortex-linux-gnueabi-linaro_4.9.3-2015.03/bin/arm-cortex-linux-gnueabi-"; ToolchainCompile="Linaro 4.9.3";;
		* ) echo "$toolchain - This option is not valid"; sleep .5; errortoolchain="ON";;
	esac
	if ! [ "$errortoolchain" == "ON" ]; then
		echo "$ToolchainCompile"
	fi
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
NR_CPUS=$(grep -c ^processor /proc/cpuinfo)
if [ "$NR_CPUS" -le "2" ]; then
	NR_CPUS="4"
fi
echo "${bldblu}Building $customkernel with $NR_CPUS jobs at once${txtrst}"
rm -rf arch/$ARCH/boot/zImage
if [ "$buildoutput" == "ON" ]; then
	make -j${NR_CPUS}
else
	make -j${NR_CPUS} &>/dev/null | loop
fi
END=$(date +"%s")
BUILDTIME=$(($END - $START))
if [ -f arch/$ARCH/boot/zImage ]; then
	buildprocesscheck="Done"
	unset cleankernelcheck
else
	buildprocesscheck="Something goes wrong"
fi
}

loop() {
LEND=$(date +"%s")
LBUILDTIME=$(($LEND - $START))
echo -ne "\r\033[K"
echo -ne "${bldgrn}Build Time: $(($LBUILDTIME / 60)) minutes and $(($LBUILDTIME % 60)) seconds.${txtrst}"
sleep 1
if ! [ -f arch/$ARCH/boot/zImage ]; then
	loop
fi
}

# Build Process - End

# Zip Process - Start

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

cp arch/$ARCH/boot/zImage zip-creator
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

# Zip Process - End

# ADB - Start

adbcopy() {
echo "Script says: You want to copy to Internal or External Card?"
echo "i) For Internal"
echo "e) For External"
read -p "Choice: " -n 1 -s adbcoping
case "$adbcoping" in
	i ) echo "Coping to Internal Card..."; adb shell rm -rf /storage/sdcard0/$zipfile &> /dev/null; adb push zip-creator/$zipfile /storage/sdcard0/$zipfile &> /dev/null; adbcopycheck="Done";;
	e ) echo "Coping to External Card..."; adb shell rm -rf /storage/sdcard1/$zipfile &> /dev/null; adb push zip-creator/$zipfile /storage/sdcard1/$zipfile &> /dev/null; adbcopycheck="Done";;
	* ) echo "$adbcoping - This option is not valid"; sleep .5;;
esac
}

# ADB - End

# Misc - Start

buildtimemisc() {
if ! [ "$BUILDTIME" == "" ]; then
	echo "${bldgrn}Build Time: $(($BUILDTIME / 60)) minutes and $(($BUILDTIME % 60)) seconds.${txtrst}"
fi
}

zippackagemisc() {
echo "${bldyel}Zip Saved to zip-creator/$zipfile${txtrst}"
}

# Misc - End

# Menu - Start

buildsh() {
kernelversion=`cat Makefile | grep VERSION | cut -c 11- | head -1`
kernelpatchlevel=`cat Makefile | grep PATCHLEVEL | cut -c 14- | head -1`
kernelsublevel=`cat Makefile | grep SUBLEVEL | cut -c 12- | head -1`
kernelname=`cat Makefile | grep NAME | cut -c 8- | head -1`
clear
echo "Simple Linux Kernel Build Script ($(date +%d"/"%m"/"%Y))"
echo "$customkernel $kernelversion.$kernelpatchlevel.$kernelsublevel - $kernelname"
echo
echo "-${bldred}Clean:${txtrst}-"
echo "1) Last Zip Package (${bldred}$cleanzipcheck${txtrst})"
echo "2) Kernel (${bldred}$cleankernelcheck${txtrst})"
echo "-${bldgrn}Main Process:${txtrst}-"
echo "3) Device Choice (${bldgrn}$target$variant${txtrst})"
echo "4) Toolchain Choice (${bldgrn}$ToolchainCompile${txtrst})"
echo "-${bldyel}Build Process:${txtrst}-"
if [ "$maindevicecheck" == "" ]; then
	echo "Use "3" first."
else
	if [ "$CROSS_COMPILE" == "" ]; then
		echo "Use "4" first."
	else
		echo "5) Build $customkernel (${bldyel}$buildprocesscheck${txtrst})"
	fi
fi
if [ -f arch/$ARCH/boot/zImage ]; then
	echo "6) Build Zip Package (${bldyel}$zippackagecheck${txtrst})"
fi
if [ -f zip-creator/$zipfile ]; then
	echo "${bldblu}Test Process:${txtrst}"
	echo "7) Copy to device - Via Adb (${bldblu}$adbcopycheck${txtrst})"
	if [ "$adbcopycheck" == "Done" ]; then
		echo "8) Reboot device to recovery"
	fi
fi
echo "-${bldmag}Status:${txtrst}-"
buildtimemisc
if [ "$maindevicecheck" == "" ]; then
	if [ -f arch/$ARCH/boot/zImage ]; then
		echo "${bldblu}You have old Kernel build!${txtrst}"
		buildprocesscheck="Old build"
	fi
elif [ "$CROSS_COMPILE" == "" ]; then
	if [ -f arch/$ARCH/boot/zImage ]; then
		echo "${bldblu}You have old Kernel build!${txtrst}"
		buildprocesscheck="Old build"
	fi
fi
if [ -f zip-creator/$zipfile ]; then
	zippackagemisc
elif [ `ls zip-creator/*.zip 2>/dev/null | wc -l` -ge 2 ]; then
	echo "${bldblu}You have old Zips Saved on zip-creator folder!${txtrst}"
	ls zip-creator/*.zip ${coloroutputzip}
elif [ `ls zip-creator/*.zip 2>/dev/null | wc -l` -ge 1 ]; then
	echo "${bldblu}You have old Zip Saved on zip-creator folder!${txtrst}"
	ls zip-creator/*.zip ${coloroutputzip}
fi
echo "-${bldcya}Menu:${txtrst}-"
if [ "$coloroutput" == "ON" ]; then
echo "c) Color (${bldcya}$coloroutput${txtrst})"
else
echo "c) Color ($coloroutput)"
fi
if [ "$buildoutput" == "ON" ]; then
echo "o) View Build Output (${bldcya}$buildoutput${txtrst})"
else
echo "o) View Build Output ($buildoutput)"
fi

echo "q) Quit"
read -n 1 -p "${txtbld}Choice: ${txtrst}" -s x
case $x in
	1) echo "$x - Cleaning Zips"; cleanzip; buildsh;;
	2) echo "$x - Cleaning $customkernel"; cleankernel; buildsh;;
	3) echo "$x - Device choice"; maindevice; buildsh;;
	4) echo "$x - Toolchain choice"; maintoolchain; buildsh;;
	5) if [ -f .config ]; then
		echo "$x - Building $customkernel"; buildprocess; buildsh
	else
		echo "$x - This option is not valid"; sleep .5; buildsh
	fi;;
	6) if [ -f arch/$ARCH/boot/zImage ]; then
		echo "$x - Ziping $customkernel"; zippackage; buildsh
	else
		echo "$x - This option is not valid"; sleep .5; buildsh
	fi;;
	7) if [ -f zip-creator/*.zip ]; then
		echo "$x - Coping $customkernel"; adbcopy; buildsh
	else
		echo "$x - This option is not valid"; sleep .5; buildsh
	fi;;
	8) if [ "$adbcopycheck" == "Done" ]; then
		echo "$x - Rebooting to Recovery..."; adb reboot recovery; buildsh
	else
		echo "$x - This option is not valid"; sleep .5; buildsh
	fi;;
	c) setcustomcoloroutput; customcoloroutput; buildsh;;
	o) setcustombuildoutput; buildsh;;
	q) echo "Ok, Bye!"; unset zippackagecheck;;
	*) echo "$x - This option is not valid"; sleep .5; buildsh;;
esac
}

# Menu - End

# The core of script is here!

if [ -e build.sh ]; then
	customkernel=CAFKernel
	export ARCH=arm
	daytime=$(date +%d""%m""%Y)
	zipfile="$customkernel-$target$variant-$daytime.zip"

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

	if [ "$coloroutput" == "" ]; then
		coloroutput="ON"
	fi

	if [ "$buildoutput" == "" ]; then
		buildoutput="ON"
	fi

	customcoloroutput

	if [[ "$1" = "--help" || "$1" = "-h" ]]; then
		echo "Simple Linux Kernel Build Script"
		echo "Use: . build.sh [OPTION]"
		echo
		echo "  -l, --live           use this for fast building without menu"
		echo "  -C, --clean          use this for fast cleaning (Zip/Kernel)"
		echo
		echo "This is an open source script, feel free to use, edit and share it."
		echo "By default this have a menu to interactive with kernel building."
		echo "And for complete usage download:"
		echo "https://github.com/TeamVee/android_prebuilt_toolchains"
		echo "to same folder of kernel for fast toolchain select."
	elif [[ "$1" = "--live" || "$1" = "-l" ]]; then
		echo "--Choose a Device--"
		maindevice
		echo "--Choose a Toolchain--"
		maintoolchain; buildprocess; buildtimemisc; zippackage; zippackagemisc
	elif [[ "$1" = "--clean" || "$1" = "-C" ]]; then
		cleanzip; cleankernel
	else
		buildsh
	fi
else
	echo
	echo "Ensure you run this file from the SAME folder as where it was,"
	echo "otherwise the script will have problems running the commands."
	echo "After you 'cd' to the correct folder, start the build script"
	echo "with the ./build.sh command, NOT with any other command!"
	echo; sleep 1
fi

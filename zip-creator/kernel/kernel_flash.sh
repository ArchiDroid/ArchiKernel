#!/sbin/sh
cd /tmp/
/tmp/dd if=/dev/block/mmcblk0p9 of=/tmp/boot.img
/tmp/unpackbootimg /tmp/boot.img
/tmp/mkbootimg --kernel /tmp/zImage --ramdisk /tmp/boot.img-ramdisk.gz --cmdline 'androidboot.hardware=m4 androidboot.selinux=permissive lge.signed_image=false' --base 0x00200000  --ramdiskaddr 0x01400000 -o /tmp/newboot.img
/tmp/dd if=/tmp/newboot.img of=/dev/block/mmcblk0p9
busybox chmod 644 /system/lib/modules/*

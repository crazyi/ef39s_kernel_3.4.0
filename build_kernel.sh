#!/bin/bash
###############################################################################
#
#                           Kernel Build Script 
#
###############################################################################
# 2011-10-24 effectivesky : modified
# 2010-12-29 allydrop     : created
###############################################################################
##############################################################################
# set toolchain
##############################################################################
# export ARCH=arm
# export CROSS_COMPILE=$PWD/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

##############################################################################
# make zImage
##############################################################################
mkdir -p ./obj/KERNEL_OBJ/
make O=./obj/KERNEL_OBJ/ msm8660_im-a800s_perf_defconfig
make -j4 O=./obj/KERNEL_OBJ/

##############################################################################
# Copy Kernel Image
##############################################################################
cp -f ./obj/KERNEL_OBJ/arch/arm/boot/zImage .


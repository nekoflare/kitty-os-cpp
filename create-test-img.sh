#!/bin/bash
set -e

# Create a 74MB image file
dd if=/dev/zero of=image.img bs=1M count=76

# Create a loopback device for the image
LOOP_DEVICE=$(losetup -f)
losetup -P $LOOP_DEVICE image.img

# Ensure cleanup on exit
trap "losetup -d $LOOP_DEVICE" EXIT

# Create a GPT partition table
parted $LOOP_DEVICE --script mklabel gpt

# Add partitions: EXT4, FAT32, NTFS
parted $LOOP_DEVICE --script mkpart primary ext4 1MiB 5MiB   # EXT4 4MB
parted $LOOP_DEVICE --script mkpart primary fat32 5MiB 51MiB  # FAT32 46MB
parted $LOOP_DEVICE --script mkpart primary ntfs 51MiB 75MiB   # NTFS 24MB

# Format the partitions
mkfs.ext4 ${LOOP_DEVICE}p1
mkfs.fat -F 32 ${LOOP_DEVICE}p2
mkfs.ntfs -Q ${LOOP_DEVICE}p3

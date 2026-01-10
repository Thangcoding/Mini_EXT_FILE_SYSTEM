#!/bin/bash

####################################
# CONFIG
####################################
DISK_IMG="disk.img"
DISK_SIZE_MB=64

EXT4_MNT="/mnt/ext4_test"
MINI_MNT="/mnt/mini_ext_test"

RESULT="eval_result.txt"

####################################
# INIT
####################################
echo "EXT4 vs MINI-EXT TIME EVALUATION" > $RESULT
echo "===============================" >> $RESULT
echo "" >> $RESULT

####################################
# CLEAN & PREPARE
####################################
sudo umount $EXT4_MNT 2>/dev/null
sudo umount $MINI_MNT 2>/dev/null

sudo mkdir -p $EXT4_MNT $MINI_MNT
sudo chmod 777 $EXT4_MNT $MINI_MNT

####################################
# CREATE DISK IMAGE
####################################
dd if=/dev/zero of=$DISK_IMG bs=1M count=$DISK_SIZE_MB status=none

####################################
# FORMAT EXT4 (NO JOURNAL, 1 GROUP)
####################################
sudo mkfs.ext4 -F -O ^has_journal -g 8192 $DISK_IMG > /dev/null 2>&1
sudo mount -o loop $DISK_IMG $EXT4_MNT

####################################
# FORMAT MINI-EXT
####################################
./mkfs > /dev/null
sudo mount -o loop $DISK_IMG $MINI_MNT

####################################
# FUNCTION: TIME TEST
####################################
run_test() {
    FS_NAME=$1
    BASE=$2

    echo "[$FS_NAME]" >> $RESULT

    # Test 1: Create 3000 dirs
    t=$( (time for i in {1..3000}; do mkdir $BASE/dir_$i 2>/dev/null; done) 2>&1 | grep real | awk '{print $2}')
    echo "Create 3000 dirs: $t" >> $RESULT

    # Test 2: Delete 3000 dirs
    t=$( (time for i in {1..3000}; do rmdir $BASE/dir_$i 2>/dev/null; done) 2>&1 | grep real | awk '{print $2}')
    echo "Delete 3000 dirs: $t" >> $RESULT

    # Test 3: Create 100 files (1MB)
    t=$( (time for i in {1..100}; do dd if=/dev/zero of=$BASE/file_$i bs=1M count=1 status=none; done) 2>&1 | grep real | awk '{print $2}')
    echo "Create 100 files (1MB): $t" >> $RESULT

    # Test 4: Read 100 files
    t=$( (time for i in {1..100}; do cat $BASE/file_$i > /dev/null 2>/dev/null; done) 2>&1 | grep real | awk '{print $2}')
    echo "Read 100 files (1MB): $t" >> $RESULT

    # Test 5: Delete 100 files
    t=$( (time for i in {1..100}; do rm -f $BASE/file_$i 2>/dev/null; done) 2>&1 | grep real | awk '{print $2}')
    echo "Delete 100 files (1MB): $t" >> $RESULT

    echo "" >> $RESULT
}

####################################
# RUN TESTS
####################################
run_test "EXT4" $EXT4_MNT
run_test "MINI-EXT" $MINI_MNT

####################################
# CLEANUP
####################################
sudo umount $EXT4_MNT
sudo umount $MINI_MNT

echo "DONE. Results saved in $RESULT"

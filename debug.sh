#!/bin/bash

pushd `dirname $0` > /dev/null
MP3_DIR=`pwd`
popd > /dev/null

WORKING_DIR=$MP3_DIR/student-distrib

setsid qemu-system-i386 -hda $WORKING_DIR/mp3.img -m 256 -s -S -name mp3 &
QEMU_PID=$!

gdb -cd=$WORKING_DIR -ex 'target remote localhost:1234' bootimg

kill $QEMU_PID >/dev/null 2>&1

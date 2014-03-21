#!/bin/bash

pushd `dirname $0` > /dev/null
mp3_dir=`pwd`
popd > /dev/null

qemu-system-i386 -hda $mp3_dir/student-distrib/mp3.img -m 256 -gdb tcp:127.0.0.1:1234 -S -name mp3

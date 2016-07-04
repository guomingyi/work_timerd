#!/bin/bash
#
###############################################################
build_prj=v3991
build_type=userdebug
prj_path=/home/android/work/prj/3991/debug/
#
TIMER_SET=11:10
CMD_EXEC_PATH=/home/android/work/script/work/my-task.sh
###############################################################


echo "start repo sync & build $prj_path" > ~/every-day-build.log
echo ""

cd $prj_path
repo sync -j4 >> ~/every-day-build.log
cd baseline;

source build/envsetup.sh ;
source mbldenv.sh;

lunch full_$build_prj-$build_type
#exit 1

make clean;
make -j8 2>&1 |tee build.log;

~/work/script/release.sh;


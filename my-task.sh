#!/bin/bash
# 
TIMER_SET=17:28


# develop.
###############################################################
build_prj=v3991
build_type=userdebug
prj_path=/home/android/work/prj/3991/debug/
###############################################################
echo "start repo sync & build" > ~/every-day-build.log
echo "$build_prj | $build_type |$prj_path" >> ~/every-day-build.log
echo ""
rm -rf /home/android/timer-log.txt

cd $prj_path
repo sync -j4 >> ~/every-day-build.log
cd baseline;

source build/envsetup.sh ;
source mbldenv.sh;
lunch full_$build_prj-$build_type >> ~/every-day-build.log

echo ""
echo "start make... " >> ~/every-day-build.log

#make clean;
make -j8 2>&1 |tee build.log;

echo ""
echo "start release.sh... " >> ~/every-day-build.log
~/work/script/release.sh >> ~/every-day-build.log
###############################################################




# master.
###############################################################
build_prj=v3991
build_type=user
prj_path=/home/android/work/prj/3991/master/
###############################################################
echo "start repo sync & build" > ~/every-day-build.log
echo "$build_prj | $build_type |$prj_path" >> ~/every-day-build.log
echo ""
rm -rf /home/android/timer-log.txt

cd $prj_path
repo sync -j4 >> ~/every-day-build.log
cd baseline;

source build/envsetup.sh ;
source mbldenv.sh;
lunch full_$build_prj-$build_type >> ~/every-day-build.log

echo ""
echo "start make... " >> ~/every-day-build.log

make clean;
make -j8 2>&1 |tee build.log;

echo ""
echo "start release.sh... " >> ~/every-day-build.log
~/work/script/release.sh >> ~/every-day-build.log
###############################################################

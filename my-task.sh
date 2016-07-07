#!/bin/bash
# 
# default config.
###############################################################
TIMER_SET=23:01
root_dir=/home/android
build_prj=v3991
build_type=userdebug
prj_path=$root_dir/work/prj/3991/debug/
###############################################################

if [ -n "$1" ] ; then
    build_prj=$1
fi

if [ -n "$2" ] ; then
    build_type=$2
fi

if [ -n "$3" ] ; then
   prj_path=$3
fi

if [ ! -d $root_dir/build-log ] ; then
   mkdir -p $root_dir/build-log
fi

rm -rf $root_dir/build-log/everyday-build-$build_prj-$build_type.log 
rm -rf $root_dir/build-log/$build_prj-$build_type.log

echo "$build_prj | $build_type |$prj_path" >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log

#exit 0

cd $prj_path
repo sync -j4 2>&1 |tee $root_dir/build-log/everyday-build-$build_prj-$build_type.log &&
cd baseline &&

source build/envsetup.sh &&
source mbldenv.sh &&
lunch full_$build_prj-$build_type >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log &&

echo "start make... " >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log &&

make clean &&

make -j8 2>&1 |tee build.log && 
$root_dir/work/script/release.sh &&
echo "--end-- " >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log
###############################################################




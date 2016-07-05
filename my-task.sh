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
rm -rf $root_dir/build-log/$build_prj-$build_type.log.txt

echo "start repo sync & build-$build_prj" >> $root_dir/build-log/$build_prj-$build_type.log.txt
echo "$build_prj | $build_type |$prj_path" >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log
echo ""

cd $prj_path
repo sync -j4 2>&1 |tee $root_dir/build-log/everyday-build-$build_prj-$build_type.log
cd baseline;

source build/envsetup.sh ;
source mbldenv.sh;
lunch full_$build_prj-$build_type >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log

echo ""
echo "start make... " >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log

make clean;

make -j8 2>&1 |tee build.log;

echo ""
echo "start release.sh... " >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log
$root_dir/work/script/release.sh >> $root_dir/build-log/everyday-build-$build_prj-$build_type.log
###############################################################

exit 0;


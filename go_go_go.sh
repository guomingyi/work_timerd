#!/bin/bash
#
# 1 > ./go_go_go.sh 
# 2 > ./go_go_go.sh -d 
#
# default config.
#
# ./work_timerd  debug  prj-name  build-type  prj-path  timer-set
#

./work_timerd $1 p4661 userdebug /home/android/work/prj/3991/debug/ 22:00

./work_timerd $1 v3991 user /home/android/work/prj/3991/debug/ 01:00

./work_timerd $1 v3991 user /home/android/work/prj/3991/master/ 04:00



exit 0

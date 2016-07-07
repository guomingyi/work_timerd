#!/bin/bash
#
# 1 > ./go_go_go.sh 
# 2 > ./go_go_go.sh -d 
#
# default config.
#
# ./work_timerd  -d  -p prj-name  -b build-type  -r prj-path  -t timer-set &
#

./work_timerd $1 -p v3991 -b userdebug -r /home/android/work/prj/3991/debug/ -t 13:30 &

sleep 1

./work_timerd $1 -p v3991 -b user -r /home/android/work/prj/3991/master/ -t 14:30 &

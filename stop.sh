#!/bin/bash
# 

pid=$(ps -e |grep "work_timerd" |awk '{print $1}')
if [ -n "$pid" ] ; then
	echo "kill work_timerd - $pid"
	kill -9 $pid
fi

pid2=$(ps -e |grep "make" |awk '{print $1}')
if [ -n "$pid2" ] ; then
	echo "kill make - $pid2"
	kill -9 $pid2
fi

pid3=$(ps -e |grep "ssh" |awk '{print $1}')
if [ -n "$pid3" ] ; then
	echo "kill ssh - $pid3"
	kill -9 $pid3
fi







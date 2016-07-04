#!/bin/bash
# 

pid=$(ps -e |grep $1 |awk '{print $1}')
echo "$pid"

kill $pid

exit 0;

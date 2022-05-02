#!/usr/bin/bash

rm -f statistic.txt file_for_lock.txt.lck
echo "---Statistic---" > statistic.txt

for (( i=1; i<11; i++ ))
do
  ./lock & sleep 1
done

sleep 60
killall -SIGINT lock
sleep 1

cat statistic.txt
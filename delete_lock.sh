#!/usr/bin/bash

rm -f statistic.txt file_for_lock.txt.lck
echo "---Statistic---" > statistic.txt

./lock &
sleep 1
rm -f file_for_lock.txt.lck
sleep 1

cat statistic.txt

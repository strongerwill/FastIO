#!/bin/bash

i=30
while [ $i -gt 0 ]
do
	netperf -H 192.168.1.99 -l 60 -- -s 1024 -S 2048 -D >>./data/small_buffer_perf_firefox_aft_opt.txt
	i=$[$i - 1]
done

#!/bin/bash

i=30
while [ $i -gt 0 ]
do
	netperf -H 192.168.1.99 -l 60 -- -D >>./data/default_perf_firefox_aft_opt.txt
	#netperf -l 60 -4 -f m -t TCP_CRR -H 192.168.1.99 -p 9991 -- -r 64,64 >>perf_firefox_bef_opt.txt
	i=$[$i - 1]
done

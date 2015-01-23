#!/usr/bin/env python

f = open('netperf.txt', 'r')
allLines = f.readlines()
f.close()
count = 0
thr_ave = 0
thr = 0
for line in allLines:
	if len(line) > 1:
		if '87380' in line:
			count +=1
	        	line.strip()
			str(line)
			index = line.find('60.00',0, len(line))
			index += 5
			temp_word = '' 
			temp_word += line[index:]
			print temp_word.strip()
			thr += float(temp_word)

thr_ave = thr/count
print '%f' % thr_ave 


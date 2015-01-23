#!/usr/bin/env python

import os

dir_t = '/home/henry/enabled_steady_netperf/data/temp'

def processfile(file_t):
   f = open(file_t, 'r')
   allLines = f.readlines()
   f.close()
   count = 0
   thr_ave = 0
   throughput = 0
   min_num = 0
   max_num = 0
   for line in allLines:
	if len(line) > 1:
		if '87380' in line:
			count +=1
	        	line.strip()
			str(line)
			index = line.find('60.',0, len(line))
			index += 8 
			temp_word = '' 
			temp_word += line[index:]
			#print temp_word.strip()
			f_temp_word = float(temp_word)
			if count == 1:
				min_num = f_temp_word
				max_num = f_temp_word
			if min_num > f_temp_word:
				min_num = f_temp_word
			if max_num < f_temp_word:
				max_num = f_temp_word
			
			throughput += f_temp_word
	

   thr_ave = throughput/count
   print '%f  range: [%f %f]' % (thr_ave, min_num, max_num)

def readfile(dir_t):
	for f in os.listdir(dir_t):
		file = os.path.join(dir_t,f)
		if os.path.isdir(file):
			continue
		elif os.path.isfile(file):
			print 'file is' + file
			processfile(file)

readfile(dir_t)



   	


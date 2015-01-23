#!/usr/bin/python
import os
import time
import base64
i=0
j=0
os.system('/usr/bin/firefox &')
time.sleep(1)
while j<294:
	os.system('/usr/bin/firefox -safe-mode -new-tab http://www.baidu.com')
	#env1=base64.b64encode(v1)
	#env2=base64.b64decode(env1)
	#print env2	
	time.sleep(1)
	j += 1 
j=0
os.system('wmctrl -c firefox')
time.sleep(5)
 
while i<30:
	os.system('/usr/bin/firefox &')
	time.sleep(1)
	while j<297:
		os.system('/usr/bin/firefox -safe-mode -new-tab http://www.baidu.com')
		#env1=base64.b64encode(v1)
		#env2=base64.b64decode(env1)
		#print env2	
		time.sleep(1)
		j += 1 
	j=0
	os.system('wmctrl -c firefox')
	#os.system('killall -9 firefox')
	#os.system('kill -9 `jobs -p`')
	time.sleep(2)
	i += 1 

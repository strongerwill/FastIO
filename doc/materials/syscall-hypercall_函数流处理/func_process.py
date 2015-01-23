#!/usr/bin/python
#C:\python33\
import string
import re
finalResult={}   

def readBefore():
	with open("result.txt") as f:
		while True:
			line=f.readline().strip('\n')
			if line:
				finalResult[line]=int(f.readline().strip('\n'))
				#print finalResult[line]
			else:
				break

def readLog():
	pid_trace_flag=0
	start_flag=0
	#test_flag=0
	midFile=file('midResult.txt','w') 
	with open("hypercall.log") as f:
		for line in f.readlines():
			if '\n' == line:
				continue
			if re.search('.+\[.+\].?\-\-\-\-start\-\-\-\-.?',line) is not None:
				start_flag=1
				continue
			if re.search('.+\[.+\].?\-\-\-\-end\-\-\-\-.?',line) is not None:
				pid_trace_flag=0
				start_flag=0
				midFile.write('\n')
				continue
			if start_flag==1:
				if re.search('.+\[.+\].?Pid.+comm\:.+Tainted\:.?G.+',line) is not None:
					s3=line.split('comm:')
					s4=s3[1].split('Tainted')
					s5=s4[0].strip()
					pid_trace_flag+=1
					#test_flag+=1
					continue
				if re.search('.+\[.+\].?Call.?Trace\:.?',line) is not None:
					pid_trace_flag+=1
					#test_flag+=1
					continue
				#if test_flag==2:
				#	if 'hrtimer_cpu_notify' in line:
				#		print line
				#test_flag=0
				if re.search('.+\>\[.+\].+\[\<.+\>\].+\+0x.+\/.+',line) is not None:
					s1=line.split('>]')
					s2=s1[1].split('+')
						
					if pid_trace_flag==2:
						midFile.write('function flow: ' + s5 + s2[0] + '->')
						pid_trace_flag=0
						s5=0
					else:
						midFile.write(s2[0]+"->")
						#midFile.write(s2[0]+"->")
					
				else:
					#print line
					midFile.write('fatal error'+'->')
					
	midFile.close()

def refresh():
	counter=0
	counter_new=0
	with open('midResult.txt') as f:
		for line in f.readlines():
			if '\n' == line:
				continue
			if 'fatal error' in line:
				continue
			if re.search('^function flow\:.+',line) is None:
					continue
			line=line.strip('\n')
			counter+=1
			if line in finalResult.keys():  
				finalResult[line]+=1
				#print finalResult[line]
			else:
				counter_new+=1
				finalResult[line]=1
	finalFile=file('result.txt','w')
	result1=sorted(finalResult.iteritems(), key=lambda d:d[1], reverse=True) 
	for i in result1:
		finalFile.write("%s\n%s\n" %(i[0],i[1]))
		#finalFile.write(i[0])
		#finalFile.write('\n')
		#finalFile.write(i[1])
		#finalFile.write('\n')
	finalFile.close()
	print("Count %d, new count %d" %(counter,counter_new))

readBefore()
readLog()
refresh()	

#!/usr/bin/python
# -*- coding: utf-8 -*-
import time
import socket
import sys
import subprocess
import os
import ast

global code
code = 0
 
if len(sys.argv) >= 2:
	try:
		socket.inet_aton(sys.argv[1])
	except socket.error:
		print ("ERROR... Invalid IP address")
	print "Connecting to " + str(sys.argv[1]) + "..."
	
	try:
		client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client.settimeout(1)	
		client.connect((sys.argv[1], 333))
		print "TCP client connected successfully."
		
	except Exception,e:
		sys.exit(-1)

	try:
		if client.recv(1024)=="OK\n" :
			print "Connection was sucessfuly.\n"
		else:
			print "ERROR... Connection wasn't successfuly\n."
			sys.exit(-1)
	except:
            print "\nERROR... Timeout.\n"
else:
	print "Input argument invalid."
	sys.exit(-1)
	
time.sleep(1)

if len(sys.argv) == 3:
	period = sys.argv[2];
else:
	period = "200";
client.send("imu:" + period + '\n')
	
	

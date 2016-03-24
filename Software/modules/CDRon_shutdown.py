#!/usr/bin/python
import time
import serial
import sys

def shutdown(fd_serial):
	print "Shutting down system..."
	fd_serial.write("0\n")
	time.sleep(0.1)
	if(fd_serial.inWaiting() != 0):
	        receive = fd_serial.readline()
		if receive == "OK\n":
			print("Downed system.\n")
			sys.exit(1)
		else:
			print("ERROR... System cannot be downed\n")
	else:
		print("ERROR... System cannot be downed\n")
	

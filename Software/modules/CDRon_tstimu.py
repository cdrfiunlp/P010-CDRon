#!/usr/bin/python
import time
import getpass
import serial

def tst_imu(fd_serial):
	print "Initialized IMU test...\n"

	fd_serial.write("3\n")
	time.sleep(0.1)
	if(fd_serial.inWaiting() != 0):
	        receive = fd_serial.readline()
		if receive == "OK\n":
			try:
				print("Reading IMU...\n")
				while(True):
					if(fd_serial.inWaiting() != 0):
						receive = fd_serial.readline()
						print receive
			except KeyboardInterrupt:
				print("Finished reading Imu.\n")
		else:
			print("ERROR... Connection cannot be established.\n\n")
	else:
		print("ERROR... Connection cannot be established.\n\n")
	

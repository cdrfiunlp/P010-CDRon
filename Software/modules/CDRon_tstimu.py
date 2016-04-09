#!/usr/bin/python
import time
import getpass
import serial

def tst_imu(fd_serial):
   print "Initialized IMU test...\n"
   try:
	while(True):
		time.sleep(1)
		fd_serial.flushInput()
		fd_serial.write("3\n")
		time.sleep(0.1)
		if(fd_serial.inWaiting() != 0):
			receive = fd_serial.readline()
			receive = receive.split(':')
			print "Pitch: " + receive[0] + " || Roll: " + receive[1] + " || Yaw: " + receive[2]
		else:
			print("ERROR... Cannot be read the IMU.\n\n")
			return
   except KeyboardInterrupt:
	print("\n\nReading of IMU finished.\n")
	

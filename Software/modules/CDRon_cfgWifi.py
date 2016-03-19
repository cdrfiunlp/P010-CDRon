#!/usr/bin/python
import time
import getpass
import serial

def configure_wifi(fd_serial):
	print "Initialized Wifi configuration...\n"
	SSID= raw_input("-> SSID: ")
	PASS= getpass.getpass("-> Password: ")
	fd_serial.write("1\n" + SSID + "\n" + PASS + "\n")
	time.sleep(0.1)
	if(fd_serial.inWaiting() != 0):
	        receive = fd_serial.readline()
		if receive == "OK\n":
			print("Wifi configured correctly")
		else:
			print("ERROR... Wifi can not be configured correctly\n\n")
	else:
		print("ERROR... Wifi can not be configured correctly\n\n")
	

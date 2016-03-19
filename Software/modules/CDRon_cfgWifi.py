#!/usr/bin/python
import time
import getpass
import serial

def configure_wifi(fd_serial):
	print "Initialized Wifi configuration...\n"
	SSID= raw_input("-> SSID: ")
	PASS= getpass.getpass("-> Password: ")
	fd_serial.write("startup config\n")
	while(1):
		time.sleep(0.1)

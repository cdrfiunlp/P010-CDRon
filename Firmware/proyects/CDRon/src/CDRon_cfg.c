/** \brief Configuration source file
 **
 ** Configuration source file of the CDRon project
 **
 **/


/*==================[inclusions]=============================================*/
#include "CDRon_cfg.h"
#include "CDRon_lib.h"
#include "CDRon.h"
#include "ciaaDriverUart.h"
#include "os.h"               /* <= operating system header */

/*==================[macros and definitions]=================================*/
#define SERIAL_WIFI fd_uartWIFI

/*==================[external data declaration]==============================*/
extern int32_t fd_i2c, fd_uartUSB, fd_uartWIFI;

extern struct status_struct status;
extern struct BRUSHLESS_struct BRUSHLESS;
extern volatile struct WIFI_struct WIFI;
extern volatile struct IMU_struct IMU;



/****************************************************************/
/******************* Configuration Tasks ************************/
/****************************************************************/

/** \brief Startup configuration task
 *
 * This task startup the configuration mode, if receive "startup config\n"
 */

TASK(StartupConfig)
{
	   char buf[64]={0};   // buffer for uart operation (modified length in ciaaDriverUart.c)
	   int32_t ret=0;      // return value variable for posix calls


	   // Startup configuration mode
	   ciaaPOSIX_write(fd_uartUSB, "startup config?\n", ciaaPOSIX_strlen("startup config?\n"));
	   CDRon_delayMs(100);
	   ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);

	   // If PC send "startup config\n" -> Start configuration mode
	   if(ret > 0){
		   ciaaPOSIX_read(fd_uartUSB, buf, 64);
		   if(ciaaPOSIX_strcmp(buf,"startup config\n")==0){
			   status.mode = MODE_CONFIG;	// Change system status
		       ActivateTask(ConfigMode);	// init config task
		   }
	   }

	  TerminateTask();
}

/** \brief Main configuration task
 *
 * This task wait indefinitely for a remote user instruction
 */

TASK(ConfigMode){
	char buf[64]={0};   /* buffer for uart operation (modified length in ciaaDriverUart.c    */
	int32_t ret=0;      /* return value variable for posix calls  */

	WIFI.clientID = -2;	//indicate tcp server off

	status.mode = MODE_TEST; // Activate mode test

	while(1){
		ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
		if(ret > 0){ // Wait for remote user instruction

			ciaaPOSIX_read(fd_uartUSB, buf, 3);
			ret = strtol(buf,NULL,10);
			switch (ret){

				/* Wifi configuration */
				case 1:
					ActivateTask(Wifi_cfg);
					break;

				/* Brushless motor test */
				case 2:
					ActivateTask(Brushless_tst);
					break;

				/* IMU test */
				case 3:
					ActivateTask(IMU_tst);
					break;

				/* Wifi test */
				case 4:
					if(WIFI.clientID == -2)
						/* if clientID == -2 =! Active TCP server */
						if(WIFI_serverTCP() != -1)
							SetRelAlarm(wifiPeriodicCheck,100,100);
						else // TCP server not connected
							break;
					/* Send IP address for connection */
					ciaaPOSIX_write(fd_uartUSB, WIFI.IPaddress, ciaaPOSIX_strlen(WIFI.IPaddress));
					CDRon_delayMs(1);
					ciaaPOSIX_write(fd_uartUSB, "\n", ciaaPOSIX_strlen("\n"));

					break;

				/* Finish configuration mode, shutdown the system */
				case 0:
					ciaaPOSIX_write(fd_uartUSB, "OK\n", ciaaPOSIX_strlen("OK\n"));
					ShutdownOS(0);
					/* terminate task */
					TerminateTask();

			}
		}
	}
	TerminateTask();

}

/** \brief Wifi configuration task
 *
 * This task allows change the SSID and password of the Wifi connection
 */

TASK(Wifi_cfg){
	char buf[64]={0};   /* buffer for uart operation (modificado en ciaaDriverUart.c    */
	int32_t ret=0;      /* return value variable for posix calls  */
	char* chr;
	int len;


	CDRon_delayMs(10);
	ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	if(ret > 0){
		ciaaPOSIX_read(fd_uartUSB, buf, 64);

		/* Data receive <SSID>:<password> */
		len=strcspn(buf, ":");
		strncpy(WIFI.SSID,buf,len);
		chr = strchr(buf,':');
		strcpy(WIFI.password, &chr[1]);

		/* Configuration successfully, send "OK" and write EEPROM */
		ciaaPOSIX_write(fd_uartUSB, "OK\n", ciaaPOSIX_strlen("OK\n"));
		WIFI_saveWIFI();	// Write EEPROM memory
	}

	TerminateTask();
}


/** \brief Brushless testing task
 *
 * This task updates the duty cycle data of the BRUSHLESS from the receive data, and call Brushless update task.
 */

TASK(Brushless_tst){
	char buf[64]={0};   /* buffer for uart operation (modificado en ciaaDriverUart.c    */
	int32_t ret=0;      /* return value variable for posix calls  */
	uint8_t s_motor;

	CDRon_delayMs(1);

	// Waiting for data receive (1 ms)

	ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	if(ret > 0){	// if receive data
		ciaaPOSIX_read(fd_uartUSB, buf, 64);

		//update BRUSHLESS data
		//Data struct <% duty>:[<brush 1>:<brush 2>:<brush 3>:[brush 4>] optional, at lease one brushless motor
		refreshBrushless(buf);

		// Testing was successfuly, send "OK" and call Brushless update task
		ciaaPOSIX_write(fd_uartUSB, "OK\n", ciaaPOSIX_strlen("OK\n"));
		ChainTask(brushlUpdate);

	}
	TerminateTask();
}



/** \brief IMU testing task
 *
 * This task sends IMU data.
 */

TASK(IMU_tst){
	char str[30] = {0};

	MPU6050_packaging(str); //data arrangement => <pitch>:<roll>:<yaw>\n

	/* Send IMU data */
	ciaaPOSIX_write(fd_uartUSB, str, ciaaPOSIX_strlen(str));
	TerminateTask();
}


/** \brief Wifi testing task
 *
 * This task interprets the incoming data and client's connections
 */

TASK(Wifi_tst){
	char* chr;
	int index,i;
	double duty;
	char buf[64];
	char str[30] = {0};
	WIFI.busy = 1;

	ciaaPOSIX_read(fd_uartWIFI, buf, 64);

	switch (WIFI_readData(buf)){
	    /* Process incoming data */
		case 0:

			/* if receive "brushless\n" => init duty update */
			if(strstr(buf, "brushless\n") != NULL){
				chr = &buf[ciaaPOSIX_strlen("brushless\n")];

				//update BRUSHLESS data
				//Data struct <% duty>:[<brush 1>:<brush 2>:<brush 3>:[brush 4>] optional, at lease one brushless motor
				refreshBrushless(chr);

				// testing was sucessfully, send "OK" and call brushless update task
				WIFI_sendData("OK\n",ciaaPOSIX_strlen("OK\n"));
				WIFI.busy = 0;
				ChainTask(brushlUpdate);

			/* if receive "close\n" => re-init clientID */
			} else if(strstr(buf, "close\n") != NULL)
				WIFI.clientID = -1;

			/* if receive "imu\n" => send IMU data */
			else if(strstr(buf, "imu\n") != NULL){

				MPU6050_packaging(str);
				WIFI_sendData(str,ciaaPOSIX_strlen(str));
			}
			break;

		/* Client connection was successfully, send "OK" */
		case 1:
			WIFI_sendData("OK\n",ciaaPOSIX_strlen("OK\n"));
			break;

		/* Reading data causes error*/
		case -1:
			break;
	}

	WIFI.busy = 0;
	TerminateTask();
}


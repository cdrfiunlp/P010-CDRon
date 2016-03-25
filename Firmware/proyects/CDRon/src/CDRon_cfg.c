
#include "CDRon_cfg.h"
#include "CDRon_lib.h"
#include "CDRon.h"
#include "ciaaDriverUart.h"
#include "os.h"               /* <= operating system header */

/* Declaraciones externas */
extern int32_t fd_i2c, fd_uartUSB;
extern struct status_struct;
extern struct status_struct status;

extern struct struct_motor cfg_motor;
extern volatile struct WIFI_struct WIFI;
extern volatile struct IMU_struct IMU;

/****************************************************************/
/**************** Funciones de la CIAA **************************/
/****************************************************************/
#define SERIAL_WIFI fd_uartWIFI

/** \brief Startup configuration task
 *
 * This task startup the configuration mode
 */

TASK(StartupConfig)
{
	   char buf[64]={0};   // buffer for uart operation (modificado en ciaaDriverUart.c)
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

TASK(ConfigMode){
	char buf[64]={0};   /* buffer for uart operation (modificado en ciaaDriverUart.c    */
	int32_t ret=0;      /* return value variable for posix calls  */

	while(1){
		ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
		if(ret > 0){
			ciaaPOSIX_read(fd_uartUSB, buf, 3);
			ret = strtol(buf,NULL,10);
			switch (ret){
				case 1:
					ActivateTask(Wifi_cfg);
					break;
				case 2:
					ActivateTask(Motor_tst);
					break;
				case 3:
					ActivateTask(IMU_tst);
					break;
				case 4:
					break;

				case 0:
					ciaaPOSIX_write(fd_uartUSB, "OK\n", ciaaPOSIX_strlen("OK\n"));
					ShutdownOS(0);
					/* terminate task */
					TerminateTask();

			}
		}
	}

}

TASK(Wifi_cfg){
	char buf[64]={0};   /* buffer for uart operation (modificado en ciaaDriverUart.c    */
	int32_t ret=0;      /* return value variable for posix calls  */
	char* chr;
	int len;


	CDRon_delayMs(10);
	ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	if(ret > 0){
		ciaaPOSIX_read(fd_uartUSB, buf, 64);
		len=strcspn(buf, ":");
		strncpy(WIFI.SSID,buf,len);
		chr = strchr(buf,':');
		strcpy(WIFI.password, &chr[1]);


	}
	ciaaPOSIX_write(fd_uartUSB, "OK\n", ciaaPOSIX_strlen("OK\n"));
	WIFI_saveWIFI();
	TerminateTask();
}


/** \brief Motor test task
 *
 * This task update the duty cycle of the motors.
 */
TASK(Motor_tst){
	char* chr;
	char buf[64]={0};   /* buffer for uart operation (modificado en ciaaDriverUart.c    */
	int32_t ret=0;      /* return value variable for posix calls  */
	uint8_t s_motor;
	int index,i;
	double duty;

	CDRon_delayMs(1);

	ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	if(ret > 0){
		ciaaPOSIX_read(fd_uartUSB, buf, 64);
		duty= AUX_sstr2float(buf);
		for(i=0;i<3;i++){
			chr = strchr(buf,':');
			if (chr != NULL){
				chr[0] = ' ';
				index = chr[1] - 48; //Conver to number
				cfg_motor.PWMduty[index-1] = duty;
			}
		}
		ciaaPOSIX_write(fd_uartUSB, "OK\n", ciaaPOSIX_strlen("OK\n"));
		ChainTask(motorUpdate);

	}
}

TASK(IMU_tst){
	char str[30] = {0};

	ftoa (IMU.pitch,str, 2);
	str[ciaaPOSIX_strlen(str)] = ':';
	ftoa (IMU.roll,&str[ciaaPOSIX_strlen(str)], 2);
	str[ciaaPOSIX_strlen(str)] = ':';
	ftoa (IMU.yaw,&str[ciaaPOSIX_strlen(str)], 2);
	str[ciaaPOSIX_strlen(str)] = '\n';
	ciaaPOSIX_write(fd_uartUSB, str, ciaaPOSIX_strlen(str));
	TerminateTask();
}


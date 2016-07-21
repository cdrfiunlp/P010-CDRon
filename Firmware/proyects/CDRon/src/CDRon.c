/* Copyright 2014, Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief CDRon source file
 **
 ** Main source file of the CDRon project
 **
 **/


/*==================[inclusions]=============================================*/

#include "CDRon.h"         /* <= own header */
#include "CDRon_lib.h"
#include "CDRon_cfg.h"
#include "CDRon_MPU6050.h"
#include "CDRon_ESP8266.h"
#include "os.h"               /* <= operating system header */

#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */


/*==================[macros and definitions]=================================*/


/*==================[internal data declaration]==============================*/
struct status_struct status;
struct BRUSHLESS_struct BRUSHLESS;
struct BATTERY_struct BATTERY;
volatile struct WIFI_struct WIFI;
volatile struct IMU_struct IMU;

uint8_t f_i2cerror = 0,data;
unsigned int a= 0;

int32_t fd_uartUSB,fd_uartWIFI;
int32_t fd_i2c;
int32_t fd_pwm[4];
int32_t fd_pwm2;



/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void)
{
   /* Starts the operating system in the Application Mode 1 */
	StartOS(AppMode1);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */

   return 0;
}

void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}




/****************************************************************/
/*************************** TASKS ******************************/
/****************************************************************/


/** \brief Initial task
 *
 * This task is started automatically in the application mode 1.
 */
TASK(InitTask)
{

   /* init CIAA kernel and devices */
   ciaak_start();

   /* CDRon initialization */
   CDRon_initialization();

   /* Activates startupConfigtasks */
   ActivateTask(StartupConfig);

   /* terminate task */
   TerminateTask();
}

/** \brief Brushless motor update
 *
 * This task update the duty cycle of each brushless motor with the value in BRUSHLESS.
 */

TASK(brushlUpdate){ // Error with ¿brushlessUpdate?
	  int i;
	  uint32_t PWMduty;
	  double f;

	  /* for brushless motor 1 to 4 */
	  for(i=0;i<4;i++){
	  	  f= BRUSHLESS.duty[i] * 4250.0; // Highest register value; 425.000, Highest duty value: 100.0
		  PWMduty = (int) (f);	// Round down duty

		  ciaaPOSIX_write(fd_pwm[i], &PWMduty, 4);
	  }
	  TerminateTask();
}

/** \brief IMU update
 *
 * This task update the IMU data into IMU struct.
 * This task is called from GPIO5 interrupt.
 */

TASK(IMUUpdate){
		MPU6050_getData();
		IMU.DRDY = 1;
		TerminateTask();
}



/** \brief Measurement battery charge level
 *
 *
 */

TASK(batteryUpdate){

	batteryMeasure(ADC_CH1);

	batteryMeasure(ADC_CH2);

	batteryMeasure(ADC_CH3);

	TerminateTask();
}



/** \brief Periodic wifi task
 *
 * This task is called each 100 ms to check the incoming data
 */

TASK(wifiTask){
	int32_t ret=0;  // return value variable for posix calls

	ciaaPOSIX_ioctl(fd_uartWIFI, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	if(ret > 0){
		// if incoming data...
		switch (status.mode){
			// if MODE_TEST => go to wifi test
			case MODE_TEST:
				if(WIFI.busy == 0) ActivateTask(Wifi_tst);
				break;
			// if MODE_TEST => go to wifi
			case MODE_NORMAL:
				break;
		}
	}
	TerminateTask();
}



/** \brief I2c error task
 *
 * This task is called for timeout (¿Useful?), leaves the blocking mode
 */

TASK(I2Cerror){

	volatile uint32_t * CONSET = 0x400A1000;
	*CONSET = I2C_CON_STO_user;
	f_i2cerror=1;

	/* terminate task */
	TerminateTask();
}



/****************************************************************/
/********************* Program functions ************************/
/****************************************************************/

/** \brief Initialization function
 *
 * This function initialize the CIAA modules:
 * 		- Serial uart FT2232
 * 		- Serial uart Wifi
 * 		- Serial I2C
 * 		- PWM output
 *
 * 	Also, initialize the external modules:
 * 		-MPU6050
 * 		-ESP8266
 *
 *
 */

int CDRon_initialization(void){
	   char buf[64]={0};
	   int32_t ret=0;


	   /*********************************/
	   /** initialize the CIAA modules **/
	   /*********************************/

	   /* open CIAA serial FT2232 */
	   fd_uartUSB = ciaaPOSIX_open("/dev/serial/uart/1", ciaaPOSIX_O_RDWR);
	   /* open CIAA serial WIFI */
	   fd_uartWIFI = ciaaPOSIX_open("/dev/serial/uart/2", ciaaPOSIX_O_RDWR);

	   /* setting uart configuration */
	   // change baud rate for uart usb & uart wifi
	   ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_SET_BAUDRATE, (void *)ciaaBAUDRATE_115200);
	   ciaaPOSIX_ioctl(fd_uartWIFI, ciaaPOSIX_IOCTL_SET_BAUDRATE, (void *)ciaaBAUDRATE_115200);
	   // change FIFO TRIGGER LEVEL for uart usb & uart wifi */
	   ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_SET_FIFO_TRIGGER_LEVEL, (void *)ciaaFIFO_TRIGGER_LEVEL3);
	   ciaaPOSIX_ioctl(fd_uartWIFI, ciaaPOSIX_IOCTL_SET_FIFO_TRIGGER_LEVEL, (void *)ciaaFIFO_TRIGGER_LEVEL3);



	   /* open CIAA I2C */
	   fd_i2c = ciaaPOSIX_open("/dev/i2c/0", ciaaPOSIX_O_RDWR);

	   /* setting i2c configuration */
	   /* change register addressing width for i2c to 8 bits */
	   ciaaPOSIX_ioctl(fd_i2c,(int32_t) ciaaPOSIX_IOCTL_SET_REGISTERADDWIDTH, (uint32_t) ciaaPOSIX_IOCTL_REGISTERADDWIDTH_8bits );



	   /* initialization PWM outputs for brushless motors*/

	   /* Configuration info:
	    * 	- Clock frequency = 204 MHz	=> Highest frequency
	    * 	- ESC frequency = up to 499 Hz => F_esc = 480 Hz.
	    * 	- Highest register value = 425.000
	    * 	- Time resolution ~= 4.9 ns
	    */

	   GetResource(BRUSHLESSR);

	   fd_pwm[0]= ciaaPOSIX_open("/dev/dio/pwm/0", ciaaPOSIX_O_RDWR);
	   fd_pwm[1]= ciaaPOSIX_open("/dev/dio/pwm/1", ciaaPOSIX_O_RDWR);
	   fd_pwm[2]= ciaaPOSIX_open("/dev/dio/pwm/2", ciaaPOSIX_O_RDWR);
	   fd_pwm[3]= ciaaPOSIX_open("/dev/dio/pwm/3", ciaaPOSIX_O_RDWR);

	   ReleaseResource(BRUSHLESSR);

	   CDRon_delayMs(100);





	   /*************************************/
	   /** initialize the external modules **/
	   /*************************************/

		IMU.active = 0;
	   /* initialization MPU6050 device */
	   if(MPU6050_init() == 0)
		   if(MPU6050_initDMP()== 0){
			   IMU.active = 1; // if OK IMU.active = 1 else = 0
			   IMU.DRDY = 1; // DRDY=1 for interrupt
		   }

	   /* initialization ESP8266 device */
	   WIFI.active = 0;
	   if(WIFI_init() == 0) // if OK WIFI.active = 1 else = 0
	   	   WIFI.active = 1;



	   ADC_CLOCK_SETUP_T setup;             /** <= adc setup */
	   ADC_RESOLUTION_T resolution;


	   BATTERY.active = 0;
	   /* ADC0 Init */
	   Chip_ADC_Init(LPC_ADC0, &(setup));
	   BATTERY.gain[0] = 3.3/255.0;
	   BATTERY.gain[1] = 3.3/255.0;
	   BATTERY.gain[2] = 3.3/255.0;

	   SetRelAlarm(batteryPeriodicMeasure,1000,1000);

	   BATTERY.active = 1;
	   return 0;

}



/****************************************************************/
/********************* Time functions ***************************/
/****************************************************************/

int CDRon_delayMs(unsigned int ms){

	unsigned int time;

	time = GetCounter(0);
	while (GetCounter(0)-time < ms){}

	return 0;
}

int CDRon_getMs(unsigned long *count){

	if(!count)
		return 1;
	count[0] = GetCounter(0);
	return 0;
}


/****************************************************************/
/************************** Interrupts **************************/
/****************************************************************/

/** \brief MPU6050 interrupt
 *
 * This interrupt actives the IMU reading
 */

ISR(GPIO5_IRQHandler)
{

	if(IMU.DRDY == 1){ // if IMU reading is off
		IMU.DRDY = 0;
		ActivateTask(IMUUpdate);
	}

	MPU6050_clearInterrupt();
}

/*==================[end of file]============================================*/

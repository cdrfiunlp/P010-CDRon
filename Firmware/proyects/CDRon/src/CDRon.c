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

/** \brief Blinking_echo example source file
 **
 ** This is a mini example of the CIAA Firmware.
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Blinking Blinking_echo example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 * PR           Pablo Ridolfi
 * JuCe         Juan Cecconi
 * GMuro        Gustavo Muro
 * ErPe         Eric Pernia
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20150603 v0.0.3   ErPe change uint8 type by uint8_t
 *                        in line 172
 * 20141019 v0.0.2   JuCe add printf in each task,
 *                        remove trailing spaces
 * 20140731 v0.0.1   PR   first functional version
 */

/*==================[inclusions]=============================================*/
//#undef FALSE
//#undef TRUE

#include "CDRon.h"         /* <= own header */
#include "CDRon_lib.h"
#include "os.h"               /* <= operating system header */

#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */


//#include "scu_18xx_43xx.h"
//#include "pinint_18xx_43xx.h"
//#include "gpio_18xx_43xx.h"



/*==================[macros and definitions]=================================*/


/*==================[internal data declaration]==============================*/
/*==================[internal functions declaration]=========================*/
/*==================[internal data definition]===============================*/


/** \brief File descriptor for digital output ports
 *
 * Device path /dev/dio/out/0
 */
int32_t fd_uartUSB,fd_uartWIFI;
int32_t fd_i2c;
int32_t fd_pwm1,fd_pwm2,fd_pwm3;

uint32_t PWMduty;
int PWMselect;
uint8_t f_i2cerror = 0;
unsigned int a= 0;
extern struct status_struct;
struct status_struct status;



/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
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
   /* This example has only one Application Mode */
   StartOS(AppMode1);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */
   return 0;
}

/** \brief Error Hook function
 *
 * This fucntion is called from the os if an os interface (API) returns an
 * error. Is for debugging proposes. If called this function triggers a
 * ShutdownOs which ends in a while(1).
 *
 * The values:
 *    OSErrorGetServiceId
 *    OSErrorGetParam1
 *    OSErrorGetParam2
 *    OSErrorGetParam3
 *    OSErrorGetRet
 *
 * will provide you the interface, the input parameters and the returned value.
 * For more details see the OSEK specification:
 * http://portal.osek-vdx.org/files/pdf/specs/os223.pdf
 *
 */
void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}

/** \brief Initial task
 *
 * This task is started automatically in the application mode 1.
 */
TASK(InitTask)
{
   int ret;
   uint32_t outputs;
   /* init CIAA kernel and devices */
   ciaak_start();

   CDRon_initialization();


   /* Activates tasks */
   //ActivateTask(RefreshPWM);
   ActivateTask(StartupConfig);

   /* terminate task */
   TerminateTask();
}


TASK(StartupConfig)
{
	   char buf[64]={0};   /* buffer for uart operation (modificado en ciaaDriverUart.c    */
	   int32_t ret=0;      /* return value variable for posix calls  */


	   // Startup configuration mode
	   ciaaPOSIX_write(fd_uartUSB, "startup config?\n", ciaaPOSIX_strlen("startup config?\n"));
	   CDRon_delayMs(100);
	   ciaaPOSIX_ioctl(fd_uartUSB, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	   if(ret > 0){
		   ciaaPOSIX_read(fd_uartUSB, buf, 64);
		   if(ciaaPOSIX_strcmp(buf,"startup config\n")==0){
			   status.mode = MODE_CONFIG;
		       ActivateTask(ConfigMode);
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
			ciaaPOSIX_read(fd_uartUSB, buf, 64);
			ret = strtol(buf,NULL,10);
			switch (ret){
				case 1:
					break;
				case 2:
					break;

				case 0:
					/* terminate task */
					TerminateTask();

			}
		}


	}


}

TASK(RefreshPWM){
	while (1){

	  WaitEvent(NEW_PWM);	// suspend task up to NEW_PWM event
	  ClearEvent(NEW_PWM);


	  if( (PWMselect & 0x1) == 1)
		  ciaaPOSIX_write(fd_pwm1, &PWMduty, 4);

	  if( ((PWMselect>>1) & 0x1) == 1)
		  ciaaPOSIX_write(fd_pwm2, &PWMduty, 4);

	  if( ((PWMselect>>2) & 0x1) == 1 )
		  ciaaPOSIX_write(fd_pwm3, &PWMduty, 4);

	}
}

TASK(I2Cerror){
	volatile uint32_t * CONSET = 0x400A1000;
	*CONSET = I2C_CON_STO_user;
	f_i2cerror=1;
	/* terminate task */
	TerminateTask();
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */


/****************************************************************/
/***************** Funciones de programa ************************/
/****************************************************************/

int CDRon_initialization(void){
	   char buf[64]={0};
	   int32_t ret=0;

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

	   /* Datos de configuración:
	    * 	- Frecuencia clock = 204 MHz	=> Máxima frecuencia posible
	    * 	- Frecuencia ESC = up to 499 Hz => Se configura F_esc = 480 Hz.
	    * 	- Registro máximo = 425.000
	    * 	- Resolución en tiempo ~= 4.9 ns
	    */

	   fd_pwm1= ciaaPOSIX_open("/dev/dio/pwm/0", ciaaPOSIX_O_RDWR);
	   fd_pwm2= ciaaPOSIX_open("/dev/dio/pwm/1", ciaaPOSIX_O_RDWR);
	   fd_pwm3= ciaaPOSIX_open("/dev/dio/pwm/2", ciaaPOSIX_O_RDWR);

	   /* initialization MPU6050 device */
	   //if(MPU6050_init() != 0)
	   //   ShutdownOS(0);

	   //if(MPU6050_initDMP()!= 0)
	   //	   ShutdownOS(0);

	   CDRon_delayMs(100);

	   //WIFI_init();


}



/****************************************************************/
/***************** Funciones del tiempo *************************/
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
/*********************** Interrupciones *************************/
/****************************************************************/

ISR(GPIO5_IRQHandler)
{
	MPU6050_getData();
	MPU6050_clearInterrupt();
}

/*==================[end of file]============================================*/

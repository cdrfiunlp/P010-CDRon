/* Copyright CIAA
 * Copyright 2014, Mariano Cerdeiro
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


#ifndef _CDRON_H_
#define _CDRON_H_

/*==================[inclusions]=============================================*/
#include "Os_Internal.h"              /* <= operating system header */
#include "ctype.h"



/*==================[macros]=================================================*/
#define CDRON
#define MPU6050



#define LPC_I2C0_BASE_user             0x400A1000
#define LPC_GPIO_PORT_user			   0x400F4000;

#define I2C_CON_STO_user           (1UL << 4)	/*!< I2C STOP bit */

#define MODE_UNDEF -1
#define MODE_CONFIG 0
#define MODE_TEST 	1
#define MODE_NORMAL 2

/*==================[typedef]================================================*/
struct status_struct{
	int mode;
	int wifiMode;
};

struct BRUSHLESS_struct{
	double duty[4];
	int active;
};

struct WIFI_struct{
	char SSID[20];
	char password[20];
	char IPaddress[20];
	int active;
	int busy;
	int clientID;
};

volatile struct IMU_struct{
	float quad[4];
    float yaw;
    float pitch;
    float roll;
    int active;
    char DRDY;
};

struct BATTERY_struct{
	float gain[3];
	float S[3];
	float measure;
	int active;
};

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
int CDRon_delayMs(unsigned int);
int CDRon_getMs(unsigned long *);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _BLINKING_H_ */


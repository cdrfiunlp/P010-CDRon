
#include "CDRon_lib.h"
#include "CDRon.h"
#include "ciaaDriverUart.h"
/* Declaraciones externas */
extern int32_t fd_i2c, fd_uartWIFI;

/****************************************************************/
/**************** Funciones de la CIAA **************************/
/****************************************************************/
#define SERIAL_WIFI fd_uartWIFI

extern volatile struct WIFI_struct WIFI;


int CDRon_i2c_write(unsigned char slave_addr,
                     unsigned char reg_addr,
                     unsigned char length,
                     unsigned char const *data)
{
	unsigned char address = slave_addr;
	unsigned char command = reg_addr;

	ciaaPOSIX_ioctl(fd_i2c,(int32_t) ciaaPOSIX_IOCTL_SET_SLAVEADD, (int8_t) address);
	ciaaPOSIX_ioctl(fd_i2c,(int32_t) ciaaPOSIX_IOCTL_SET_REGISTERADD, (uint32_t) reg_addr);


    if (!length)
        return 0;

    if(ciaaPOSIX_write(fd_i2c, data, length)==length+1){ // RA + data

    	return 0;
    }
    return -1;

}

int CDRon_i2c_read(unsigned char slave_addr,
                    unsigned char reg_addr,
                    unsigned char length,
                    unsigned char *data)
{
	unsigned char address = slave_addr;
	unsigned char command = reg_addr;

	ciaaPOSIX_ioctl(fd_i2c,(int32_t) ciaaPOSIX_IOCTL_SET_SLAVEADD, (int8_t) address);
	ciaaPOSIX_ioctl(fd_i2c,(int32_t) ciaaPOSIX_IOCTL_SET_REGISTERADD, (uint32_t) reg_addr);


    if (!length)
        return 0;


    if(ciaaPOSIX_read(fd_i2c, data, length)== length){

    	return 0;
    }
    return -1;
}

void CDRon_initEEPROM(void){
	Chip_EEPROM_Init(LPC_EEPROM);
	Chip_EEPROM_SetAutoProg(LPC_EEPROM,EEPROM_AUTOPROG_AFT_1WORDWRITTEN);
}

void CDRon_disableEEPROM(void){
	Chip_EEPROM_SetAutoProg(LPC_EEPROM,EEPROM_AUTOPROG_OFF);
	Chip_EEPROM_EnablePowerDown(LPC_EEPROM);
}

/****************************************************************/
/******************* Funciones del WIFI *************************/
/****************************************************************/

int WIFI_init(void){
	  char command[20] = {0};
	  int ret = 0;

	  WIFI_configure();

/*
	  if(WIFI_sendCommand("AT\r\n","OK") != 0)
		  return -1;
	  if(WIFI_sendCommand("AT+CWMODE=1\r\n","OK") != 0)
		  return -1;
	  if(WIFI_sendCommand("AT+RST\r\n","OK") != 0)
		  return -1;

	  CDRon_delayMs(1000);
	  WIFI_connectAP();
*/

}

int WIFI_sendCommand(char * command, char * respond){
	  char buf[64] = {0};
	  uint8_t i;
	  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);

	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(100);
	  ciaaPOSIX_read(SERIAL_WIFI, buf, 64);
	  if(strstr(buf,"busy")!= NULL){	// Si se encuentra ocupado, repetir luego de 1 s
		  CDRon_delayMs(1000);
		  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);
		  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
		  CDRon_delayMs(10);
		  ciaaPOSIX_read(SERIAL_WIFI, buf, 64);

	  }
	  if(strstr(buf,respond)!= NULL)
		  return 0;
	  return -1;

}

int WIFI_readStatus(char * command, char * buf){
	  uint8_t i;
	  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);

	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(10);
	  ciaaPOSIX_read(SERIAL_WIFI, buf, 64);
	  if(strstr(buf,"busy")!= NULL){	// Si se encuentra ocupado, repetir luego de 1 s
		  CDRon_delayMs(1000);
		  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);
		  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
		  CDRon_delayMs(10);
		  ciaaPOSIX_read(SERIAL_WIFI, buf, 64);
		  if(strstr(buf,"busy")!= NULL)
			  return -1;
	  }
	  return 0;
}

void WIFI_configure(void){
	volatile uint32_t * EEPROM = 0x20040000;
	uint32_t * CONFIG = &WIFI;
	uint8_t i,len;

	CDRon_initEEPROM();
	len = sizeof(WIFI) >>2;
	for(i=0;i<len;i++){
		CONFIG[i] = EEPROM[i];
		CDRon_delayMs(10);
	}
	CDRon_delayMs(10);
	CDRon_disableEEPROM();
}

void WIFI_saveWIFI(void){
	volatile uint32_t * EEPROM = 0x20040000;
	uint32_t * CONFIG = &WIFI;
	uint8_t i, len;

	CDRon_initEEPROM();
	for(i=0;i<10;i++){
		EEPROM[i] = 0;
		CDRon_delayMs(10);
	}
	CDRon_delayMs(10);
	len = sizeof(WIFI) >>2;
	for(i=0;i<len;i++){
		EEPROM[i] = CONFIG[i];
		CDRon_delayMs(10);
	}
	CDRon_delayMs(10);
	CDRon_disableEEPROM();
}

int WIFI_connectAP(void){
	  char command[50] = {0};
	  char buf[64] = {0};
	  //ciaaPOSIX_strcpy(command,"AT+CWJAP=\"");
	  ciaaPOSIX_strcpy(command,"AT+CWJAP=\"");
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command)-1);
	  CDRon_delayMs(1);
	  ciaaPOSIX_strcpy(command,WIFI.SSID);
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command)-1);
	  CDRon_delayMs(1);
	  ciaaPOSIX_strcpy(command,"\",\"");
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command)-1);
	  CDRon_delayMs(1);
	  ciaaPOSIX_strcpy(command,WIFI.password);
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command)-1);
	  CDRon_delayMs(1);
	  ciaaPOSIX_strcpy(command,"\"\r\n");

	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(4000);

	  WIFI_sendCommand("AT\r\n","OK"); // Al configurar el dispositivo, tira mensajes de más

	  WIFI_getIP(buf);
}

void WIFI_getIP(char * buf){
	uint8_t len;
	WIFI_readStatus("AT+CIFSR\r\n",buf);
	buf = strstr(buf,"STAIP");
	buf = strstr(buf,"\"");
	len =strcspn(&buf[1], "\"");
	strncpy(buf, &buf[1], len);
	buf[len] = '\0';

	strcpy(WIFI.IPaddress,buf);
}

/****************************************************************/
/**************** Funciones del MPU6050 *************************/
/****************************************************************/


int MPU6050_init(void){
    int ret;
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    unsigned long timestamp;

	ret = mpu_init(NULL);
    if (mpu_init(NULL)!=0)
    	return -1;


    /* Turn on GYRO & ACCEL */
	if(mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL)!=0)
		return -1;

	/* Push both gyro and accel data into the FIFO. */
    if(mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL)!=0)
    	return -1;
    /* Set sample rate to 100 Hz */
    if(mpu_set_sample_rate(DEFAULT_MPU_HZ)!=0)
	    return -1;

    /* Verification */
    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&gyro_rate);
    	if (gyro_rate != DEFAULT_MPU_HZ)
    		return -1;
    mpu_get_gyro_fsr(&gyro_fsr);
    	if(gyro_fsr != 2000) // Full range
    		return -1;

    mpu_get_accel_fsr(&accel_fsr);
    	if(accel_fsr != 2) // Full range
    	return -1;


    return 0;
}

int MPU6050_initDMP(void){
	if(dmp_load_motion_driver_firmware() !=0)
		return -1;
	if(dmp_set_orientation(
	        AUX_inv_orientation_matrix_to_scalar(gyro_orientation))!=0)
		return -1;

    if(dmp_enable_feature(DMP_FEATURE_GYRO_CAL|DMP_FEATURE_6X_LP_QUAT)!=0)
    	return -1;
    if(dmp_set_fifo_rate(DEFAULT_MPU_HZ)!=0)
    	return -1;
    if(mpu_set_dmp_state(1)!=0)
    	return -1;
    if(dmp_set_interrupt_mode(DMP_INT_CONTINUOUS)!=0)
    	return -1;

    MPU6050_setInterrupt();
	return 0;

}

void MPU6050_getData(void){

    unsigned char more;
    long quat[4];
    unsigned long sensor_timestamp;
    short sensors;

int ret;
	ret = dmp_read_fifo(NULL, NULL, quat, &sensor_timestamp, &sensors,
	                &more);
	if((sensors & INV_WXYZ_QUAT) == INV_WXYZ_QUAT){
		IMU.quad[0]= quat[0]/1073741824.0f;
		IMU.quad[1] = quat[1]/1073741824.0f;
		IMU.quad[2] = quat[2]/1073741824.0f;
		IMU.quad[3] = quat[3]/1073741824.0f;

		MATH_getYawPitchRoll(&IMU.yaw,IMU.quad);
	}
	if (more >10){
		return;
	}

}

void MPU6050_setInterrupt(void){
	Chip_SCU_PinMux(6, 12, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P6_12: MPU interrupt */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 2, 8);
	Chip_SCU_GPIOIntPinSel(5,2,8);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH5);
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH5);
	Chip_PININT_DisableIntLow(LPC_GPIO_PIN_INT, PININTCH5);
}

void MPU6050_clearInterrupt(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,(1 << 5));
}

/****************************************************************/
/**************** Funciones de conversión ***********************/
/****************************************************************/

void MATH_getEuler(float *data, float *q) {
	float sqx,sqy,sqz,t;
	t = q[1] * q[2] + q[3] * q[0];
	        if (t > 0.4999){
	            //heading
				data[0]= 2 * atan2(q[1], q[0]);
	            //attitude
				data[1]= PI/ 2;
	            //bank
				data[2]= 0;
			}else if (t < -0.4999){
	            data[0] = -2 * atan2(q[1], q[0]);
	            data[1] = -PI / 2;
	            data[2] = 0;
	        }else{
	            sqx = q[1] * q[1] ;
	            sqy = q[2] * q[2] ;
	            sqz = q[3] * q[3] ;
	            data[0] = atan2(2 * q[2] * q[0] - 2 * q[1] * q[3],1 - 2 * sqy - 2 * sqz);
	            data[1] = asin(2 * t);
	            data[2] = atan2(2 * q[1] * q[0] - 2 * q[2] * q[3],1 - 2 * sqx - 2 * sqz);
	        }
    /*data[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);   // psi
    data[1] = -asin(2*q[1]*q[3] + 2*q[0]*q[2]);                              // theta
    data[2] = atan2(2*q[2]*q[3] - 2*q[0]*q[1], 2*q[0]*q[0] + 2*q[3]*q[3] - 1);   // phi*/

    data[0] *= 180.0/PI;
    data[1] *= 180.0/PI;
    data[2] *= 180.0/PI;
    return;
}

void MATH_getYawPitchRoll(float *data, float *q){
	float v[3];
    v[0]= 2 * (q[1]*q[3] - q[0]*q[2]);
    v[1] = 2 * (q[0]*q[1] + q[2]*q[3]);
    v[2] = 2*q[0]*q[0] + 2*q[3]*q[3] - 1;

    data[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
     // pitch: (nose up/down, about Y axis)
    data[1] = atan(v[0] / sqrt(v[1]*v[1] + v[2]*v[2]));
     // roll: (tilt left/right, about X axis)
    data[2] = atan(v[1] / sqrt(v[0]*v[0] + v[2]*v[2]));

    data[0] *= 180.0/PI;
    data[1] *= 180.0/PI;
    data[2] *= 180.0/PI;

}

/****************************************************************/
/**************** Funciones de auxiliares ***********************/
/****************************************************************/

/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */

unsigned short AUX_inv_orientation_matrix_to_scalar(const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = AUX_inv_row_2_scale(mtx);
    scalar |= AUX_inv_row_2_scale(mtx + 3) << 3;
    scalar |= AUX_inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

unsigned short AUX_inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

double AUX_sstr2float(char *s)
{
	double a = 0.0;
	int e = 0;
	int c;
	while ((c = *s++) != '\0' && isdigit(c)) {
		a = a*10.0 + (c - '0');
	}
	if (c == '.') {
		while ((c = *s++) != '\0' && isdigit(c)) {
			a = a*10.0 + (c - '0');
			e = e-1;
		}
	}
	if (c == 'e' || c == 'E') {
		int sign = 1;
		int i = 0;
		c = *s++;
		if (c == '+')
			c = *s++;
		else if (c == '-') {
			c = *s++;
			sign = -1;
		}
		while (isdigit(c)) {
			i = i*10 + (c - '0');
			c = *s++;
		}
		e += i*sign;
	}
	while (e > 0) {
		a *= 10.0;
		e--;
	}
	while (e < 0) {
		a *= 0.1;
		e++;
	}
	return a;
}

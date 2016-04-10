
/** \brief CDRon library source file
 **
 ** CDRon library source file of the CDRon project
 **
 **/

/*==================[inclusions]=============================================*/
#include "CDRon_lib.h"
#include "CDRon.h"
#include "ciaaDriverUart.h"

/*==================[external data declaration]==============================*/

extern int32_t fd_i2c;
extern struct BRUSHLESS_struct BRUSHLESS;
extern struct BATTERY_struct BATTERY;

/****************************************************************/
/********************** CIAA functions **************************/
/****************************************************************/


/** \brief Write I2C data
 *
 */
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

/** \brief Read I2C data
 *
 */

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


/** \brief Init EEPROM memory
 *
 */

void CDRon_initEEPROM(void){
	Chip_EEPROM_Init(LPC_EEPROM);
	Chip_EEPROM_SetAutoProg(LPC_EEPROM,EEPROM_AUTOPROG_AFT_1WORDWRITTEN);
}

/** \brief Disable EEPROM memory
 *
 */
void CDRon_disableEEPROM(void){
	Chip_EEPROM_SetAutoProg(LPC_EEPROM,EEPROM_AUTOPROG_OFF);
	Chip_EEPROM_EnablePowerDown(LPC_EEPROM);
}

/** \brief Update BRUSHLESS struct
 *
 *	This function update de brushless information from a string.
 *	String format: <% duty>:[<brush 1>:<brush 2>:<brush 3>:[brush 4>] optional, at lease one brushless motor
 */


void refreshBrushless(char * buf){
	double duty;
	int index,i;
	char* chr;

	duty= AUX_sstr2float(buf);	//first data was <% duty>

	/* For brushless 1 to 4, update duty data */
	for(i=0;i<3;i++){
		chr = strchr(buf,':');
		if (chr != NULL){
			chr[0] = ' ';		 // Erase ':'
			index = chr[1] - 48; //char to int
			BRUSHLESS.duty[index-1] = duty; //update duty data for brushless <index>
		}
	}
	return;
}

void batteryMeasure(ADC_CHANNEL_T CH){
	unsigned long time,time_now;
	uint8_t adc_value;

	Chip_ADC_EnableChannel(LPC_ADC0, CH, ENABLE);

	Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
	CDRon_getMs(&time);
	time_now = time;
	while((Chip_ADC_ReadStatus(LPC_ADC0, CH, ADC_DR_DONE_STAT)!=SET) && (time_now-time < 100))
		   CDRon_getMs(&time_now);

	if (time_now-time < 100){
		   Chip_ADC_ReadByte(LPC_ADC0, CH, &adc_value);
		   BATTERY.S[CH-1] = BATTERY.gain[CH-1]*adc_value;
		   if (CH == 3)
			   BATTERY.measure = BATTERY.gain[2]*adc_value;
	}else
		   BATTERY.S[CH-1] = -1.0;


	Chip_ADC_EnableChannel(LPC_ADC0, CH, DISABLE);
	return;
}


/****************************************************************/
/******************* Auxiliary functions ************************/
/****************************************************************/


/** \brief Convert string to float
 *
 */


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

/** \brief Convert float to string
 *
 */


void AUX_ftoa (double n_float,char *str, int32_t  cifras_decimales)
{
  int8_t  i;
  uint32_t factor=1,aux,entero, decimal;
  char cadena[cifras_decimales];
  str[0]='\0';
  if (n_float<0)
  {
      n_float=n_float*-1;
      strcat(str,"-");
  }
  for(i=1;i<=cifras_decimales;i++) factor=factor*10;
  entero  = (uint32_t) n_float;
  aux=(uint32_t)(n_float*factor);
  decimal = (uint32_t)(aux-(entero*factor));
  itoa(entero,cadena,10);
  strcat(str,cadena);
  strcat(str,".");
  itoa(decimal,cadena,10);
  strcat(str,cadena);
}

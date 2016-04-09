

#include "CDRon.h"
#include "CDRon_lib.h"
#include "ciaaDriverUart.h"
#include "CDRon_ESP8266.h"

/* Declaraciones externas */
extern int32_t fd_uartWIFI;


#define SERIAL_WIFI fd_uartWIFI

extern volatile struct WIFI_struct WIFI;


/****************************************************************/
/********************** WIFI functions **************************/
/****************************************************************/

int WIFI_init(void){
	  char command[20] = {0};
	  int ret = 0;

	  WIFI_configure();


	  if(WIFI_sendCommand("AT\r\n","OK") != 0)
		  return -1;
	  if(WIFI_sendCommand("AT+CWMODE=1\r\n","OK") != 0)
		  return -1;
	  if(WIFI_sendCommand("AT+RST\r\n","OK") != 0)
		  return -1;

	  CDRon_delayMs(2000);

	  if(WIFI_connectAP() != 0)
		  return -1;
	  return 0;

}

int WIFI_sendCommand(char * command, char * respond){
	  char buf[64] = {0};
	  uint8_t i;
	  int32_t ret=0;      // return value variable for posix calls
	  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);

	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(100);

	  ciaaPOSIX_ioctl(SERIAL_WIFI, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	  if(ret > 0){
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
	  }
	  return -1;

}

int WIFI_readStatus(char * command, char * buf){
	  uint8_t i;
	  int32_t ret=0;      // return value variable for posix calls

	  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);

	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(500);
	  ciaaPOSIX_ioctl(SERIAL_WIFI, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	  if(ret > 0){
		  ciaaPOSIX_read(SERIAL_WIFI, buf, 64);
		  if(strstr(buf,"busy")!= NULL){	// Si se encuentra ocupado, repetir luego de 1 s
			  CDRon_delayMs(1000);
			  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);
			  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
			  CDRon_delayMs(500);
			  ciaaPOSIX_ioctl(SERIAL_WIFI, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
			  if(ret > 0){
				  ciaaPOSIX_read(SERIAL_WIFI, buf, 64);
				  if(strstr(buf,"busy")!= NULL)
					  return -1;
			  }
		  }
		  return 0;
	  }
	  return -1;
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
	  char * aux;
	  int32_t ret=0,i;      // return value variable for posix calls

	  CDRon_delayMs(500);
	  ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);

	  // Send: "AT+CWJAP="SSID","PASSWRD"\r\n
	  ciaaPOSIX_strcpy(command,"AT+CWJAP=\"");
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(10);
	  ciaaPOSIX_strcpy(command,WIFI.SSID);
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(10);
	  ciaaPOSIX_strcpy(command,"\",\"");
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(10);
	  ciaaPOSIX_strcpy(command,WIFI.password);
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));
	  CDRon_delayMs(10);
	  ciaaPOSIX_strcpy(command,"\"\r\n");
	  ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));

	  for(i=0;i<6;i++){
			CDRon_delayMs(1000);
			if(WIFI_readStatus("AT\r\n",buf)==0) break;
			if (i==5) return -1;

	  }

	  //CDRon_delayMs(1000);
	  // Obtiene IP de conexión
	  if(WIFI_getIP(buf)!= 0)
		  return -1;
	  return 0;
}

int WIFI_getIP(char * buf){
	uint8_t len;
	CDRon_delayMs(100);
	WIFI_readStatus("AT+CIFSR\r\n",buf);
	if(strstr(buf,"busy")!= NULL){	// Si se encuentra ocupado, repetir luego de 1 s
	  CDRon_delayMs(1000);
	  WIFI_readStatus("AT+CIFSR\r\n",buf);
	  if(strstr(buf,"busy")!= NULL)
		  return -1;
	}
	buf = strstr(buf,"STAIP");
	if (buf == NULL)
			return -1;
	buf = strstr(buf,"\"");
	len =strcspn(&buf[1], "\"");
	strncpy(buf, &buf[1], len);
	buf[len] = '\0';

	strcpy(WIFI.IPaddress,buf);

	if (!strcmp(buf,"0.0.0.0"))
		return -1;
	return 0;
}

int WIFI_serverTCP(void){
	if(WIFI_sendCommand("AT+CIPMUX=1\r\n","OK") != 0)
		return -1;
	if(WIFI_sendCommand("AT+CIPSERVER=1\r\n","OK") != 0)
		return -1;
	WIFI.clientID = -1;

}

int WIFI_readData(char* buf){
	   char *ret;
	   char temp[5];
	   const char ch = ',';
	   const char str[2] =":";
	   int len,aux;

	   ret = strstr(buf, "+IPD");
	   if(ret != NULL){
		   ret = strrchr(ret,ch);
		   aux = strcspn(ret,str);
		   strncpy(temp, ret+1, aux-1);
		   len = strtol(temp,NULL,10);
		   strncpy(buf, ret+aux+1, len);
		   buf[len] = '\0';
		   return 0;
	   } else if (ret = strstr(buf, "CONNECT")){
		   ret = ret - 1;
		   len = strtol(&ret[-1],NULL,10);
		   WIFI.clientID = len;
		   return 1;
	   }




	   return -1;
}

int WIFI_sendData(char* buf,int len){
	  char command[50] = {0};
	  int32_t ret;
	  char* receive;

	if (WIFI.clientID == -1) return -1;

	// SEND: AT+CIPSEND=<id>,<lenght>\r\n
	ciaaPOSIX_ioctl(SERIAL_WIFI,(int32_t) ciaaPOSIX_IOCTL_CLEAR_RX_BUFFER, 0);

	ciaaPOSIX_strcpy(command,"AT+CIPSEND=");
	ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));

	CDRon_delayMs(10);
	command[0] = WIFI.clientID + 48;	// int to string
	command[1] = ',';
    command[2] = '\0';
	ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));

	CDRon_delayMs(10);
	itoa(len,command,10);
	ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));

	CDRon_delayMs(10);
	ciaaPOSIX_strcpy(command,"\r\n");
	ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));

	CDRon_delayMs(10);

	// if receive ">" => send buf
	ciaaPOSIX_ioctl(SERIAL_WIFI, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
	if(ret > 0){
		ciaaPOSIX_read(SERIAL_WIFI, command, 64);

		receive = strstr(command, ">");
		if(receive != NULL){
			ciaaPOSIX_strcpy(command,buf);
			ciaaPOSIX_write(SERIAL_WIFI, command, ciaaPOSIX_strlen(command));

			CDRon_delayMs(10);

			// if receive "OK" => send complete
			ciaaPOSIX_ioctl(SERIAL_WIFI, ciaaPOSIX_IOCTL_GET_RX_COUNT, &ret);
			if(ret > 0){
				ciaaPOSIX_read(SERIAL_WIFI, command, 64);
				receive = strstr(command, "SEND OK");
				if(receive != NULL) return 0;
			}
		}

	}

    return -1;
}


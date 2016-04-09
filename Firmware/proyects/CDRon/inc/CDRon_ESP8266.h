/*
 * Initials     Name
 * ---------------------------
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * yyyymmdd v0.0.1 initials initial version
 */

#include "ciaaPOSIX_stdlib.h"
#include "ciaaPOSIX_stdio.h"


#if (1)
#undef FALSE
#undef TRUE
#include "chip.h"
#endif

int WIFI_init(void);
int WIFI_sendCommand(char *, char *);
int WIFI_readStatus(char *, char *);
void WIFI_configure(void);
void WIFI_saveWIFI(void);
int WIFI_connectAP(void);
int WIFI_getIP(char *);
int WIFI_serverTCP(void);
int WIFI_readData(char*);
int WIFI_sendData(char*,int);


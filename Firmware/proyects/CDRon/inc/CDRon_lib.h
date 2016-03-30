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


//typedef struct {
//   uint8_t hwbuf[64];
//   uint8_t rxcnt;
//} ciaaDriverUartControl2;


/* CIAA functions*/
int CDRon_i2c_read(unsigned char, unsigned char, unsigned char, unsigned char *);
int CDRon_i2c_write(unsigned char, unsigned char, unsigned char,unsigned char const *);
void CDRon_initEEPROM(void);
void CDRon_disableEEPROM(void);

void refreshBrushless(char *);

/* Auxiliary functions */
double AUX_sstr2float(char*);
void AUX_ftoa (double,char *, int32_t);

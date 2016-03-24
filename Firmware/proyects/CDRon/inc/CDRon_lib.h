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


#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

#define PI 3.14159
#define DEFAULT_MPU_HZ  (200)
short gyro[3], accel[3], sensors;

static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};
typedef struct {
   uint8_t hwbuf[64];
   uint8_t rxcnt;
} ciaaDriverUartControl2;

/* Funciones de la CIAA */
int CDRon_i2c_read(unsigned char, unsigned char, unsigned char, unsigned char *);
int CDRon_i2c_write(unsigned char, unsigned char, unsigned char,unsigned char const *);

/* Funciones del MPU6050*/
int MPU6050_initDMP(void);
int MPU6050_init(void);
void MPU6050_getData(void);
void MPU6050_setInterrupt(void);
void MPU6050_clearInterrupt(void);

/* Funciones de conversión */
void MATH_getEuler(float *data, float *q);
void MATH_getYawPitchRoll(float *data, float *q);

/* Funciones auxiliares */
unsigned short AUX_inv_orientation_matrix_to_scalar(const signed char *);
unsigned short AUX_inv_row_2_scale(const signed char *);
double AUX_sstr2float(char*);


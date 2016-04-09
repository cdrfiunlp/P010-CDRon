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


int MPU6050_init(void);
int MPU6050_initDMP(void);
void MPU6050_getData(void);
void MPU6050_setInterrupt(void);
void MPU6050_clearInterrupt(void);
void MPU6050_packaging(char*);

/****************************************************************/
/******************* Auxiliary functions ************************/
/****************************************************************/

void AUX_getEuler(float *, float *);
void AUX_getYawPitchRoll(float *, float *);
unsigned short AUX_inv_orientation_matrix_to_scalar(const signed char *);
unsigned short AUX_inv_row_2_scale(const signed char *);

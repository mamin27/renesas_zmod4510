/*****************************************************************************
 * Copyright (c) 2024 Renesas Electronics Corporation
 * All Rights Reserved.
 *****************************************************************************/

#include "hal/hal.h" 
#include "../sensors/zmod4xxx_types.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

#ifndef ZMOD4510_I2C_BUS_NUMBER
#define ZMOD4510_I2C_BUS_NUMBER 1   // Adjust to match your OlinuXino physical bus
#endif

#ifndef ZMOD4510_I2C_ADDR
#define ZMOD4510_I2C_ADDR       0x33  // Default ZMOD4510 address
#endif

extern uint8_t data_set_4510_init[];
extern uint8_t data_set_4510_no2_o3[];
extern zmod4xxx_conf zmod_no2_o3_sensor_cfg[];

static int i2c_fd = -1;

/* ====================== LINUX I2C ====================== */

static int8_t hal_i2c_write(void* handle, uint8_t slaveAddr,
                            uint8_t* reg, uint8_t regLen,
                            uint8_t* data, uint8_t len)
{
    uint8_t buf[64];
    (void)handle; (void)regLen;

    if (len > 63) return -1;

    buf[0] = reg[0];
    memcpy(&buf[1], data, len);

    if (ioctl(i2c_fd, I2C_SLAVE, slaveAddr) < 0)
        return -1;

    return (write(i2c_fd, buf, len + 1) == len + 1) ? 0 : -1;
}

static int8_t hal_i2c_read(void* handle, uint8_t slaveAddr,
                           uint8_t* reg, uint8_t regLen,
                           uint8_t* data, uint8_t len)
{
    (void)handle; (void)regLen;

    if (ioctl(i2c_fd, I2C_SLAVE, slaveAddr) < 0)
        return -1;

    if (write(i2c_fd, reg, 1) != 1)
        return -1;

    return (read(i2c_fd, data, len) == len) ? 0 : -1;
}

static void hal_ms_sleep(uint32_t ms)
{
    usleep(ms * 1000);
}

/* ====================== HAL ERROR ====================== */
void HAL_HandleError(int errorCode, void const* context)
{
    const char* msg = (const char*)context;
    printf("HAL ERROR: %s (code %d)\n", msg ? msg : "unknown", errorCode);
}

/* ====================== HAL INIT ====================== */
int HAL_Init(Interface_t *hal)
{
    char path[32];

    snprintf(path, sizeof(path), "/dev/i2c-%d", ZMOD4510_I2C_BUS_NUMBER);

    i2c_fd = open(path, O_RDWR);
    if (i2c_fd < 0) {
        perror("HAL_Init: Failed to open I2C bus");
        return -1;
    }

    hal->handle   = NULL;
    hal->i2cWrite = (I2CImpl_t)hal_i2c_write;
    hal->i2cRead  = (I2CImpl_t)hal_i2c_read;
    hal->msSleep  = hal_ms_sleep;

    printf("HAL_Init: ZMOD4510 opened successfully on i2c-%d (addr 0x%02X)\n",
           ZMOD4510_I2C_BUS_NUMBER, ZMOD4510_I2C_ADDR);

    return 0;
}

int HAL_Deinit(Interface_t *hal)
{
    (void)hal;
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
    return 0;
}

/* ====================== ERROR HANDLING ====================== */
static struct {
  int                     error;
  int                     scope;
  ErrorStringGenerator_t  errStrFn;
} lastError;

int HAL_SetError ( int error, int scope, ErrorStringGenerator_t fn ) {
  lastError.error    = error;
  lastError.scope    = scope;
  lastError.errStrFn = fn;

  if ( scope == esSensor )
    return error;
  else
    return ecHALError;
}

char const* HAL_GetErrorInfo ( int* error, int* scope, char* str, int bufLen ) {
  *error = lastError.error;
  *scope = lastError.scope;
  if ( str && bufLen ) {
    if ( lastError.errStrFn )
      lastError.errStrFn ( lastError.error, lastError.scope, str, bufLen );
    else
      snprintf ( str, bufLen, "No additional error information available" );
    return str;
  }
  return NULL;
}

char const* HAL_GetErrorString ( int error, int scope, char* str, int bufLen ) {
  char buf [ 100 ];
  char const* msg;
  switch ( error ) {
  case heNoInterface:    msg = "Interface not found"; break;
  case heNotImplemented: msg = "Function not implemented"; break;
  case heI2CReadMissing: msg = "I2CRead function pointer not set"; break;
  case heI2CWriteMissing:msg = "I2CWrite function pointer not set"; break;
  case heSleepMissing:   msg = "msSleep function pointer not set"; break;
  case heResetMissing:   msg = "reset function pointer not set"; break;
  default:
    sprintf(buf, "Unknown error %d", error);
    msg = buf;
  }
  snprintf(str, bufLen, "HAL Error: %s", msg);
  return str;
}

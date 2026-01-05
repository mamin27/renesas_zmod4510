/*****************************************************************************
 * Copyright (c) 2024 Renesas Electronics Corporation
 * All Rights Reserved.
 * 
 * This code is proprietary to Renesas, and is license pursuant to the terms and
 * conditions that may be accessed at:
 * https://www.renesas.com/eu/en/document/msc/renesas-software-license-terms-gas-sensor-software
 *****************************************************************************/

/**
 * @addtogroup raspi_hal
 * @{
 * @file    rpi.c
 * @brief   Raspberry PI HAL function definitions
 * @version 2.7.1
 * @author  Renesas Electronics Corporation
 */


#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "hal/raspi/rpi.h"
#include "hal/hal.h"


#define I2C_BUS_FILE "/dev/i2c-1"
#define I2C_ADDRESS 0x33

static int i2c_file_descriptor = -1;

// remember hal object for deinitialization
static Interface_t*   _hal = NULL;

static char const*
_GetErrorString(int error, int scope, char* str, int bufLen) {
    if (scope == resI2C) {
        if (error == recI2CLenMismatch) {
            snprintf(str, bufLen, "I2C Error: Data length mismatch");
        } else {
            snprintf(str, bufLen, "I2C Error: %s (errno %d)", strerror(error), error);
        }
    } else {
        snprintf(str, bufLen, "System Error: %s (errno %d)", strerror(error), error);
    }
    return str;
}

static void
_SleepMS ( uint32_t  ms ) {
  usleep ( ms * 1000 );
}

static int
_Connect ( ) {
  // Close existing file descriptor if open
  if (i2c_file_descriptor >= 0) {
    close(i2c_file_descriptor);
  }

  // Open the I2C device file
  i2c_file_descriptor = open(I2C_BUS_FILE, O_RDWR);
  if (i2c_file_descriptor < 0) {
    perror("Failed to open the I2C bus file");
    return ecHALError;
  }

  // Set the I2C slave address
  if (ioctl(i2c_file_descriptor, I2C_SLAVE, I2C_ADDRESS) < 0) {
    perror("Failed to acquire I2C bus access and/or set slave address");
    close(i2c_file_descriptor);
    i2c_file_descriptor = -1;
    return ecHALError;
  }

  return ecSuccess;
}

static int
_I2CRead(void *handle, uint8_t slAddr, uint8_t *wrData, int wrLen, uint8_t *rdData, int rdLen)
{
  if (i2c_file_descriptor < 0)
  {
    fprintf(stderr, "I2C bus not initialized or open.\n");
    return ecHALError;
  }

  struct i2c_msg msgs[2];
  struct i2c_rdwr_ioctl_data msgset;
  int num_msgs = 0;

  // 1. Write the register address (if provided)
  if (wrLen > 0) {
    msgs[num_msgs].addr = slAddr;
    msgs[num_msgs].flags = 0; // Write
    msgs[num_msgs].len = wrLen;
    msgs[num_msgs].buf = wrData;
    num_msgs++;
  }

  // 2. Read the data
  msgs[num_msgs].addr = slAddr;
  msgs[num_msgs].flags = I2C_M_RD; // Read
  msgs[num_msgs].len = rdLen;
  msgs[num_msgs].buf = rdData;
  num_msgs++;

  msgset.msgs = msgs;
  msgset.nmsgs = num_msgs;

  if (ioctl(i2c_file_descriptor, I2C_RDWR, &msgset) < 0) {
    perror("Failed to read from the I2C device");
    return ecHALError;
  }

  return ecSuccess;
}

static int
_I2CWrite(void *handle, uint8_t slAddr, uint8_t *wrData1, int wrLen1, uint8_t *wrData2, int wrLen2)
{
  if (i2c_file_descriptor < 0)
  {
    fprintf(stderr, "I2C bus not initialized or open.\n");
    return ecHALError;
  }

  uint8_t buf[wrLen1 + wrLen2];
  memcpy(buf, wrData1, wrLen1);
  memcpy(buf + wrLen1, wrData2, wrLen2);

  struct i2c_msg msg;
  struct i2c_rdwr_ioctl_data msgset;

  msg.addr = slAddr;
  msg.flags = 0;
  msg.len = wrLen1 + wrLen2;
  msg.buf = buf;

  msgset.msgs = &msg;
  msgset.nmsgs = 1;

  if (ioctl(i2c_file_descriptor, I2C_RDWR, &msgset) < 0) {
    perror("Failed to write to the I2C device");
    return ecHALError;
  }

  return ecSuccess;
}

static int
_Reset ( ) {
  //TODO
  return ecSuccess;  
}

int
HAL_Init ( Interface_t*  hal ) {
  _hal = hal;

  int errorCode = _Connect ( );

  if ( ! errorCode ) {
    hal -> msSleep        = _SleepMS;
    hal -> i2cRead        = _I2CRead;
    hal -> i2cWrite       = _I2CWrite;
    hal -> reset          = _Reset;
  }
  return errorCode;
}


int
HAL_Deinit ( Interface_t*  hal ) {
  int  errorCode = 0;
  if ( i2c_file_descriptor > -1 ) {
    errorCode = close ( i2c_file_descriptor );
    i2c_file_descriptor = -1;
  }
  if ( errorCode )
    return HAL_SetError ( errorCode, resPiGPIO, _GetErrorString );
  return ecSuccess;
}


void
HAL_HandleError ( int  errorCode, void const*  contextV ) {
  char const*  context = ( char const* ) contextV;
  int  error, scope;
  char  msg [ 200 ];
  if ( errorCode ) {
    printf ( "ERROR code %i received during %s\n", errorCode, context );
    printf ( "  %s\n", HAL_GetErrorInfo ( &error, &scope, msg, 200 ) );
  }
  errorCode = HAL_Deinit ( _hal );
  if ( errorCode ) {
    printf ( "ERROR code %i received during interface deinitialization\n", 
             errorCode );
    printf ( "  %s\n", HAL_GetErrorInfo ( &error, &scope, msg, 200 ) );
  }

  printf ( "\nExiting\n" );
  exit ( errorCode );
}

/** @} */

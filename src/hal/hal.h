/*****************************************************************************
 * Copyright (c) 2024 Renesas Electronics Corporation
 * All Rights Reserved.
 *****************************************************************************/

#ifndef HAL_H
#define HAL_H

#include <stdint.h>

/* Error codes */
typedef enum {
  ecSuccess  = 0,
  ecHALError = 0x100
} GenericError_t;

typedef enum {
  esSensor    = 0x0000,
  esAlgorithm = 0x1000,
  esInterface = 0x2000,
  esHAL       = 0x3000,
} ErrorScope_t;

typedef enum {
  heNoInterface = 1,
  heNotImplemented,
  heI2CReadMissing,
  heI2CWriteMissing,
  heSleepMissing,
  heResetMissing
} HALError_t;

typedef int (*I2CImpl_t)(void*, uint8_t, uint8_t*, int, uint8_t*, int);

typedef struct {
  void*       handle;
  I2CImpl_t   i2cRead;
  I2CImpl_t   i2cWrite;
  void      (*msSleep)(uint32_t ms);
  int       (*reset)(void* handle);
} Interface_t;

int  HAL_Init(Interface_t* hal);
int  HAL_Deinit(Interface_t* hal);
void HAL_HandleError(int errorCode, void const* context);

typedef char const* (*ErrorStringGenerator_t)(int, int, char*, int);

int HAL_SetError(int error, int scope, ErrorStringGenerator_t fn);

char const* HAL_GetErrorInfo(int* error, int* scope, char* str, int bufLen);
char const* HAL_GetErrorString(int error, int scope, char* str, int bufLen);

#endif /* HAL_H */

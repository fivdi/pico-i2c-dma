#include <stdarg.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"

void mprintf(const char* format, ...) {
  static bool mutex_initialized = false;
  static SemaphoreHandle_t mprintf_mutex;

  if (!mutex_initialized) {
    mprintf_mutex = xSemaphoreCreateMutex();
    mutex_initialized = true;
  }

  xSemaphoreTake(mprintf_mutex, portMAX_DELAY);

  va_list argptr;
  va_start(argptr, format);
  vprintf(format, argptr);
  va_end(argptr);

  xSemaphoreGive(mprintf_mutex);
}


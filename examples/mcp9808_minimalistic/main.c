#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "i2c_dma.h"
#include "mprintf.h"

static const uint8_t MCP9808_ADDR = 0x18;
static const uint8_t MCP9808_TEMP_REG = 0x05;
static const int32_t MCP9808_POWER_UP_DELAY_MS = 300;

static void mcp9808_task(void *args) {
  i2c_dma_t *i2c_dma;
  i2c_dma_init(&i2c_dma, i2c0, (1000 * 1000), 4, 5);

  vTaskDelay(pdMS_TO_TICKS(MCP9808_POWER_UP_DELAY_MS));

  while (true) {
    uint16_t raw_temp;
    i2c_dma_read_word_swapped(
      i2c_dma, MCP9808_ADDR, MCP9808_TEMP_REG, &raw_temp
    );

    double temp = (raw_temp & 0x0fff) / 16.0 - (raw_temp & 0x1000 ? 256 : 0);

    mprintf("temp: %.4f\n", temp);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  stdio_init_all();

  xTaskCreate(
    mcp9808_task, "mcp9808-task", configMINIMAL_STACK_SIZE,
    NULL, configMAX_PRIORITIES - 2, NULL
  );

  vTaskStartScheduler();
}


#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "i2c_dma.h"
#include "mprintf.h"

static const uint8_t MCP9808_ADDR = 0x18;
static const uint8_t MCP9808_TEMP_REG = 0x05;

// After power-up the MCP9808 typically requires 250 ms to perform the first
// conversion at the power-up default resolution. See datasheet.
static const int32_t MCP9808_POWER_UP_DELAY_MS = 300;

static const uint8_t BME280_ADDR = 0x76;
static const uint8_t BME280_ID_REG = 0xd0;

static i2c_dma_t *i2c0_dma;
static i2c_dma_t *i2c1_dma;

static void blink_led_task(void *args) {
  (void) args;

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
  gpio_put(PICO_DEFAULT_LED_PIN, !PICO_DEFAULT_LED_PIN_INVERTED);

  while (true) {
    gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static double mcp9808_raw_temp_to_celsius(uint16_t raw_temp) {
  double celsius = (raw_temp & 0x0fff) / 16.0;

  if (raw_temp & 0x1000) {
    celsius -= 256;
  }

  return celsius;
}

// Note that two instances of this task run concurrently.
static void access_all_devices_task(void *args) {
  (void) args;

  vTaskDelay(pdMS_TO_TICKS(MCP9808_POWER_UP_DELAY_MS));

  for (int err_cnt = 0, i = 0; true; i += 1) {
    uint16_t raw_temp0;
    const int rc0 = i2c_dma_read_word_swapped(
      i2c0_dma, MCP9808_ADDR, MCP9808_TEMP_REG, &raw_temp0
    );

    uint16_t raw_temp1;
    const int rc1 = i2c_dma_read_word_swapped(
      i2c1_dma, MCP9808_ADDR, MCP9808_TEMP_REG, &raw_temp1
    );

    uint8_t id;
    const int rc2 = i2c_dma_read_byte(
      i2c1_dma, BME280_ADDR, BME280_ID_REG, &id
    );

    if (rc0 != PICO_OK || rc1 != PICO_OK || rc2 != PICO_OK) {
      err_cnt += 1;
      mprintf(
        "access all, "
        "error (i: %d, rc0: %d, rc1: %d, rc2: %d, errors: %d)\n",
        i, rc0, rc1, rc2, err_cnt
      );
    } else if (i % 10000 == 0) {
      const double celsius0 = mcp9808_raw_temp_to_celsius(raw_temp0);
      const double celsius1 = mcp9808_raw_temp_to_celsius(raw_temp1);
      mprintf(
        "access all, "
        "temp0: %.4f, temp1: %.4f, id: %d (i: %d, errors: %d)\n",
        celsius0, celsius1, id, i, err_cnt
      );
    }
  }
}

static void waste_time_task(void *args) {
  (void) args;

  double billion_iterations = 0;

  while (true) {
    for (int j = 0; j != 100 * 1000 * 1000; j += 1) {
      __asm__("nop");
    }

    billion_iterations += 0.1;

    mprintf("%.1f billion iterations\n", billion_iterations);
  }
}

int main(void) {
  stdio_init_all();

  int rc = i2c_dma_init(&i2c0_dma, i2c0, (1000 * 1000), 4, 5);
  if (rc != PICO_OK) {
    mprintf("can't configure I2C0\n");
    return rc;
  }

  rc = i2c_dma_init(&i2c1_dma, i2c1, (1000 * 1000), 6, 7);
  if (rc != PICO_OK) {
    mprintf("can't configure I2C1\n");
    return rc;
  }

  xTaskCreate(
    blink_led_task,
    "blink-led-task",
    configMINIMAL_STACK_SIZE,
    NULL,
    configMAX_PRIORITIES - 2,
    NULL
  );

  xTaskCreate(
    access_all_devices_task,
    "access-all-devices-task-1",
    configMINIMAL_STACK_SIZE,
    NULL,
    configMAX_PRIORITIES - 2,
    NULL
  );

  xTaskCreate(
    access_all_devices_task,
    "access-all-devices-task-2",
    configMINIMAL_STACK_SIZE,
    NULL,
    configMAX_PRIORITIES - 2,
    NULL
  );

  xTaskCreate(
    waste_time_task,
    "waste-time-task",
    configMINIMAL_STACK_SIZE,
    NULL,
    configMAX_PRIORITIES - 4,
    NULL
  );

  vTaskStartScheduler();
}


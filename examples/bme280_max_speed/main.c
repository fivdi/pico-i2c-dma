#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "i2c_dma.h"
#include "mprintf.h"

#define BME280_ADDR      0x76
#define BME280_ID_REG    0xd0

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

static void bme280_task(void *args) {
  i2c_dma_t *i2c_dma = (i2c_dma_t *) args;

  for (int err_cnt = 0, i = 0; true; i += 1) {
    uint8_t id;
    const int rc = i2c_dma_read_byte(
      i2c_dma, BME280_ADDR, BME280_ID_REG, &id
    );

    if (rc != PICO_OK) {
      err_cnt += 1;
      mprintf("error (i: %d, rc: %d, errors: %d)\n", i, rc, err_cnt);
    } else if (i % 10000 == 0) {
      mprintf("id: %d (i: %d, errors: %d)\n", id, i, err_cnt);
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

int main() {
  stdio_init_all();

  static i2c_dma_t *i2c1_dma;
  const int rc = i2c_dma_init(&i2c1_dma, i2c1, (1000 * 1000), 6, 7);
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
    bme280_task,
    "bme280-task",
    configMINIMAL_STACK_SIZE,
    i2c1_dma,
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


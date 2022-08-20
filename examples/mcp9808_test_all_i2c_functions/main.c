#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "i2c_dma.h"
#include "mprintf.h"

static const uint8_t MCP9808_ADDR = 0x18;
static const uint8_t MCP9808_CRIT_TEMP_REG = 0x04;
static const uint8_t MCP9808_TEMP_REG = 0x05;
static const uint8_t MCP9808_RESOLUTION_REG = 0x08;

// After power-up the MCP9808 typically requires 250 ms to perform the first
// conversion at the power-up default resolution. See datasheet.
static const int32_t MCP9808_POWER_UP_DELAY_MS = 300;

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
  return (raw_temp & 0x0fff) / 16.0 - (raw_temp & 0x1000 ? 256 : 0);
}

static void dma_write_followed_by_dma_read(i2c_dma_t *i2c_dma) {
  mprintf("dma_write_followed_by_dma_read\n");

  for (uint8_t resolution = 0; resolution <= 3; resolution += 1) {
    const uint8_t wbuf_a[2] = {MCP9808_RESOLUTION_REG, resolution};
    int rc = i2c_dma_write(i2c_dma, MCP9808_ADDR, wbuf_a, 2);
    if (rc != PICO_OK) {
      mprintf("  1st i2c_dma_write failed, rc: %d\n", rc);
    } else {
      const uint8_t wbuf_b[1] = {MCP9808_RESOLUTION_REG};
      rc = i2c_dma_write(i2c_dma, MCP9808_ADDR, wbuf_b, 1);
      if (rc != PICO_OK) {
        mprintf("  2nd i2c_dma_write failed, rc: %d\n", rc);
      } else {
        uint8_t rbuf[1];
        rc = i2c_dma_read(i2c_dma, MCP9808_ADDR, rbuf, 1);
        if (rc != PICO_OK) {
          mprintf("  i2c_dma_read failed, rc: %d\n", rc);
        } else if (resolution != rbuf[0]) {
          mprintf("  expected resolution %d, got %d\n", resolution, rbuf[0]);
        } else {
          mprintf("  ok, resolution set to %d\n", resolution);
        }
      }
    }
  }
}

static void dma_write_byte_followed_by_dma_read_byte(i2c_dma_t *i2c_dma) {
  mprintf("dma_write_byte_followed_by_dma_read_byte\n");

  for (uint8_t resolution = 0; resolution <= 3; resolution += 1) {
    int rc = i2c_dma_write_byte(
      i2c_dma, MCP9808_ADDR, MCP9808_RESOLUTION_REG, resolution
    );
    if (rc != PICO_OK) {
      mprintf("  i2c_dma_write_byte failed, rc: %d\n", rc);
    } else {
      uint8_t byte;
      rc = i2c_dma_read_byte(
        i2c_dma, MCP9808_ADDR, MCP9808_RESOLUTION_REG, &byte
      );
      if (rc != PICO_OK) {
        mprintf("  i2c_dma_read_byte failed, rc: %d\n", rc);
      } else if (resolution != byte) {
        mprintf("  expected resolution %d, got %d\n", resolution, byte);
      } else {
        mprintf("  ok, resolution set to %d\n", resolution);
      }
    }
  }
}

static void dma_write_word_followed_by_dma_read_word(i2c_dma_t *i2c_dma) {
  mprintf("dma_write_word_followed_by_dma_read_word\n");

  for (int i = 4; i >= 0; i -= 1) {
    uint16_t crit_temp = i << 9;

    int rc = i2c_dma_write_word(
      i2c_dma,
      MCP9808_ADDR,
      MCP9808_CRIT_TEMP_REG,
      crit_temp << 8 | crit_temp >> 8
    );
    if (rc != PICO_OK) {
      mprintf("  i2c_dma_write_word failed, rc: %d\n", rc);
    } else {
      uint16_t word;
      rc = i2c_dma_read_word(
        i2c_dma, MCP9808_ADDR, MCP9808_CRIT_TEMP_REG, &word
      );
      word = word << 8 | word >> 8;
      if (rc != PICO_OK) {
        mprintf("  i2c_dma_read_word failed, rc: %d\n", rc);
      } else if (crit_temp != word) {
        mprintf(
          "  expected critical temperature 0x%04x, got 0x%04x\n",
          crit_temp, word
        );
      } else {
        mprintf(
          "  ok, critical temperature set to 0x%04x (%.4f)\n",
          crit_temp, mcp9808_raw_temp_to_celsius(crit_temp)
        );
      }
    }
  }
}

static void dma_write_word_swapped_followed_by_dma_read_word_swapped(
  i2c_dma_t *i2c_dma
) {
  mprintf("dma_write_word_swapped_followed_by_dma_read_word_swapped\n");

  for (int i = 4; i >= 0; i -= 1) {
    uint16_t crit_temp = i << 9;

    int rc = i2c_dma_write_word_swapped(
      i2c_dma, MCP9808_ADDR, MCP9808_CRIT_TEMP_REG, crit_temp
    );
    if (rc != PICO_OK) {
      mprintf("  i2c_dma_write_word_swapped failed, rc: %d\n", rc);
    } else {
      uint16_t word;
      rc = i2c_dma_read_word_swapped(
        i2c_dma, MCP9808_ADDR, MCP9808_CRIT_TEMP_REG, &word
      );
      if (rc != PICO_OK) {
        mprintf("  i2c_dma_read_word_swapped failed, rc: %d\n", rc);
      } else if (crit_temp != word) {
        mprintf(
          "  expected critical temperature 0x%04x, got 0x%04x\n",
          crit_temp, word
        );
      } else {
        mprintf(
          "  ok, critical temperature set to 0x%04x (%.4f)\n",
          crit_temp, mcp9808_raw_temp_to_celsius(crit_temp)
        );
      }
    }
  }
}

static void mcp9808_task(void *args) {
  i2c_dma_t *i2c_dma = (i2c_dma_t *) args;

  vTaskDelay(pdMS_TO_TICKS(MCP9808_POWER_UP_DELAY_MS));

  mprintf("----------------------------------------\n");

  dma_write_followed_by_dma_read(i2c_dma);
  dma_write_byte_followed_by_dma_read_byte(i2c_dma);
  dma_write_word_followed_by_dma_read_word(i2c_dma);
  dma_write_word_swapped_followed_by_dma_read_word_swapped(i2c_dma);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

int main(void) {
  stdio_init_all();

  static i2c_dma_t *i2c0_dma;
  const int rc = i2c_dma_init(&i2c0_dma, i2c0, (1000 * 1000), 4, 5);
  if (rc != PICO_OK) {
    mprintf("can't configure I2C0\n");
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
    mcp9808_task,
    "mcp9808-task",
    configMINIMAL_STACK_SIZE,
    i2c0_dma,
    configMAX_PRIORITIES - 2,
    NULL
  );

  vTaskStartScheduler();
}


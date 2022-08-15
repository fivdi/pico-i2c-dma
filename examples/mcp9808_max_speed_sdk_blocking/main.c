#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MCP9808_ADDR     0x18
#define MCP9808_TEMP_REG 0x05

// After power-up the MCP9808 typically requires 250 ms to perform the first
// conversion at the power-up default resolution. See datasheet.
#define MCP9808_POWER_UP_DELAY_MS 300

static double mcp9808_raw_temp_to_celsius(uint16_t raw_temp) {
  double celsius = (raw_temp & 0x0fff) / 16.0;

  if (raw_temp & 0x1000) {
    celsius -= 256;
  }

  return celsius;
}

static int read_word_swapped(i2c_inst_t *i2c, int addr, uint8_t reg, int16_t *word) {
  int rc = i2c_write_blocking(i2c, addr, &reg, 1, true);
  if (rc < 1) {
    return rc;
  } else if (rc != 1) {
    return PICO_ERROR_GENERIC;
  }

  rc = i2c_read_blocking(i2c, addr, (uint8_t *) word, 2, false);
  if (rc < 1) {
    return rc;
  } else if (rc != 2) {
    return PICO_ERROR_GENERIC;
  }

  *word = *word << 8 | ((*word >> 8) & 0xff);

  return PICO_OK;
}

int main() {
  stdio_init_all();

  i2c_init(i2c0, 1000 * 1000);
  gpio_set_function(4, GPIO_FUNC_I2C);
  gpio_set_function(5, GPIO_FUNC_I2C);
  gpio_pull_up(4);
  gpio_pull_up(5);

  sleep_ms(MCP9808_POWER_UP_DELAY_MS);

  for (int err_cnt = 0, i = 0; true; i += 1) {
    uint16_t raw_temp;
    const int rc = read_word_swapped(
      i2c0, MCP9808_ADDR, MCP9808_TEMP_REG, &raw_temp
    );

    if (rc != PICO_OK) {
      err_cnt += 1;
      printf("error (i: %d, rc: %d, errors: %d)\n", i, rc, err_cnt);
    } else if (i % 10000 == 0) {
      const double celsius = mcp9808_raw_temp_to_celsius(raw_temp);
      printf("temp: %.4f (i: %d)\n", celsius, i);
    }
  }
}


# pico-i2c-dma
A DMA based I2C driver for the RP2040.


## Contents

- [Installation](#installation)
- [Usage](#usage)
- [Examples](#examples)
- [API](#examples)


## Installation

#### Prerequisites

- The [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) has
been successfully installed and the `PICO_SDK_PATH` environment variable has
been appropriately set.
- The [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) has been
successfully installed and the `FREERTOS_KERNEL_PATH` environment variable has
been appropriately set.

If the Prerequisites are satisfied, this repository can be cloned and built
with the following commands:

```
git clone https://github.com/fivdi/pico-i2c-dma.git
cd pico-i2c-dma
mkdir build
cd build
cmake ..
make
```


## Usage

- Call `i2c_dma_init` to initialize an I2C peripheral, its baudrate, its SDA
pin, its SCL pin, enable the peripheral, and prepare it for DMA usage
- Call `i2c_dma_*` functions to communicate with I2C devices on an I2C bus

Here is a minimalistic example that continuously reads the temperature from an
MCP9808 temperature sensor and prints the temperature.

```c
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
```


## Examples

Examples can be found in the [examples](examples/) directory. The examples
were implemented to verify that the I2C driver functions as expected rather
than to do anything useful. Each example has a readme explaining its purpose.

Depending on the example, one or more of the following I2C devices will be
required for the example to function as expected:

- An MCP9808 temperature sensor at address 0x18 on I2C0 (GP4 and GP5)
- An MCP9808 temperature sensor at address 0x18 on I2C1 (GP6 and GP7)
- A BME280 sensor at address 0x76 on I2C1 (GP6 and GP7)


## API

The API is documented in [i2c_dma.h](https://github.com/fivdi/pico-i2c-dma/blob/master/src/include/i2c_dma.h)


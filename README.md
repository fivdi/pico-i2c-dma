# pico-i2c-dma
A DMA based I2C driver for the RP2040.


## Contents

- [Installation](#installation)
- [Examples](#examples)


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

## Examples

Examples can be found in the [examples](examples/) directory. The examples
were implemented to verify that the I2C driver functions as expected rather
than to do anything useful.

Depending on the example, one or more of the following I2C devices will be
required for the example to function as expected:

- An MCP9808 temperature sensor at address 0x18 on I2C0 (GP4 and GP5)
- An MCP9808 temperature sensor at address 0x18 on I2C1 (GP6 and GP7)
- An BME280 sensor at address 0x76 on I2C1 (GP6 and GP7)


# access_all_devices_x2_max_speed

This example can be regarded as a brute-force reliability test. It runs two
identical tasks, `access_all_devices_task`, both of which access all devices
on I2C0 and I2C1 as fast and as often as possible. The idea is that the
program should be capable of running "forever" without crashing.

This example assumes the following setup:

- An MCP9808 temperature sensor at address 0x18 on I2C0 (GP4 and GP5)
- An MCP9808 temperature sensor at address 0x18 on I2C1 (GP6 and GP7)
- A BME280 sensor at address 0x76 on I2C1 (GP6 and GP7)

This example, like other examples, performs error checking and can recover
from situations like:

- Removing any device from its I2C bus and plugging it back in again
- Temporarily connecting SDA to 0V
- Temporarily connecting SCL to 0V
- ...


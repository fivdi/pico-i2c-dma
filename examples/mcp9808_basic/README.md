# mcp9808_basic

This example demonstrates how read the ambient temperature from an MCP9808
temperature sensor.

The MCP9808 is assumed to be at address 0x18 on I2C0 (GP4 and GP5).

This example, like other examples, performs error checking and can recover
from situations like:

- A blocked or jammed I2C bus
- Temporarily connecting SDA to 0V
- Temporarily connecting SCL to 0V
- Removing the MCP9808 from the I2C bus and plugging it back in again
- ...


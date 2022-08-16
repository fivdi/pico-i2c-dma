# mcp9808_max_speed_sdk_blocking

The goal of this example is use the Pico SDK blocking I2C functions to
continuously read the 16-bit ambient temperature register on an MCP9808
temperature sensor in order to determine how often the temperature register
can be read per second.

The MCP9808 is assumed to be at address 0x18 on I2C0 (GP4 and GP5).

At a baud rate of 1,000,000 the temperature register can be read 1,000,000
times in approximately 62 seconds or 16129 times per second.

Because the Pico SDK blocking I2C functions are used, the program spends most
of its time polling and waiting for the I2C operations to complete which
prevents other tasks from being performed at the same time.


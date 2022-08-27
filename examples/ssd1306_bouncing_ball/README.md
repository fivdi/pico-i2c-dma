# ssd1306_bouncing_ball

This example demonstrates how use a 128x64 pixel SSD1306 OLED display with the
[µGUI graphic library](https://github.com/achimdoebler/UGUI).

The example continuously performs the following actions:
- Displays a 1-line message at the top of the display
- Draws a horizontal line below the 1-line message
- Moves a ball up, down, left and right in the area below the horizontal line

The 128x64 pixel SSD1306 OLED display is assumed to be at address 0x3c on I2C0
(GP4 and GP5).


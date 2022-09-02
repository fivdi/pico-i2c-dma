#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "i2c_dma.h"
#include "mprintf.h"
#include "ugui.h"

#define SSD1306_ADDR 0x3c
#define SSD1306_BYTES_PER_PAGE 128

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#define MAX_X (DISPLAY_WIDTH - 1)
#define MAX_Y (DISPLAY_HEIGHT - 1)

static uint8_t ssd1306_i2c_message[1 + (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)];
static uint8_t *ssd1306_pixel_buffer = &ssd1306_i2c_message[1];
static UG_GUI gui;

typedef enum {
  // Fundamental commands

  SET_CONTRAST = 0x81,         // Double byte command to select 1 out of
                               // 256 contrast steps.
  SET_ENTIRE_DISP_ON = 0xa4,   // Bit0 = 0: Output follows RAM content.
                               // Bit0 = 1: Output ignores RAM content,
                               //           all pixels are turned on.
  SET_NORMAL_INVERTED = 0xa6,  // Bit0 = 0: Normal display.
                               // Bit0 = 1: Inverted display.
  SET_DISP_ON_OFF = 0xae,      // Bit0 = 0: Display off, sleep mode.
                               // Bit0 = 1: Display on, normal mode.

  // Addressing setting Commands

  SET_ADDRESSING_MODE = 0x20,  // Double byte command to set memory
                               // addressing mode.
  SET_COLUMN_ADDRESS = 0x21,   // Tripple byte command to setup column start
                               // and end address.
  SET_PAGE_ADDRESS = 0x22,     // Tripple byte command to setup page start and
                               // end address.

  // Hardware configuration (panel resolution and layout related) commands

  SET_DISP_START_LINE = 0x40,  // Set display RAM display start line
                               // register. Valid values are 0 to 63.
  SET_SEGMENT_REMAP = 0xa0,    // Bit 0 = 0: Map col addr 0 to SEG0.
                               // Bit 0 = 1: Map col addr 127 to SEG0.
  SET_MUX_RATIO = 0xa8,        // Double byte command to configure display
                               // height. Valid height values are 15 to 63.
  SET_COM_OUTPUT_DIR = 0xc0,   // Bit 3 = 0: Scan from 0 to N-1.
                               // Bit 3 = 1: Scan from N-1 to 0. (N=height)
  SET_DISP_OFFSET = 0xd3,      // Double byte command to configure vertical
                               // display shift. Valid values are 0 to 63.
  SET_COM_PINS_CONFIG = 0xda,  // Double byte command to set COM pins
                               // hardware configuration.

  // Timing and driving scheme setting commands

  SET_DCLK_FOSC = 0xd5,        // Double byte command to set display clock
                               // divide ratio and oscillator frequency.
  SET_PRECHARGE_PERIOD = 0xd9, // Double byte command to set pre-charge
                               // period.
  SET_VCOM_DESEL_LEVEL = 0xdb, // Double byte command to set VCOMH deselect
                               // level.

  // Charge pump command

  SET_CHARGE_PUMP = 0x8d,      // Double byte command to enable/disable
                               // charge pump.
                               // Byte2 = 0x10: Disable charge pump.
                               // Byte2 = 0x14: Enable charge pump.
} ssd1306_command_t;

static int ssd1306_send_command(i2c_dma_t *i2c_dma, uint8_t byte) {
  return i2c_dma_write_byte(i2c_dma, SSD1306_ADDR, 0x80, byte);
}

static void ssd1306_init(i2c_dma_t *i2c_dma) {
  const uint8_t commands[] = {
    SET_DISP_ON_OFF | 0x00,         // Display off.
    SET_DCLK_FOSC, 0x80,            // Set clock divide ratio and oscillator
                                    //   frequency.
    SET_MUX_RATIO, 0x3f,            // Set display height.
    SET_DISP_OFFSET, 0x00,          // Set vertical display shift to 0.
    SET_DISP_START_LINE,            // Set display RAM display start line
                                    //   register to 0.
    SET_CHARGE_PUMP, 0x14,          // Enable charge pump.
    SET_SEGMENT_REMAP | 0x01,       // Map col addr 127 to SEG0.
    SET_COM_OUTPUT_DIR | 0x08,      // Scan from N-1 to 0. (N=height)
    SET_COM_PINS_CONFIG, 0x12,      // Set COM pins hardware configuration to
                                    //   0x12.
    SET_CONTRAST, 0xcf,             // Set contrast to 0xcf
    SET_PRECHARGE_PERIOD, 0xf1,     // Set pre-charge to 0xf1
    SET_VCOM_DESEL_LEVEL, 0x40,     // Set VCOMH deselect to 0x40
    SET_ENTIRE_DISP_ON,             // Output follows RAM content.
    SET_NORMAL_INVERTED | 0x00,     // Normal display.
    SET_ADDRESSING_MODE, 0x00,      // Set addressing mode to horizontal mode.
    SET_COLUMN_ADDRESS, 0x00, 0x7f, // Set column start and end address.
    SET_PAGE_ADDRESS, 0x00, 0x07,   // Set page start and end address.
    SET_DISP_ON_OFF | 0x01,         // Display on.
  };

  for(int i = 0; i < sizeof(commands); i++) {
    ssd1306_send_command(i2c_dma, commands[i]);
  }

  ssd1306_i2c_message[0] = 0x40;
}

static void ssd1306_update(i2c_dma_t *i2c_dma) {
  i2c_dma_write(
    i2c_dma, SSD1306_ADDR, ssd1306_i2c_message, sizeof(ssd1306_i2c_message)
  );
}

static void ugui_draw_pixel_callback(UG_S16 x, UG_S16 y, UG_COLOR color) {
  if (x < 0 || x > MAX_X || y < 0 || y > MAX_Y)
    return;

  const uint8_t page_number = y / 8;
  const uint8_t column_number = x;
  uint8_t *byte = &ssd1306_pixel_buffer[
    page_number * SSD1306_BYTES_PER_PAGE + column_number
  ];

  const uint8_t bit_number = y % 8;
  const uint8_t bit_mask = 1 << bit_number;

  switch (color) {
    case C_BLACK:
      *byte &= ~bit_mask; // Black -> clear pixel.
      break;
    case C_WHITE:
      *byte |= bit_mask;  // White -> set pixel.
      break;
    default:
      *byte ^= bit_mask;  // Any other color -> invert pixel.
  }
}

static void ugui_init() {
  UG_Init(&gui, ugui_draw_pixel_callback, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_WHITE);
  UG_FillScreen(C_BLACK);
}

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

static void ssd1306_bouncing_ball_task(void *args) {
  i2c_dma_t *i2c_dma = (i2c_dma_t *) args;

  ssd1306_init(i2c_dma);
  ugui_init();

  // Start position of ball is the top left just below the horizontal line.
  const UG_S16 ball_radius = 8;
  UG_S16 ball_x = 7;
  UG_S16 ball_y = 21;
  UG_S16 x_inc = 1; // 1: move right, -1: move left
  UG_S16 y_inc = 1; // 1: move down, -1: move up

  const UG_FONT *font = &FONT_5X12;
  UG_FontSelect(font);

  for (int i = 0; true; i += 1) {
    // Clear the display.
    UG_FillScreen(C_BLACK);

    // Print a message at the top left of the display.
    char message[20];
    sprintf(message, "%d", i + 1);
    UG_PutString(0, 0, message);

    // Draw a horizontal line below the message.
    // The ball bounces in the area below the horizontal line.
    const UG_S16 H_LINE_Y = font->char_height + 1;
    UG_DrawLine(0, H_LINE_Y, MAX_X, H_LINE_Y, C_WHITE);

    // If the ball has bounced off the left or right, update x_inc.
    if (ball_x - ball_radius <= 0) {
      // Ball bounced off left, begin moving right.
      x_inc = 1;
    } else if (ball_x + ball_radius >= MAX_X) {
      // Ball bounced off right, begin moving left.
      x_inc = -1;
    }

    // If the ball has bounced off the top or bottom, update y_inc.
    if (ball_y - ball_radius <= H_LINE_Y + 1) {
      // Ball bounced off top, begin moving down.
      y_inc = 1;
    } else if (ball_y + ball_radius >= MAX_Y) {
      // Ball bounced off bottom, begin moving up.
      y_inc = -1;
    }

    // Move the ball one pixel left or right, and up or down.
    ball_x += x_inc;
    ball_y += y_inc;

    // Draw the ball.
    UG_DrawCircle(ball_x, ball_y, ball_radius, C_WHITE);

    // Send all pixels to the SSD1306.
    ssd1306_update(i2c_dma);
  }
}

int main(void) {
  stdio_init_all();

  i2c_dma_t *i2c0_dma;
  const int rc = i2c_dma_init(&i2c0_dma, i2c0, 1000000, 4, 5);
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
    ssd1306_bouncing_ball_task,
    "ssd1306-bouncing-ball-task",
    configMINIMAL_STACK_SIZE,
    i2c0_dma,
    configMAX_PRIORITIES - 2,
    NULL
  );

  vTaskStartScheduler();
}


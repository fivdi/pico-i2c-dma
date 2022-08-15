#ifndef _I2C_DMA_H
#define _I2C_DMA_H

#include "hardware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct i2c_dma_s i2c_dma_t;

int i2c_dma_init(
  i2c_dma_t **pi2c_dma,
  i2c_inst_t *i2c,
  uint baudrate,
  uint sda_gpio,
  uint scl_gpio
);

// S             Start condition.
// Sr            Repeated start condition, used to switch from write to read
//               mode.
// P             Stop condition.
// Rd/Wr (1 bit) Read/Write bit. Rd equals 1, Wr equals 0.
// A, NA (1 bit) Acknowledge (ACK) and Not Acknowledge (NACK) bit
// addr (7 bits) I2C 7 bit address.
// reg (8 bits)  Register byte, a data byte which typically selects a register
//               on the device.
// [..]          Data sent by I2C device, as opposed to data sent by the host
//               adapter.

// S addr Wr [A] wbuf(0) [A] wbuf(1) [A] ... [A] wbuf(wbuf_len-1) [A] P
// or
// S addr Rd [A] [rbuf(0)] A [rbuf(1)] A ... A [rbuf(rbuf_len-1)] NA P
// or
// S addr Wr [A] wbuf(0) [A] wbuf(1) [A] ... [A] wbuf(wbuf_len-1) [A]
//   Sr addr Rd [A] [rbuf(0)] A [rbuf(1)] A ... A [rbuf(rbuf_len-1)] NA P
int i2c_dma_write_read(
  i2c_dma_t *i2c_dma,
  uint8_t addr,
  const uint8_t *wbuf,
  size_t wbuf_len,
  uint8_t *rbuf,
  size_t rbuf_len
);

// S addr Wr [A] wbuf(0) [A] wbuf(1) [A] ... [A] wbuf(wbuf_len-1) [A] P
static inline int i2c_dma_write(
  i2c_dma_t *i2c_dma, uint8_t addr, const uint8_t *wbuf, size_t wbuf_len
) {
  return i2c_dma_write_read(i2c_dma, addr, wbuf, wbuf_len, NULL, 0);
}

// S addr Rd [A] [rbuf(0)] A [rbuf(1)] A ... A [rbuf(rbuf_len-1)] NA P
static inline int i2c_dma_read(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t *rbuf, size_t rbuf_len
) {
  return i2c_dma_write_read(i2c_dma, addr, NULL, 0, rbuf, rbuf_len);
}

// S addr Wr [A] reg [A] byte [A] P
static inline int i2c_dma_write_byte(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t reg, uint8_t byte
) {
  const uint8_t wbuf[2] = {reg, byte};
  return i2c_dma_write_read(i2c_dma, addr, wbuf, 2, NULL, 0);
}

// S addr Wr [A] reg [A] Sr addr Rd [A] [byte] NA P
static inline int i2c_dma_read_byte(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t reg, uint8_t *byte
) {
  return i2c_dma_write_read(i2c_dma, addr, &reg, 1, byte, 1);
}

// S addr Wr [A] reg [A] word lsb [A] word msb [A] P
static inline int i2c_dma_write_word(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t reg, uint16_t word
) {
  const uint8_t wbuf[3] = {reg, word & 0xff, word >> 8};
  return i2c_dma_write_read(i2c_dma, addr, wbuf, 3, NULL, 0);
}

// S addr Wr [A] reg [A] Sr addr Rd [A] [word lsb] A [word msb] NA P
static inline int i2c_dma_read_word(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t reg, uint16_t *word
) {
  return i2c_dma_write_read(i2c_dma, addr, &reg, 1, (uint8_t *) word, 2);
}

// S addr Wr [A] reg [A] word msb [A] word lsb [A] P
static inline int i2c_dma_write_word_swapped(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t reg, uint16_t word
) {
  const uint8_t wbuf[3] = {reg, word >> 8, word & 0xff};
  return i2c_dma_write_read(i2c_dma, addr, wbuf, 3, NULL, 0);
}

// S addr Wr [A] reg [A] Sr addr Rd [A] [word msb] A [word lsb] NA P
static inline int i2c_dma_read_word_swapped(
  i2c_dma_t *i2c_dma, uint8_t addr, uint8_t reg, uint16_t *word
) {
  int rc = i2c_dma_write_read(i2c_dma, addr, &reg, 1, (uint8_t *) word, 2);
  *word = *word << 8 | *word >> 8;
  return rc;
}

#ifdef __cplusplus
}
#endif

#endif


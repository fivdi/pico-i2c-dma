# bme280_max_speed

The goal of this example is to use DMA based I2C functions to continuously read
the 8-bit ID register on a BME280 sensor in order to determine how often the
ID register can be read per second.

The BME280 is assumed to be at address 0x76 on I2C1 (GP6 and GP7).

At a baud rate of 1,000,000 the ID register can be read 1,000,000 times in
approximately 61 seconds or 16393 times per second.

Because DMA based I2C functions are used, the program does not spend any of
its time polling for the I2C operations to complete which allows other tasks
to be performed at the same time.

In this program two others tasks are performed at the same time. The first
task, `blink_led_task`, blinks an LED at a frequency of 1Hz. The second task,
`waste_time_task`, increments a counter in an endless loop.

In the 61 seconds that are required to read the ID register 1,000,000 times,
`waste_time_task` can increment it's counter 1,000,000,000 times. Each
iteration of the endless loop incrementing the counter executes 4 instructions
requiring 5 processor cycles:

```
100002ee:       46c0            nop                                    ; 1 cycle
100002f0:       3b01            subs    r3, #1                         ; 1 cycle
100002f2:       2b00            cmp     r3, #0                         ; 1 cycle
100002f4:       d1fb            bne.n   100002ee <waste_time_task+0xa> ; 2 cycles
```

This information can be used to calculate approximately how much processor
time DMA based I2C requires.

Processor cycles per second required by `waste_time_task` to increment its
counter:

```
1e9 * 5 / 61 = 81967213
```

If the processor is running at 125MHz and we assume that all remaining
processor cycles are used for DMA based I2C, then the number of processor
cycles per second required for DMA based I2C is:

```
125000000 - 81967213 = 43032787
```

The program spends 65.57% of its time incrementing the counter and 34.43% of
its time reading from the BME280 sensor.

Typical program output:

```
id: 96 (i: 0, errors: 0)
id: 96 (i: 10000, errors: 0)
id: 96 (i: 20000, errors: 0)
id: 96 (i: 30000, errors: 0)
id: 96 (i: 40000, errors: 0)
id: 96 (i: 50000, errors: 0)
id: 96 (i: 60000, errors: 0)
id: 96 (i: 70000, errors: 0)
id: 96 (i: 80000, errors: 0)
id: 96 (i: 90000, errors: 0)
0.1 billion iterations
id: 96 (i: 100000, errors: 0)
id: 96 (i: 110000, errors: 0)
id: 96 (i: 120000, errors: 0)
id: 96 (i: 130000, errors: 0)
id: 96 (i: 140000, errors: 0)
id: 96 (i: 150000, errors: 0)
id: 96 (i: 160000, errors: 0)
id: 96 (i: 170000, errors: 0)
id: 96 (i: 180000, errors: 0)
id: 96 (i: 190000, errors: 0)
0.2 billion iterations
id: 96 (i: 200000, errors: 0)
id: 96 (i: 210000, errors: 0)
id: 96 (i: 220000, errors: 0)
id: 96 (i: 230000, errors: 0)
id: 96 (i: 240000, errors: 0)
id: 96 (i: 250000, errors: 0)
id: 96 (i: 260000, errors: 0)
id: 96 (i: 270000, errors: 0)
id: 96 (i: 280000, errors: 0)
id: 96 (i: 290000, errors: 0)
0.3 billion iterations
id: 96 (i: 300000, errors: 0)
id: 96 (i: 310000, errors: 0)
id: 96 (i: 320000, errors: 0)
id: 96 (i: 330000, errors: 0)
id: 96 (i: 340000, errors: 0)
id: 96 (i: 350000, errors: 0)
id: 96 (i: 360000, errors: 0)
id: 96 (i: 370000, errors: 0)
id: 96 (i: 380000, errors: 0)
id: 96 (i: 390000, errors: 0)
0.4 billion iterations
id: 96 (i: 400000, errors: 0)
id: 96 (i: 410000, errors: 0)
id: 96 (i: 420000, errors: 0)
id: 96 (i: 430000, errors: 0)
id: 96 (i: 440000, errors: 0)
id: 96 (i: 450000, errors: 0)
id: 96 (i: 460000, errors: 0)
id: 96 (i: 470000, errors: 0)
id: 96 (i: 480000, errors: 0)
id: 96 (i: 490000, errors: 0)
0.5 billion iterations
id: 96 (i: 500000, errors: 0)
id: 96 (i: 510000, errors: 0)
id: 96 (i: 520000, errors: 0)
id: 96 (i: 530000, errors: 0)
id: 96 (i: 540000, errors: 0)
id: 96 (i: 550000, errors: 0)
id: 96 (i: 560000, errors: 0)
id: 96 (i: 570000, errors: 0)
id: 96 (i: 580000, errors: 0)
id: 96 (i: 590000, errors: 0)
0.6 billion iterations
id: 96 (i: 600000, errors: 0)
id: 96 (i: 610000, errors: 0)
id: 96 (i: 620000, errors: 0)
id: 96 (i: 630000, errors: 0)
id: 96 (i: 640000, errors: 0)
id: 96 (i: 650000, errors: 0)
id: 96 (i: 660000, errors: 0)
id: 96 (i: 670000, errors: 0)
id: 96 (i: 680000, errors: 0)
id: 96 (i: 690000, errors: 0)
0.7 billion iterations
id: 96 (i: 700000, errors: 0)
id: 96 (i: 710000, errors: 0)
id: 96 (i: 720000, errors: 0)
id: 96 (i: 730000, errors: 0)
id: 96 (i: 740000, errors: 0)
id: 96 (i: 750000, errors: 0)
id: 96 (i: 760000, errors: 0)
id: 96 (i: 770000, errors: 0)
id: 96 (i: 780000, errors: 0)
id: 96 (i: 790000, errors: 0)
0.8 billion iterations
id: 96 (i: 800000, errors: 0)
id: 96 (i: 810000, errors: 0)
id: 96 (i: 820000, errors: 0)
id: 96 (i: 830000, errors: 0)
id: 96 (i: 840000, errors: 0)
id: 96 (i: 850000, errors: 0)
id: 96 (i: 860000, errors: 0)
id: 96 (i: 870000, errors: 0)
id: 96 (i: 880000, errors: 0)
id: 96 (i: 890000, errors: 0)
0.9 billion iterations
id: 96 (i: 900000, errors: 0)
id: 96 (i: 910000, errors: 0)
id: 96 (i: 920000, errors: 0)
id: 96 (i: 930000, errors: 0)
id: 96 (i: 940000, errors: 0)
id: 96 (i: 950000, errors: 0)
id: 96 (i: 960000, errors: 0)
id: 96 (i: 970000, errors: 0)
id: 96 (i: 980000, errors: 0)
id: 96 (i: 990000, errors: 0)
1.0 billion iterations
id: 96 (i: 1000000, errors: 0)
id: 96 (i: 1010000, errors: 0)
id: 96 (i: 1020000, errors: 0)
id: 96 (i: 1030000, errors: 0)
id: 96 (i: 1040000, errors: 0)
id: 96 (i: 1050000, errors: 0)
id: 96 (i: 1060000, errors: 0)
id: 96 (i: 1070000, errors: 0)
id: 96 (i: 1080000, errors: 0)
1.1 billion iterations
id: 96 (i: 1090000, errors: 0)
id: 96 (i: 1100000, errors: 0)
id: 96 (i: 1110000, errors: 0)
...
```


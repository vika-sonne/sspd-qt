SSPD
====

[![GitHub release](https://img.shields.io/github/release/vika-sonne/sspd-qt.svg)](https://github.com/vika-sonne/sspd-qt/releases/latest)&nbsp;&nbsp;&nbsp;&nbsp;![](https://img.shields.io/badge/platform-linux--64%20|%20win--64-success)

**Simple serial port dump.**

Utility that helps capturing the log especially from serial ports (USART, e.t.c.) and includes features:
* precise timestamps;
* C-style escaped string for bytes dump;
* port autoreconnect;
* USB port search by VID:PID;
* support for Linux & Windows.

Developed as primary logs grabber tool for electronic device developers.
There is python utilities for the same purpose: [PyQt pqcom](https://github.com/vika-sonne/pqcom/) & [python sspd](https://github.com/vika-sonne/electronix-python/blob/master/README.md#SSPD).

## Download last release

![](https://img.shields.io/badge/-Linux-red)

[&nbsp;&nbsp;&nbsp;ELF for Qt5 (Kubuntu 18)&nbsp;&nbsp;&nbsp;](https://github.com/vika-sonne/sspd-qt/releases/latest/download/sspd)

![](https://img.shields.io/badge/-Windows-informational)

[&nbsp;&nbsp;&nbsp;Windows x64 standalone executable&nbsp;&nbsp;&nbsp;](https://github.com/vika-sonne/sspd-qt/releases/latest/download/sspd.exe)

## Usage:

```sh
Usage: ./sspd [options]

Simple serial port dump (Qt).
Dumps com port incoming data as C-style escaped string and optionally as HEX.

Options:
  -h, --help     Displays this help.
  -v, --version  Displays version information.
  -l             Lists available serial ports
  -t             Dump bytes instead of lines
  -x             Dump in HEX format
  -p <PORT>      Port name
  -b <BAUD>      Port baudrate
  -m <DPS>       Port parameters: Data bits 5..8; Parity NEOSM; Stop bits 1,
                 1.5, 2
  -u <VID:PID>   USB VID:PID hex list to search: VID:PID[,VID:PID[...]]
```

##### Usage examples:
* USB VID:PID search (Atmel μC):
```sh
sspd -u 03eb:2404,03eb:6124
```

* Baudrate & port (name for Linux):
```sh
sspd -b 250000 -p ttyACM0
```

* Port & write to log file and console simultaneously (for Linux):
```sh
sspd -p ttyACM0 | tee ~/1.log
```

* Port, log file name is current date_time (example: 2019-09-10_19-37-59.log) (for Linux):
```sh
sspd -p ttyACM0 > ~/$(date +"%Y-%d-%m_%H-%M-%S").log
```

* Lits of com ports allowable in the system:
```sh
sspd -l
ttyUSB1
        1a86:7523
        USB2.0-Serial
        1a86
ttyUSB0
        0403:6001
        FT232R USB UART
        FTDI
ttyUSB2
        1a86:7523
        USB2.0-Serial
        1a86
ttyS0
```

##### Log example with automatic reconnection:
Log of AM2320 Digital Temperature and Humidity Sensor:
```sh
sspd -u 1a86:7523 -b 2400
START 2019-11-01T23:17:45
23:17:47.982 <opened found ttyUSB0 USB 1a86:7523 @ 2400 8N1>
23:17:51.337 11 << RH=47.8   \n
23:17:51.850 12 << \rT=+11.0   \n
23:17:55.894 12 << \rRH=48.0   \n
23:17:56.407 12 << \rT=+11.0   \n
23:17:56.884 01 << \r
23:17:56.884 <closed ttyUSB0: (8) ...>
23:18:01.708 <opened found ttyUSB0 USB 1a86:7523 @ 2400 8N1>
23:18:05.053 11 << RH=47.7   \n
23:18:05.566 12 << \rT=+11.0   \n
23:18:09.610 12 << \rRH=47.9   \n
23:18:10.123 12 << \rT=+11.1   \n
23:18:10.773 01 << \r
23:18:10.773 <closed ttyUSB0: (8) ...>
```

### More complex usage

One of use case of *sspd* is debugging the serial connection protocols. Key part for researching and debugging that protocols is human readable log. So, the **small** step to do - write protocol parser. But large amount of different protocols and their modifications make difficult to do this and make needs of software tools. Such tool as Python programming environment is powerful for quick make a protocol parser.

#### Usage example of [ESP32 DFU](https://github.com/vika-sonne/electronix-python/blob/master/sspd_esp32_parser.py) protocol parser (ESP32 programming)
```sh
parallel -j2 -- './sspd -tu 10c4:ea60 -b 115200 | python3 sspd_esp32_parser.py' './sspd -tu 10c4:ea60 -b 115200 | python3 sspd_esp32_parser.py'
Skip 1 bytes: \x00
Skip 1 bytes: \x00
Skip 1 bytes: \x00
Skip 1 bytes: \x00
Skip 1 bytes: \x00
Skip 1 bytes: \x00
12:41:19.259 request SYNC
12:41:19.411 request SYNC
12:41:19.561 request SYNC
12:41:19.712 request SYNC
Skip 201 bytes: Started Serial0 and LEDanimation\r\n1\r\nStarted Serial0 and LED animation\r\nets Jun  8 2016 00:22:57\r\n\r\nrst:0x1 (POWERON_RESET),boot:0x1 (DOWNLOAD_BOOT(UART0/UART1/SDIO_FEI_REO_V2))\r\nwaiting for download\r\n
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.722 response SYNC
12:41:19.723 request READ_REG {'address': 1610719244}
12:41:19.726 response READ_REG 00000000
12:41:19.727 request READ_REG {'address': 1610719244}
12:41:19.729 response READ_REG 00000000
12:41:19.731 request READ_REG {'address': 1610719248}
12:41:19.733 response READ_REG 000000
12:41:19.734 request READ_REG {'address': 1610719256}
12:41:19.737 response READ_REG 00000000
12:41:19.738 request READ_REG {'address': 1610719240}
12:41:19.740 response READ_REG 00000000
12:41:19.742 request READ_REG {'address': 1610719236}
12:41:19.744 response READ_REG 00000000
12:41:19.745 request MEM_BEGIN {'size': 3140, 'blocks': 1, 'blocksize': 6144, 'offset': 1074393088}
12:41:19.748 response MEM_BEGIN 00000000
12:41:19.751 request MEM_DATA
12:41:20.035 response MEM_DATA 00000000
12:41:20.037 request MEM_BEGIN {'size': 4, 'blocks': 1, 'blocksize': 6144, 'offset': 1073736612}
12:41:20.040 response MEM_BEGIN 00000000
12:41:20.041 request MEM_DATA
12:41:20.045 response MEM_DATA 00000000
12:41:20.046 request MEM_END {'flag': 0, 'entry': 1074394468}
12:41:20.048 response MEM_END 00000000
12:41:20.051 response !UNKNOWN COMMAND!: OHAI
12:41:20.052 request READ_REG {'address': 1610620956}
12:41:20.055 response READ_REG 0000
12:41:20.056 request READ_REG {'address': 1610620964}
12:41:20.058 response READ_REG 0000
12:41:20.059 request WRITE_REG {'address': 1610620972, 'value': 23, 'mask': 4294967295, 'delay': 0}
12:41:20.062 response WRITE_REG 0000
12:41:20.063 request WRITE_REG {'address': 1610620956, 'value': 2415919104, 'mask': 4294967295, 'delay': 0}
12:41:20.066 response WRITE_REG 0000
12:41:20.067 request WRITE_REG {'address': 1610620964, 'value': 1879048351, 'mask': 4294967295, 'delay': 0}
12:41:20.070 response WRITE_REG 0000
12:41:20.071 request WRITE_REG {'address': 1610621056, 'value': 0, 'mask': 4294967295, 'delay': 0}
12:41:20.074 response WRITE_REG 0000
12:41:20.075 request WRITE_REG {'address': 1610620928, 'value': 262144, 'mask': 4294967295, 'delay': 0}
12:41:20.078 response WRITE_REG 0000
12:41:20.079 request READ_REG {'address': 1610620928}
12:41:20.081 response READ_REG 0000
12:41:20.083 request READ_REG {'address': 1610621056}
12:41:20.085 response READ_REG 0000
12:41:20.086 request WRITE_REG {'address': 1610620956, 'value': 2147483712, 'mask': 4294967295, 'delay': 0}
12:41:20.089 response WRITE_REG 0000
12:41:20.090 request WRITE_REG {'address': 1610620964, 'value': 1879048192, 'mask': 4294967295, 'delay': 0}
12:41:20.093 response WRITE_REG 0000
12:41:20.094 request SPI_SET_PARAMS {'id': 0, 'total_size': 4194304, 'block_size': 65536, 'sector_size': 4096, 'page_size': 256, 'status_mask': 65535}
12:41:20.098 response SPI_SET_PARAMS 0000
12:41:20.099 request FLASH_DEFL_BEGIN {'write_size': 8192, 'num_blocks': 1, 'pagesize': 16384, 'offset': 57344}
12:41:20.103 response FLASH_DEFL_BEGIN 0000
12:41:20.104 request FLASH_DEFL_DATA
12:41:20.111 response FLASH_DEFL_DATA 0000
12:41:20.113 request SPI_FLASH_MD5
12:41:20.213 response SPI_FLASH_MD5 e6327541e2dc394ca2c3b3280adbdcf39f0000
12:41:20.229 request FLASH_DEFL_BEGIN {'write_size': 15856, 'num_blocks': 1, 'pagesize': 16384, 'offset': 4096}
12:41:20.233 response FLASH_DEFL_BEGIN 0000
12:41:20.237 request FLASH_DEFL_DATA
12:41:21.141 response FLASH_DEFL_DATA 0000
12:41:21.145 request SPI_FLASH_MD5
12:41:21.380 response SPI_FLASH_MD5 66ba7fa3a11e53e48ecfacb2319920040000
12:41:21.433 request FLASH_DEFL_BEGIN {'write_size': 219120, 'num_blocks': 7, 'pagesize': 16384, 'offset': 65536}
12:41:21.436 response FLASH_DEFL_BEGIN 0000
12:41:21.440 request FLASH_DEFL_DATA
12:41:22.878 response FLASH_DEFL_DATA 0000
12:41:22.890 request FLASH_DEFL_DATA
12:41:24.331 response FLASH_DEFL_DATA 0000
12:41:24.343 request FLASH_DEFL_DATA
12:41:25.785 response FLASH_DEFL_DATA 0000
12:41:25.796 request FLASH_DEFL_DATA
12:41:27.236 response FLASH_DEFL_DATA 0000
12:41:27.247 request FLASH_DEFL_DATA
12:41:28.686 response FLASH_DEFL_DATA 0000
12:41:28.697 request FLASH_DEFL_DATA
12:41:30.137 response FLASH_DEFL_DATA 0000
12:41:30.148 request FLASH_DEFL_DATA
12:41:31.400 response FLASH_DEFL_DATA 0000
12:41:31.403 request SPI_FLASH_MD5
12:41:31.511 response SPI_FLASH_MD5 ddd709af6de8d5533c43f7229783c5840000
12:41:31.802 request FLASH_DEFL_BEGIN {'write_size': 3072, 'num_blocks': 1, 'pagesize': 16384, 'offset': 32768}
12:41:31.805 response FLASH_DEFL_BEGIN 0000
12:41:31.806 request FLASH_DEFL_DATA
12:41:31.821 response FLASH_DEFL_DATA 0000
12:41:31.822 request SPI_FLASH_MD5
12:41:31.870 response SPI_FLASH_MD5 b3a1040c8763614dc1f6386bd9d0f0be0000
12:41:31.876 request FLASH_BEGIN {'size': 0, 'blocks': 0, 'blocksize': 16384, 'offset': 0}
12:41:31.880 response FLASH_BEGIN 0000
12:41:31.881 request FLASH_DEFL_END
12:41:31.883 response FLASH_DEFL_END
```

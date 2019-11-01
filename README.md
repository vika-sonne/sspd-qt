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

##### Log example:
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

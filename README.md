# SSPD
**Simple serial port dump.**

Utility that helps capturing the log especially from serial ports (USART, e.t.c.) and includes features:
* precise timestamps;
* C-style escape for bytes dump;
* port autoreconnect;
* USB port search by VID:PID;
* support for Linux & Windows.

Developed as primary logs grabber tool for electronic device developers.
There is python utilities for the same purpose: [PyQt pqcom](https://github.com/vika-sonne/pqcom/) & [python sspd](https://github.com/vika-sonne/electronix-python/blob/master/README.md#SSPD).

##### Usage:
```sh
Usage: ./sspd [options]
Simple serial port dump (qt)

Options:
  -h, --help     Displays this help.
  -v, --version  Displays version information.
  -l             List available serial ports
  -t             Dump bytes instead of lines
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

##### Log example:
Log of AM2320 Digital Temperature and Humidity Sensor:
```sh
sspd -u 1a86:7523 -b 2400
START 2019-10-26T20:35:42
20:35:46.479 <found ttyUSB0 USB 1a86:7523 open @ 2400 8N1>
20:35:49.785 12 << RH=47.8   \n
20:35:50.298 14 << \rT=+23.1   \n
20:35:54.342 14 << \rRH=47.9   \n
20:35:54.855 14 << \rT=+23.1   \n
20:35:56.035 02 << \r
20:35:56.035 <closed ttyUSB0>
20:36:00.653 <found ttyUSB0 USB 1a86:7523 open @ 2400 8N1>
20:36:03.952 12 << RH=47.5   \n
20:36:04.465 14 << \rT=+23.1   \n
20:36:08.509 14 << \rRH=47.7   \n
20:36:09.022 14 << \rT=+23.1   \n
```

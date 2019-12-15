#include <iostream>
#include <math.h>
#include <QDateTime>
#include <QSerialPortInfo>
#include <QVector>
#include <QStringRef>
#include <QCommandLineParser>
#include "applicationclass.h"

static constexpr int COM_PORT_FIND_START_INTERVAL = 25; //!< Find inverval for low reconnect delay, ms
static constexpr int COM_PORT_FIND_START_ATTEMPTS = 15; //!< Attempts count for low reconnect delay
static constexpr int COM_PORT_FIND_INTERVAL = 330; //!< ms

static QString _toEscapedCString(QByteArray buff)
{
	QString ret;
	foreach(auto ch, buff) {
		switch(ch)
		{
			case 0: ret += "\\0"; break; // NULL
			case 7: ret += "\\a"; break; // Beep
			case 8: ret += "\\b"; break; // Backspace
			case 9: ret += "\\t"; break; // Horizontal tab
			case 0xa: ret += "\\n"; break; // Line feed
			case 0xb: ret += "\\v"; break; // Vertical tab
			case 0xc: ret += "\\f"; break; // Form feed
			case 0xd: ret += "\\r"; break; // Carriage return
			case 0x5c: ret += "\\\\"; break;
			default:
				if(ch < 0x20 || (uint8_t)ch > 0x7E)
				{
					// hex escape
					QByteArray a(&ch, 1);
					ret += QString("\\x") + a.toHex().toUpper();
				}
				else
					ret += ch;
			break;
		}
	}
	return ret;
}

namespace _log
{
	static inline void error(QString msg)
	{
		std::cout << qPrintable(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ")) << '<' << qPrintable(msg) << '>' << std::endl;
	}

	void msg(QString msg)
	{
		std::cout << qPrintable(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ")) << qPrintable(msg) << std::endl;
	}

	void data(QString data, uint len, bool printTimestamp=true, bool printLen=true)
	{
		std::cout << (printTimestamp ? qPrintable(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ")) : "             ")
			<< (printLen ? qPrintable(QString("%0 << ").arg(len, 2, 10, QLatin1Char('0'))) : "      ")
			<< qPrintable(data)
			<< std::endl;
	}
}

namespace _port
{
	inline QLatin1Char parity(QSerialPort *port)
	{
		switch(port->parity())
		{
			case QSerialPort::NoParity: return QLatin1Char('N');
			case QSerialPort::EvenParity: return QLatin1Char('E');
			case QSerialPort::OddParity: return QLatin1Char('O');
			case QSerialPort::SpaceParity: return QLatin1Char('S');
			case QSerialPort::MarkParity: return QLatin1Char('M');
			default: return QLatin1Char(' ');
		}
	}

	inline QString stopBits(QSerialPort *port)
	{
		switch(port->stopBits())
		{
			case QSerialPort::OneStop: return "1";
			case QSerialPort::OneAndHalfStop: return "1.5";
			case QSerialPort::TwoStop: return "2";
			default: return " ";
		}
	}

	inline QString parameters(QSerialPort *port)
	{
		auto ret = QString("%1 %2%3%4")
			.arg(port->baudRate())
			.arg(port->dataBits()).arg(parity(port)).arg(stopBits(port));
		return ret;
	}

	bool set_parameters(QString parameters,
		QSerialPort::DataBits *_dataBits,
		QSerialPort::Parity *_parity,
		QSerialPort::StopBits *_stopBits)
	{
		if(parameters.length() < 3)
			return false;

		// data bits
		switch(parameters[0].toLatin1())
		{
			case '5': *_dataBits = QSerialPort::Data5; break;
			case '6': *_dataBits = QSerialPort::Data6; break;
			case '7': *_dataBits = QSerialPort::Data7; break;
			case '8': *_dataBits = QSerialPort::Data8; break;
			default: return false;
		}

		// parity
		switch(parameters[1].toLatin1())
		{
			case 'N': *_parity = QSerialPort::NoParity; break;
			case 'E': *_parity = QSerialPort::EvenParity; break;
			case 'O': *_parity = QSerialPort::OddParity; break;
			case 'S': *_parity = QSerialPort::SpaceParity; break;
			case 'M': *_parity = QSerialPort::MarkParity; break;
			default: return false;
		}

		// stop bits
		switch(parameters[2].toLatin1())
		{
			case '2':
				if(parameters.length() != 3)
					return false;
				*_stopBits = QSerialPort::TwoStop;
				break;
			case '1':
				if(parameters.length() == 3)
					*_stopBits = QSerialPort::OneStop;
				else if(parameters.length() != 5 || parameters[3] != '.' || parameters[4] != '5')
					return false;
				else
					*_stopBits = QSerialPort::OneAndHalfStop;
				break;
			default: return false;
		}

		return true;
	}
}

ApplicationClass::ApplicationClass(int &argc, char **argv) :
	QCoreApplication(argc, argv)
{
	QCoreApplication::setApplicationName("sspd");
	QCoreApplication::setApplicationVersion("0.6");

	QCommandLineParser parser;
	parser.setApplicationDescription("\nSimple serial port dump (Qt)."
		"\nDumps com port incoming data as C-style escaped string and optionally as HEX.");
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption optionList("l", "Lists available serial ports");
	parser.addOption(optionList);

	QCommandLineOption optionBytes("t", "Dump bytes instead of lines");
	parser.addOption(optionBytes);

	QCommandLineOption optionHex("x", "Dump in HEX format");
	parser.addOption(optionHex);

	const QCommandLineOption optionPortName("p", "Port name", "PORT");
	parser.addOption(optionPortName);

	const QCommandLineOption optionPortBaud("b", "Port baudrate", "BAUD");
	parser.addOption(optionPortBaud);

	const QCommandLineOption optionPortParameters("m", "Port parameters: Data bits 5..8; Parity NEOSM; Stop bits 1, 1.5, 2", "DPS");
	parser.addOption(optionPortParameters);

	const QCommandLineOption optionVidpid("u", "USB VID:PID hex list to search: VID:PID[,VID:PID[...]]", "VID:PID");
	parser.addOption(optionVidpid);

	parser.process(*this);

	if(parser.isSet(optionList))
	{
		// list allowable serial port in the system
		for(const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts())
		{
			std::cout << qPrintable(serialPortInfo.portName()) << (serialPortInfo.isBusy() ? "\t[busy]" : "") << std::endl;
			if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
				std::cout << '\t' << qPrintable(QString("%0:%1").arg(serialPortInfo.vendorIdentifier(), 4, 16, QLatin1Char('0'))
					.arg(serialPortInfo.productIdentifier(), 4, 16, QLatin1Char('0'))) << std::endl;
			if(!serialPortInfo.description().isEmpty())
				std::cout << '\t' << qPrintable(serialPortInfo.description()) << std::endl;
			if(!serialPortInfo.manufacturer().isEmpty())
				std::cout << '\t' << qPrintable(serialPortInfo.manufacturer()) << std::endl;
		}
		isExit = true;
		return;
	}

	if(parser.isSet(optionPortName))
		_portName = parser.value(optionPortName);

	if(parser.isSet(optionPortBaud))
	{
		bool ok;
		_baud = parser.value(optionPortBaud).toInt(&ok);
		if(!ok)
		{
			std::cout << "Wrong baudrate: " << qPrintable(parser.value(optionPortBaud)) << std::endl;
			isArgsError = true;
			return;
		}
	}

	if(parser.isSet(optionPortParameters))
		if(!_port::set_parameters(parser.value(optionPortParameters),
			&_dataBits, &_parity, &_stopBits))
		{
			std::cout << "Wrong parameters: " << qPrintable(parser.value(optionPortParameters)) << std::endl;
			isArgsError = true;
			return;
		}

	if(parser.isSet(optionVidpid))
	{
		auto buff = parser.value(optionVidpid);
		foreach(auto vid_pid, buff.splitRef(','))
		{
			auto vid_and_pid = vid_pid.split(':');
			if(vid_and_pid.length() != 2)
			{
				std::cout << "Wrong VID:PID list: " << qPrintable(buff) << std::endl;
				isArgsError = true;
				return;
			}
			{
				int vid, pid;
				bool ok;
				vid = vid_and_pid[0].toInt(&ok, 16);
				pid = vid_and_pid[1].toInt(&ok, 16);
				_vidPid.append(VidPid(vid, pid));
			}
		}
	}

	if(parser.isSet(optionBytes))
		_mode = Mode::BYTES;

	if(parser.isSet(optionHex))
		_hexFormat = true;

	if(_portName.isEmpty() && _vidPid.isEmpty())
	{
		std::cout << "One of this options must be specified: "
			<< qPrintable(optionPortName.names()[0])
			<< ", " << qPrintable(optionVidpid.names()[0]) << "." << std::endl;
		isArgsError = true;
		return;
	}

	std::cout << "START " << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << std::endl;

	// start reconnect timer // start to search port in the system by timer
	_reconnectTimerId = startTimer(0);
}

void ApplicationClass::processReconnectTimer()
{
	// com port not found
	switch(_reconnectAttemptsCount)
	{
		case 0:
			// start reconnect timer with high rate
			_reconnectAttemptsCount = 1;
			killTimer(_reconnectTimerId);
			_reconnectTimerId = startTimer(COM_PORT_FIND_START_ATTEMPTS);
			break;
		case COM_PORT_FIND_START_ATTEMPTS:
			// start reconnect timer with normal rate
			_reconnectAttemptsCount++;
			killTimer(_reconnectTimerId);
			_reconnectTimerId = startTimer(COM_PORT_FIND_INTERVAL);
		case COM_PORT_FIND_START_ATTEMPTS + 1:
			// stop attempts counting
			break;
		default:
			// count: 1..COM_PORT_FIND_START_ATTEMPTS-1
			_reconnectAttemptsCount++;
			break;
	}
}

void ApplicationClass::timerEvent(QTimerEvent *event)
{
	Q_UNUSED(event)
	auto portAndVidPid = tryFindComPort();
	if(!portAndVidPid.first.isEmpty())
	{
		// com port found
		if(openSerialPort(portAndVidPid))
		{
			// stop reconnect timer
			killTimer(_reconnectTimerId);
			_reconnectTimerId = -1;
			_reconnectAttemptsCount = 0;
		}
		else
			processReconnectTimer();
	}
	else
		processReconnectTimer();
}

ApplicationClass::PortAndVidPid ApplicationClass::tryFindComPort()
{
	if(_vidPid.isEmpty())
		// not need to search com port since it specified
		return PortAndVidPid(_portName, _portName);

	// try to search com port according to VID:PID list
	foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		if(!info.isBusy() && info.hasVendorIdentifier() && info.hasProductIdentifier())
			foreach(VidPid vidPid, _vidPid)
			{
				if(info.vendorIdentifier() == vidPid.first
					&& info.productIdentifier() == vidPid.second)
					return PortAndVidPid(info.portName(),
						QString("found %0 USB %1:%2").arg(info.portName())
							.arg(info.vendorIdentifier(), 2, 16, QLatin1Char('0'))
							.arg(info.productIdentifier(), 2, 16, QLatin1Char('0'))
						);
			}
	}

	// port with USB VID:PID not found
	return PortAndVidPid(QString(), QString());
}

bool ApplicationClass::openSerialPort(ApplicationClass::PortAndVidPid portAndVidPid)
{
	// close serial port

	if(_serialPort != nullptr)
		_serialPort->close();

	// flush line
	_line.clear();

	// open serial port
	_serialPort = new QSerialPort(portAndVidPid.first, this);
	// setup serial port with parameters
	_serialPort->setBaudRate(_baud);
	_serialPort->setDataBits(_dataBits);
	_serialPort->setParity(_parity);
	_serialPort->setStopBits(_stopBits);
	// open serial port
	if(!_serialPort->open(QIODevice::ReadWrite))
	{
		if(_serialPort->error() != _lastError)
		{
			_log::error(QString("error  %0: (%1) %2").arg(portAndVidPid.first)
				.arg(_serialPort->error()).arg(_serialPort->errorString()));
			_lastError = _serialPort->error();
			if(_serialPort->isOpen())
				_serialPort->close();
			_serialPort = nullptr;
		}
		return false;
	}
	else
	{
		_log::error(QString("opened %0 @ %1").arg(portAndVidPid.second).arg(_port::parameters(_serialPort)));
	}

	// connect serial port signals
	connect(_serialPort, SIGNAL(readyRead()), this, SLOT(_serialPort_readyRead()));
	connect(_serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
		this, SLOT(_serialPort_errorOccurred(QSerialPort::SerialPortError)));

	return true;
}

void ApplicationClass::closeSerialPort()
{
	auto portName = _serialPort->portName();
	// flush line
	if(_line.length() > 0)
	{
		logData(_line);
		_line.clear();
	}
	// filter
	_lastError = _serialPort->error();
	_hasLastData = false;
	// log
	if(_serialPort->error() == QSerialPort::NoError)
		_log::error(QString("closed ") + qPrintable(portName));
	else
		_log::error(QString("closed %0: (%1) %2").arg(portName)
			.arg(_serialPort->error()).arg(_serialPort->errorString()));
	// close port
	if(_serialPort->isOpen())
		_serialPort->close();
	delete _serialPort;
	_serialPort = nullptr;
	emit serialPortClosed(portName);
	// start reconnect timer // start to search port in the system by timer
	if(_reconnectTimerId >= 0)
		killTimer(_reconnectTimerId);
	_reconnectAttemptsCount = 0;
	_reconnectTimerId = startTimer(0);
}

void ApplicationClass::_serialPort_readyRead()
{
	auto buff = _serialPort->readAll();
	if(buff.length() == 0)
	{
		if(_serialPort->error() != QSerialPort::NoError)
			closeSerialPort();
	}
	else
	{
		_hasLastData = true;
		if(_mode == Mode::BYTES)
			logData(buff);
		else
		{
			auto i = buff.indexOf('\n');
			if(i >= 0)
			{
				logData(_line + buff.left(i + 1));
				_line.clear();
				auto next_i = buff.indexOf('\n', i + 1);
				while(next_i >= 0)
				{
					logData(buff.mid(i + 1, next_i - i));
					i = next_i;
					next_i = buff.indexOf('\n', next_i + 1);
				}
			}
			_line += buff.mid(i + 1);
		}
	}
}

void ApplicationClass::_serialPort_errorOccurred(QSerialPort::SerialPortError error)
{
	if(error != QSerialPort::NoError)
		closeSerialPort();
}

void ApplicationClass::logData(QByteArray data)
{
	_log::data(_toEscapedCString(data), data.length());
	if(_hexFormat)
		_log::data(data.toHex(), 0, false, false);
}

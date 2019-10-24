#include <iostream>
#include <math.h>
#include <QDateTime>
#include <QSerialPortInfo>
#include <QCommandLineParser>
#include <QVector>
#include <QStringRef>
#include "applicationclass.h"

static constexpr int COM_PORT_FIND_START_INTERVAL = 25; //!< Find inverval for low reconnect delay, ms
static constexpr int COM_PORT_FIND_START_ATTEMPTS = 15; //!< Attempts count for low reconnect delay
static constexpr int COM_PORT_FIND_INTERVAL = 330; //!< ms

static QString _escapeToCpp(QByteArray buff)
{
	QString ret;
	foreach (auto ch, buff) {
		switch(ch)
		{
			case 0: ret += "\\0"; break;
			case 7: ret += "\\a"; break;
			case 8: ret += "\\b"; break;
			case 9: ret += "\\t"; break;
			case 0xa: ret += "\\n"; break;
			case 0xb: ret += "\\v"; break;
			case 0xc: ret += "\\f"; break;
			case 0xd: ret += "\\r"; break;
			default:
				if(ch < 0x20 || ch > 0xE7)
				{
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

	void data(QString data)
	{
		std::cout << qPrintable(QDateTime::currentDateTime().toString("hh:mm:ss.zzz "))
			<< qPrintable(QString("%0 << ").arg(data.length(), 2, 10, QLatin1Char('0')))
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
	QCoreApplication::setApplicationVersion("0.2");

	QCommandLineParser parser;
	parser.setApplicationDescription("Simple serial port dump (qt)");
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption listOption("l", "List available serial ports");
	parser.addOption(listOption);

	QCommandLineOption bytesOption("t", "Dump bytes instead of lines");
	parser.addOption(bytesOption);

	const QCommandLineOption portNameOption("p", "Port name", "PORT");
	parser.addOption(portNameOption);

	const QCommandLineOption portBaudOption("b", "Port baudrate", "BAUD");
	parser.addOption(portBaudOption);

	const QCommandLineOption portParametersOption("m", "Port parameters: Data bits 5..8; Parity NEOSM; Stop bits 1, 1.5, 2", "DPS");
	parser.addOption(portParametersOption);

	const QCommandLineOption portVidpidOption("u", "USB VID:PID hex list to search: VID:PID[,VID:PID[...]]", "VID:PID");
	parser.addOption(portVidpidOption);

	parser.process(*this);

	if(parser.isSet(listOption))
	{
		for(const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts())
		{
			std::cout << qPrintable(serialPortInfo.portName()) << std::endl;
			if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
				std::cout << '\t' << qPrintable(QString("%0:%1").arg(serialPortInfo.vendorIdentifier(), 4, 16, QLatin1Char('0'))
					.arg(serialPortInfo.productIdentifier(), 4, 16, QLatin1Char('0'))) << std::endl;
			std::cout << '\t' << qPrintable(serialPortInfo.description()) << std::endl;
			std::cout << '\t' << qPrintable(serialPortInfo.manufacturer()) << std::endl;
		}
		isExit = true;
		return;
	}

	if(parser.isSet(portNameOption))
		_portName = parser.value(portNameOption);

	if(parser.isSet(portBaudOption))
	{
		bool ok;
		_baud = parser.value(portBaudOption).toInt(&ok);
		if(!ok)
		{
			std::cout << "Wrong baudrate: " << qPrintable(parser.value(portBaudOption)) << std::endl;
			isArgsError = true;
			return;
		}
	}

	if(parser.isSet(portParametersOption))
		if(!_port::set_parameters(parser.value(portParametersOption),
			&_dataBits, &_parity, &_stopBits))
		{
			std::cout << "Wrong parameters: " << qPrintable(parser.value(portParametersOption)) << std::endl;
			isArgsError = true;
			return;
		}

	if(parser.isSet(portVidpidOption))
	{
		auto buff = parser.value(portVidpidOption);
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

	if(parser.isSet(bytesOption))
		_mode = Mode::BYTES;

	std::cout << "START " << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << std::endl;

	_findTimerId = startTimer(COM_PORT_FIND_INTERVAL);
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
			killTimer(_findTimerId);
			_findTimerId = -1;
			_findAttemptsCount = 0;
		}
	}
	else
	{
		// com port not found
		if(_findAttemptsCount < COM_PORT_FIND_START_ATTEMPTS)
			_findAttemptsCount++;
		else if(_findAttemptsCount == COM_PORT_FIND_START_ATTEMPTS)
		{
			killTimer(_findTimerId);
			_findTimerId = startTimer(COM_PORT_FIND_INTERVAL);
		}
	}
}

ApplicationClass::PortAndVidPid ApplicationClass::tryFindComPort()
{
	foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		if(info.hasVendorIdentifier() && info.hasProductIdentifier())
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
	_log::error(QString("%0 open @ %1").arg(portAndVidPid.second).arg(_port::parameters(_serialPort)));
	if(!_serialPort->open(QIODevice::ReadWrite))
	{
		_log::error(QString("error %0. %1").arg(portAndVidPid.first).arg(_serialPort->errorString()));
		return false;
	}

	// connect serial port signals
	connect(_serialPort, SIGNAL(readyRead()), this, SLOT(_serialPort_readyRead()));
	connect(_serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(_serialPort_errorOccurred(QSerialPort::SerialPortError)));

	return true;
}

void ApplicationClass::closeSerialPort()
{
	auto portName = _serialPort->portName();
	// flush line
	if(_line.length() > 0)
		_log::data(_escapeToCpp(_line));
	// log
	_log::error(QString("closed ") + qPrintable(portName));
	// close port
	if(_serialPort->isOpen())
		_serialPort->close();
	delete _serialPort;
	_serialPort = nullptr;
	emit serialPortClosed(portName);
	// starts to search port in the system by timer
	if(_findTimerId >= 0)
		killTimer(_findTimerId);
	_findAttemptsCount = 0;
	_findTimerId = startTimer(COM_PORT_FIND_START_INTERVAL);
}

void ApplicationClass::_serialPort_readyRead()
{
	auto buff = _serialPort->readAll();
	if(buff.length() == 0)
		closeSerialPort();
	else
	{
		if(_mode == Mode::BYTES)
			_log::data(_escapeToCpp(buff));
		else
		{
			auto i = buff.indexOf('\n');
			if(i >= 0)
			{
				_log::data(_escapeToCpp(_line + buff.left(i + 1)));
				_line.clear();
				auto next_i = buff.indexOf('\n', i + 1);
				while(next_i >= 0)
				{
					_log::data(_escapeToCpp(buff.mid(i + 1, next_i - i)));
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

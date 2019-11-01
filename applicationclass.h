#ifndef APPLICATIONCLASS_H
#define APPLICATIONCLASS_H

#include <QByteArray>
#include <QVector>
#include <QCoreApplication>
#include <QSerialPort>

class ApplicationClass : public QCoreApplication
{
	Q_OBJECT
public:
	typedef std::pair<int, int> VidPid;
	typedef std::pair<QString, QString> PortAndVidPid;
	enum class Mode { LINES, BYTES };
	explicit ApplicationClass(int &argc, char **argv);
	bool isArgsError = false;
	bool isExit = false;

public slots:
	//! Opens com port
	//! @param portName example: ttyACM0
	bool openSerialPort(PortAndVidPid portAndVidPid);
	void closeSerialPort();

signals:
	void serialPortClosed(QString portName);

protected slots:
	void _serialPort_readyRead();
	void _serialPort_errorOccurred(QSerialPort::SerialPortError error);

protected:
	static constexpr int DEFAULT_BAUD = 115200;
	static constexpr QSerialPort::DataBits DEFAULT_DATA_BITS = QSerialPort::Data8;
	static constexpr QSerialPort::Parity DEFAULT_PARITY = QSerialPort::NoParity;
	static constexpr QSerialPort::StopBits DEFAULT_STOP_BITS = QSerialPort::OneStop;

	unsigned int _baud = DEFAULT_BAUD;
	QSerialPort::DataBits _dataBits = DEFAULT_DATA_BITS;
	QSerialPort::Parity _parity = DEFAULT_PARITY;
	QSerialPort::StopBits _stopBits = DEFAULT_STOP_BITS;

	QSerialPort *_serialPort = nullptr;	//!< Serial COM port

	QString _portName; //!< Port name to open
	QVector<VidPid> _vidPid; //!< USB VID:PID list to search

	Mode _mode = Mode::LINES;
	QByteArray _line;

	bool _hexFormat = false;

	int _reconnectTimerId = -1; //!< Com port find interval timer id: -1 - no timer; 0.. timer id
	int _reconnectAttemptsCount = 0; //!< Com port find attempts count: 0..
	QSerialPort::SerialPortError _lastError = QSerialPort::NoError; //! Error filter
	bool _hasLastData = false; //! Open com port filter

	void timerEvent(QTimerEvent *event);

	void processReconnectTimer();

	//! Tries to find not busy com port in the system
	//! @return Com port path (if found) or empty string
	PortAndVidPid tryFindComPort();

	void logData(QByteArray data);
};

#endif // APPLICATIONCLASS_H

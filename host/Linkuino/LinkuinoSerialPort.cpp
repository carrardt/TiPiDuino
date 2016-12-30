#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <string>

#include "LinkuinoSerialPort.h"
#include "Linkuino/Linkuino.h"
using Linkuino = LinkuinoT<>;

static const char* g_serial_devices[] = {
#ifdef _WIN32
  "COM1"
, "COM2"
, "COM3"
, "COM4"
, "COM5"
, "COM6"
, "COM7"
, "COM8"
, "COM9"
#else
  "/dev/ttyAMA0"
, "/dev/ttyUSB0"
, "/dev/ttyACM0"
, "/dev/ttyAMA1"
, "/dev/ttyUSB1"
, "/dev/ttyACM1"
, "/dev/ttyUSB2"
, "/dev/ttyUSB3"
#endif
, nullptr
};

const char * * LinkuinoSerialPort::availablePorts()
{
	return g_serial_devices;
}

LinkuinoSerialPort::LinkuinoSerialPort() {}
LinkuinoSerialPort::~LinkuinoSerialPort()
{
	close();
}

bool LinkuinoSerialPort::open(const std::string& devpath)
{
#ifdef _WIN32
	char comPortName[256];
	sprintf_s<256>(comPortName, "%s", devpath.c_str());
	//printf("COM port: %s\n", comPortName);
	m_serialPort = CreateFile(comPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
	if (m_serialPort == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		//printf("\nHandle creation error code: %x\n", err);
		return false;
	}
	DCB dcbSerialParams = { 0 };
	if (!GetCommState(m_serialPort, &dcbSerialParams)) {
		printf("\nError: \ncould not get %s state!\n", comPortName);
		return false;
	}
	dcbSerialParams.fParity = 0;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.BaudRate = Linkuino::SERIAL_SPEED;
	dcbSerialParams.StopBits = 0; // 1 stop bit
	dcbSerialParams.Parity = 0;
	dcbSerialParams.fBinary = TRUE;
	dcbSerialParams.fOutxCtsFlow = FALSE;
	dcbSerialParams.fOutxDsrFlow = FALSE;
	dcbSerialParams.fDtrControl = FALSE;
	dcbSerialParams.fDsrSensitivity = FALSE;
	dcbSerialParams.fOutX = FALSE;
	dcbSerialParams.fInX = FALSE;
	if (!SetCommState(m_serialPort, &dcbSerialParams))
	{
		printf("\nError: \ncould not set %s state!\n", comPortName);
		return false;
	}
	dcbSerialParams.DCBlength = 0;
	if (!GetCommState(m_serialPort, &dcbSerialParams)) {
		printf("\nError: \ncould not get %s state!\n", comPortName);
		return false;
	}
	printf("%s State: baud=%d, CS=%d, Parity=%d, StopBits=%g\n", comPortName, dcbSerialParams.BaudRate, dcbSerialParams.ByteSize, dcbSerialParams.Parity, dcbSerialParams.StopBits*0.5f + 1.0f);
	COMMTIMEOUTS timeOut = { 1, 1, 1, 0, 0 };
	if( ! SetCommTimeouts(m_serialPort, &timeOut) )
	{
		printf("\nError: \ncould not configure %s timer-out !\n", comPortName);
		return false;
	}
#else
	m_fd = ::open(devpath.c_str(), O_RDWR | O_SYNC | O_NONBLOCK);
	if (m_fd < 0) { return false; }
	struct termios serialConfig;
	tcgetattr(m_fd, &serialConfig);
	cfmakeraw(&serialConfig);
	cfsetspeed(&serialConfig, Linkuino::SERIAL_SPEED);
	tcsetattr(m_fd, TCSANOW, &serialConfig);
#endif
	return true;
}

void LinkuinoSerialPort::close()
{
#ifdef _WIN32
	if (m_serialPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_serialPort);
		m_serialPort = INVALID_HANDLE_VALUE;
	}
#else
	if( m_fd >= 0 )
	{
		::close(m_fd);
		m_fd = -1;
	}
#endif
}

bool LinkuinoSerialPort::write(const uint8_t* buffer, int nBytes)
{
#ifdef _WIN32
	DWORD bytesWritten = 0;
	WriteFile(m_serialPort, buffer, nBytes, &bytesWritten, NULL);
	//printf("write %d/%d bytes\n", bytesWritten, nBytes);
	return bytesWritten == nBytes;
#else
	return ::write(m_fd, buffer, nBytes) == nBytes;
#endif
}

int LinkuinoSerialPort::read(uint8_t* buffer, int nBytes)
{
#ifdef _WIN32
	DWORD bytesRead = 0;
	ReadFile(m_serialPort, buffer, nBytes, &bytesRead, NULL);
	//printf("read %d/%d bytes\n", bytesRead, nBytes);
	return bytesRead;
#else
	return ::read(m_fd, buffer, nBytes);
#endif
}

int LinkuinoSerialPort::readSync(uint8_t* buffer, int nBytes)
{
	int nRemain = nBytes;
	while( nRemain > 0 )
	{
		int n = this->read(buffer,nRemain);

		if( n > 0 )
		{
			buffer += n;
			nRemain -= n;
		}
	}
	return nBytes-nRemain;
}

void LinkuinoSerialPort::flush()
{
#ifndef _WIN32
	fsync(m_fd);
#endif
}


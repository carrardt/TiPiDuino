#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>

#include "LinkuinoSerialPort.h"
#include "Linkuino/Linkuino.h"
using Linkuino = LinkuinoT<>;

static const char* g_serial_devices[] = {
  "/dev/ttyAMA0"
, "/dev/ttyUSB0"
, "/dev/ttyACM0"
, "/dev/ttyAMA1"
, "/dev/ttyUSB1"
, "/dev/ttyACM1"
, "/dev/ttyUSB2"
, "/dev/ttyUSB3"
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
	m_fd = ::open(devpath.c_str(), O_RDWR | O_SYNC | O_NONBLOCK);
	if (m_fd < 0) { return false; }
	struct termios serialConfig;
	tcgetattr(m_fd, &serialConfig);
	cfmakeraw(&serialConfig);
	cfsetspeed(&serialConfig, Linkuino::SERIAL_SPEED);
	tcsetattr(m_fd, TCSANOW, &serialConfig);
	return true;
}

void LinkuinoSerialPort::close()
{
	if( m_fd >= 0 )
	{
		::close(m_fd);
		m_fd = -1;
	}
}

bool LinkuinoSerialPort::write(const uint8_t* buffer, int nBytes)
{
	return ::write(m_fd, buffer, nBytes) == nBytes;
}

int LinkuinoSerialPort::read(uint8_t* buffer, int nBytes)
{
	return ::read(m_fd, buffer, nBytes);
}

void LinkuinoSerialPort::flush()
{
	fsync(m_fd);
}


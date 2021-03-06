#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <cstdint>
#include <mutex>

class LinkuinoSerialPort
{
public:
	LinkuinoSerialPort();
	~LinkuinoSerialPort();
	
	static const char * * availablePorts();
	
	bool open(const std::string& devpath);
	void close();
	bool write(const uint8_t* buffer, int nBytes);
	int read(uint8_t* buffer, int nBytes);
	int readSync(uint8_t* buffer, int nBytes, uint64_t timeOut=0 );

	void flush();

private:
#ifdef _WIN32
	HANDLE m_serialPort = INVALID_HANDLE_VALUE;
#else
	int m_fd = -1;
#endif
};

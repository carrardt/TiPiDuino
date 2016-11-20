#pragma once

#include <string>
#include <cstdint>

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
	void flush();

private:
	int m_fd = -1;
};

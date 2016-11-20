#ifndef __TiDuino_Linkuinoclient_h
#define __TiDuino_Linkuinoclient_h

#include "LinkuinoSerialPort.h"
#include "Linkuino/Linkuino.h"
using Linkuino = LinkuinoT<>;

#include <cstdint>

class LinkuinoClient
{
  public:

	LinkuinoClient(LinkuinoSerialPort* port);
	~LinkuinoClient();

	bool scanSerialPorts();
	bool testConnection();

	int getServerVersionMajor() const;
	int getServerVersionMinor() const;
	int getMessageRepeats() const;
	void setRegisterValue(uint8_t i, uint8_t value);
	
	//! value is in the range [0;1]. for servos, use values in the range [0.05;0.1]
	void setPWMValue(int p, float value);
	void enablePWM(int p);
	void disablePWM(int p);
	bool pwmEnabled(int p) const;

	void setDigitalOutput(uint8_t mask);

	//! Note: auto send on invocation
	void forwardMessage( uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3 );

	void resetTo50Hz();
	void resetTo100Hz();

	void quiet();

	void printStatus();
	void printBuffer();
	
	void send();

  private:
	int64_t timeSinceLastSend();
	void updateTimeStamp();
	void updateSendTime();

	struct timespec m_sendTime;
	LinkuinoSerialPort* m_serial;
	int m_serverMajor;
	int m_serverMinor;
	int m_messageRepeats;
	uint8_t m_buffer[Linkuino::CMD_COUNT];
	uint8_t m_timeStamp;
	uint8_t m_pwmEnable;
};

#endif

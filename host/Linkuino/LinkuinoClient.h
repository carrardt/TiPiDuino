#ifndef __TiDuino_Linkuinoclient_h
#define __TiDuino_Linkuinoclient_h

#include "LinkuinoSerialPort.h"
#include "Linkuino/Linkuino.h"
using Linkuino = LinkuinoT<>;

#include <cstdint>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

struct LinkuinoClient
{
  public:

	LinkuinoClient(LinkuinoSerialPort* port);
	~LinkuinoClient();

	int scanSerialPorts();
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

	float requestAnalogRead(uint8_t channel, int samples=1);
	int32_t requestDigitalRead();

	//! Note: auto send on invocation
	void forwardMessage( uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3 );

	void resetTo50Hz();
	void resetTo100Hz();

	void quiet();

	void printStatus();
	void printBuffer();

	void send();

	inline LinkuinoSerialPort* serialPort() { return m_serial;  }

	inline void forceMessageRepeats(int mr) { m_messageRepeats=mr; }

  private:
	static constexpr std::chrono::duration<int64_t, std::milli> c_minTimeBetweenSend = std::chrono::duration<int64_t, std::milli>(20);
	static constexpr std::chrono::duration<int64_t, std::milli> c_connectionTimeOut = std::chrono::duration<int64_t, std::milli>(1000);

	void updateTimeStamp();

	void waitClearToSend();
	void flushInput();
	void sendReplyRequest(uint8_t req, uint8_t d0=0, uint8_t d1=0, uint8_t d2=0, uint8_t d3=0);
	void stopServerReply();
	void pushDataToDevice(const uint8_t buffer[Linkuino::CMD_COUNT]);
	void asyncPushDataToDevice();

	// worker thread management
	void startWorkerThread();
	static void runWorkerThread(LinkuinoClient* self);
	void workerThreadMain();

	std::thread* m_workerThread = nullptr;
	std::mutex m_mutex;
	std::atomic<bool> m_terminate;
	
	std::condition_variable m_readyToSendCond;
	std::atomic<bool> m_readyToSend;
	std::chrono::high_resolution_clock::time_point m_sendTime;

	std::condition_variable m_clearToSendCond;
	std::atomic<bool> m_clearToSend;

	LinkuinoSerialPort* m_serial;
	int m_serverMajor;
	int m_serverMinor;
	int m_messageRepeats;
	uint8_t m_timeStamp;
	uint8_t m_pwmEnable;
	uint8_t m_buffer[Linkuino::CMD_COUNT];
	volatile uint8_t m_bufferCopy[Linkuino::CMD_COUNT];
};

#endif

#include "LinkuinoClient.h"
#include "LinkuinoSerialPort.h"

#include <iostream>
#include <cassert>
#include <cstring>

#ifndef _WIN32
#include <time.h>
#endif

LinkuinoClient::LinkuinoClient(LinkuinoSerialPort* port)
		: m_serial(port)
		, m_timeStamp(0)
		, m_serverMajor(0)
		, m_serverMinor(0)
		, m_messageRepeats(24)
		, m_terminate(false)
		, m_readyToSend(false)
		, m_clearToSend(true)
{
	setRegisterValue(Linkuino::TSTMP_ADDR, 0);
	for (int i = 0; i<Linkuino::PWM_COUNT; i++) { setPWMValue(i, 0.1f); }
	setRegisterValue(Linkuino::PWMEN_ADDR, 0);
	setRegisterValue(Linkuino::DOUT_ADDR, 0);
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOREPLY);
	setRegisterValue(Linkuino::REQ_DATA0_ADDR, 0);
	setRegisterValue(Linkuino::REQ_DATA1_ADDR, 0);
	setRegisterValue(Linkuino::REQ_DATA2_ADDR, 0);
	setRegisterValue(Linkuino::REQ_DATA3_ADDR, 0);
	updateSendTime();
}

LinkuinoClient::~LinkuinoClient()
{
	m_terminate = true; // ok not to use std::lock_guard<std::mutex> because m_terminate is ans std::atomic
	m_readyToSendCond.notify_one();
	m_workerThread->join();
	delete m_workerThread;
	m_workerThread = nullptr;
}

int LinkuinoClient::getServerVersionMajor() const
{ 
	return m_serverMajor;
}

int LinkuinoClient::getServerVersionMinor() const 
{ 
	return m_serverMinor;
}

int LinkuinoClient::getMessageRepeats() const
{ 
	return m_messageRepeats;
}

void LinkuinoClient::setRegisterValue(uint8_t i, uint8_t value)
{
	if (i>Linkuino::CMD_COUNT) return;

	if (i == 0) { m_buffer[i] = value & 0x3F; }
	else
	{
		uint8_t clk = ((i - 1) % 3) + 1; // 1, 2 or 3
		m_buffer[i] = (clk << 6) | (value & 0x3F);
	}
}

void LinkuinoClient::enablePWM(int p)
{
	m_pwmEnable |= 1 << p;
	m_pwmEnable &= 0x3F;
}

void LinkuinoClient::disablePWM(int p)
{
	m_pwmEnable &= ~(1 << p);
	m_pwmEnable &= 0x3F;
}

bool LinkuinoClient::pwmEnabled(int p) const
{
	return (m_pwmEnable & (1 << p)) != 0;
}

//! value is in the range 0-10000
void LinkuinoClient::setPWMValue(int p, float fval)
{
	uint16_t value = 0;
	if( fval <= 0.0f ) value = 0;
	else if( fval >= 1.0f ) value = 10000;
	else value = static_cast<uint16_t>( fval * 10000.0f );
	
	// PWM cycle are 10mS or 20mS long. all values are treated as if the cycle is 10mS long, and everything is doubled internally for 20mS cycles.
	// pulse length can be 0 (always LOW), 10mS (always HIGH) or in the range [400;9508]
	// a value below 400 meads to a constant low state
	// a value between 9509 and 9613
	// encoding : values in range[0;1536] encode pulse lengths in [400;1936]
	// values in range [1536;4095] encode pulse lengths in [1936;9613]
	
	if (p<0 || p>5) return;
	if( value < 400 ) { value=400; disablePWM(p); }
	else { enablePWM(p); }
	if( value > 9508 ) { value=9600; }
	
	//  encode to 12bits
	uint16_t valueEnc = Linkuino::encodePulseLength(value);
	
	// write corresponding values to state buffer
	setRegisterValue(Linkuino::PWMEN_ADDR, m_pwmEnable);
	setRegisterValue(Linkuino::PWM0H_ADDR + 2 * p, (valueEnc >> 6) & 0x3F);
	setRegisterValue(Linkuino::PWM0L_ADDR + 2 * p, valueEnc & 0x3F);
}

void LinkuinoClient::setDigitalOutput(uint8_t mask)
{
	setRegisterValue(Linkuino::DOUT_ADDR, mask & 0x3F);
}

// wait worker thread to be ready to send
void LinkuinoClient::waitClearToSend()
{
	std::unique_lock<std::mutex> lk(m_mutex);
	m_clearToSendCond.wait( lk, [this](){ return m_clearToSend.load(); } );
}

void LinkuinoClient::flushInput()
{
	//auto T1 = std::chrono::high_resolution_clock::now();
	uint8_t tmp;
	int n = 0;
	do {
		n = m_serial->readSync( &tmp, 1, 1000 );
	} while( n==1 );
	//auto T2 = std::chrono::high_resolution_clock::now();
	//std::cout << "flushInput time = " << std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1).count() << "uS\n";
}

void LinkuinoClient::stopServerReply()
{
	waitClearToSend();
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOREPLY );
	setRegisterValue(Linkuino::REQ_DATA0_ADDR, 0x00 );
	setRegisterValue(Linkuino::REQ_DATA1_ADDR, 0x00 );
	setRegisterValue(Linkuino::REQ_DATA2_ADDR, 0x00 );
	setRegisterValue(Linkuino::REQ_DATA3_ADDR, 0x00 );
	send();
	waitClearToSend();
	flushInput();
}

void LinkuinoClient::sendReplyRequest( uint8_t req, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3 )
{
	//flushInput();
	//auto T1 = std::chrono::high_resolution_clock::now();
	setRegisterValue(Linkuino::REQ_ADDR, req );
	setRegisterValue(Linkuino::REQ_DATA0_ADDR, d0 );
	setRegisterValue(Linkuino::REQ_DATA1_ADDR, d1 );
	setRegisterValue(Linkuino::REQ_DATA2_ADDR, d2 );
	setRegisterValue(Linkuino::REQ_DATA3_ADDR, d3 );
	send();
	//auto T2 = std::chrono::high_resolution_clock::now();
	//std::cout << "Req #"<<req<<" send time = " << std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1).count() << "uS\n";
}

float LinkuinoClient::requestAnalogRead(uint8_t channel, int nSamples)
{
	sendReplyRequest( Linkuino::REQ_ANALOG_READ , channel & 0x3F );

	bool ok = true;
	uint32_t value = 0;
	
	for( int i=0;i<nSamples && ok;i++)
	{
		uint8_t tmp[3]={Linkuino::REPLY_NULL,0,0};
		int n = m_serial->readSync( tmp, 3, 200000); // 200ms max read time
		if( n==3 && tmp[0]==Linkuino::REPLY_ANALOG_READ && (tmp[1]&0xFC)==0 )
		{
			uint32_t sample = tmp[1];
			sample <<= 8;
			sample |= tmp[2];
			value += sample;
		}
		else
		{
			std::cout<< "bad reply : "<< std::hex << (int)tmp[0]<<' '<<(int)tmp[1]<<' '<<(int)tmp[2] << std::dec <<"\n";
			ok = false;
		}
	}

	stopServerReply();

	return ok ? value/(nSamples*1023.0f) : -1.0f;
}

int32_t LinkuinoClient::requestDigitalRead()
{
	sendReplyRequest( Linkuino::REQ_DIGITAL_READ );
	
	bool ok = false;
	uint16_t value = 0;
	uint8_t tmp[3]={Linkuino::REPLY_NULL,0xFF,0};
	int n = m_serial->readSync( tmp, 3, 200000); // 200ms max read time

	if( n==3 && tmp[0]==Linkuino::REPLY_DIGITAL_READ && tmp[2]==Linkuino::REPLY_DIGITAL_READ && (tmp[1]&0xC0)==0 )
	{
		value = tmp[1];
	}
	else 
	{
		std::cout<< "bad reply : "<< std::hex << (int)tmp[0]<<' '<<(int)tmp[1]<<' '<<(int)tmp[2] << std::dec <<"\n";
		value = -1;
	}

	stopServerReply();

	return value;
}


void LinkuinoClient::forwardMessage( uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3 )
{
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_FWD_SERIAL);
	setRegisterValue(Linkuino::REQ_DATA0_ADDR, d0);
	setRegisterValue(Linkuino::REQ_DATA1_ADDR, d1);
	setRegisterValue(Linkuino::REQ_DATA2_ADDR, d2);
	setRegisterValue(Linkuino::REQ_DATA3_ADDR, d3);
	send();
}

void LinkuinoClient::resetTo50Hz()
{
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_RESET);
	setRegisterValue(Linkuino::REQ_DATA0_ADDR,Linkuino::RESET_TO_50Hz);
	send();
	send();
	testConnection();
}

void LinkuinoClient::resetTo100Hz()
{
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_RESET);
	setRegisterValue(Linkuino::REQ_DATA0_ADDR,Linkuino::RESET_TO_100Hz);
	send();
	send();
	testConnection();
}

void LinkuinoClient::quiet()
{
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOREPLY);
	setRegisterValue(Linkuino::REQ_DATA0_ADDR, 0);
	setRegisterValue(Linkuino::REQ_DATA1_ADDR, 0);
	setRegisterValue(Linkuino::REQ_DATA2_ADDR, 0);
	setRegisterValue(Linkuino::REQ_DATA3_ADDR, 0);
	send();
}

void LinkuinoClient::printStatus()
{
	std::cout<<"Connected to Linkuino server v"<<getServerVersionMajor()<<'.'<<getServerVersionMinor()<<'\n';
	std::cout<<"Serial speed is "<<Linkuino::SERIAL_SPEED<<'\n';
	std::cout<<"Messages are repeated "<<getMessageRepeats()<<" times\n";
}

void LinkuinoClient::updateTimeStamp()
{
	m_timeStamp = (m_timeStamp + 1) & 0x3F;
	setRegisterValue(Linkuino::TSTMP_ADDR, m_timeStamp);
}

int LinkuinoClient::scanSerialPorts()
{
	const char * * availPorts = LinkuinoSerialPort::availablePorts();
	m_serial->close();
	int p = 0;
	while( availPorts[p] != nullptr )
	{
		if( m_serial->open(availPorts[p]) )
		{
			std::cout<<"testing connection on "<<availPorts[p]<<'\n';
			if( testConnection() ) { return p; }
			else { m_serial->close(); }
		}
		++p;
	}
	return -1;
}

bool LinkuinoClient::testConnection()
{
	// request the controller to introduce itself
	std::cout << "Send revision request\n";
	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_REV);
	pushDataToDevice(m_buffer);
	pushDataToDevice(m_buffer);

	std::chrono::high_resolution_clock::time_point T0 = std::chrono::high_resolution_clock::now();

	std::cout << "wait for reply\n";

	uint8_t reply[256];
	int R = 32;
	int l = 0;
	while (l<R)
	{

		if( (std::chrono::high_resolution_clock::now()-T0) > c_connectionTimeOut )
		{
			reply[l] = '\0';
			std::cerr << "Linkuino: client: receive timeout, buffer='";
			std::cerr << reply << "'\n";
			return false;
		}
		int n = m_serial->read( reply + l, R - l);
		if (n > 0) { l += n; }
	}

	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOREPLY);
	pushDataToDevice(m_buffer);
	pushDataToDevice(m_buffer);

	// flush input buffer
	flushInput();

	reply[R - 1] = '\0';
	//printf("REQ_REV='%s'\n",reply);
	l = 0;
	while (reply[l] != 0x00 && l<R) ++l;
	if (l >= (R - 2) || reply[l] != 0x00 || reply[l + 1] == 0 || reply[l + 2] == 0) { return false; }
	m_serverMajor = (reply[l + 1] & 0x03) + 1;
	m_messageRepeats = reply[l + 1] >> 2;
	m_serverMinor = reply[l + 2] - 1;
	if (m_serverMajor == Linkuino::REV_MAJOR && m_serverMinor == Linkuino::REV_MINOR)
	{
		startWorkerThread();
		return true;
	}
	else
	{
		// version mismatch
		std::cerr << "Linkuino: version mismatch: client is ";
		std::cerr << (int)Linkuino::REV_MAJOR << '.' << (int)Linkuino::REV_MINOR;
		std::cerr << ", server is " << m_serverMajor << '.' << m_serverMinor << std::endl;
		m_serverMajor = 0;
		m_serverMinor = 0;
		return false;
	}
}

void LinkuinoClient::printBuffer()
{
	for (int i = 0; i<Linkuino::CMD_COUNT; i++)
	{
		std::cout << (m_buffer[i] >> 6) << '|' << (m_buffer[i] & 0x3F) << ' ';
	}
	std::cout << "\n";
}

void LinkuinoClient::updateSendTime()
{
	m_sendTime = std::chrono::high_resolution_clock::now();
}

// in uS
std::chrono::high_resolution_clock::duration LinkuinoClient::timeSinceLastSend()
{
	return std::chrono::high_resolution_clock::now() - m_sendTime;
}

void LinkuinoClient::send()
{
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		for (int i = 0; i < Linkuino::CMD_COUNT; i++) { m_bufferCopy[i] = m_buffer[i]; }
		m_readyToSend = true;
	}
	m_readyToSendCond.notify_one();
}

void LinkuinoClient::startWorkerThread()
{
	assert(m_workerThread == nullptr);
	m_workerThread = new std::thread(runWorkerThread, this);
}

void LinkuinoClient::runWorkerThread(LinkuinoClient* self)
{
	self->workerThreadMain();
}

void LinkuinoClient::workerThreadMain()
{
	while (!m_terminate)
	{
		asyncPushDataToDevice();
	}
}

void LinkuinoClient::pushDataToDevice(const uint8_t buffer[Linkuino::CMD_COUNT])
{
	uint8_t send_buffer[m_messageRepeats*Linkuino::CMD_COUNT+1];
	for (int i = 0; i < m_messageRepeats; i++)
	{
		std::memcpy( send_buffer+i*Linkuino::CMD_COUNT, buffer, Linkuino::CMD_COUNT );
	}
	send_buffer[ m_messageRepeats*Linkuino::CMD_COUNT ] = buffer[0]; // finish with a timestamp marker
	m_serial->write(send_buffer, m_messageRepeats*Linkuino::CMD_COUNT+1); 
	m_serial->flush();
	updateSendTime();
	updateTimeStamp();
}

void LinkuinoClient::asyncPushDataToDevice()
{
	bool sendBuf = false;
	uint8_t send_buffer[Linkuino::CMD_COUNT];
	{
		std::unique_lock<std::mutex> lk(m_mutex);
		m_readyToSendCond.wait( lk, [this](){ return m_terminate.load() || m_readyToSend.load(); } );
		if ( !m_terminate && m_readyToSend )
		{
			m_readyToSend = false;
			for (int i = 0; i < Linkuino::CMD_COUNT; i++) { send_buffer[i] = m_bufferCopy[i]; }
			sendBuf = true;
		}
	}
	if (sendBuf)
	{
		pushDataToDevice(send_buffer);
	}
	m_clearToSend = true;
	m_clearToSendCond.notify_one();
}


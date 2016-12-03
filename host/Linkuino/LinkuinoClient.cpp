#include "LinkuinoClient.h"
#include "LinkuinoSerialPort.h"

#include <iostream>
#include <cassert>

LinkuinoClient::LinkuinoClient(LinkuinoSerialPort* port)
		: m_serial(port)
		, m_timeStamp(0)
		, m_serverMajor(0)
		, m_serverMinor(0)
		, m_messageRepeats(24)
		, m_terminate(false)
		, m_rts(false)
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
	m_mutex.lock();
	m_terminate = true;
	m_mutex.unlock();
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

bool LinkuinoClient::scanSerialPorts()
{
	const char * * availPorts = LinkuinoSerialPort::availablePorts();
	m_serial->close();
	int p = 0;
	while( availPorts[p] != nullptr )
	{
		if( m_serial->open(availPorts[p]) )
		{
			std::cout<<"testing connection on "<<availPorts[p]<<'\n';
			if( testConnection() ) { return true; }
			else { m_serial->close(); }
		}
		++p;
	}
	return false;
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
		if (n>0) l += n;
	}

	setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOREPLY);
	pushDataToDevice(m_buffer);
	pushDataToDevice(m_buffer);

	// flush input buffer
	{
		uint8_t tmp;
		while( m_serial->read( &tmp, 1) == 1);
	}

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
	m_mutex.lock();
	for (int i = 0; i < Linkuino::CMD_COUNT; i++) { m_bufferCopy[i] = m_buffer[i]; }
	m_rts = true;
	m_mutex.unlock();
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
	for (int i = 0; i < m_messageRepeats; i++)
	{
		m_serial->write(buffer, Linkuino::CMD_COUNT);
	}
	m_serial->write(buffer, 1); // finish with a timestamp marker	
	m_serial->flush();
	updateSendTime();
	updateTimeStamp();
}

void LinkuinoClient::asyncPushDataToDevice()
{
	std::chrono::duration<int64_t, std::nano> timeToNextSend = c_minTimeBetwwenSend - timeSinceLastSend();
	if ( timeToNextSend > std::chrono::duration<int64_t, std::nano>(100) )
	{
		//int nMilliSeconds = (timeToNextSend + 999) / 1000;
		Sleep(1);
		return;
	}

	bool rts = false;
	uint8_t send_buffer[Linkuino::CMD_COUNT];

	m_mutex.lock();
	if (m_rts)
	{
		for (int i = 0; i < Linkuino::CMD_COUNT; i++) { send_buffer[i] = m_bufferCopy[i]; }
		m_rts = false;
		rts = true;
	}
	m_mutex.unlock();

	if (rts)
	{
		pushDataToDevice(send_buffer);
	}
}


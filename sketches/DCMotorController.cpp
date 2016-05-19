//#define DCMOTOR_SERIAL_DEBUG 1
#define DCMOTOR_ATTINY_PINS 1

#include <AvrTL.h>
#include <AvrTLPin.h>
#include <avr/interrupt.h>
#include <TimeScheduler/TimeScheduler.h>

#include <FastSerial/FastSerial.h>

#ifdef DCMOTOR_SERIAL_DEBUG
#include <SoftSerial/SoftSerial.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#endif

/*
 * to send a command :
 * echo "7 50 7 50" | ./testdcmotorcontroller /dev/ttyUSB1
 * values are : (TR-40)/8 , RR/4 , (TL-40)/8 and RL/4
 * TR = target tick count between 2 rotation steps (right)
 * RR = target rotation (right)
 * TL = target tick count (left)
 * RL = target rotation (left)
 * targetTickCount, similar to 1/speed, must be in the range [40;160] (the bigger the slower)
 * minimum targetTuickCount must be 50 for precise stop
 * 
 *   ATtiny85 wiring
 *   ___    ___
 *   RST ->| v |-- VCC
 * R PWM <-|   |<- Right speed encoder
 * L PWM <-|   |<- Left speed encoder
 *   GND --|___|<- RX 
 * 
*/

using namespace avrtl;

#ifdef DCMOTOR_ATTINY_PINS
#define RX_PIN 		    		0
#define motorLeft_SPEED_PIN 	1
#define motorRight_SPEED_PIN 	2
#define motorRight_PWM_PIN  	3
#define motorLeft_PWM_PIN  		4
#else
#define motorRight_PWM_PIN  8
#define motorLeft_PWM_PIN  9
#define motorRight_SPEED_PIN 2
#define motorLeft_SPEED_PIN 3
#define RX_PIN 		    0
#define TX_PIN 		    1
#endif

static auto motorRightPWM = StaticPin<motorRight_PWM_PIN>();
static auto motorRightSpeed = StaticPin<motorRight_SPEED_PIN>();
static auto motorLeftPWM = StaticPin<motorLeft_PWM_PIN>();
static auto motorLeftSpeed = StaticPin<motorLeft_SPEED_PIN>();
static auto rx = StaticPin<RX_PIN>();

static auto fastSerial = make_fastserial(rx,NullPin());

#ifdef DCMOTOR_SERIAL_DEBUG
static auto serialIO = make_softserial<38400>(NullPin(),motorLeftPWM); // we'll use the same pin for debug output
static ByteStreamAdapter<decltype(serialIO),100000UL> serialAdapter = { serialIO };
static PrintStream cout;
#endif


// the setup function runs once when you press reset or power the board
void setup()
{
  cli(); // who needs interrupts ?
  
  motorRightPWM.SetOutput();
  motorRightSpeed.SetInput();
  motorLeftPWM.SetOutput();
  motorLeftSpeed.SetInput();

#ifdef DCMOTOR_SERIAL_DEBUG
  serialAdapter.m_rawIO.begin();
  cout.begin( &serialAdapter );
#endif
	fastSerial.begin();

	motorRightPWM = LOW;
	motorLeftPWM = LOW;
}

// the loop function runs over and over again forever
void loop()
{
	static constexpr uint8_t pwmCycleTicks = 64;
	static constexpr uint8_t startUpPWMDuty = 32;
	static constexpr uint16_t warmUpTicks = 4;
	
	uint16_t rotationR=0, rotationL=0;
	int16_t tickCountR=0, tickCountL=0;
	int16_t lastTickCountR=0, lastTickCountL=0;
	int16_t lastTickDeltaR=0, lastTickDeltaL=0;
	int16_t targetTickCountR=0, targetTickCountL=0;
	uint16_t targetRotationR=0, targetRotationL=0;

	uint8_t ticks = 0;
	uint8_t digit = 0;
	uint8_t pwmDutyR = startUpPWMDuty;
	uint8_t pwmDutyL = startUpPWMDuty;

	// read command from RX pin, using FastSerial protocol
	uint32_t command = fastSerial.read<24>();
	uint16_t commandTR = (command>>20) & 0x0F;
	uint16_t commandRR = (command>>12) & 0xFF;
	uint16_t commandTL = (command>>8) & 0x0F;
	uint16_t commandRL = command & 0xFF;

	targetTickCountR = 40 + commandTR * 8;
	targetRotationR = commandRR * 4;
	targetTickCountL = 40 + commandTL * 8;
	targetRotationL = commandRL * 4;

	TimeScheduler ts;

	if( targetTickCountR<40 || targetTickCountR>200 || targetTickCountL<40 || targetTickCountL>200 || targetRotationR<=4 || targetRotationL<=4 )
	{
#ifdef DCMOTOR_SERIAL_DEBUG
		motorLeftPWM = HIGH;
		ts.start();	ts.exec( 10000, [&](){} ); ts.stop();
		cout<<"Error: R:"<<targetTickCountR<<'/'<<targetRotationR<<", L:"<<targetTickCountL<<'/'<<targetRotationL;
		ts.start();	ts.exec( 10000, [&](){} ); ts.stop();
		motorLeftPWM = LOW;
#endif
		return;
	}

	bool rightEncoder = motorRightSpeed.Get();
	bool leftEncoder = motorLeftSpeed.Get();
	motorRightPWM.Set( HIGH ); // Vrooooom !
	motorLeftPWM.Set( HIGH ); // Vrooooom !
	
	ts.start();
	bool turnWheels = true;
	while( turnWheels )
	{
		ts.exec( 256, [&]()
		{
			bool encR = motorRightSpeed.Get();
			bool encL = motorLeftSpeed.Get();
			if( encR && !rightEncoder )
			{ 
				lastTickDeltaR = tickCountR - lastTickCountR;
				lastTickCountR = tickCountR;
				tickCountR = 0;
				++rotationR;
			}
			else
			{ 
				++tickCountR;
			}
			rightEncoder = encR;
			bool warmR = ( rotationR >= warmUpTicks ) ;

			if( encL && !leftEncoder )
			{ 
				lastTickDeltaL = tickCountL - lastTickCountL;
				lastTickCountL = tickCountL;
				tickCountL = 0;
				++rotationL;
			}
			else
			{ 
				++tickCountL;
			}
			leftEncoder = encL;
			bool warmL = ( rotationL >= warmUpTicks ) ;

			// speed limitation when close to target rotation
			int16_t stepsBeforeEndL = targetRotationL - rotationL;
			if( stepsBeforeEndL <= 32 )
			{
				int16_t i = 32 - stepsBeforeEndL;
				int16_t minTicksL = 64+4*i;
				if( minTicksL > targetTickCountL ) targetTickCountL = minTicksL;
			}
			int16_t stepsBeforeEndR = targetRotationR - rotationR;
			if( stepsBeforeEndR <= 32 )
			{
				int16_t i = 32 - stepsBeforeEndR;
				int16_t minTicksR = 64+4*i;
				if( minTicksR > targetTickCountR ) targetTickCountR = minTicksR;
			}

			motorRightPWM.Set( ticks<pwmDutyR && rotationR<targetRotationR );
			motorLeftPWM.Set( ticks<pwmDutyL && rotationL<targetRotationL );

			++ticks;
			if( ticks == pwmCycleTicks )
			{
				if(warmR)
				{
					int16_t targetTickDeltaR = (targetTickCountR - lastTickCountR)/4;
					if( lastTickDeltaR<targetTickDeltaR && pwmDutyR>1 ) -- pwmDutyR;
					if( lastTickDeltaR>targetTickDeltaR && pwmDutyR<pwmCycleTicks ) ++ pwmDutyR;
				}
				if(warmL)
				{
					int16_t targetTickDeltaL = (targetTickCountL - lastTickCountL)/4;
					if( lastTickDeltaL<targetTickDeltaL && pwmDutyL>1 ) -- pwmDutyL;
					if( lastTickDeltaL>targetTickDeltaL && pwmDutyL<pwmCycleTicks ) ++ pwmDutyL;
				}
				ticks = 0;
			}

			if( tickCountL >= 4096 ) tickCountL=4096;
			if( tickCountR >= 4096 ) tickCountR=4096;

#ifdef DCMOTOR_SERIAL_DEBUG
			if( tickCountL >= 4096 && tickCountR >= 4096 ) { turnWheels = false; }
#else
			if( tickCountL >= 4096  ) {	rotationL = targetRotationL; }
			if( tickCountR >= 4096  ) {	rotationR = targetRotationR; }
			if( rotationR>=targetRotationR && rotationL>=targetRotationL ) { turnWheels = false; }
#endif
		} );
	}
	motorRightPWM.Set( LOW );
	motorLeftPWM.Set( LOW );
	ts.stop();

#ifdef DCMOTOR_SERIAL_DEBUG
	motorLeftPWM = HIGH;
	ts.start();	ts.exec( 10000, [&](){} ); ts.stop();
	cout<<"R:"<<targetTickCountR<<'/'<<targetRotationR<<", L:"<<targetTickCountL<<'/'<<targetRotationL;
	cout<<", tc="<<tickCountR<<'/'<<tickCountL;
	cout<<", rot="<<rotationR<<'/'<<rotationL<<endl;
	ts.start();	ts.exec( 10000, [&](){} ); ts.stop();
	motorLeftPWM = LOW;
#endif
}

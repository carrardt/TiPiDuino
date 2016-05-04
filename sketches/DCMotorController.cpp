#define DCMOTOR_SERIAL_DEBUG 1
#define DCMOTOR_ATTINY_PINS 1

#include <AvrTL.h>
#include <AvrTLPin.h>
#include <SoftSerial/SoftSerial.h>
#include <avr/interrupt.h>
#include <TimeScheduler/TimeScheduler.h>

#ifdef DCMOTOR_SERIAL_DEBUG
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>
#endif

/*
 * to send a command :
 * echo "S0060R00500E" > /dev/ttyUSB0 
 * targetTickCount, similar to 1/speed, must be in the range [40;160] (the bigger the slower)
 * 
 * deceleration :
 * step period		steps to stop
 * 40 				18
 * 50				12
 * 70				8
 * 140+				2
 * 
 * i = max( rotation-targetRotation+20 , 0 )
 * minimum step period is : 32+64*i
*/

using namespace avrtl;

#ifdef DCMOTOR_ATTINY_PINS
#define motorRight_PWM_PIN  	3
#define motorLeft_PWM_PIN  		4
#define motorRight_SPEED_PIN 	1
#define motorLeft_SPEED_PIN 	2
#define RX_PIN 		    		0
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
static auto serialIO = make_softserial<38400>(rx,motorLeftPWM); // we'll use the same pin for debug output

#ifdef DCMOTOR_SERIAL_DEBUG
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
#else
  serialIO.begin();
#endif

	motorRightPWM = LOW;
	motorLeftPWM = LOW;
}

// the loop function runs over and over again forever
void loop()
{
	static constexpr uint8_t pwmCycleTicks = 64;
	
	uint16_t rotationR=0, rotationL=0;
	int16_t tickCountR=0, tickCountL=0;
	int16_t lastTickCountR=0, lastTickCountL=0;
	int16_t lastTickDeltaR=0, lastTickDeltaL=0;
	int16_t targetTickCountR=0, targetTickCountL=0;
	uint16_t targetRotationR=0, targetRotationL=0;

	uint8_t ticks = 0;
	uint8_t digit = 0;
	uint8_t pwmDutyR = pwmCycleTicks/4;
	uint8_t pwmDutyL = pwmCycleTicks/4;

	while( serialIO.readByte() != 'R' ) ;
	while( (digit=serialIO.readByte()) != 'r' ) { targetTickCountR=targetTickCountR*10+digit-'0'; }
	while( (digit=serialIO.readByte()) != 'L' ) { targetRotationR=targetRotationR*10+digit-'0'; }
	while( (digit=serialIO.readByte()) != 'l' ) { targetTickCountL=targetTickCountL*10+digit-'0'; }
	while( (digit=serialIO.readByte()) != '\n' ) { targetRotationL=targetRotationL*10+digit-'0'; }

	TimeScheduler ts;

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
				if( tickCountR >= 4096 ) { turnWheels = false; }
			}
			rightEncoder = encR;
			bool warmR = ( rotationR >= 4 ) ;

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
				if( tickCountL >= 4096 ) { turnWheels = false; }
			}
			leftEncoder = encL;
			bool warmL = ( rotationL >= 4 ) ;
			
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
			//if( r>=rotationR && l>=rotationL ) { turnWheels = false; }
		} );
	}
	motorRightPWM.Set( LOW );
	motorLeftPWM.Set( LOW );
	ts.stop();

#ifdef DCMOTOR_SERIAL_DEBUG
	motorLeftPWM = HIGH;
	ts.start();	ts.exec( 10000, [&](){} ); ts.stop();
	cout<<"R:"<<targetTickCountR<<'/'<<targetRotationR<<", L:"<<targetTickCountL<<'/'<<targetRotationL<<endl;
	cout<<"r="<<rotationR<<", l="<<rotationL<<endl;
	ts.start();	ts.exec( 10000, [&](){} ); ts.stop();
	motorLeftPWM = LOW;
#endif
}

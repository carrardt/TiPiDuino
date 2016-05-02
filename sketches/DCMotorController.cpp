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
	echo "S005R00500E" > /dev/ttyUSB0 
*/

using namespace avrtl;

#ifdef DCMOTOR_ATTINY_PINS
#define MOTOR_PWM_PIN   1
#define MOTOR_SPEED_PIN 2
#define RX_PIN 		    3
#define TX_PIN 		    4
#else
#define MOTOR_SPEED_PIN 7
#define RX_PIN 		    8
#define TX_PIN 			9
#define MOTOR_PWM_PIN   13
#endif

static auto motorPWM = StaticPin<MOTOR_PWM_PIN>();
static auto motorSpeed = StaticPin<MOTOR_SPEED_PIN>();
static auto rx = StaticPin<RX_PIN>();
#ifdef DCMOTOR_SERIAL_DEBUG
static auto tx = StaticPin<TX_PIN>();
#else
static auto tx = NullPin();
#endif

static auto serialIO = make_softserial<38400>(rx,tx);

#ifdef DCMOTOR_SERIAL_DEBUG
static ByteStreamAdapter<decltype(serialIO),100000UL> serialAdapter = { serialIO };
static PrintStream cout;
#endif

// the setup function runs once when you press reset or power the board
void setup()
{
  cli(); // who needs interrupts ?
  motorPWM.SetOutput();
  motorSpeed.SetInput();
#ifdef DCMOTOR_SERIAL_DEBUG
  serialAdapter.m_rawIO.begin();
  cout.begin( &serialAdapter );
  cout<<"F_CPU="<<F_CPU<<endl;
  cout<<"Ready"<<endl;
#else
  serialIO.begin();
#endif
}

// the loop function runs over and over again forever
void loop()
{
	uint16_t targetTickCount =  0;
	uint16_t rotation = 0;
	uint8_t digit=0;
	while( serialIO.readByte() != 'S' ) ;
	while( (digit=serialIO.readByte()) != 'R' ) { targetTickCount=targetTickCount*10+digit-'0'; }
	while( (digit=serialIO.readByte()) != 'E' ) { rotation=rotation*10+digit-'0'; }

	
#ifdef DCMOTOR_SERIAL_DEBUG
	cout<<"target="<<targetTickCount<<", rotation="<<rotation<<endl;
#endif

	// turn the wheel of fortune !
	uint16_t r=0;
	uint16_t tickCount = 0;
	uint16_t lastTickCount = 0;
	uint16_t minTickCount = 4096;
	uint16_t maxTickCount = 0;
	uint32_t ticks = 0;

	TimeScheduler ts;
	bool s = motorSpeed.Get();
	motorPWM.Set( HIGH );
	ts.start();
	while( r<rotation )
	{
		ts.exec( 256, [&]()
		{
			bool ns = motorSpeed.Get();
			bool warm = ( r >= 3 ) ;
			if( ns != s )
			{ 
				if( warm )
				{
					if( tickCount<minTickCount) minTickCount = tickCount;
					if( tickCount>maxTickCount) maxTickCount = tickCount;
				}
				lastTickCount = tickCount;
				tickCount = 0;
				++r;
			}
			else
			{ 
				++tickCount;
				if( tickCount >= 4096 ) { r = rotation+1; }
			}
			s = ns;
			bool motorState = !warm || lastTickCount>targetTickCount;
			motorPWM.Set( motorState );
			++ticks;
		} );
	}
	motorPWM.Set( LOW );
	ts.stop();

#ifdef DCMOTOR_SERIAL_DEBUG
	cout<<"min="<<minTickCount<<", max="<<maxTickCount<<", avg="<<ticks/r<<", r="<<r<<endl;
#endif
}

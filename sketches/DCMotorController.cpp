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
	uint8_t commandStart = serialIO.readByte();
	if( commandStart != 'S' ) return;
	
	uint8_t targetTickTime = serialIO.readByte() - '0';
	uint8_t rotation = serialIO.readByte() - '0';
	rotation = (rotation+1)*8;
	targetTickTime = (targetTickTime+1)*8;

	uint8_t commandEnd = serialIO.readByte();
	if( commandEnd != 'E' ) return;
	
#ifdef DCMOTOR_SERIAL_DEBUG
	cout<<"target="<<targetTickTime<<", rotation="<<rotation<<endl;
#endif
	
	// turn the wheel of fortune !
	uint16_t r=0;
	uint32_t c=0;
	uint16_t tickTime=0;
	uint16_t lastTickTime=0;
	
	TimeScheduler ts;
	ts.start();
	bool s = motorSpeed.Get();
	motorPWM.Set( r < rotation );
	while( r<rotation && c<10000 ) // can't run more than 5 Sec (just in case)
	{
		ts.exec( 500, [&]()
			{
				bool ns = motorSpeed.Get();			
				if( ns != s )
				{ 
					++r;
					lastTickTime = tickTime;
					tickTime = 0;
				}
				else { ++tickTime; }
				s = ns;
				bool motorPower = r<rotation ;
				if( tickTime<targetTickTime && lastTickTime<targetTickTime ) motorPower = LOW;
				motorPWM.Set( motorPower );
				++c;
			} );
	}
	motorPWM.Set(LOW);
	ts.stop();

#ifdef DCMOTOR_SERIAL_DEBUG
	cout<<"Done: c="<<c<<", r="<<r<<endl;
#endif
}

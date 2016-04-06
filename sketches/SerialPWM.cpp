#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <TimeScheduler.h>
#include <HWSerialNoInt.h>
#include <BasicIO/ByteStream.h>
#include <BasicIO/PrintStream.h>

using namespace avrtl;

#define PWM_PIN 13

#define SMOOTHING 2 // 0,1,2 or 3 level of input value smoothing

static auto pwm = StaticPin<PWM_PIN>();
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
PrintStream cout;

static TimeScheduler ts;

void setup()
{
	serialIO.m_rawIO.begin(57600);
	cout.begin(&serialIO);
	pwm.SetOutput();
	cout<<"SerialPWM Ready"<<endl;
	cli();
}

void loop()
{
	uint8_t addr = 0;
	uint8_t clk = 255;
	uint8_t cmd[4] = {255,255,255,255};

	uint16_t pwmValue = 1024;
	uint16_t pwmTargetValue = 1024;

	/*
	buf[0] = serialIO.readByte();
	buf[1] = serialIO.readByte();
	buf[2] = serialIO.readByte();
	buf[3] = serialIO.readByte();
	uint32_t x = buf[0];
	x <<= 8; x |= buf[1];
	x <<= 8; x |= buf[2];
	x <<= 8; x |= buf[3];
	
	for(int i=0;i<4;i++)
	{
		cout<<(int)buf[i]<<endl;
	}
	cout<<'='<<x<<endl;
	*/
	
	ts.start();

	while( true )
	{
		ts.exec( 400,
			[&addr,&clk]()
			{
				pwm = HIGH;
				addr = 0;
				clk = 255;
			} );
			
		ts.loop( 2000,
			[pwmValue](int16_t t)
			{
				pwm = (t<pwmValue);
			} );

		ts.exec( 600,
				[&cmd]()
				{
					pwm = LOW;
					cmd[0]=255;
					cmd[1]=255;
					cmd[2]=255;
					cmd[3]=255;
				} );
		ts.loop( 5000, // we have 5 milliseconds to listen for serial commands
				[&clk,&addr,&cmd](int16_t t)
				{
					uint16_t x = HWSerialNoInt::readByteAsync();
					if( x )
					{
						uint8_t r = x;
						uint8_t data = r & 0x3F;
						r >>= 6;
						uint8_t nRS = r & 0x01;
						uint8_t nCLK = r >> 1;
						if(nRS) { addr = data; }
						else if(nCLK==clk) { cmd[addr] = data; }
						clk = nCLK;
					}
				} );
				
		ts.exec( 1000,
				[cmd,&pwmTargetValue]()
				{
					if( cmd[1]!=255 && cmd[2]!=255 )
					{
						pwmTargetValue = cmd[1];
						pwmTargetValue <<= 6;
						pwmTargetValue |= cmd[2];
					}
				} );
				
		ts.exec( 1000,
				[&pwmValue,&pwmTargetValue]()
				{
					uint16_t target = pwmTargetValue;
					if( target <= 400 ) target=0;
					else if( target >= 2000 ) target=1600;
					else target -= 400;
					pwmValue = ( (pwmValue<<SMOOTHING)-pwmValue+target ) >> SMOOTHING ;
				} );
	}

	ts.stop();
	
}


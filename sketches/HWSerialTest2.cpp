#include <AvrTL.h>
#include <BasicIO/PrintStream.h>
#include <BasicIO/InputStream.h>
#include <HWSerialNoInt/HWSerialNoInt.h>

using namespace avrtl;

ByteStreamAdapter<HWSerialNoInt> serialIO;
PrintStream cout;
InputStream cin;

void setup()
{
	serialIO.m_rawIO.begin(57600);
	cout.begin( &serialIO );
	cin.begin( &serialIO );
	cout<<"Ready"<<endl;
}

static uint32_t counter = 0;
void loop()
{
	char buf[32];
	uint8_t i=0;
	while( ( buf[i]=serialIO.readByte() ) != '\n' && i<31 ) ++i;
	buf[i]='\0';
	cout<<counter<<": "<<buf<<endl;
	++ counter;
}

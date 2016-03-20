#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <AvrTL.h>
#include <BasicIO/PrintStream.h>
#include <BasicIO/InputStream.h>
#include <BasicIO/HWSerialIO.h>

using namespace avrtl;

// auto soft_rx = pin(10);
// auto soft_tx = pin(11);

HWSerialIO hwserial;
// auto softIO = make_softserial<19200>(soft_rx,soft_tx);

PrintStream cout;
InputStream cin;

void setup()
{
	//softIO.begin();
	hwserial.begin(9600);

	cout.begin( &hwserial );
	cin.begin( &hwserial );
	cout<<"Ready"<<endl;
}

void loop()
{
	char c = 0;
	cin >> c;
	cout<<"'"<<c<<"' TXr="<<HWSerialIO::Tx_ready<<" RXr="<<HWSerialIO::Rx_read<<" RXa="<<HWSerialIO::Rx_avail<<endl;
}

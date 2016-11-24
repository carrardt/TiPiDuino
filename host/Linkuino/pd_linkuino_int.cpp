/*
 * puredata external object to connce to an avr using the Linkuino interface
 * once compiled, copy the dynamic library liblinkuino.so to puredata externals
 * On Linux :
 * mkdir -p ~/pd-externals/linkuino
 * cp liblinkuino.so ~/pd-externals/linkuino/linkuino.pd_linux
 */

#include "LinkuinoClient.h"
#include "LinkuinoSerialPort.h"
#include <vector>
#include <sstream>

#include "pd_linkuino.h"

LinkuinoClient* linkuino_open_device(int i)
{
	static LinkuinoSerialPort serial;
	LinkuinoClient* li = new LinkuinoClient(&serial);
	
	const char * * availPorts = LinkuinoSerialPort::availablePorts();
	int nPorts = 0;
	while( availPorts[nPorts] != nullptr ) { ++ nPorts; }
	
	bool connected = false;
	if( i>=0 && i<nPorts )
	{
		post("Linkuino: open device");
		serial.open( availPorts[i] );
		connected = li->testConnection();
	}
	else
	{
		post("Linkuino: scan for device");
		connected = li->scanSerialPorts();
	}
	
	if( ! connected )
	{
		delete li;
		return nullptr;
	}
	li->printStatus();
	return li;
}

int linkuino_getversionmajor(struct LinkuinoClient* li)
{
	if(li==0) return -1;
	return li->getServerVersionMajor();
}

int linkuino_getversionminor(struct LinkuinoClient* li)
{
	if(li==0) return -1;
	return li->getServerVersionMinor();
}

void linkuino_set_pwm_value(struct LinkuinoClient* li, int pwmI, float pwmValue)
{
	if( pwmValue < 0.0f ) pwmValue = 0.0f;
	if( pwmValue > 1.0f ) pwmValue = 1.0f;
	li->setPWMValue( pwmI, pwmValue );
}

void linkuino_set_dout(struct LinkuinoClient* li,int v)
{
	li->setRegisterValue(Linkuino::DOUT_ADDR, v);
}

void linkuino_send(struct LinkuinoClient* li)
{
	li->send();
}

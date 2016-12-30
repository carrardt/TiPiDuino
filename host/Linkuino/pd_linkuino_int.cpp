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
#include <map>
#include <sstream>

#include "pd_linkuino.h"

static std::map<int, LinkuinoClient*> g_linkuinoInstances;

int linkuino_get_device_id(LinkuinoClient* li)
{
	for( auto it : g_linkuinoInstances )
	{
		if (it.second == li) { return it.first;  }
	}
	return -1;
}

void linkuino_close_device( LinkuinoClient* li )
{
	if (li == nullptr) { return;  }
	int i = linkuino_get_device_id(li);
	if (i != -1)
	{
		g_linkuinoInstances.erase(i);
	}
	LinkuinoSerialPort* s = li->serialPort();
	delete li;
	delete s;
}

LinkuinoClient* linkuino_get_device(int i)
{
	auto it = g_linkuinoInstances.find(i);
	if (it != g_linkuinoInstances.end())
	{
		return it->second;
	}
	return 0;
}

LinkuinoClient* linkuino_open_device(int i)
{
	auto it = g_linkuinoInstances.find(i);
	if (it != g_linkuinoInstances.end())
	{
		return it->second;
	}

	LinkuinoSerialPort* serial = new LinkuinoSerialPort();
	LinkuinoClient* li = new LinkuinoClient(serial);
	
	const char * * availPorts = LinkuinoSerialPort::availablePorts();
	int nPorts = 0;
	while( availPorts[nPorts] != nullptr ) { ++ nPorts; }
	
	bool connected = false;
	if( i>=0 && i<nPorts )
	{
		post("Linkuino: open device");
		serial->open( availPorts[i] );
		connected = li->testConnection();
	}
	else
	{
		post("Linkuino: scan for device");
		i = li->scanSerialPorts();
		connected = (i>=0);
	}
	
	if( ! connected )
	{
		delete li;
		delete serial;
		return nullptr;
	}
	li->printStatus();
	g_linkuinoInstances[i] = li;
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

float linkuino_read_analog(struct LinkuinoClient* li, int ch)
{
	return li->requestAnalogRead(ch);
}

void linkuino_send(struct LinkuinoClient* li)
{
	li->send();
}

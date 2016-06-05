/*
 * puredata external object to connce to an avr using the Linkuino interface
 * once compiled, copy the dynamic library liblinkuino.so to puredata externals
 * On Linux :
 * mkdir -p ~/pd-externals/linkuino
 * cp liblinkuino.so ~/pd-externals/linkuino/linkuino.pd_linux
 */

extern "C"
{
	#include "pd/m_pd.h"

	struct LinkuinoClient;
	int linkuino_enum_devices();
	struct LinkuinoClient* linkuino_open_device(int);
	int linkuino_getversionmajor(struct LinkuinoClient*);
	int linkuino_getversionminor(struct LinkuinoClient*);
	void linkuino_set_pwm_value(struct LinkuinoClient*,int,float);
	void linkuino_set_dout(struct LinkuinoClient*,int);
	void linkuino_send(struct LinkuinoClient*);

	static t_class *linkuino_class;  

	typedef struct _linkuino {  
	  t_object  x_obj;
	  t_float f_serialdev; 
	  t_float f_a0; 
	  t_float f_a1; 
	  t_float f_a2; 
	  t_float f_a3; 
	  t_float f_a4; 
	  t_float f_a5;
	  t_float f_amode0; 
	  t_float f_amode1; 
	  t_float f_amode2; 
	  t_float f_amode3; 
	  t_float f_amode4; 
	  t_float f_amode5;
	  t_float f_d0;
	  t_float f_d1;
	  t_float f_d2;
	  t_float f_d3;
	  t_float f_d4;
	  t_float f_d5;
	  t_outlet *f_revmajor;
	  t_outlet *f_revminor;
	  struct LinkuinoClient* x_link;
	} t_linkuino;

#define PWM_FLOAT_VALUE(x,i) ( (x->f_amode##i==0) ? (x->f_a##i*0.01f) : (0.05f+x->f_a##i*0.0015f) )

	void linkuino_bang(t_linkuino *x)  
	{
	  if( x->x_link == NULL )
	  {
		x->x_link = linkuino_open_device( (int)x->f_serialdev );
		post("Linkuino ready !");
	  }
	  int vmaj = linkuino_getversionmajor(x->x_link);
	  int vmin = linkuino_getversionminor(x->x_link);
	  //printf("serialdev=%d, client=%p, V%d.%d\n", (int)x->f_serialdev,x->x_link,vmaj,vmin);
	  if( x->x_link == 0 ) return;
	  outlet_float(x->f_revmajor, vmaj);  
	  outlet_float(x->f_revminor, vmin);
	  
	  // update PWM state
	  linkuino_set_pwm_value(x->x_link,0, PWM_FLOAT_VALUE(x,0) );
	  linkuino_set_pwm_value(x->x_link,1, PWM_FLOAT_VALUE(x,1) );
	  linkuino_set_pwm_value(x->x_link,2, PWM_FLOAT_VALUE(x,2) );
	  linkuino_set_pwm_value(x->x_link,3, PWM_FLOAT_VALUE(x,3) );
	  linkuino_set_pwm_value(x->x_link,4, PWM_FLOAT_VALUE(x,4) );
	  linkuino_set_pwm_value(x->x_link,5, PWM_FLOAT_VALUE(x,5) );
	  
	  // update digital output state
	  int d = 0;
	  d  = (x->f_d0 > 0.5) ? 1 : 0;
	  d |= (x->f_d1 > 0.5) ? 2 : 0;
	  d |= (x->f_d2 > 0.5) ? 4 : 0;
	  d |= (x->f_d3 > 0.5) ? 8 : 0;
	  d |= (x->f_d4 > 0.5) ? 16 : 0;
	  d |= (x->f_d5 > 0.5) ? 32 : 0;
	  linkuino_set_dout(x->x_link,d);
	  
	  // send new state and commands to device
	  linkuino_send(x->x_link);
	}

#undef PWM_FLOAT_VALUE

	void *linkuino_new()  
	{
	  linkuino_enum_devices();
	  t_linkuino *x = (t_linkuino *)pd_new(linkuino_class);  
	  x->f_serialdev = 0;
	  x->f_a0 = 0;
	  x->f_a1 = 0;
	  x->f_a2 = 0;
	  x->f_a3 = 0;
	  x->f_a4 = 0;
	  x->f_a5 = 0;
	  x->f_amode0 = 0;
	  x->f_amode1 = 0;
	  x->f_amode2 = 0;
	  x->f_amode3 = 0;
	  x->f_amode4 = 0;
	  x->f_amode5 = 0;
	  floatinlet_new(&x->x_obj, &x->f_serialdev);
	  floatinlet_new(&x->x_obj, &x->f_a0);
	  floatinlet_new(&x->x_obj, &x->f_a1);
	  floatinlet_new(&x->x_obj, &x->f_a2);
	  floatinlet_new(&x->x_obj, &x->f_a3);
	  floatinlet_new(&x->x_obj, &x->f_a4);
	  floatinlet_new(&x->x_obj, &x->f_a5);
	  floatinlet_new(&x->x_obj, &x->f_amode0);
	  floatinlet_new(&x->x_obj, &x->f_amode1);
	  floatinlet_new(&x->x_obj, &x->f_amode2);
	  floatinlet_new(&x->x_obj, &x->f_amode3);
	  floatinlet_new(&x->x_obj, &x->f_amode4);
	  floatinlet_new(&x->x_obj, &x->f_amode5);
	  floatinlet_new(&x->x_obj, &x->f_d0);
	  floatinlet_new(&x->x_obj, &x->f_d1);
	  floatinlet_new(&x->x_obj, &x->f_d2);
	  floatinlet_new(&x->x_obj, &x->f_d3);
	  floatinlet_new(&x->x_obj, &x->f_d4);
	  floatinlet_new(&x->x_obj, &x->f_d5);
	  x->f_revmajor = outlet_new(&x->x_obj, &s_float);
	  x->f_revminor = outlet_new(&x->x_obj, &s_float);
	  x->x_link = 0;
	  return (void *)x;  
	}  
	 
	void linkuino_setup(void)
	{
	  linkuino_class = class_new(gensym("linkuino"),  
			(t_newmethod)linkuino_new,  
			0, sizeof(t_linkuino),  
			CLASS_DEFAULT, (t_atomtype)NULL );  
			
	  class_addbang(linkuino_class, linkuino_bang);  
	}
}

#include "LinkuinoClient.h"
#include <vector>
#include <sstream>

static const char* serial_devices[] = {
	  "/dev/ttyAMA0"
	, "/dev/ttyAMA1"
	, "/dev/ttyUSB0"
	, "/dev/ttyUSB1"
	, "/dev/ttyUSB2"
	, "/dev/ttyUSB3"
	, "/dev/ttyACM0"
	, "/dev/ttyACM1"
	, nullptr
	};

static std::vector<std::string> linkuino_devices;

LinkuinoClient* linkuino_open_device(int i)
{
	const int maxRetry = 3;
	int retryCount = 0;
	bool connected = false;
	int fd = LinkuinoClient::openSerialDevice(linkuino_devices[i]);
	if(fd<0) return 0;
	LinkuinoClient* li = new LinkuinoClient(fd);
	while( !connected && retryCount<maxRetry )
	{
		connected = li->testConnection() ;
		if( ! connected ) { sleep(1); }
		++ retryCount;
	}
	if( ! connected )
	{
		delete li;
		return 0;
	}
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
	int value;
	if( pwmValue < 0.0f ) pwmValue = 0.0f;
	if( pwmValue > 1.0f ) pwmValue = 1.0f;
	value = pwmValue*10000.0f;
	li->setPWMValue( pwmI, value );
}

void linkuino_set_dout(struct LinkuinoClient* li,int v)
{
	li->setRegisterValue(Linkuino::DOUT_ADDR, v);
}

void linkuino_send(struct LinkuinoClient* li)
{
	li->send();
}

int linkuino_enum_devices()
{
	if( linkuino_devices.empty() )
	{
		post("Linkuino: detect servers");
		for(int i=0;serial_devices[i]!=0;i++)
		{
			std::ostringstream oss;
			oss<<"Linkuino: Test server on "<<serial_devices[i]<<" ... ";
			bool devok = false;
			int fd = LinkuinoClient::openSerialDevice(serial_devices[i]);
			if( fd >= 0 )
			{
				LinkuinoClient li(fd);
				if( li.testConnection() )
				{
					devok = true;
					linkuino_devices.push_back(serial_devices[i]);
				}
			}
			if(devok) oss<<"Ok";
			else oss<<"Failed";
			post( oss.str().c_str() );
		}
	}
	return linkuino_devices.size();
}

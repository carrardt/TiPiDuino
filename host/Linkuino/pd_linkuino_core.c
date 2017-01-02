/*
 * puredata external object to connce to an avr using the Linkuino interface
 * once compiled, copy the dynamic library liblinkuino.so to puredata externals
 * On Linux :
 * mkdir -p ~/pd-externals/linkuino
 * cp liblinkuino.so ~/pd-externals/linkuino/linkuino.pd_linux
 */

#ifdef _WIN32
#include "m_pd.h"
#define PD_EXPORT_FUNC	__declspec(dllexport)
#define PD_CDECL		__cdecl
#else
#include "pd/m_pd.h"
#define PD_EXPORT_FUNC
#define PD_CDECL
#endif

#include "pd_linkuino.h"

static t_class *linkuino_class = 0;  
static t_class *lnka_class = 0;
static t_class *lnkd_class = 0;

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
	t_outlet *f_ID;
	int m_id;
	struct LinkuinoClient* x_link;
} t_linkuino;

typedef struct _lnka {
	t_object  x_obj;
	t_float f_id;
	t_float f_ch;
	t_outlet *f_value;
	float m_value[6];
} t_lnka;

typedef struct _lnkd {
	t_object  x_obj;
	t_float f_id;
	t_outlet *f_d0;
	t_outlet *f_d1;
	t_outlet *f_d2;
	t_outlet *f_d3;
	t_outlet *f_d4;
	t_outlet *f_d5;
	uint8_t m_value;
} t_lnkd;

#define PWM_FLOAT_VALUE(x,i) ( (x->f_amode##i==0) ? (x->f_a##i*0.01f) : (0.05f+x->f_a##i*0.002f) )

void linkuino_bang(t_linkuino *x)  
{
	if( x->x_link == NULL )
	{
		x->x_link = linkuino_open_device( (int)x->f_serialdev );
		x->m_id = linkuino_get_device_id(x->x_link);
		post("Linkuino ready !");
	}
	if (x->x_link == NULL) { return;  }

	int vmaj = linkuino_getversionmajor(x->x_link);
	int vmin = linkuino_getversionminor(x->x_link);
	//printf("serialdev=%d, client=%p, V%d.%d\n", (int)x->f_serialdev,x->x_link,vmaj,vmin);
	if( x->x_link == 0 ) return;
	outlet_float(x->f_revmajor, vmaj);  
	outlet_float(x->f_revminor, vmin);
	outlet_float(x->f_ID, x->m_id);

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
	t_linkuino *x = (t_linkuino *)pd_new(linkuino_class);  
	x->f_serialdev = 99;
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
	x->f_ID = outlet_new(&x->x_obj, &s_float);
	x->m_id = -1;
	x->x_link = 0;
	return (void *)x;  
}  

void linkuino_free(t_linkuino *x)
{
	if (x == 0) { return;  }
	if (x->x_link == 0) { return; }
	linkuino_close_device(x->x_link);
}

void *lnka_new()
{
	int i=0;
	t_lnka *x = (t_lnka *)pd_new(lnka_class);
	x->f_ch = 0;
	x->f_id = -1;
	for (i = 0; i < 6; i++) { x->m_value[i] = 50.0f; }
	floatinlet_new(&x->x_obj, &x->f_id);
	floatinlet_new(&x->x_obj, &x->f_ch);
	x->f_value = outlet_new(&x->x_obj, &s_float);
	return (void *)x;
}

void lnka_bang(t_lnka *x)
{
	//char tmp[128];
	struct LinkuinoClient* li = linkuino_get_device(x->f_id);
	int chi = x->f_ch;
	if (li == 0 || chi<0 || chi>=6 ) { return; }
	float s = linkuino_read_analog(li, chi);
	if (s >= 0.0f)
	{ 
		//sprintf(tmp, "Analog#%d = %f", chi, x->m_value[chi]);
		x->m_value[chi] = 100.0f * s;
	}
	else
	{
		//sprintf(tmp, "Error while reading Analog#%d", chi);
	}
	//post(tmp);
	outlet_float(x->f_value, x->m_value[chi]);
}

void *lnkd_new()
{
	t_lnkd *x = (t_lnkd *)pd_new(lnkd_class);
	x->f_id = -1;
	x->m_value = 0x00;
	floatinlet_new(&x->x_obj, &x->f_id);
	x->f_d0 = outlet_new(&x->x_obj, &s_float);
	x->f_d1 = outlet_new(&x->x_obj, &s_float);
	x->f_d2 = outlet_new(&x->x_obj, &s_float);
	x->f_d3 = outlet_new(&x->x_obj, &s_float);
	x->f_d4 = outlet_new(&x->x_obj, &s_float);
	x->f_d5 = outlet_new(&x->x_obj, &s_float);
	return (void *)x;
}

void lnkd_bang(t_lnkd *x)
{
	//char tmp[128];
	struct LinkuinoClient* li = linkuino_get_device(x->f_id);
	//sprintf(tmp, "Connected to device #%f", x->f_id);
	//post(tmp);
	if (li == 0) { return; }
	int s = linkuino_read_digital(li);
	if ( s >= 0 ) { x->m_value = s; }
	//sprintf(tmp, "Digital = 0x%02X", x->m_value);
	//post(tmp);
	outlet_float(x->f_d0, x->m_value & 0x01);
	outlet_float(x->f_d1, (x->m_value >> 1) & 0x01);
	outlet_float(x->f_d2, (x->m_value >> 2) & 0x01);
	outlet_float(x->f_d3, (x->m_value >> 3) & 0x01);
	outlet_float(x->f_d4, (x->m_value >> 4) & 0x01);
	outlet_float(x->f_d5, (x->m_value >> 5) & 0x01);
}

PD_EXPORT_FUNC void PD_CDECL linkuino_setup(void)
{
	linkuino_class = class_new(
		gensym("linkuino"),  
		(t_newmethod)linkuino_new,  
		(t_method)linkuino_free,
		sizeof(t_linkuino),
		CLASS_DEFAULT, (t_atomtype)NULL );  
	class_addbang(linkuino_class, linkuino_bang);

	lnka_class = class_new(
		gensym("lnka"),
		(t_newmethod)lnka_new,
		0,
		sizeof(t_lnka),
		CLASS_DEFAULT, (t_atomtype)NULL);
	class_addbang(lnka_class, lnka_bang);

	lnkd_class = class_new(
		gensym("lnkd"),
		(t_newmethod)lnkd_new,
		0,
		sizeof(t_lnkd),
		CLASS_DEFAULT, (t_atomtype)NULL);
	class_addbang(lnkd_class, lnkd_bang);
}

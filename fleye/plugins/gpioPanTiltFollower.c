#include <stdio.h>
#include "../cpu_tracking.h"

#include "../thirdparty/bcm2835.h"
#include "../thirdparty/bcm2835.c"

#define SERVO_X_VALUE_MIN 250
#define SERVO_X_VALUE_MAX 750

#define SERVO_Y_VALUE_MIN 250
#define SERVO_Y_VALUE_MAX 750

void gpioPanTiltFollower_setup()
{
  int i;
  
  if (!bcm2835_init())
  {
    printf("bcm2835_init failed\n");
    return;
  }

  // reserve 24 GPIO pins as output
  for(i=0;i<24;++i)
  {
	bcm2835_gpio_fsel(i, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(i, LOW);
  }

  printf("GPIO pan/tilt follower ready\n");
}

void gpioPanTiltFollower_run(CPU_TRACKING_STATE * state)
{
	double xf=0.0,yf=0.0;
	int i=0;
	unsigned int xi=0,yi=0;
	uint32_t bits=0, set_mask=0, clr_mask=0;
	int laserSwitch = 0;
	
	if( state->objectCount < 1 ) return;
	
	xf = state->objectCenter[0][0];
	yf = state->objectCenter[0][1];

	if(xf<0.0f) xf=0.0f;
	else if(xf>1.0f) xf=1.0f;

	if(yf<0.0f) yf=0.0f;
	else if(yf>1.0f) yf=1.0f;

	xi = SERVO_X_VALUE_MIN + (unsigned int)( xf * (SERVO_X_VALUE_MAX-SERVO_X_VALUE_MIN) );
	yi = SERVO_Y_VALUE_MIN + (unsigned int)( yf * (SERVO_Y_VALUE_MAX-SERVO_Y_VALUE_MIN) );

	if( xi>=1024 ) xi=1023;
	if( yi>=1024 ) yi=1023;

	bits = yi*1024 + xi;
	
	for(i=0;i<20;++i)
	{
		if( (bits>>i) & 1 ) { set_mask |= 1<<i; }
		else { clr_mask |= 1<<i; }
	}

	if( laserSwitch ) { set_mask |= 1<<20; }
	else { clr_mask |= 1<<20; }

	bcm2835_gpio_clr_multi( clr_mask );
	bcm2835_gpio_set_multi( set_mask );
}


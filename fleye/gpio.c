#include <stdio.h>
 
#include "bcm2835.h"
#include "gpio.h"
 

int init_gpio()
{
  int i;
    if (!bcm2835_init())
  {
    printf("bcm2835_init failed\n");
    return -1;
  }

  // we use 2x 10bits digital outputs
  for(i=0;i<20;++i)
  {
	bcm2835_gpio_fsel(i, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(i, LOW);
  }
  
  return 0;
}

void gpio_write_xy_f(float xf, float yf)
{
	unsigned int xi,yi;
	
	if(xf<0.0f) xf=0.0f;
	else if(xf>1.0f) xf=1.0f;
	
	if(yf<0.0f) yf=0.0f;
	else if(yf>1.0f) yf=1.0f;

	xi = (unsigned int)( xf * 1024 );
	yi = (unsigned int)( yf * 1024 );
	
	gpio_write_xy_i(xi,yi);
}

void gpio_write_xy_i(unsigned int xi, unsigned int yi)
{
	unsigned int i;
	unsigned int set_mask = 0;
	unsigned int clr_mask = 0;
	unsigned int  bits;
	
	if( xi>=1024 ) xi=1023;
	if( yi>=1024 ) yi=1023;

	bits = yi*1024 + xi;
	
	for(i=0;i<20;++i)
	{
		if( (bits>>i) & 1 ) { set_mask |= 1<<i; }
		else { clr_mask |= 1<<i; }
	}

	//printf("clr=%04X, set=%04X\n",clr_mask,set_mask);
	bcm2835_gpio_set_multi(set_mask);
	bcm2835_gpio_clr_multi(clr_mask);
	// usleep(130); // have to wait 
}


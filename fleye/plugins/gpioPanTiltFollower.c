#include <stdio.h>
#include <math.h>
#include "../cpu_tracking.h"

#include "../thirdparty/bcm2835.h"
#include "../thirdparty/bcm2835.c"

#define SERVO_X_VALUE_MIN 250
#define SERVO_X_VALUE_MAX 750

#define SERVO_Y_VALUE_MIN 250
#define SERVO_Y_VALUE_MAX 750

static int autoCalibration = 1;

static double ServoRefX = 0.5;
static double ServoRefY = 0.25;

static const double ServoDeltaX = 0.125;
static const double ServoDeltaY = 0.125;

static const double TargetPosX = 0.5;
static const double TargetPosY = 0.5;

static double refPosX=0.0, refPosY=0.0;
static double DXPosX=0.0, DXPosY=0.0;
static double DYPosX=0.0, DYPosY=0.0;

static double ServoInvMat[2][2];

static void gpio_write_values(float xf, float yf, int laserSwitch)
{
	uint32_t bits=0, set_mask=0, clr_mask=0;
	int xi=0,yi=0;
	int i=0;
	
	if(xf<0.0f) xf=0.0f;
	else if(xf>1.0f) xf=1.0f;

	if(yf<0.0f) yf=0.0f;
	else if(yf>1.0f) yf=1.0f;

	xi = SERVO_X_VALUE_MIN + (unsigned int)( xf * (SERVO_X_VALUE_MAX-SERVO_X_VALUE_MIN) );
	yi = SERVO_Y_VALUE_MIN + (unsigned int)( yf * (SERVO_Y_VALUE_MAX-SERVO_Y_VALUE_MIN) );

	if( xi>=1024 ) xi=1023;
	if( yi>=1024 ) yi=1023;

	//printf("xi=%d, yi=%d, L=%d\n",xi,yi,laserSwitch);
	bits = (yi<<10) | xi | (laserSwitch<<20);
	
	for(i=0;i<21;++i)
	{
		if( (bits>>i) & 1 ) { set_mask |= 1<<i; }
		else { clr_mask |= 1<<i; }
	}

	bcm2835_gpio_clr_multi( clr_mask );
	bcm2835_gpio_set_multi( set_mask );
}

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

  gpio_write_values(ServoRefX,ServoRefY,0);
  printf("GPIO pan/tilt follower ready\n");
}

#define INIT_COUNT 256
#define ACQUIRE_COUNT 128
void gpioPanTiltFollower_run(CPU_TRACKING_STATE * state)
{
	double xf = atan( (state->objectCenter[0][0] - 0.5) ) + 0.5;
	double yf = atan( (state->objectCenter[0][1] - 0.5) ) + 0.5;
	if( state->objectCount < 1 ) return;

	if( autoCalibration )
	{
		static int autoCalibrationState = 0;
		static int recCount = INIT_COUNT;
		
		switch( autoCalibrationState )
		{
			case 0 :
				gpio_write_values(ServoRefX,ServoRefY,0);
				--recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;
				
			case 1 :
				refPosX += xf;
				refPosY += yf;
				-- recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					refPosX /= ACQUIRE_COUNT;
					refPosY /= ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;

			case 2 :
				gpio_write_values(ServoRefX+ServoDeltaX,ServoRefY,0);
				--recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;

			case 3 :
				DXPosX += xf;
				DXPosY += yf;
				-- recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					DXPosX /= ACQUIRE_COUNT;
					DXPosY /= ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;

			case 4 :
				gpio_write_values(ServoRefX,ServoRefY+ServoDeltaY,0);
				--recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;

			case 5 :
				DYPosX += xf;
				DYPosY += yf;
				-- recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					DYPosX /= ACQUIRE_COUNT;
					DYPosY /= ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;
				
			case 6 :
				DXPosX = (DXPosX-refPosX) / ServoDeltaX;
				DXPosY = (DXPosY-refPosY) / ServoDeltaX;

				DYPosX = (DYPosX-refPosX) / ServoDeltaY;
				DYPosY = (DYPosY-refPosY) / ServoDeltaY;

				printf("DX=%g,%g DY=%g,%g\n",DXPosX,DXPosY,DYPosX,DYPosY);
				++ autoCalibrationState;
				break;

			case 7 :
				gpio_write_values(ServoRefX,ServoRefY,0);
				--recCount;
				if(recCount==0)
				{
					recCount = ACQUIRE_COUNT;
					++ autoCalibrationState;
				}
				break;
			case 8 :
				autoCalibration = 0;
				break;
		}
	}
	else
	{
		double dx = TargetPosX - xf;
		double dy = TargetPosY - yf ;
		double l2norm = dx*dx+dy*dy;
		if( (fabs(dx)+fabs(dy)) < 0.001 ) { return; }
		
		double servo_dx = dx / DXPosX;
		double servo_dy = dy / DYPosY;
		
		ServoRefX += servo_dx*0.1;
		ServoRefY += servo_dy*0.1;
		
		if( ServoRefX < 0.0 ) ServoRefX=0.0;
		else if( ServoRefX > 1.0 ) ServoRefX=1.0;

		if( ServoRefY < 0.0 ) ServoRefY=0.0;
		else if( ServoRefY > 1.0 ) ServoRefY=1.0;
		gpio_write_values( ServoRefX, ServoRefY, l2norm<0.001 ? 1 : 0 );
	}
}


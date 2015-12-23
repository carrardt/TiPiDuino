#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"
#include "fleye/compiledshader.h"
#include "thirdparty/bcm2835.h"

#define SERVO_X_VALUE_MIN 250
#define SERVO_X_VALUE_MAX 750

#define SERVO_Y_VALUE_MIN 250
#define SERVO_Y_VALUE_MAX 750

static int autoCalibration = 0;

static double servoStateX = 0.5;
static double servoStateY = 0.5;

static double targetPrevX = 0.5;
static double targetPrevY = 0.5;
static double targetPosX = 0.5;
static double targetPosY = 0.5;
static double targetSpeedX = 0.0;
static double targetSpeedY = 0.0;
static double targetDirX = 0.0;
static double targetDirY = 0.0;

static double laserPosX = 0.5;
static double laserPosY = 0.5;

static int autoCalibrationStateOld = -1;
static int autoCalibrationState = 0;
static int counter = 0;

static struct timeval prevTime={0,0};

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

struct gpioPanTiltFollower : public FleyePlugin
{
	void setup(FleyeContext* ctx)
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

		//autoCalibration = 1;

	  gpio_write_values(0.5,0.5,0);
	  printf("GPIO pan/tilt follower ready\n");

	  gettimeofday(&prevTime,NULL);
	}

#define INIT_COUNT 256
#define ACQUIRE_COUNT 128
	void run(FleyeContext* ctx)
	{
		CpuWorkerState* state = & ctx->ip->cpu_tracking_state;

		struct timeval T2;
		gettimeofday(&T2,NULL);
		double deltaT = (T2.tv_sec-prevTime.tv_sec)*1000.0 + (T2.tv_usec-prevTime.tv_usec)*0.001;
		prevTime = T2;

		if( state->objectCount < 1 ) return;

		double xf = ( state->objectCenter[0][0] - 0.5 ) * 2.0;
		double yf = ( state->objectCenter[0][1] - 0.5 ) * 2.0;
		double dist2 = xf*xf + yf*yf;

		laserPosX = ( state->objectCenter[1][0] - 0.5 ) * 2.0;
		laserPosY = ( state->objectCenter[1][1] - 0.5 ) * 2.0;

		if( autoCalibration )
		{		
			if( autoCalibrationState != autoCalibrationStateOld )
			{
				printf("calibration step %d\n",autoCalibrationState);
				autoCalibrationStateOld = autoCalibrationState;
			}
			
			switch( autoCalibrationState )
			{
				case 0 :
					if( dist2 > 0.0001 )
					{
						targetPosX = xf;
						targetPosY = yf;
						laserPosX = 0.0;
						laserPosY = 0.0;
						gpio_write_values(0.5,0.5,(counter++)%2);
					}
					else
					{
						gpio_write_values(0.5,0.5,0);
						counter = 0;
						targetPosX = 0.0;
						targetPosY = 0.0;
						++ autoCalibrationState;
					}
					break;
					
				case 1 :
					targetPosX += xf;
					targetPosY += yf;
					++ counter;
					if(counter==32)
					{
						targetPosX /= counter;
						targetPosY /= counter;
						++ autoCalibrationState;
						counter=0;
					}
					break;

				case 2 :
					autoCalibration = 0;
					break;
			}
		}
		else
		{
			const double MaxUncertainty = 1000.0;
			double Dx = (xf-targetPrevX) / deltaT;
			double Dy = (yf-targetPrevY) / deltaT;
			double d2 = Dx*Dx+Dy*Dy;
			double d = sqrt(d2);
			double motionStability = 0.0;
			double Nx = 0.0;
			double Ny = 0.0;
			if( d > 1e-8 )
			{
				Nx = Dx / d;
				Ny = Dy / d;
				double dirStability = ( Nx * targetDirX + Ny * targetDirY );
				double Ax = ( Dx - targetSpeedX ) / deltaT;
				double Ay = ( Dy - targetSpeedY ) / deltaT;
				double accelNorm = sqrt(Ax*Ax+Ay*Ay);
				// motionStability = ( M_PI - angle ) * d;
				// printf("instability=%5.5f, angle=%5.5f, accel=%5.5f,\n",motionUncertainty,angle,accel);
			}
			else
			{
				
			}
			
			double prevCoef = 1000.0;
			
			targetPosX = xf;
			targetPosY = yf;
			
			
			targetSpeedX = Dx;
			targetSpeedY = Dy;
			targetDirX	 = Nx;
			targetDirY	 = Ny;
			targetPrevX = targetPosX;
			targetPrevY = targetPosY;
			
			
			/*
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
			* */
		}
	}
};

FLEYE_REGISTER_PLUGIN(gpioPanTiltFollower);

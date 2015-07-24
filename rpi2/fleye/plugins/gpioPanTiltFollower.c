#include <stdio.h>
#include <math.h>
#include "../cpu_tracking.h"
#include "../RaspiTex.h"

#include "../thirdparty/bcm2835.h"
#include "../thirdparty/bcm2835.c"

#define SERVO_X_VALUE_MIN 250
#define SERVO_X_VALUE_MAX 750

#define SERVO_Y_VALUE_MIN 250
#define SERVO_Y_VALUE_MAX 750

static int autoCalibration = 1;

static double ServoRefX = 0.5;
static double ServoRefY = 0.25;

static double ServoDeltaX = 0.125;
static double ServoDeltaY = 0.125;

static double TargetPosX = 0.5;
static double TargetPosY = 0.5;

static double refPosX=0.0, refPosY=0.0;
static double DXPosX=0.0, DXPosY=0.0;
static double DYPosX=0.0, DYPosY=0.0;

static int autoCalibrationStateOld = -1;
static int autoCalibrationState = 0;
static int counter = 0;

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

	autoCalibration = 1;

	ServoRefX = 0.5;
	ServoRefY = 0.5;
	ServoDeltaX = 0.125;
	ServoDeltaY = 0.125;
	TargetPosX = 0.5;
	TargetPosY = 0.5;
	refPosX=0.0; refPosY=0.0;
	DXPosX=0.0; DXPosY=0.0;
	DYPosX=0.0; DYPosY=0.0;

  gpio_write_values(ServoRefX,ServoRefY,0);
  printf("GPIO pan/tilt follower ready\n");
}

#define INIT_COUNT 256
#define ACQUIRE_COUNT 128
void gpioPanTiltFollower_run(CPU_TRACKING_STATE * state)
{
	if( state->objectCount < 1 ) return;

	double xf = ( state->objectCenter[0][0] - 0.5 ) * 2.0;
	double yf = ( state->objectCenter[0][1] - 0.5 ) * 2.0;
	double dist2 = xf*xf + yf*yf;

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
					refPosX = xf;
					refPosY = yf;
					state->objectCenter[0][0] = 0.5;
					state->objectCenter[0][1] = 0.5;
					gpio_write_values(ServoRefX,ServoRefY,(counter++)%2);
				}
				else
				{
					gpio_write_values(ServoRefX,ServoRefY,0);
					counter = 0;
					++ autoCalibrationState;
					refPosX = 0.0;
					refPosY = 0.0;
				}
				break;
				
			case 1 :
				refPosX += xf;
				refPosY += yf;
				++ counter;
				if(counter==32)
				{
					refPosX /= counter;
					refPosY /= counter;
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
		refPosX = xf;
		refPosY = yf;
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

void drawOverlay(struct RASPITEX_STATE* state,CompiledShaderCache* compiledShader, int pass)
{
	if( autoCalibrationState==0 || !autoCalibration )
	{
		GLfloat varray[12] = { -0.1,-0.1,0, 0.1,0.1,0, -0.1,0.1,0, 0.1,-0.1,0 };
		int i;
		for(i=0;i<4;i++)
		{
			varray[i*3+0] += refPosX;
			varray[i*3+1] += refPosY;
		}

		glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
		glVertexAttribPointer(compiledShader->shader.attribute_locations[0], 3, GL_FLOAT, GL_FALSE, 0, varray);
		glDrawArrays(GL_LINES, 0, 4);
		glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
	}
}

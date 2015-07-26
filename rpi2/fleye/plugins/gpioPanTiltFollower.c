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

static double servoStateX = 0.5;
static double servoStateY = 0.5;

static double targetPrevX = 0.5;
static double targetPrevY = 0.5;
static double targetPosX = 0.5;
static double targetPosY = 0.5;

static double laserPosX = 0.5;
static double laserPosY = 0.5;

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

  gpio_write_values(0.5,0.5,0);
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
		const double minCoef = 0.0001;
		const double maxSqDist = 0.005;
		double Dx = (xf-targetPrevX);
		double Dy = (yf-targetPrevY);
		double c = Dx*Dx+Dy*Dy;
		if( c < minCoef ) c = minCoef;
		double nf = 1.0 / (c+minCoef);
		if( c < maxSqDist )
		{
			targetPosX = (targetPrevX*minCoef + xf*c) * nf;
			targetPosY = (targetPrevY*minCoef + yf*c) * nf;
		}
		else
		{
			targetPosX = xf;
			targetPosY = yf;
		}
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



void drawOverlay(struct RASPITEX_STATE* state,CompiledShaderCache* compiledShader, int pass)
{
	if( autoCalibrationState==0 || !autoCalibration )
	{
		GLfloat varray[24];
		int i;
		for(i=0;i<4;i++)
		{
			int x = i%2;
			int y = ((i/2)+x)%2;
			double ox = x ? -0.05 : 0.05;
			double oy = y ? -0.05 : 0.05;
			varray[i*3+0] = targetPosX +ox;
			varray[i*3+1] = targetPosY +oy;
			varray[i*3+2] = 0.333;
		}
		for(i=4;i<8;i++)
		{
			int x = i%2;
			int y = ((i/2)+x)%2;
			double ox = x ? -0.05 : 0.05;
			double oy = y ? -0.05 : 0.05;
			varray[i*3+0] = laserPosX+ox;
			varray[i*3+1] = laserPosY+oy;
			varray[i*3+2] = 0.0;
		}

		glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
		glVertexAttribPointer(compiledShader->shader.attribute_locations[0], 3, GL_FLOAT, GL_FALSE, 0, varray);
		glDrawArrays(GL_LINES, 0, 8);
		glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
	}
}

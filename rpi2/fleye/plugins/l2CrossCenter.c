#include <stdio.h>
#include "../cpu_tracking.h"

void l2CrossCenter_setup()
{
	printf("L2CrossCenter plugin ready\n");
}

void l2CrossCenter_run(CPU_TRACKING_STATE * state)
{
	const uint32_t* base_ptr = (uint32_t*) state->image;
	int x,y;
	int obj1_sumx=0,obj1_sumy=0;
	int obj2_sumx=0,obj2_sumy=0;
	int obj1_count=0, obj2_count=0;
	int obj1_L2max=1, obj2_L2max=1;
	for(y=0;y<state->height;y++)
	{
		const uint32_t* p = base_ptr + y * state->width;
		for(x=0;x<state->width;x++)
		{
			uint32_t value = *(p++);
			uint32_t mask1 = ( value & 0x00008080 ) ;
			uint32_t mask2 = ( value & 0x80800000 ) ;
			if( mask1 )
			{
				int r1 = ( value & 0x0000007F ) >> 4;
				int u1 = ( value & 0x00007F00 ) >> 12;

				int m1 = (r1>u1) ? r1 : u1;

				if( m1>obj1_L2max ) { obj1_count = obj1_sumx = obj1_sumy = 0; obj1_L2max=m1; }
				else if( m1==obj1_L2max )
				{
					obj1_sumx += x;
					if( r1 >= 1 ) obj1_sumx += 1<<(r1-1);
					obj1_sumy += y;
					if( u1 >= 1 ) obj1_sumy += 1<<(u1-1);
					++ obj1_count;
				}
			}
			if( mask2 )
			{
				int r2 = ( value & 0x007F0000 ) >> 20;
				int u2 = ( value & 0x7F000000 ) >> 28;
				int m2 = (r2>u2) ? r2 : u2;
				if( m2>obj2_L2max ) { obj2_count = obj2_sumx = obj2_sumy = 0; obj2_L2max=m2; }
				else if( m2==obj2_L2max )
				{
					obj2_sumx += x;
					if( r2 >= 1 ) obj2_sumx += 1<<(r2-1);
					obj2_sumy += y;
					if( u2 >= 1 ) obj2_sumy += 1<<(u2-1);
					++ obj2_count;
				}
			}
		}
	}

	state->objectCount = 0;
	
	if(obj1_count>0)
	{
		//printf("%d %fx%f\n",count,state->width,state->height);
		state->objectCenter[state->objectCount][0] = (double)obj1_sumx / (double)( obj1_count * state->width );
		state->objectCenter[state->objectCount][1] = (double)obj1_sumy / (double)( obj1_count * state->height );
		//printf("%f, %f\n",state->objectCenter[0][0],state->objectCenter[0][1]);
		state->trackedObjects[ state->objectCount ++ ] = 0;
	}

	if(obj2_count>0)
	{
		//printf("%d %fx%f\n",count,state->width,state->height);
		state->objectCenter[state->objectCount][0] = (double)obj2_sumx / (double)( obj2_count * state->width );
		state->objectCenter[state->objectCount][1] = (double)obj2_sumy / (double)( obj2_count * state->height );
		//printf("%f, %f\n",state->objectCenter[0][0],state->objectCenter[0][1]);
		state->trackedObjects[ state->objectCount ++ ] = 1;
	}
}

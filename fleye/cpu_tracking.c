#include "cpu_tracking.h"
#include <stdio.h>

void *cpuTrackingWorker(void *arg)
{
	CPU_TRACKING_STATE * state = (CPU_TRACKING_STATE *) arg;
	
	while( state->do_processing )
	{
		vcos_semaphore_wait( & state->start_processing_sem );
		/*
		const GLubyte* p = state->image;
		int x,y;
		int sumx=0,sumy=0;
		int count=0;
		int L2max=1;
		for(y=0;y<state->height;y++)
		{
			for(x=0;x<state->width;x++)
			{
				int mask = ( (*p) & 0x80 ) ;
				int l = ( (*p++) & 0x7F ) >> 4;
				int r = ( (*p++) & 0x7F ) >> 4;
				int b = ( (*p++) & 0x7F ) >> 4;
				int u = ( (*p++) & 0x7F ) >> 4;
				if( mask )
				{
					int h = (l<r) ? l : r;
					int v = (b<u) ? b : u;
					int m = (h<v) ? h : v;
					if( m>L2max ) { count=sumx=sumy=0; L2max=m; }
					else if( m==L2max )
					{
						sumx += x;
						sumy += y;
						++count;
					}
				}
			}
		}

		if(count>0)
		{
			state->objectCount = 1;
			state->objectCenter[0][0] = (double)sumx / (double)( count * state->width );
			state->objectCenter[0][1] = (double)sumy / (double)( count * state->height );
			//printf("%f , %f\n",state->objectCenter[0][0],state->objectCenter[0][1]);
		}
		else
		{
			state->objectCount = 0;
			//printf("vide\n");
		}
		* */
		vcos_semaphore_post( & state->end_processing_sem );
		
		// ici, transmission des données au mcu
	}
	
	return NULL;
}

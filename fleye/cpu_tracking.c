#include "cpu_tracking.h"
#include "RaspiTex.h"
#include <stdio.h>

void *cpuTrackingWorker(void *arg)
{
	CPU_TRACKING_STATE * state = (CPU_TRACKING_STATE *) arg;
	
	fprintf(stderr,"cpuTrackingWorker started: %dx%d\n",state->width,state->height); fflush(stderr);
	
	vcos_semaphore_wait( & state->start_processing_sem );
	
	while( state->do_processing )
	{
		if( state->cpu_processing!=0)
		{
			( * state->cpu_processing )( state );
		}

		vcos_semaphore_post( & state->end_processing_sem );
		
		// wait signal to start next procesing 
		vcos_semaphore_wait( & state->start_processing_sem );
	}
	
	return NULL;
}

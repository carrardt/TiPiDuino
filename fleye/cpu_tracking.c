#include "cpu_tracking.h"
#include <stdio.h>

void *cpuTrackingWorker(void *arg)
{
	CPU_TRACKING_STATE * state = (CPU_TRACKING_STATE *) arg;
	
	printf("cpuTrackingWorker started: %dx%d\n",state->width,state->height);
	state->cpuFunc = 0;
	
	vcos_semaphore_wait( & state->start_processing_sem );
	
	while( state->do_processing )
	{
		if( state->cpu_processing[ state->cpuFunc ] !=0 )
		{
			( * state->cpu_processing[ state->cpuFunc ] )( state );
			
			// signal that one more task has finished
			vcos_semaphore_post( & state->end_processing_sem );
			
			// step to the next function to execute
			++ state->cpuFunc;
		}
		else
		{
			state->cpuFunc = 0;
		}

		// wait signal to start next procesing 
		vcos_semaphore_wait( & state->start_processing_sem );
	}
	
	return NULL;
}

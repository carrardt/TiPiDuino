#include <stdio.h>
#include "fleye/cpuworker.h"
#include "fleye/fleye_c.h"
#include "fleye/imageprocessing.h"

void *cpuTrackingWorker(void *arg)
{
	ImageProcessingState * ip = (ImageProcessingState *) arg;
	CPU_TRACKING_STATE* state = & ip->cpu_tracking_state;
	
	printf("cpuTrackingWorker started\n");
	state->cpuFunc = 0;
	
	//vcos_semaphore_wait( & state->start_processing_sem );
	waitStartProcessingSem( state->fleye_state );
	
	while( state->do_processing )
	{
		if( state->cpu_processing[ state->cpuFunc ] !=0 )
		{
			( * state->cpu_processing[ state->cpuFunc ] )( ip, state );
			
			// signal that one more task has finished
			postEndProcessingSem( state->fleye_state );
			//vcos_semaphore_post( & state->end_processing_sem );
			
			// step to the next function to execute
			++ state->cpuFunc;
		}
		else
		{
			state->cpuFunc = 0;
		}

		// wait signal to start next procesing 
		waitStartProcessingSem( state->fleye_state );
		//vcos_semaphore_wait( & state->start_processing_sem );
	}
	
	return NULL;
}

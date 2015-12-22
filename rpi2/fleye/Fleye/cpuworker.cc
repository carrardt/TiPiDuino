#include <stdio.h>
#include "fleye/cpuworker.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"

void *cpuWorker(void *arg)
{
	FleyeContext * ctx = (FleyeContext *) arg;
	CPU_TRACKING_STATE* state = & ctx->ip->cpu_tracking_state;
	
	printf("cpuTrackingWorker started\n");
	state->cpuFunc = 0;
	
	//vcos_semaphore_wait( & state->start_processing_sem );
	waitStartProcessingSem( ctx );
	
	while( state->do_processing )
	{
		if( state->cpu_processing[ state->cpuFunc ] !=0 )
		{
			( * state->cpu_processing[ state->cpuFunc ] )( ctx );
			
			// signal that one more task has finished
			postEndProcessingSem( ctx );
			//vcos_semaphore_post( & state->end_processing_sem );
			
			// step to the next function to execute
			++ state->cpuFunc;
		}
		else
		{
			state->cpuFunc = 0;
		}

		// wait signal to start next procesing 
		waitStartProcessingSem( ctx );
		//vcos_semaphore_wait( & state->start_processing_sem );
	}
	
	return NULL;
}

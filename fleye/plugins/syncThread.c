#include <stdio.h>
#include "../cpu_tracking.h"

void syncThread_setup()
{
	printf("syncThread plugin ready\n");
}

void syncThread_run(CPU_TRACKING_STATE * state)
{
	int nToWait = state->nAvailCpuFuncs - state->nFinishedCpuFuncs;
	while( nToWait > 0 )
	{
		vcos_semaphore_wait( & state->end_processing_sem );
		-- nToWait;
	}
	state->nFinishedCpuFuncs = state->nAvailCpuFuncs; 
}

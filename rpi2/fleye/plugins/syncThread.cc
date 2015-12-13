#include <stdio.h>

#include "fleye/cpuworker.h"
#include "fleye/fleye_c.h"
#include "fleye/plugin.h"

FLEYE_REGISTER_PLUGIN(syncThread)

void syncThread_setup()
{
	printf("syncThread plugin ready\n");
}

void syncThread_run(CPU_TRACKING_STATE * state)
{
	int nToWait = state->nAvailCpuFuncs - state->nFinishedCpuFuncs;
	while( nToWait > 0 )
	{
		waitEndProcessingSem( state->fleye_state );
		// vcos_semaphore_wait( & state->end_processing_sem );
		-- nToWait;
	}
	state->nFinishedCpuFuncs = state->nAvailCpuFuncs; 
}

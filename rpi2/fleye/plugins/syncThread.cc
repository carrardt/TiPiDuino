#include <stdio.h>

#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"

struct syncThread : public FleyePlugin
{
	void syncThread_setup(FleyeContext* ctx)
	{
		printf("syncThread plugin ready\n");
	}

	void syncThread_run(FleyeContext* ctx)
	{
		CpuWorkerState * state = & ctx->ip->cpu_tracking_state;
		
		int nToWait = state->nAvailCpuFuncs - state->nFinishedCpuFuncs;
		while( nToWait > 0 )
		{
			waitEndProcessingSem( ctx );
			// vcos_semaphore_wait( & state->end_processing_sem );
			-- nToWait;
		}
		state->nFinishedCpuFuncs = state->nAvailCpuFuncs; 
	}
};

FLEYE_REGISTER_PLUGIN(syncThread);

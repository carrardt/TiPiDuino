#include "cpu_tracking.h"
#include "RaspiTex.h"
#include "gpio.h"
#include <stdio.h>

void *cpuTrackingWorker(void *arg)
{
	RASPITEX_STATE * global_state = (RASPITEX_STATE *) arg;
	CPU_TRACKING_STATE * state = & global_state->cpu_tracking_state;
	
	fprintf(stderr,"cpuTrackingWorker started: %dx%d\n",state->width,state->height); fflush(stderr);
	
	vcos_semaphore_wait( & state->start_processing_sem );	
	while( state->do_processing )
	{
		if(global_state->imageProcessing!=0 && global_state->imageProcessing->cpu_processing!=0)
		{
			(* global_state->imageProcessing->cpu_processing)( state );
		}

		vcos_semaphore_post( & state->end_processing_sem );
		
		// ici, transmission des données au mcu
		if( state->objectCount > 0 )
		{
			gpio_write_xy_f(state->objectCenter[0][0], state->objectCenter[0][1], 0);
		}

		// wait signal to start next procesing 
		vcos_semaphore_wait( & state->start_processing_sem );
	}
	
	return NULL;
}

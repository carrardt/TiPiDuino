#ifndef __fleye_ImageProcessing_H_
#define __fleye_ImageProcessing_H_

#include <EGL/egl.h>
#include "fleye/config.h"
#include "fleye/texture.h"
#include "fleye/shaderprogram.h"
#include "fleye/fbo.h"
#include "fleye/processingstep.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ImageProcessingState
{
	int nProcessingSteps;
	ProcessingStep processing_step[IMGPROC_MAX_STEPS];
	int nTextures;
	RASPITEX_Texture processing_texture[MAX_TEXTURES+IMGPROC_MAX_STEPS];
	int nFBO;
	RASPITEX_FBO processing_fbo[MAX_FBOS];
	VCOS_THREAD_T cpuTrackingThread;
	CPU_TRACKING_STATE cpu_tracking_state;
};

#ifdef __cplusplus
}
#endif

#endif

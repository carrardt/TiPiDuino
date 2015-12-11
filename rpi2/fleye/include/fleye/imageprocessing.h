#ifndef __fleye_ImageProcessing_H_
#define __fleye_ImageProcessing_H_

#include <EGL/egl.h>
#include "fleye/config.h"
#include "fleye/texture.h"
#include "fleye/fbo.h"
#include "fleye/processingstep.h"

struct UserEnv;

#ifdef __cplusplus
extern "C" {
#endif

struct ImageProcessingState
{
	int nProcessingSteps;
	struct ProcessingStep processing_step[IMGPROC_MAX_STEPS];
	int nTextures;
	struct RASPITEX_Texture processing_texture[MAX_TEXTURES+IMGPROC_MAX_STEPS];
	int nFBO;
	struct FrameBufferObject processing_fbo[MAX_FBOS];
	VCOS_THREAD_T cpuTrackingThread;
	struct CPU_TRACKING_STATE cpu_tracking_state;
};

extern int create_image_processing(struct ImageProcessingState* ip, struct UserEnv* env, const char* filename);

#ifdef __cplusplus
}
#endif

#endif

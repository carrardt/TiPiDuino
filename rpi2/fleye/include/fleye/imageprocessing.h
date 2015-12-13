#ifndef __fleye_ImageProcessing_H_
#define __fleye_ImageProcessing_H_

#include <GLES2/gl2.h>
#include "fleye/config.h"
#include "fleye/texture.h"
#include "fleye/processingstep.h"
#include "fleye/fbo.h"

struct ImageProcessingState
{
	GLuint cameraTextureId;
	int nProcessingSteps;
	struct ProcessingStep processing_step[IMGPROC_MAX_STEPS];
	int nTextures;
	struct RASPITEX_Texture processing_texture[MAX_TEXTURES+IMGPROC_MAX_STEPS];
	int nFBO;
	struct FrameBufferObject processing_fbo[MAX_FBOS];
	struct CPU_TRACKING_STATE cpu_tracking_state;
};

#endif

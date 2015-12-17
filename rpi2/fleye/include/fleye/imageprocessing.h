#ifndef __fleye_ImageProcessing_H_
#define __fleye_ImageProcessing_H_

#include <GLES2/gl2.h>
#include "fleye/config.h"
#include "fleye/texture.h"
#include "fleye/processingstep.h"
#include "fleye/fbo.h"
#include "fleye/cpuworker.h"

#include <string>
#include <map>
#include <vector>

struct ImageProcessingState
{
	GLuint cameraTextureId;
	CPU_TRACKING_STATE cpu_tracking_state;
	std::vector<ProcessingStep> processing_step;
	std::map<std::string,GLTexture*> texture;
	std::map<std::string,FrameBufferObject*> fbo;
};

#endif

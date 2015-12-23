#ifndef __fleye_ImageProcessing_H_
#define __fleye_ImageProcessing_H_

#include "fleye/config.h"
#include "fleye/processingstep.h"
#include "fleye/cpuworker.h"

#include <string>
#include <map>
#include <vector>

struct GLTexture;
struct FrameBufferObject;
struct FleyeCommonState;
struct UserEnv;
struct FleyeRenderWindow;

struct ImageProcessingState
{
	CpuWorkerState cpu_tracking_state;
	std::vector<ProcessingStep> processing_step;
	std::map<std::string,GLTexture*> texture;
	std::map<std::string,FrameBufferObject*> fbo;
	
	FleyeRenderWindow* getRenderBuffer(const std::string& name) const;
};

int read_image_processing_script(FleyeContext* ctx);

#endif

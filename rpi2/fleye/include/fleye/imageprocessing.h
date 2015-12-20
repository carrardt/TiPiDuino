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
	CPU_TRACKING_STATE cpu_tracking_state;
	std::vector<ProcessingStep> processing_step;
	std::map<std::string,GLTexture*> texture;
	std::map<std::string,FrameBufferObject*> fbo;
	
	FleyeRenderWindow* getRenderBuffer(const std::string& name) const;
};

int create_image_processing(FleyeCommonState* state,ImageProcessingState* ip, UserEnv* env, const std::string& filename);


#endif

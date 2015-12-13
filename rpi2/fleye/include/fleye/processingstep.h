#ifndef __fleye_ProcessingStep_H_
#define __fleye_ProcessingStep_H_

#include "fleye/cpuworker.h"
#include "fleye/shaderpass.h"

struct FleyeState;
struct CompiledShaderCache;

struct ProcessingStep
{
	int exec_thread; // 0=main thread, 1=async thread, -1=not a cpu pass (gpu shader)
	int numberOfPasses; 
	ShaderPass shaderPass;
	void(*gl_draw)(struct CompiledShaderCache*,int);
	void(*cpu_processing)(CPU_TRACKING_STATE*);
};

#endif

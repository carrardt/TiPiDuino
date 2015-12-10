#ifndef __fleye_ProcessingStep_H_
#define __fleye_ProcessingStep_H_

#include "fleye/cpuworker.h"
#include "fleye/shaderpass.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RASPITEX_STATE;

struct ProcessingStep
{
	int exec_thread; // 0=main thread, 1=async thread, -1=not a cpu pass (gpu shader)
	int numberOfPasses; 
	ShaderPass shaderPass;
	void(*gl_draw)(struct CompiledShaderCache*,int);
	void(*cpu_processing)(CPU_TRACKING_STATE*);
};

#ifdef __cplusplus
}
#endif

#endif

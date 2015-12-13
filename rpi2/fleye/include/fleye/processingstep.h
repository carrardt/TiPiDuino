#ifndef __fleye_ProcessingStep_H_
#define __fleye_ProcessingStep_H_

#include "fleye/cpuworker.h"
#include "fleye/shaderpass.h"

struct FleyeState;
struct CompiledShaderCache;

typedef void(*GLRenderFunctionT)(struct CompiledShaderCache*,int) ;

struct ProcessingStep
{
	int exec_thread; // 0=main thread, 1=async thread, -1=not a cpu pass (gpu shader)
	int numberOfPasses; 
	ShaderPass shaderPass;
	GLRenderFunctionT gl_draw; //void(*gl_draw)(struct CompiledShaderCache*,int);
	CpuProcessingFunc cpu_processing; 
};

#endif

#ifndef __fleye_ProcessingStep_H_
#define __fleye_ProcessingStep_H_

#include "fleye/cpuworker.h"

struct CpuPass
{
	CpuProcessingFunc cpu_processing;
	int exec_thread; // 0=main thread, 1=async thread, etc.
	inline CpuPass() : exec_thread(0), cpu_processing(0) {}
};

struct ShaderPass;

struct ProcessingStep
{
	ShaderPass * shaderPass;
	CpuPass * cpuPass;
	inline ProcessingStep() :  shaderPass(0), cpuPass(0) {}
};

#endif

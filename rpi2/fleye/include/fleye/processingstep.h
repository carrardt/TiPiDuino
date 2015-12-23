#ifndef __fleye_ProcessingStep_H_
#define __fleye_ProcessingStep_H_

#include "fleye/plugin.h"
#include "fleye/cpuworker.h"

struct CpuPass
{
	FleyePlugin* cpu_processing;
	int exec_thread; // 0=main thread, 1=async thread, etc.
	inline CpuPass() : exec_thread(0), cpu_processing(0) {}
};

struct FrameSet
{
	int32_t modulus;
	int32_t value;
	inline FrameSet() : modulus(1), value(0) {}
	inline FrameSet(int32_t m, int32_t v) : modulus(m), value(v) {}
	inline bool contains(int32_t frame) const { return (frame%modulus)==value; }
};

struct ShaderPass;

struct ProcessingStep
{
	FrameSet onFrames;
	ShaderPass * shaderPass;
	CpuPass * cpuPass;
	inline ProcessingStep() :  shaderPass(0), cpuPass(0) {}
};

#endif

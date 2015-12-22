#ifndef __FLEYE_CPU_TRACKING_H
#define __FLEYE_CPU_TRACKING_H

#include "fleye/config.h"
#include <stdint.h>

struct FleyeContext;

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*CpuProcessingFunc)(struct FleyeContext*);

typedef struct CPU_TRACKING_STATE
{
  volatile  int do_processing;

  // output
  volatile int objectCount;
  volatile int trackedObjects[MAX_TRACKED_OBJECTS];
  volatile float objectCenter[MAX_TRACKED_OBJECTS][2];
  volatile float objectArea[MAX_TRACKED_OBJECTS];
  volatile float objectLength[MAX_TRACKED_OBJECTS];
  
  volatile CpuProcessingFunc cpu_processing[IMGPROC_MAX_CPU_FUNCS];
  volatile int nAvailCpuFuncs;
  volatile int nFinishedCpuFuncs;
  volatile int cpuFunc;

} CPU_TRACKING_STATE;

// entry point for cpu processing thread
void *cpuWorker(void *arg);

#ifdef __cplusplus
}
#endif


#endif

#ifndef __FLEYE_CPU_TRACKING_H
#define __FLEYE_CPU_TRACKING_H

#include "fleye/config.h"
#include "fleye/plugin.h"
#include <stdint.h>

struct FleyeContext;

typedef struct CpuWorkerState
{
  // output
  int objectCount;
  int trackedObjects[MAX_TRACKED_OBJECTS];
  float objectCenter[MAX_TRACKED_OBJECTS][2];
  float objectArea[MAX_TRACKED_OBJECTS];
  float objectLength[MAX_TRACKED_OBJECTS];
  
  FleyePlugin* cpu_processing[IMGPROC_MAX_CPU_FUNCS];

  volatile int do_processing;
  volatile int nAvailCpuFuncs;
  volatile int nFinishedCpuFuncs;
  volatile int cpuFunc;

} CpuWorkerState;

// entry point for cpu processing thread
void *cpuWorker(void *arg);

#endif

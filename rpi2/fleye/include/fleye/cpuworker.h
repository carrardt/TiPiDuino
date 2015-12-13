#ifndef __FLEYE_CPU_TRACKING_H
#define __FLEYE_CPU_TRACKING_H

#include "fleye/config.h"
#include <stdint.h>

struct CPU_TRACKING_STATE;
struct FleyeState;

typedef void(*CpuProcessingFunc)(struct CPU_TRACKING_STATE*);

typedef struct CPU_TRACKING_STATE
{
  struct FleyeState* fleye_state;
  
  // input
  int width, height;
  uint8_t* image;

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

#endif

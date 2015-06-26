#ifndef __FLEYE_CPU_TRACKING_H
#define __FLEYE_CPU_TRACKING_H

#include <GLES2/gl2.h>
#include "interface/vcos/vcos.h"

#define MAX_TRACKED_OBJECTS 8
#define IMGPROC_MAX_CPU_FUNCS 16

struct CPU_TRACKING_STATE;

typedef void(*CpuProcessingFunc)(struct CPU_TRACKING_STATE*);

typedef struct CPU_TRACKING_STATE
{
  VCOS_SEMAPHORE_T start_processing_sem;
  VCOS_SEMAPHORE_T end_processing_sem;
  volatile  int do_processing;

  // input
  volatile int width, height;
  volatile GLubyte* image;

  // output
  volatile int objectCount;
  volatile float objectCenter[MAX_TRACKED_OBJECTS][2];
  volatile float objectArea[MAX_TRACKED_OBJECTS];
  volatile float objectLength[MAX_TRACKED_OBJECTS];
  
  volatile CpuProcessingFunc cpu_processing[IMGPROC_MAX_CPU_FUNCS];
  volatile int nCpuFuncs;
  volatile int cpuFunc;

} CPU_TRACKING_STATE;

extern void *cpuTrackingWorker(void *arg);

#endif

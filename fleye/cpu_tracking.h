#ifndef __FLEYE_CPU_TRACKING_H
#define __FLEYE_CPU_TRACKING_H

#include <GLES2/gl2.h>
#include "interface/vcos/vcos.h"

#define MAX_TRACKED_OBJECTS 8

typedef struct CPU_TRACKING_STATE
{
  VCOS_SEMAPHORE_T start_processing_sem;
  VCOS_SEMAPHORE_T end_processing_sem;
  int do_processing;

  // input
  int width, height;
  GLubyte* image;

  // output
  int objectCount;
  float objectCenter[MAX_TRACKED_OBJECTS][2];
  float objectArea[MAX_TRACKED_OBJECTS];
  float objectLength[MAX_TRACKED_OBJECTS];

} CPU_TRACKING_STATE;

extern void *cpuTrackingWorker(void *arg);

#endif

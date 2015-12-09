#include <GLES2/gl2.h>
#include <stdio.h>

#include "fleye/cpuworker.h"

void glReadBack_setup()
{
	printf("glReadBack plugin ready\n");
}

void glReadBack_run(CPU_TRACKING_STATE * state)
{
	glReadPixels(0, 0, state->width, state->height,GL_RGBA,GL_UNSIGNED_BYTE, state->image);
}

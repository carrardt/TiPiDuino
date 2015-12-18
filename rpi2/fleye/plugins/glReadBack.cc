#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>

#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/config.h"
#include "fleye/fleye_c.h"

FLEYE_REGISTER_PLUGIN(glReadBack)

void glReadBack_setup()
{
	printf("glReadBack plugin ready\n");
}

void glReadBack_run(CPU_TRACKING_STATE * state)
{
	fleye_readback( state->fleye_state, state->image );
	
	//GLCHK( glReadPixels(0, 0, state->width, state->height,GL_RGBA,GL_UNSIGNED_BYTE, state->image) );
	//GLCHK( glReadPixels(0, 0, state->width, state->height,GL_RGB_422_APPLE,GL_UNSIGNED_SHORT, state->image) );
}

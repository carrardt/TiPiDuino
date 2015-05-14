/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

static char* readFile(const char* fileName)
{
	char filePath[256];
	snprintf(filePath,255,"%s/%s",GLSL_SRC_DIR,fileName);
	FILE* fp=fopen(filePath,"rb");
	if(fp==0)
	{
		vcos_log_error("Can't open file %s\n",filePath);
		return 0;
	}
	fseek(fp,0,SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	vcos_log_trace("file size is %d\n",fsize);
	char* buf = (char*) malloc(fsize+1);
	fread(buf,fsize,1,fp);
	buf[fsize]='\0';
	fclose(fp);
	return buf;
}

/* Draw a scaled quad showing the the entire texture with the
 * origin defined as an attribute */
static RASPITEXUTIL_SHADER_PROGRAM_T tracking_shader;

static GLfloat varray[] =
{
   -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
   1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
};

static const EGLint tracking_egl_config_attribs[] =
{
   EGL_RED_SIZE,   8,
   EGL_GREEN_SIZE, 8,
   EGL_BLUE_SIZE,  8,
   EGL_ALPHA_SIZE, 8,
   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
   EGL_NONE
};

static int tracking_init(RASPITEX_STATE *state)
{
   int rc;
    state->egl_config_attribs = tracking_egl_config_attribs;
    rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;

	memset(&tracking_shader,0,sizeof(RASPITEXUTIL_SHADER_PROGRAM_T));
	tracking_shader.vertex_source = readFile("masking_vs.glsl");
	tracking_shader.fragment_source = readFile("masking_fs.glsl");
	tracking_shader.attribute_names[0] = "vertex";
	tracking_shader.uniform_names[0] = "tex";
	tracking_shader.uniform_names[1] = "inv_width";
	tracking_shader.uniform_names[2] = "inv_height";
	
	//vcos_log_trace(tracking_shader.fragment_source);
	
    rc = raspitexutil_build_shader_program(&tracking_shader);
    GLCHK(glUseProgram(tracking_shader.program));
    GLCHK(glUniform1i(tracking_shader.uniform_locations[0], 0)); // tex unit
    if( tracking_shader.uniform_locations[1] != -1 )
    {
		GLCHK(glUniform1f(tracking_shader.uniform_locations[1], 1.0 / state->width));
	}
    if( tracking_shader.uniform_locations[2] != -1 )
    {
		GLCHK(glUniform1f(tracking_shader.uniform_locations[2], 1.0 / state->height)); // tex height
	}
end:
    return rc;
}

static int tracking_redraw(RASPITEX_STATE *raspitex_state)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLCHK(glUseProgram(tracking_shader.program));
    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glEnableVertexAttribArray(tracking_shader.attribute_locations[0]));
    GLCHK(glVertexAttribPointer(tracking_shader.attribute_locations[0],
             2, GL_FLOAT, GL_FALSE, 0, varray));

    // Y plane
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));
    //GLCHK(glVertexAttrib2f(tracking_shader.attribute_locations[1], -1.0f, 1.0f));
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    GLCHK(glDisableVertexAttribArray(tracking_shader.attribute_locations[0]));
    GLCHK(glUseProgram(0));
    return 0;
}

int tracking_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = tracking_init;
   state->ops.redraw = tracking_redraw;
   state->ops.update_texture = raspitexutil_update_texture;
   state->ops.update_y_texture = 0;
   state->ops.update_u_texture = 0;
   state->ops.update_v_texture = 0;
   return 0;
}

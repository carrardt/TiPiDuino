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

static RASPITEXUTIL_SHADER_PROGRAM_T masking_shader;
static RASPITEXUTIL_SHADER_PROGRAM_T postproc_shader;

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

typedef struct
{
	GLuint width, height; // dimensions
	GLuint format; // texture internal format
	GLuint tex; // color texture
	GLuint drb; // depth renderbuffer
	GLuint fb; // frame buffer object
} FBOTexture;

static FBOTexture mask_fbo;

static int init_fbo(FBOTexture* fbo, GLint colorFormat, GLint depthFormat, GLint w, GLint h)
{
   fbo->format = colorFormat;
   fbo->width = w;
   fbo->height = h;

   glGenFramebuffers(1, & fbo->fb);

   glGenTextures(1, & fbo->tex );
   glBindTexture(GL_TEXTURE_2D, fbo->tex);
   glTexImage2D(GL_TEXTURE_2D, 0, fbo->format, fbo->width, fbo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glBindTexture(GL_TEXTURE_2D, 0);

	if( depthFormat != GL_NONE )
	{
	   // Create a texture to hold the depth buffer
		glGenTextures(1, &(fbo->drb) );
		glBindTexture(GL_TEXTURE_2D, fbo->drb);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				fbo->width, fbo->height,
				0, depthFormat, GL_UNSIGNED_SHORT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);        
	}
	else
	{
		fbo->drb = 0;
	}

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);

    // Associate the textures with the FBO.

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, fbo->tex, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D, fbo->drb, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	
    if ( status == GL_FRAMEBUFFER_COMPLETE ) return 0;
    else return 1;
}

static int tracking_init(RASPITEX_STATE *state)
{
   int rc;
    state->egl_config_attribs = tracking_egl_config_attribs;
    rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
    {
		vcos_log_error("unable to init GLES2\n");
		return rc;
	}

	memset(&masking_shader,0,sizeof(RASPITEXUTIL_SHADER_PROGRAM_T));
	masking_shader.vertex_source = readFile("masking_vs.glsl");
	masking_shader.fragment_source = readFile("masking_fs.glsl");
	masking_shader.attribute_names[0] = "vertex";
	masking_shader.uniform_names[0] = "tex";
	masking_shader.uniform_names[1] = "xstep";
	masking_shader.uniform_names[2] = "ystep";
    rc = raspitexutil_build_shader_program(&masking_shader);
    GLCHK(glUseProgram(masking_shader.program));
    GLCHK(glUniform1i(masking_shader.uniform_locations[0], 0)); // tex unit
    if( masking_shader.uniform_locations[1] != -1 )
    {
		GLCHK(glUniform1f(masking_shader.uniform_locations[1], 1.0 / state->width));
	}
    if( masking_shader.uniform_locations[2] != -1 )
    {
		GLCHK(glUniform1f(masking_shader.uniform_locations[2], 1.0 / state->height)); // tex height
	}
	
	memset(&postproc_shader,0,sizeof(RASPITEXUTIL_SHADER_PROGRAM_T));
	postproc_shader.vertex_source = readFile("masking_vs.glsl");
	postproc_shader.fragment_source = readFile("postproc_fs.glsl");
	postproc_shader.attribute_names[0] = "vertex";
	postproc_shader.uniform_names[0] = "tex";
    rc = raspitexutil_build_shader_program(&postproc_shader);
    GLCHK(glUseProgram(postproc_shader.program));
    GLCHK(glUniform1i(postproc_shader.uniform_locations[0], 0)); // tex unit
	
	rc = init_fbo(&mask_fbo,GL_RGBA,GL_NONE,state->width/2,state->height/2);
    if (rc != 0)
    {
		vcos_log_error("FBO failed\n");
		return rc;
	}
	
	glDisable(GL_DEPTH_TEST);
	
    return rc;
}


static int tracking_redraw(RASPITEX_STATE *raspitex_state)
{
	glBindFramebuffer(GL_FRAMEBUFFER, mask_fbo.fb);
	
    //glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

    GLCHK(glUseProgram(masking_shader.program));
    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));

    GLCHK(glEnableVertexAttribArray(masking_shader.attribute_locations[0]));
    GLCHK(glVertexAttribPointer(masking_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    GLCHK(glDisableVertexAttribArray(masking_shader.attribute_locations[0]));
    
    glFinish();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLCHK(glUseProgram(postproc_shader.program));
    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES,0));
    GLCHK(glBindTexture(GL_TEXTURE_2D, mask_fbo.tex));

    GLCHK(glEnableVertexAttribArray(masking_shader.attribute_locations[0]));
    GLCHK(glVertexAttribPointer(masking_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    GLCHK(glDisableVertexAttribArray(masking_shader.attribute_locations[0]));
    
    GLCHK(glBindTexture(GL_TEXTURE_2D,0));
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

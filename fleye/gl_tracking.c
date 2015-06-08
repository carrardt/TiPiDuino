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
//#define CHECK_GL_ERRORS 1

#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "cpu_tracking.h"

int tracking_ccmd = 4;

static char* readShader(const char* fileName)
{
	char filePath[256];
	snprintf(filePath,255,"%s/%s.glsl",GLSL_SRC_DIR,fileName);
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

typedef struct
{
	GLuint width, height; // dimensions
	GLuint format; // texture internal format
	GLenum target; // target associated with texture. 0 if renderbuffer
	GLuint tex; // texture
	GLuint rb; // renderbuffer
	GLuint fb; // frame buffer
	
} FBOTexture;

static FBOTexture ping_pong_fbo[2];
//static FBOTexture final_fbo;
static FBOTexture window_fbo;

static RASPITEXUTIL_SHADER_PROGRAM_T masking_shader;
static RASPITEXUTIL_SHADER_PROGRAM_T dist_shader;
static RASPITEXUTIL_SHADER_PROGRAM_T draw_shader;
static VCOS_THREAD_T cpuTrackingThread;
static CPU_TRACKING_STATE cpu_tracking_state;

static GLfloat varray[] =
{
   -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
   1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
};

static GLfloat tarray[] =
{
   0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
   1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
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


static int create_fbo(RASPITEX_STATE *state, FBOTexture* fbo, GLint colorFormat, GLint w, GLint h)
{
   fbo->format = colorFormat;
   fbo->width = w;
   fbo->height = h;
   fbo->target = 0;
   fbo->rb = 0;

   glGenFramebuffers(1, & fbo->fb);

	glGenTextures(1, & fbo->tex );
	fbo->target = GL_TEXTURE_2D;
	glBindTexture(fbo->target, fbo->tex);
	glTexImage2D(fbo->target, 0, fbo->format, fbo->width, fbo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(fbo->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(fbo->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(fbo->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(fbo->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(fbo->target, 0);

	/*if( depthFormat != GL_NONE )
	{
	   // Create a texture to hold the depth buffer
		glGenTextures(1, &(fbo->depht_rb) );
		glBindTexture(GL_TEXTURE_2D, fbo->depht_rb);
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
		fbo->depht_rb = 0;
	}*/

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);

    // Associate the textures with the FBO.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    fbo->target, fbo->tex, 0);

	// no depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D, /*null texture object*/ 0, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	
    if ( status == GL_FRAMEBUFFER_COMPLETE ) return 0;
    else return 1;
    
    
/*
glGenTextures(1, (GLuint *) 0x777ada0c);
glBindTexture(GL_TEXTURE_2D, 3);
glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) 0x752d700c);

glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, 3, 0);
 */
    
}

static int create_shader(RASPITEXUTIL_SHADER_PROGRAM_T* shader, const char* fsFile, const char* uniforms[])
{
	int i;
	// generate score values corresponding to color matching of target
	memset(shader,0,sizeof(RASPITEXUTIL_SHADER_PROGRAM_T));
	shader->vertex_source = readShader("common_vs");
	shader->attribute_names[0] = "vertex";
	shader->attribute_names[1] = "tcoord";
	shader->fragment_source = readShader(fsFile);
	for(i=0;uniforms[i]!=0;++i)
	{
		shader->uniform_names[i] = uniforms[i];
	}
	printf("Compiling %s ...\n",fsFile);
    int rc = raspitexutil_build_shader_program(shader);
	return rc;
}

static int shader_uniform1i(RASPITEXUTIL_SHADER_PROGRAM_T* shader, int i, GLint value)
{
    GLCHK(glUseProgram(shader->program));
    if( shader->uniform_locations[i]!=-1 )
    {
		GLCHK(glUniform1i(shader->uniform_locations[i], value));
		return 0;
	}
	else { return -1; }
}

static int shader_uniform1f(RASPITEXUTIL_SHADER_PROGRAM_T* shader, int i, GLfloat value)
{
    GLCHK(glUseProgram(shader->program));
    if( shader->uniform_locations[i]!=-1 )
    {
		GLCHK(glUniform1f(shader->uniform_locations[i], value));
		return 0;
	}
	else { return -1; }
}

static int tracking_init(RASPITEX_STATE *state)
{
   int i,rc;
    state->egl_config_attribs = tracking_egl_config_attribs;
    rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
    {
		vcos_log_error("unable to init GLES2\n");
		return rc;
	}

	window_fbo.tex = 0;
	window_fbo.fb = 0;
	window_fbo.format = GL_NONE;
	window_fbo.rb = 0;
	window_fbo.width = state->width/2;
	window_fbo.height = state->height/2;

   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES, state->texture) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES,0) );

	// generate score values corresponding to color matching of target
	{
		const char* uniform[] = { "tex","xstep", "ystep", 0 };
		create_shader(&masking_shader,"maskInitL2Cross_fs",uniform);
		shader_uniform1i(&masking_shader,0, 0);
		shader_uniform1f(&masking_shader,1, 1.0 / (state->width) );
		shader_uniform1f(&masking_shader,2, 1.0 / (state->height) );
	}

	// processing done with half resoluton, to maximize color (U,V) resolution
	
	// update distance to connected component border
	{
		const char* uniform[] = { "tex", "xstep", "ystep", 0 };
		create_shader(&dist_shader,"L2CrossIteration_fs",uniform);
		shader_uniform1i(&dist_shader,0, 0);
		shader_uniform1f(&dist_shader,1, 2.0 / state->width);
		shader_uniform1f(&dist_shader,2, 2.0 / state->height);
	}

	// draw score values
	{
		const char* uniform[] = { "tex","target_x","target_y", 0 };
		create_shader(&draw_shader,"drawL2Cross_fs",uniform);
		shader_uniform1i(&draw_shader,0, 0);
	}

	for(i=0;i<2;i++)
	{
		rc = create_fbo(state,&ping_pong_fbo[i],GL_RGBA,state->width/2,state->height/2);
		if (rc != 0)
		{
			vcos_log_error("ping pong FBO failed\n");
			return rc;
		}
	}
/*
	rc = create_fbo(state,&final_fbo,GL_RGBA,state->width/2,state->height/2, (GLubyte*) malloc(state->width*state->height) );
	if (rc != 0)
	{
		vcos_log_error("final FBO failed\n");
		return rc;
	}
*/
	GLCHK( glDisable(GL_DEPTH_TEST) );
	
	// allocate space for CPU processing
	
	printf( "EGL_CLIENT_APIS: %s\n", eglQueryString( state->display, EGL_CLIENT_APIS ) );
	printf( "EGL_VENDOR: %s\n", eglQueryString( state->display, EGL_VENDOR ) );
	printf( "EGL_VERSION: %s\n", eglQueryString( state->display, EGL_VERSION ) );
	printf( "EGL_EXTENSIONS: %s\n", eglQueryString( state->display, EGL_EXTENSIONS ) );
	printf( "GL_VERSION: %s\n", glGetString( GL_VERSION ) );
	printf( "GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
	printf( "GL_EXTENSIONS: %s\n", glGetString( GL_EXTENSIONS ) );

	cpu_tracking_state.width = state->width/2;
	cpu_tracking_state.height = state->height/2;
	cpu_tracking_state.image = malloc(cpu_tracking_state.width*cpu_tracking_state.height*4);
	cpu_tracking_state.objectCount = 0;
	cpu_tracking_state.do_processing = 1;
	rc = vcos_semaphore_create(&cpu_tracking_state.start_processing_sem,"start_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 vcos_log_error("Failed to start cpu processing start semaphor %d",rc);
		 return rc;
	}
	rc = vcos_semaphore_create(&cpu_tracking_state.end_processing_sem,"end_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 vcos_log_error("Failed to start cpu processing end semaphor %d",rc);
		 return rc;
	}
	vcos_semaphore_post(&cpu_tracking_state.end_processing_sem);

	rc = vcos_thread_create(&cpuTrackingThread, "cpu-tracking-worker", NULL, cpuTrackingWorker, &cpu_tracking_state);
	if (rc != VCOS_SUCCESS)
	{
      vcos_log_error("Failed to start cpu processing thread %d",rc);
      return rc;
	}

    return rc;
}

static void apply_shader_pass(RASPITEX_STATE *state, RASPITEXUTIL_SHADER_PROGRAM_T* shader, GLenum srcTarget, GLuint srcTex, FBOTexture* destFBO)
{        
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER,destFBO->fb));
	{
		GLenum attachements[1];
		attachements[0] = (destFBO->fb==0) ? GL_COLOR_EXT : GL_COLOR_ATTACHMENT0 ;
		GLCHK( glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,attachements) );
	}
	
//    GLCHK(glViewport(state->width - destFBO->width, state->height - destFBO->height, destFBO->width, destFBO->height));
    GLCHK(glViewport(0,0,destFBO->width,destFBO->height));

    GLCHK(glUseProgram(shader->program));
    
    //GLCHK(glEnable(srcTarget));
    GLCHK(glBindTexture(srcTarget, srcTex));
    
    GLCHK(glEnableVertexAttribArray(shader->attribute_locations[0]));
    GLCHK(glVertexAttribPointer(shader->attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
    GLCHK(glEnableVertexAttribArray(shader->attribute_locations[1]));
    GLCHK(glVertexAttribPointer(shader->attribute_locations[1], 2, GL_FLOAT, GL_FALSE, 0, tarray));
    
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    
    GLCHK(glDisableVertexAttribArray(shader->attribute_locations[0]));
    GLCHK(glDisableVertexAttribArray(shader->attribute_locations[1]));
    
    GLCHK(glBindTexture(srcTarget,0));
    //GLCHK(glDisable(srcTarget));
	
    GLCHK(glFinish());
}

static int tracking_redraw(RASPITEX_STATE *state)
{
	static int FrameN=0;
	int fboIndex = 0;
	int i;
    //glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
    GLCHK( glActiveTexture(GL_TEXTURE0) );

	apply_shader_pass(state,&masking_shader,GL_TEXTURE_EXTERNAL_OES,state->texture,&ping_pong_fbo[fboIndex]);

	for(i=0;i<tracking_ccmd;i++)
	{
		GLuint inputTexture = ping_pong_fbo[fboIndex].tex;
		GLuint inputTarget = ping_pong_fbo[fboIndex].target;
		int w = ping_pong_fbo[fboIndex].width;
		int h = ping_pong_fbo[fboIndex].height;
		double p2i = 1<<i;
		fboIndex = (fboIndex+1)%2;
		shader_uniform1f(&dist_shader,1, p2i / w);
		shader_uniform1f(&dist_shader,2, p2i / h);
		apply_shader_pass(state,&dist_shader,inputTarget,inputTexture,&ping_pong_fbo[fboIndex]);
	}
/*
	{
		GLuint inputTexture = ping_pong_fbo[fboIndex].tex;
		GLuint inputTarget = ping_pong_fbo[fboIndex].target;
		int w = final_fbo.width;
		int h = final_fbo.height;
		double p2i = 1<<(tracking_ccmd-1);
		shader_uniform1f(&dist_shader,1, p2i / w);
		shader_uniform1f(&dist_shader,2, p2i / h);
		apply_shader_pass(state,&dist_shader,inputTarget,inputTexture,&final_fbo);
	}
*/
	// TODO: find an alternative to glReadPixels, way too slow !!!
	vcos_semaphore_wait(&cpu_tracking_state.end_processing_sem); 
	GLCHK( glReadPixels(0, 0, ping_pong_fbo[fboIndex].width, ping_pong_fbo[fboIndex].height, GL_RGBA,GL_UNSIGNED_BYTE, cpu_tracking_state.image) );
	vcos_semaphore_post(&cpu_tracking_state.start_processing_sem);
	/*
	shader_uniform1f(&draw_shader,1, cpu_tracking_state.objectCenter[0][0]);
	shader_uniform1f(&draw_shader,2, cpu_tracking_state.objectCenter[0][1]);
	apply_shader_pass(state,&draw_shader,GL_TEXTURE_2D,ping_pong_fbo[fboIndex].tex,&window_fbo);
*/
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

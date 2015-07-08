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

static GLfloat varray[8] =
{
   0.0f, 0.0f
};

static const EGLint tracking_egl_config_attribs[] =
{
   EGL_RED_SIZE,   8,
   EGL_GREEN_SIZE, 8,
   EGL_BLUE_SIZE,  8,
   EGL_ALPHA_SIZE, 8,
   //EGL_SURFACE_TYPE , EGL_LOCK_SURFACE_BIT_KHR,
   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
   EGL_NONE
};

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

static int shader_uniform2f(RASPITEXUTIL_SHADER_PROGRAM_T* shader, int i, GLfloat v0, GLfloat v1)
{
    GLCHK(glUseProgram(shader->program));
    if( shader->uniform_locations[i]!=-1 )
    {
		GLCHK(glUniform2f(shader->uniform_locations[i], v0, v1));
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

	state->window_fbo.tex = 0;
	state->window_fbo.fb = 0;
	state->window_fbo.format = GL_NONE;
	state->window_fbo.rb = 0;
	state->window_fbo.width = state->width;
	state->window_fbo.height = state->height;

   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES, state->texture) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
   // GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
   // GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES,0) );

	create_image_processing( state, state->tracking_script );

	for(i=0;i<2;i++)
	{
		rc = create_fbo(state,&(state->ping_pong_fbo[i]),GL_RGBA,state->width,state->height);
		if (rc != 0)
		{
			vcos_log_error("ping pong FBO failed\n");
			return rc;
		}
	}
	GLCHK( glDisable(GL_DEPTH_TEST) );
	
	// allocate space for CPU processing
	
	printf( "EGL_CLIENT_APIS: %s\n", eglQueryString( state->display, EGL_CLIENT_APIS ) );
	printf( "EGL_VENDOR: %s\n", eglQueryString( state->display, EGL_VENDOR ) );
	printf( "EGL_VERSION: %s\n", eglQueryString( state->display, EGL_VERSION ) );
	printf( "EGL_EXTENSIONS: %s\n", eglQueryString( state->display, EGL_EXTENSIONS ) );
	printf( "GL_VERSION: %s\n", glGetString( GL_VERSION ) );
	printf( "GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
	printf( "GL_EXTENSIONS: %s\n", glGetString( GL_EXTENSIONS ) );

	state->cpu_tracking_state.width = state->width;
	state->cpu_tracking_state.height = state->height;
	state->cpu_tracking_state.image = malloc(state->cpu_tracking_state.width*state->cpu_tracking_state.height*4);
	state->cpu_tracking_state.objectCount = 0;
	state->cpu_tracking_state.do_processing = 1;
	rc = vcos_semaphore_create(& state->cpu_tracking_state.start_processing_sem,"start_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 vcos_log_error("Failed to create start_processing_sem semaphor %d",rc);
		 return rc;
	}

	rc = vcos_semaphore_create(& state->cpu_tracking_state.end_processing_sem,"end_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 vcos_log_error("Failed to create end_processing_sem semaphor %d",rc);
		 return rc;
	}

	rc = vcos_thread_create(& state->cpuTrackingThread, "cpu-tracking-worker", NULL, cpuTrackingWorker, & state->cpu_tracking_state);
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
	
    GLCHK(glViewport(0,0,destFBO->width,destFBO->height));

    GLCHK(glUseProgram(shader->program));
    
    GLCHK(glEnable(srcTarget));
    GLCHK(glBindTexture(srcTarget, srcTex));
    
    GLCHK(glEnableVertexAttribArray(shader->attribute_locations[0]));
    GLCHK(glVertexAttribPointer(shader->attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
        
    GLCHK(glDrawArrays(GL_POINTS, 0, 1));
    
    GLCHK(glDisableVertexAttribArray(shader->attribute_locations[0]));
    
    GLCHK(glBindTexture(srcTarget,0));
    GLCHK(glDisable(srcTarget));
	
    GLCHK(glFinish());
}

static int tracking_redraw(RASPITEX_STATE *state)
{
	static int FrameN=0;
	int fboIndex = 0;
	int step,i;
	GLint inTexTarget = GL_TEXTURE_EXTERNAL_OES;
	GLint inTex = state->texture;
	FBOTexture* destFBO = & state->ping_pong_fbo[fboIndex];
	int w = state->width;
	int h = state->height;
	int swapBuffers = 0;

	// wait previous async cycle to be finished
	int nPrevTasksToWait = state->cpu_tracking_state.nAvailCpuFuncs - state->cpu_tracking_state.nFinishedCpuFuncs;
	//printf("waiting %d (%d/%d) previous tasks\n",nPrevTasksToWait,state->cpu_tracking_state.nFinishedCpuFuncs,state->cpu_tracking_state.nAvailCpuFuncs);
	while( nPrevTasksToWait > 0 )
	{
		vcos_semaphore_wait( & state->cpu_tracking_state.end_processing_sem );
		-- nPrevTasksToWait;
	}
	state->cpu_tracking_state.nAvailCpuFuncs = 0;
	state->cpu_tracking_state.nFinishedCpuFuncs = 0;

    //glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
    GLCHK( glActiveTexture(GL_TEXTURE0) );

	for(step=0; step<state->n_processing_steps; ++step)
	{
		int nPasses = state->processing_step[step].numberOfPasses;

		if ( nPasses == CPU_PROCESSING_PASS )
		{
			if( state->processing_step[step].exec_thread == 0 )
			{
				//printf("sync exec cpu step #%d\n",step);
				( * state->processing_step[step].cpu_processing ) (& state->cpu_tracking_state);
			}
			else
			{
				//printf("async start cpu step #%d\n",step);
				state->cpu_tracking_state.cpu_processing[ state->cpu_tracking_state.nAvailCpuFuncs ] = state->processing_step[step].cpu_processing;
				++ state->cpu_tracking_state.nAvailCpuFuncs;
				vcos_semaphore_post(& state->cpu_tracking_state.start_processing_sem);
			}
		}
		else if( nPasses != 0 )
		{
			destFBO = & state->ping_pong_fbo[fboIndex];
			
			if( nPasses == SHADER_DISPLAY_PASS )
			{
				nPasses = 1;
				destFBO = & state->window_fbo;
				swapBuffers = 1;
			}

			//printf("shader step %d : %d passes\n",step,nPasses);
			RASPITEXUTIL_SHADER_PROGRAM_T* shader = & state->processing_step[step].gl_shader;

			shader_uniform1i( shader, 0, 0 ); // sampler always refers to active texture 0
			shader_uniform2f( shader, 1, 1.0 / w, 1.0 / h ); 
			shader_uniform2f( shader, 2, w, h ); 
			shader_uniform2f( shader, 6, state->cpu_tracking_state.objectCenter[0][0] , state->cpu_tracking_state.objectCenter[0][1] ); 
			shader_uniform2f( shader, 7, state->cpu_tracking_state.objectCenter[1][0] , state->cpu_tracking_state.objectCenter[1][1] ); 

			for(i=0;i<nPasses;i++)
			{
				//printf("%s : pass #%d\n",state->imageProcessing->gpu_pass[gpu_shader_index].shaderFile,i);
				double p2i = 1<<i;
				shader_uniform1f( shader, 3, i);
				shader_uniform1f( shader, 4, p2i);
				shader_uniform2f( shader, 5, p2i/w , p2i/h );
				apply_shader_pass(state, shader, inTexTarget, inTex, destFBO );
				inTexTarget = destFBO->target;
				inTex = destFBO->tex;
				fboIndex = ( fboIndex + 1 ) % 2; 
			}
		}
	}
	
	// terminate async processing cycle
	state->cpu_tracking_state.cpu_processing[ state->cpu_tracking_state.nAvailCpuFuncs ] = 0;
	vcos_semaphore_post(& state->cpu_tracking_state.start_processing_sem);

    GLCHK(glUseProgram(0));
	if(swapBuffers)
	{
		eglSwapBuffers(state->display, state->surface);
	}

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

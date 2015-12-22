#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>

#include <iostream>

#include "fleye/cpuworker.h"
#include "fleye/imageprocessing.h"
#include "fleye/plugin.h"
#include "fleye/shaderpass.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/texture.h"
#include "fleye/FleyeContext.h"
#include "glworker.h"

FLEYE_REGISTER_GL_DRAW(gl_fill)

int glworker_init(struct FleyeContext* ctx)
{	
	static const EGLint egl_config_attribs[] =
	{
	   EGL_RED_SIZE,   8,
	   EGL_GREEN_SIZE, 8,
	   EGL_BLUE_SIZE,  8,
	   EGL_ALPHA_SIZE, 8,
	   EGL_DEPTH_SIZE, 16,
	   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	   EGL_NONE
	};
 
	// create the main render window
	ctx->render_window = new FleyeRenderWindow(ctx->x,ctx->y,ctx->width,ctx->height,egl_config_attribs);
	assert( ctx->render_window != 0 );

	// create camera texture
    ctx->cameraTextureId = 0;
	glGenTextures(1, & ctx->cameraTextureId );
	assert( ctx->cameraTextureId != 0 );
   
   FleyeRenderWindow* renwin = ctx->render_window;
   ImageProcessingState* ip = new ImageProcessingState;
   ctx->ip = ip;
      
	GLTexture* nullTexture = new GLTexture;
	nullTexture->format = GL_RGB;
	nullTexture->target = GL_TEXTURE_2D;
	nullTexture->texid = 0;
	ip->texture["NULL"] = nullTexture;

	// configure camera input texture
   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES, ctx->cameraTextureId) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES,0) );
   
   // define input image as an available input texture named "INPUT"
    GLTexture* camTexture = new GLTexture;
	camTexture->texid = ctx->cameraTextureId;
	camTexture->target = GL_TEXTURE_EXTERNAL_OES;
	camTexture->format = GL_RGB;
	ip->texture["CAMERA"] = camTexture;

	// define display window as an available output FBO named "DISPLAY"
	// define null texture associated with final display (not a real FBO)
	// ip->texture["DISPLAY"] = nullTexture;
	FrameBufferObject* displayFBO = new FrameBufferObject;
	displayFBO->texture = nullTexture;
	displayFBO->width = ctx->width;
	displayFBO->height = ctx->height;
	displayFBO->fb = 0;
	displayFBO->render_window = ctx->render_window;
	ip->fbo["DISPLAY"] = displayFBO;

	int rc = read_image_processing_script( ctx );
	if( rc != 0 )
	{
		std::cerr<<"Failed to initialize image processing pipeline\n";
		return rc;
	}

	GLCHK( glDisable(GL_DEPTH_TEST) );
	
	// allocate space for CPU processing
	
	printf( "EGL_CLIENT_APIS: %s\n", eglQueryString( renwin->display, EGL_CLIENT_APIS ) );
	printf( "EGL_VENDOR: %s\n", eglQueryString( renwin->display, EGL_VENDOR ) );
	printf( "EGL_VERSION: %s\n", eglQueryString( renwin->display, EGL_VERSION ) );
	printf( "EGL_EXTENSIONS: %s\n", eglQueryString( renwin->display, EGL_EXTENSIONS ) );
	printf( "GL_VERSION: %s\n", glGetString( GL_VERSION ) );
	printf( "GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
	printf( "GL_EXTENSIONS: %s\n", glGetString( GL_EXTENSIONS ) );

	ip->cpu_tracking_state.objectCount = 0;
	ip->cpu_tracking_state.do_processing = 1;

    return 0;
}

// default drawing function
void gl_fill(struct CompiledShaderCache* compiledShader, int pass)
{
	static GLfloat tstrip[12] = {
		-1,-1,0,
		-1,1,0, 
		1,-1,0, 
		1,1,0
		};

    GLCHK( glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]));
	GLCHK( glVertexAttribPointer(compiledShader->shader.attribute_locations[0], 3, GL_FLOAT, GL_FALSE, 0, tstrip));
	GLCHK( glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    GLCHK( glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]));
}

static void apply_shader_pass(ShaderPass* shaderPass, int passCounter, int* needSwapBuffers)
{
	CompiledShaderCache* compiledShader = get_compiled_shader( shaderPass, passCounter );
	if(compiledShader==0)
	{
		std::cerr<<"Error compiling shader\n";
		return;
	}
	
	int fboPoolSize = shaderPass->fboPool.size();
	if( fboPoolSize == 0 )
	{
		std::cerr<<"Error: no output fbo\n";
		return ;
	}
	FrameBufferObject* destFBO = shaderPass->fboPool[ passCounter % fboPoolSize ];

	// bind FBO and discard color buffer content (not using blending and writing everything)
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER,destFBO->fb));
    GLenum colorAttachment = (destFBO->fb==0) ? GL_COLOR_EXT : GL_COLOR_ATTACHMENT0;
    if( destFBO->fb==0 )
	{
		*needSwapBuffers = 1;
	}
	else
	{
		GLCHK( glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,&colorAttachment) );
	}

	// set viewport to full surface
    GLCHK(glViewport(0,0,destFBO->width,destFBO->height));

	// select compiled shader program to use
    GLCHK(glUseProgram(compiledShader->shader.program));

	// enable input textures
	// TODO: minimize the number of texture units used in case not all textures are referenced
	int nInputs = shaderPass->inputs.size();
	for(int i=nInputs-1;i>=0;i--)
	{
		GLTexture* tex = 0;
		if( ! shaderPass->inputs[i].texPool.empty() )
		{
			tex = shaderPass->inputs[i].texPool[ passCounter % shaderPass->inputs[i].texPool.size() ];
		}
		if( tex!=0 && compiledShader->samplerUniformLocations[i] != -1 )
		{
			GLCHK( glActiveTexture( GL_TEXTURE0 + i ) );
			/*GLenum enableTarget = tex->target;
			if( enableTarget == GL_TEXTURE_EXTERNAL_OES ) { enableTarget=GL_TEXTURE_2D; }
			if( enableTarget != GL_NONE ) {	std::cout<<(void*)enableTarget<<"\n"; GLCHK( glEnable(enableTarget) ); }*/
			GLCHK( glBindTexture(tex->target, tex->texid) );
			GLCHK( glUniform1i(compiledShader->samplerUniformLocations[i], i) );
		}
	}

	// set uniform values
	double p2i = 1<<passCounter;
	int loc;
	if( (loc=compiledShader->shader.uniform_locations[0]) != -1 ) { GLCHK( glUniform2f(loc, 1.0 / destFBO->width, 1.0 / destFBO->height ) ); }
	if( (loc=compiledShader->shader.uniform_locations[1]) != -1 ) { GLCHK( glUniform2f(loc, destFBO->width, destFBO->height ) ); }
	if( (loc=compiledShader->shader.uniform_locations[2]) != -1 ) { GLCHK( glUniform1f(loc, passCounter ) ); }
	if( (loc=compiledShader->shader.uniform_locations[3]) != -1 ) { GLCHK( glUniform1f(loc, p2i ) ); }
	if( (loc=compiledShader->shader.uniform_locations[4]) != -1 ) { GLCHK( glUniform2f(loc, p2i/destFBO->width, p2i/destFBO->height ) ); }

	// call custom drawing function
	if( shaderPass->gl_draw != 0 )
	{
		( * shaderPass->gl_draw ) (compiledShader,passCounter);
	}
	// detach textures
 	for(int i=nInputs-1;i>=0;i--)
	{
		GLTexture* tex = 0;
		if( ! shaderPass->inputs[i].texPool.empty() )
		{
			tex = shaderPass->inputs[i].texPool[ passCounter % shaderPass->inputs[i].texPool.size() ];
		}
		if( tex!=0 && compiledShader->samplerUniformLocations[i] != -1 )
		{
			GLCHK( glActiveTexture( GL_TEXTURE0 + i ) );
			GLCHK( glBindTexture(tex->target,0) );
			/*GLenum enableTarget = tex->target;
			if( enableTarget == GL_TEXTURE_EXTERNAL_OES ) { enableTarget=GL_TEXTURE_2D; }
			if( enableTarget != GL_NONE ) {	GLCHK( glDisable(enableTarget) ); }*/
		}
	}
	GLCHK( glActiveTexture( GL_TEXTURE0 ) ); // back to default

	// TODO: test if this is necessary
    // GLCHK(glFinish());
}

int glworker_redraw(struct FleyeContext* ctx)
{
	int step = 0;
	int swapBuffers = 0;
    FleyeRenderWindow* mainwin = ctx->render_window;
    FleyeRenderWindow* renwin = mainwin;

	//std::cout<<"glworker_redraw: mainwin "<<mainwin<<"\n";

	// wait previous async cycle to be finished
	int nPrevTasksToWait = ctx->ip->cpu_tracking_state.nAvailCpuFuncs - ctx->ip->cpu_tracking_state.nFinishedCpuFuncs;
	//printf("waiting %d (%d/%d) previous tasks\n",nPrevTasksToWait,state->ip->cpu_tracking_state.nFinishedCpuFuncs,state->ip->cpu_tracking_state.nAvailCpuFuncs);
	while( nPrevTasksToWait > 0 )
	{
		waitEndProcessingSem( ctx );
		-- nPrevTasksToWait;
	}
	ctx->ip->cpu_tracking_state.nAvailCpuFuncs = 0;
	ctx->ip->cpu_tracking_state.nFinishedCpuFuncs = 0;

    GLCHK( glActiveTexture(GL_TEXTURE0) );

	for( ProcessingStep& ps : ctx->ip->processing_step )
	{
		if( ps.onFrames.contains( ctx->frameCounter ) )
		{
			if( ps.shaderPass!=0 )
			{				
				int nPasses = ps.shaderPass->numberOfPasses;
				int fboPoolSize = ps.shaderPass->fboPool.size();
				for(int i=0;i<nPasses;i++)
				{
					//std::cout<<"pass "<<i<<"\n";
					FrameBufferObject* fbo = ps.shaderPass->fboPool[ i % fboPoolSize ];
					FleyeRenderWindow* nextRenWin = fbo->render_window;
					if( nextRenWin != renwin )
					{
						//std::cout<<"context switch : "<<renwin<<" -> "<<nextRenWin<<"\n";
						if(swapBuffers)
						{
							//std::cout<<"swap buffers on "<<renwin<<"\n";
							glFlush();
							glFinish();
							eglSwapBuffers(renwin->display, renwin->surface);
							swapBuffers = 0;
						}
						renwin = nextRenWin;
						//std::cout<<"make current on "<<renwin<<"\n";
						eglMakeCurrent(renwin->display, renwin->surface, renwin->surface, renwin->context);
					}
					apply_shader_pass( ps.shaderPass, i, &swapBuffers);
				}
				if( nPasses>0 && fboPoolSize>0 )
				{
					FrameBufferObject* finalFBO = ps.shaderPass->fboPool[(nPasses-1)%fboPoolSize];
					// copy last written fbo's attached texture to shader alias texture
					* ps.shaderPass->finalTexture = * finalFBO->texture;
				}
				else
				{
					ps.shaderPass->finalTexture->texid = 0;
					ps.shaderPass->finalTexture->target = GL_TEXTURE_2D;
					ps.shaderPass->finalTexture->format = GL_RGB;
				}
			}
			if( ps.cpuPass != 0 )
			{
				if( ps.cpuPass->exec_thread == 0 )
				{
					//printf("sync exec cpu step #%d\n",step);
					( * ps.cpuPass->cpu_processing ) ( ctx );
				}
				else
				{
					//printf("async start cpu step #%d\n",step);
					ctx->ip->cpu_tracking_state.cpu_processing[ ctx->ip->cpu_tracking_state.nAvailCpuFuncs ] = ps.cpuPass->cpu_processing;
					++ ctx->ip->cpu_tracking_state.nAvailCpuFuncs;
					postStartProcessingSem( ctx );
					//vcos_semaphore_post(& ip->cpu_tracking_state.start_processing_sem);
				}
			}
		}
	}
	
	// terminate async processing cycle
	ctx->ip->cpu_tracking_state.cpu_processing[ ctx->ip->cpu_tracking_state.nAvailCpuFuncs ] = 0;
	postStartProcessingSem( ctx );

    GLCHK(glUseProgram(0));

	if( mainwin != renwin )
	{
		//std::cout<<"final context switch "<<renwin<<" -> "<<mainwin<<"\n";
		if(swapBuffers)
		{
			eglSwapBuffers(renwin->display, renwin->surface);
			swapBuffers = 0;
		}
		renwin = mainwin;
		eglMakeCurrent(renwin->display, renwin->surface, renwin->surface, renwin->context);
	}

	if(swapBuffers)
	{
		eglSwapBuffers(renwin->display, renwin->surface);
	}
	
	++ ctx->frameCounter;

    return 0;
}

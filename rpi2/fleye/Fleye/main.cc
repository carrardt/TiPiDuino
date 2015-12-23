extern "C" {
#include "interface/mmal/mmal.h"
#include "interface/vcos/vcos.h"
}

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "EGL/eglext_brcm.h"
#include <sys/time.h>

#include <iostream>
#include <sstream>

#include "CameraStream.h"
#include "glworker.h"
#include "fleye/FleyeContext.h"
#include "fleye/cpuworker.h"
#include "fleye/config.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/imageprocessing.h"
#include "fleye/fbo.h"

struct FleyeContextInternal
{
   EGLImageKHR egl_image;            /// The current preview EGL image
   MMAL_BUFFER_HEADER_T *image_buf;  /// MMAL buffer currently bound to texture(s)

   /* cpu worker thread */
   VCOS_THREAD_T cpuTrackingThread;

   /* synchronization semaphores between cpu workers and gpu worker */
   VCOS_SEMAPHORE_T start_processing_sem;
   VCOS_SEMAPHORE_T end_processing_sem;
};

int postStartProcessingSem( struct FleyeContext* ctx )
{
	vcos_semaphore_post( & ctx->priv->start_processing_sem );
}

int waitStartProcessingSem( struct FleyeContext* ctx )
{
	vcos_semaphore_wait( & ctx->priv->start_processing_sem );
}

int postEndProcessingSem( struct FleyeContext* ctx )
{
	vcos_semaphore_post( & ctx->priv->end_processing_sem );
}

int waitEndProcessingSem( struct FleyeContext* ctx )
{
	vcos_semaphore_wait( & ctx->priv->end_processing_sem );
}

static void update_fps()
{
   static int frame_count = 0;
   static long long time_start = 0;
   long long time_now;
   struct timeval te;
   float fps;

   frame_count++;

   gettimeofday(&te, NULL);
   time_now = te.tv_sec * 1000LL + te.tv_usec / 1000;

   if (time_start == 0)
   {
      time_start = time_now;
   }
   else if (time_now - time_start > 5000)
   {
      fps = (float) frame_count / ((time_now - time_start) / 1000.0);
      frame_count = 0;
      time_start = time_now;
      std::cout<<fps<<" FPS\n";
   }
}

static int user_process(void* user_data)
{
	FleyeContext* ctx = (FleyeContext*) user_data;
	glworker_redraw( ctx );
	update_fps();
	return 0;
}

static int user_initialize(void* user_data)
{
	struct FleyeContext* ctx = (struct FleyeContext*) user_data;
	int rc=0;
	
	// create semaphores for CPU Worker / GL Worker synchronization
	VCOS_SEMAPHORE_T* sem;
	sem = & ctx->priv->start_processing_sem;
	memset(sem,0,sizeof(*sem));
	rc = vcos_semaphore_create( sem, "start_processing_sem", 1);
	assert( rc == VCOS_SUCCESS );

	sem = & ctx->priv->end_processing_sem;
	memset(sem,0,sizeof(*sem));
	rc = vcos_semaphore_create( sem,"end_processing_sem", 1);
	assert( rc == VCOS_SUCCESS );
	
	// initialize GL and read processinf script
	rc = glworker_init(ctx);
	assert(rc==0);
	
	// create the cpu worker thread
	VCOS_THREAD_T* cpuThread = & ctx->priv->cpuTrackingThread;
	memset(cpuThread,0,sizeof(*cpuThread));
	rc = vcos_thread_create( cpuThread, "cpu_worker", NULL, cpuWorker, ctx );
	assert (rc == VCOS_SUCCESS);
	
	if( ctx->verbose ) { std::cout<<"Application initialized\n"; }
	return 0;
}

static MMAL_BUFFER_HEADER_T * user_copy_buffer(MMAL_BUFFER_HEADER_T *buf, void* user_data)
{
	struct FleyeContext* ctx = (struct FleyeContext*) user_data;

   GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, ctx->cameraTextureId));
   if (ctx->priv->egl_image != EGL_NO_IMAGE_KHR)
   {
      // Discard the EGL image for the preview frame 
      eglDestroyImageKHR(ctx->render_window->display, ctx->priv->egl_image);
      ctx->priv->egl_image = EGL_NO_IMAGE_KHR;
   }
   ctx->priv->egl_image = eglCreateImageKHR(ctx->render_window->display, EGL_NO_CONTEXT, EGL_IMAGE_BRCM_MULTIMEDIA, (EGLClientBuffer) buf->data, NULL);
   GLCHK(glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, ctx->priv->egl_image));

   // return previous buffer we don't need anymore
   MMAL_BUFFER_HEADER_T * previous_buffer = ctx->priv->image_buf;
   ctx->priv->image_buf = buf;
   return previous_buffer;
}

static int user_finalize(void* user_data)
{
	struct FleyeContext* ctx = (struct FleyeContext*) user_data;
	
	delete ctx;
	
	if( ctx->verbose ) { std::cout<<"Application terminated\n"; }
	return 0;
}

FleyeContext::FleyeContext()
	: x(0)
	, y(0)
	, width(648)
	, height(486)
	, captureWidth(1296)
	, captureHeight(972)
	, frameCounter(0)
	, cameraTextureId(0)
	, render_window(0)
	, ip(0)
	, priv(0)
	, verbose(false)
{
	priv = new FleyeContextInternal;	
	priv->egl_image = EGL_NO_IMAGE_KHR;
	priv->image_buf = 0;
}

FleyeContext::~FleyeContext()
{
	if( verbose ) { std::cout<<"Cleaning resources...\n"; }
	for( auto fbo : ip->fbo )
	{
		if( fbo.second->render_window != render_window )
		{
			delete fbo.second->render_window;
			fbo.second->render_window = 0;
		}
	}
	
	delete render_window;
	render_window = 0;
	
	delete ip;
	ip = 0;
	
	delete priv;
	priv = 0;
}

void FleyeContext::setIntegerVar(const std::string& key, int value)
{
	std::ostringstream oss;
	oss<<value;
	this->vars[key] = oss.str();
}

int main(int argc, char * argv[])
{
	// must be called first, otherwise mml or vcos calls will fail
	camera_streamer_init();

	FleyeContext * ctx = new FleyeContext;

	ctx->script = "passthru";

	int nArgs=1;
	for(int i=1;i<argc;i++)
	{
		if( strcmp(argv[i],"-v")==0 )
		{
			ctx->verbose = true;
		}
		else if( strcmp(argv[i],"-res")==0 )
		{
			++i;
			sscanf(argv[i],"%dx%d",&ctx->captureWidth,&ctx->captureHeight);
		}
		else if( strcmp(argv[i],"-geom")==0 )
		{
			++i;
			sscanf(argv[i],"%dx%d+%d+%d",&ctx->width,&ctx->height,&ctx->x,&ctx->y);
		}
		else if( strcmp(argv[i],"-script")==0 )
		{
			++i;
			ctx->script = argv[i];
		}
		else if( strcmp(argv[i],"-set")==0 )
		{
			++i;
			char key[256], value[256];
			sscanf(argv[i],"%s %s",key,value);
			ctx->vars[key] = value;
		}
		else
		{
			//printf("passing arg %d to %d (%s)\n",i,nArgs,argv[i]);
			argv[nArgs++] = argv[i];
		}
	}
	argc=nArgs;

	ctx->vars["SCRIPT"] = ctx->script;
	ctx->setIntegerVar("WIDTH",ctx->width);
	ctx->setIntegerVar("HEIGHT",ctx->height);
	ctx->setIntegerVar("CAM_WIDTH",ctx->captureWidth);
	ctx->setIntegerVar("CAM_HEIGHT",ctx->captureHeight);

	if( ctx->verbose )
	{
		std::cout<<"Fleye vars:\n";
		for( auto var : ctx->vars )
		{
			std::cout<<"\t"<<var.first.c_str()<<" = "<<var.second.c_str()<<"\n";
		}
	}

	camera_stream(argc,argv,0,ctx->captureWidth,ctx->captureHeight,user_initialize,user_copy_buffer,user_process,user_finalize,ctx);
	printf("Bye!\n");
}

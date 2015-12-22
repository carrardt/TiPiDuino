#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "EGL/eglext_brcm.h"
#include "interface/mmal/mmal.h"
#include "interface/vcos/vcos.h"

#include "CameraStream.h"
#include "glworker.h"
#include "fleye/FleyeContext.h"
#include "fleye/render_window.h"
#include "fleye/cpuworker.h"
#include "fleye/config.h"

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
      printf("%3.2f FPS\n", fps);
   }
}

static int user_process(void* user_data)
{
	struct FleyeContext* ctx = (struct FleyeContext*) user_data;
	glworker_redraw( ctx );
	update_fps();
	return 0;
}

static int user_initialize(void* user_data)
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
	struct FleyeContext* ctx = (struct FleyeContext*) user_data;
	int rc=0;
		
	// create the main render window
	ctx->render_window = create_render_window(ctx->x,ctx->y,ctx->width,ctx->height,egl_config_attribs);
	assert( ctx->render_window != 0 );

	// create camera texture
    ctx->cameraTextureId = 0;
	glGenTextures(1, & ctx->cameraTextureId );
	assert( ctx->cameraTextureId != 0 );
	
	// create semaphores for CPU Worker / GL Worker synchronization
	rc = vcos_semaphore_create(& ctx->priv->start_processing_sem,"start_processing_sem", 1);
	assert( rc == VCOS_SUCCESS );
	rc = vcos_semaphore_create(& ctx->priv->end_processing_sem,"end_processing_sem", 1);
	assert( rc == VCOS_SUCCESS );
	
	rc = glworker_init(ctx);
	assert(rc==0);
	
	rc = vcos_thread_create(& ctx->priv->cpuTrackingThread, "cpu_worker", NULL, cpuWorker, ctx );
	assert (rc == VCOS_SUCCESS);
	
	printf("Application initialized\n");
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
	printf("Application terminated\n");
	return 0;
}

int main(int argc, char * argv[])
{
	static struct FleyeContext ctx;
	static struct FleyeContextInternal privCtx;
	
	camera_streamer_init();
	
	memset(&ctx,0,sizeof(struct FleyeContext));
	memset(&privCtx,0,sizeof(struct FleyeContextInternal));

	ctx.priv = &privCtx;	
	ctx.priv->egl_image = EGL_NO_IMAGE_KHR;
	ctx.x = 0;
	ctx.y = 0;
	ctx.width = 648;
	ctx.height = 486;
	ctx.captureWidth = 1296;
	ctx.captureHeight = 972;

	fleye_create_user_env(&ctx);
	fleye_set_processing_script(&ctx,"passthru");
	fleye_add_optional_value(&ctx,"WIDTH","648");
	fleye_add_optional_value(&ctx,"HEIGHT","486");

	camera_stream(argc,argv,0,user_initialize,user_copy_buffer,user_process,user_finalize,&ctx);
	printf("Bye!\n");
}

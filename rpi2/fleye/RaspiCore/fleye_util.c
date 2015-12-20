#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <bcm_host.h>

#include <dlfcn.h>
#include <string.h>

#include "fleye_util.h"
#include "fleye_core.h"
#include "fleye/fleye_c.h"
#include "fleye/render_window.h"

/**
 * \file fleye_util.c
 *
 * Provides default implementations for the fleye_scene_ops functions
 * and general utility functions.
 */

/**
 * Creates an OpenGL ES 2.X context.
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_gl_init(FleyeState *fleye_state)
{
   int rc = 0;
   const EGLint* default_attribs = 0;

   printf("%s\n", __PRETTY_FUNCTION__);

	// create a GL render window
   default_attribs = glworker_egl_config( & fleye_state->common );
   fleye_state->render_window = create_render_window( fleye_state->common.x, fleye_state->common.y,
											fleye_state->common.width, fleye_state->common.height,
											default_attribs);

    // create camera texture Id
    fleye_state->common.cameraTextureId = 0;
	glGenTextures(1, & fleye_state->common.cameraTextureId );
    if (fleye_state->common.cameraTextureId == 0)
    {
		fprintf(stderr,"unable to create camera texture\n");
		return -1;
	}

	// create semaphores for CPU Worker / GL Worker synchronization
	rc = vcos_semaphore_create(& fleye_state->start_processing_sem,"start_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 fprintf(stderr,"Failed to create start_processing_sem semaphor %d",rc);
		 return -1;
	}
	rc = vcos_semaphore_create(& fleye_state->end_processing_sem,"end_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 fprintf(stderr,"Failed to create end_processing_sem semaphor %d",rc);
		 return -1;
	}

	// initialize GL Worker and read processing script
	fleye_state->common.fleye_state = fleye_state;
    fleye_state->ip = glworker_init( &fleye_state->common , fleye_state->user_env );
    if( fleye_state->ip == NULL ) return -1;

	// start CPU Worker thread
	rc = vcos_thread_create(& fleye_state->cpuTrackingThread, "cpu_tracking_worker", NULL, cpuTrackingWorker, fleye_state->ip );
	if (rc != VCOS_SUCCESS)
	{
      fprintf(stderr,"Failed to start cpu processing thread %d",rc);
      return -1;
	}

   return 0;
}

/**
 * Deletes textures and EGL surfaces and context.
 * @param   fleye_state  Pointer to the Raspi
 */
void fleyeutil_gl_term(FleyeState *fleye_state)
{
   printf("%s\n", __PRETTY_FUNCTION__);

   /* Delete OES textures */
   glDeleteTextures(1, & fleye_state->common.cameraTextureId);
   eglDestroyImageKHR(fleye_state->render_window->display, fleye_state->egl_image);
   fleye_state->egl_image = EGL_NO_IMAGE_KHR;
   
   destroy_render_window(fleye_state->render_window);   
}

/**
 * Advances the texture and EGL image to the next MMAL buffer.
 *
 * @param display The EGL display.
 * @param target The EGL image target e.g. EGL_IMAGE_BRCM_MULTIMEDIA
 * @param mm_buf The EGL client buffer (mmal opaque buffer) that is used to
 * create the EGL Image for the preview texture.
 * @param egl_image Pointer to the EGL image to update with mm_buf.
 * @param texture Pointer to the texture to update from EGL image.
 * @return Zero if successful.
 */
static int fleyeutil_do_update_texture(EGLDisplay display, EGLenum target,
      EGLClientBuffer mm_buf, GLuint texture, EGLImageKHR *egl_image)
{
   //printf("%s: mm_buf %u\n", __PRETTY_FUNCTION__, (unsigned) mm_buf);
   GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture));
   if (*egl_image != EGL_NO_IMAGE_KHR)
   {
      /* Discard the EGL image for the preview frame */
      eglDestroyImageKHR(display, *egl_image);
      *egl_image = EGL_NO_IMAGE_KHR;
   }

   *egl_image = eglCreateImageKHR(display, EGL_NO_CONTEXT, target, mm_buf, NULL);
   GLCHK(glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, *egl_image));

   return 0;
}

/**
 * Updates the RGBX texture to the specified MMAL buffer.
 * @param fleye_state A pointer to the GL preview state.
 * @param mm_buf The MMAL buffer.
 * @return Zero if successful.
 */
int fleyeutil_update_texture(FleyeState *fleye_state,
      EGLClientBuffer mm_buf)
{
   return fleyeutil_do_update_texture(fleye_state->render_window->display,
         EGL_IMAGE_BRCM_MULTIMEDIA, mm_buf,
         fleye_state->common.cameraTextureId, &fleye_state->egl_image);
}

void waitStartProcessingSem(struct FleyeState* state)
{
	vcos_semaphore_wait( & state->start_processing_sem );
}

void postStartProcessingSem(struct FleyeState* state)
{
	vcos_semaphore_post( & state->start_processing_sem );
}

void waitEndProcessingSem(struct FleyeState* state)
{
	vcos_semaphore_wait( & state->end_processing_sem );
}

void postEndProcessingSem(struct FleyeState* state)
{
	vcos_semaphore_post( & state->end_processing_sem );
}

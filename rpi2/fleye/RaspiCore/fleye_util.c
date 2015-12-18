#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <bcm_host.h>

#include <dlfcn.h>
#include <string.h>

#include "fleye_util.h"
#include "fleye_core.h"
#include "fleye/fleye_c.h"
#include "fleye_window.h"

/**
 * \file fleye_util.c
 *
 * Provides default implementations for the fleye_scene_ops functions
 * and general utility functions.
 */

/**
 * Deletes textures and EGL surfaces and context.
 * @param   fleye_state  Pointer to the Raspi
 */
void fleyeutil_gl_term(FleyeState *fleye_state)
{
   printf("%s\n", __PRETTY_FUNCTION__);

   /* Delete OES textures */
   GLuint cameraTextureId = fleye_get_camera_texture_id(fleye_state->ip);
   glDeleteTextures(1, &cameraTextureId);
   eglDestroyImageKHR(fleye_state->common.display, fleye_state->egl_image);
   fleye_state->egl_image = EGL_NO_IMAGE_KHR;

   /* Terminate EGL */
   eglMakeCurrent(fleye_state->common.display, EGL_NO_SURFACE,
         EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(fleye_state->common.display, fleye_state->common.context);
   eglDestroySurface(fleye_state->common.display, fleye_state->common.surface);
   eglTerminate(fleye_state->common.display);
}

/** Creates the EGL context and window surface for the native window
 * using specified arguments.
 * @param fleye_state  A pointer to the GL preview state. This contains
 *                        the native_window pointer.
 * @param attribs         The config attributes.
 * @param context_attribs The context attributes.
 * @return Zero if successful.
 */
static int fleyeutil_gl_common(FleyeState *fleye_state,
      const EGLint attribs[], const EGLint context_attribs[])
{
   EGLConfig config;
   EGLint num_configs;

   printf("%s\n", __PRETTY_FUNCTION__);

   if (fleye_state->fleye_window->native_window == NULL)
   {
      fprintf(stderr,"%s: No native window\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

   fleye_state->common.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (fleye_state->common.display == EGL_NO_DISPLAY)
   {
      fprintf(stderr,"%s: Failed to get EGL display\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

   if (! eglInitialize(fleye_state->common.display, 0, 0))
   {
      fprintf(stderr,"%s: eglInitialize failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

   if (! eglChooseConfig(fleye_state->common.display, attribs, &config,
            1, &num_configs))
   {
      fprintf(stderr,"%s: eglChooseConfig failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

   fleye_state->common.surface = eglCreateWindowSurface(fleye_state->common.display,
         config, fleye_state->fleye_window->native_window, NULL);
   if (fleye_state->common.surface == EGL_NO_SURFACE)
   {
      fprintf(stderr,"%s: eglCreateWindowSurface failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

   fleye_state->common.context = eglCreateContext(fleye_state->common.display,
         config, EGL_NO_CONTEXT, context_attribs);
   if (fleye_state->common.context == EGL_NO_CONTEXT)
   {
      fprintf(stderr,"%s: eglCreateContext failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

   if (!eglMakeCurrent(fleye_state->common.display, fleye_state->common.surface,
            fleye_state->common.surface, fleye_state->common.context))
   {
      fprintf(stderr,"%s: Failed to activate EGL context\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      fleyeutil_gl_term(fleye_state);
      return -1;
   }

    eglSwapInterval(fleye_state->common.display,0); // no VSync

   return 0;
}


/**
 * Creates an OpenGL ES 2.X context.
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_gl_init(FleyeState *fleye_state)
{
   int rc;
   const EGLint* default_attribs = glworker_egl_config( & fleye_state->common );
   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   printf("%s\n", __PRETTY_FUNCTION__);
   rc = fleyeutil_gl_common(fleye_state, default_attribs, context_attribs);
   if (rc != 0) return rc;

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

	struct CPU_TRACKING_STATE* cpuThreadCtx = NULL;
	fleye_state->common.fleye_state = fleye_state;
    fleye_state->ip = glworker_init( &fleye_state->common , fleye_state->user_env, &cpuThreadCtx );
    if( fleye_state->ip == NULL ) return -1;

	rc = vcos_thread_create(& fleye_state->cpuTrackingThread, "cpu_tracking_worker", NULL, cpuTrackingWorker, cpuThreadCtx );
	if (rc != VCOS_SUCCESS)
	{
      fprintf(stderr,"Failed to start cpu processing thread %d",rc);
      return -1;
	}

   return 0;
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
   GLuint texture = fleye_get_camera_texture_id(fleye_state->ip);
   return fleyeutil_do_update_texture(fleye_state->common.display,
         EGL_IMAGE_BRCM_MULTIMEDIA, mm_buf,
         texture, &fleye_state->egl_image);
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

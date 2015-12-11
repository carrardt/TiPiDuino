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

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <bcm_host.h>

#include <dlfcn.h>
#include <string.h>

#include "fleye_util.h"
#include "fleye_core.h"

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
   glDeleteTextures(1, &fleye_state->texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->egl_image);
   fleye_state->egl_image = EGL_NO_IMAGE_KHR;

   glDeleteTextures(1, &fleye_state->y_texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->y_egl_image);
   fleye_state->y_egl_image = EGL_NO_IMAGE_KHR;

   glDeleteTextures(1, &fleye_state->u_texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->u_egl_image);
   fleye_state->u_egl_image = EGL_NO_IMAGE_KHR;

   glDeleteTextures(1, &fleye_state->v_texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->v_egl_image);
   fleye_state->v_egl_image = EGL_NO_IMAGE_KHR;

   /* Terminate EGL */
   eglMakeCurrent(fleye_state->display, EGL_NO_SURFACE,
         EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(fleye_state->display, fleye_state->context);
   eglDestroySurface(fleye_state->display, fleye_state->surface);
   eglTerminate(fleye_state->display);
}

/** Creates a native window for the GL surface using dispmanx
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful, otherwise, -1 is returned.
 */
int fleyeutil_create_native_window(FleyeState *fleye_state)
{
   VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
   VC_RECT_T src_rect = {0};
   VC_RECT_T dest_rect = {0};
   uint32_t disp_num = 0; // Primary
   uint32_t layer_num = 0;
   DISPMANX_ELEMENT_HANDLE_T elem;
   DISPMANX_UPDATE_HANDLE_T update;

   alpha.opacity = fleye_state->opacity;
   dest_rect.x = fleye_state->x;
   dest_rect.y = fleye_state->y;
   dest_rect.width = fleye_state->width;
   dest_rect.height = fleye_state->height;

   printf("%s: %d,%d,%d,%d %d,%d,0x%x,0x%x\n", __PRETTY_FUNCTION__,
         src_rect.x, src_rect.y, src_rect.width, src_rect.height,
         dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height);

   src_rect.width = dest_rect.width << 16;
   src_rect.height = dest_rect.height << 16;

   fleye_state->disp = vc_dispmanx_display_open(disp_num);
   if (fleye_state->disp == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to open display handle\n");
      goto error;
   }

   update = vc_dispmanx_update_start(0);
   if (update == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to open update handle\n");
      goto error;
   }

   elem = vc_dispmanx_element_add(update, fleye_state->disp, layer_num,
         &dest_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, &alpha, NULL,
         DISPMANX_NO_ROTATE);
   if (elem == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to create element handle\n");
      goto error;
   }

   fleye_state->win.element = elem;
   fleye_state->win.width = fleye_state->width;
   fleye_state->win.height = fleye_state->height;
   vc_dispmanx_update_submit_sync(update);

   fleye_state->native_window = (EGLNativeWindowType*) &fleye_state->win;

   return 0;
error:
   return -1;
}

/** Destroys the pools of buffers used by the GL renderer.
 * @param fleye_state A pointer to the GL preview state.
 */
void fleyeutil_destroy_native_window(FleyeState *fleye_state)
{
   printf("%s\n", __PRETTY_FUNCTION__);
   if (fleye_state->disp != DISPMANX_NO_HANDLE)
   {
      vc_dispmanx_display_close(fleye_state->disp);
      fleye_state->disp = DISPMANX_NO_HANDLE;
   }
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

   if (fleye_state->native_window == NULL)
   {
      fprintf(stderr,"%s: No native window\n", __PRETTY_FUNCTION__);
      goto error;
   }

   fleye_state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (fleye_state->display == EGL_NO_DISPLAY)
   {
      fprintf(stderr,"%s: Failed to get EGL display\n", __PRETTY_FUNCTION__);
      goto error;
   }

   if (! eglInitialize(fleye_state->display, 0, 0))
   {
      fprintf(stderr,"%s: eglInitialize failed\n", __PRETTY_FUNCTION__);
      goto error;
   }

   if (! eglChooseConfig(fleye_state->display, attribs, &config,
            1, &num_configs))
   {
      fprintf(stderr,"%s: eglChooseConfig failed\n", __PRETTY_FUNCTION__);
      goto error;
   }

   fleye_state->surface = eglCreateWindowSurface(fleye_state->display,
         config, fleye_state->native_window, NULL);
   if (fleye_state->surface == EGL_NO_SURFACE)
   {
      fprintf(stderr,"%s: eglCreateWindowSurface failed\n", __PRETTY_FUNCTION__);
      goto error;
   }

   fleye_state->context = eglCreateContext(fleye_state->display,
         config, EGL_NO_CONTEXT, context_attribs);
   if (fleye_state->context == EGL_NO_CONTEXT)
   {
      fprintf(stderr,"%s: eglCreateContext failed\n", __PRETTY_FUNCTION__);
      goto error;
   }

   if (!eglMakeCurrent(fleye_state->display, fleye_state->surface,
            fleye_state->surface, fleye_state->context))
   {
      fprintf(stderr,"%s: Failed to activate EGL context\n", __PRETTY_FUNCTION__);
      goto error;
   }

   return 0;

error:
   fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
   fleyeutil_gl_term(fleye_state);
   return -1;
}

/* Creates the RGBA and luma textures with some default parameters
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_create_textures(FleyeState *fleye_state)
{
   GLCHK(glGenTextures(1, &fleye_state->texture));
   return 0;
}

/**
 * Creates an OpenGL ES 2.X context.
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_gl_init_2_0(FleyeState *fleye_state)
{
   int rc;
   const EGLint* attribs = fleye_state->egl_config_attribs;;

   const EGLint default_attribs[] =
   {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 16,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };

   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   if (! attribs)
      attribs = default_attribs;

   printf("%s\n", __PRETTY_FUNCTION__);
   rc = fleyeutil_gl_common(fleye_state, attribs, context_attribs);
   if (rc != 0) return rc;

   rc = fleyeutil_create_textures(fleye_state);

   return rc;
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
int fleyeutil_do_update_texture(EGLDisplay display, EGLenum target,
      EGLClientBuffer mm_buf, GLuint *texture, EGLImageKHR *egl_image)
{
   //printf("%s: mm_buf %u\n", __PRETTY_FUNCTION__, (unsigned) mm_buf);
   GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, *texture));
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
   return fleyeutil_do_update_texture(fleye_state->display,
         EGL_IMAGE_BRCM_MULTIMEDIA, mm_buf,
         &fleye_state->texture, &fleye_state->egl_image);
}

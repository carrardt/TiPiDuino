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

#ifndef fleye_RASPITEX_H_
#define fleye_RASPITEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "EGL/eglext_brcm.h"
#include "interface/mmal/mmal.h"

#ifdef __cplusplus
}
#endif

#include "fleye/config.h"
#include "fleye/userenv.h"

struct ImageProcessingState;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contains the internal state and configuration for the GL rendered
 * preview window.
 */
typedef struct FleyeState
{
   int version_major;                  /// For binary compatibility
   int version_minor;                  /// Incremented for new features
   MMAL_PORT_T *preview_port;          /// Source port for preview opaque buffers
   MMAL_POOL_T *preview_pool;          /// Pool for storing opaque buffer handles
   MMAL_QUEUE_T *preview_queue;        /// Queue preview buffers to display in order
   VCOS_THREAD_T preview_thread;       /// Preview worker / GL rendering thread
   uint32_t preview_stop;              /// If zero the worker can continue

   /* Copy of preview window params */
   int32_t preview_x;                  /// x-offset of preview window
   int32_t preview_y;                  /// y-offset of preview window
   int32_t preview_width;              /// preview y-plane width in pixels
   int32_t preview_height;             /// preview y-plane height in pixels
   
   /* processing options*/
   char tracking_script[64];
    
   struct UserEnv user_env;
   struct ImageProcessingState* ip;
    
   /* Display rectangle for the native window */
   int32_t x;                          /// x-offset in pixels
   int32_t y;                          /// y-offset in pixels
   int32_t width;                      /// width in pixels
   int32_t height;                     /// height in pixels
   int opacity;                        /// Alpha value for display element
   int gl_win_defined;                 /// Use rect from --glwin instead of preview

   /* DispmanX info. This might be unused if a custom create_native_window
    * does something else. */
   DISPMANX_DISPLAY_HANDLE_T disp;     /// Dispmanx display for GL preview
   EGL_DISPMANX_WINDOW_T win;          /// Dispmanx handle for preview surface

   EGLNativeWindowType* native_window; /// Native window used for EGL surface
   EGLDisplay display;                 /// The current EGL display
   EGLSurface surface;                 /// The current EGL surface
   EGLContext context;                 /// The current EGL context
   const EGLint *egl_config_attribs;   /// GL scenes preferred EGL configuration

   GLuint texture;                     /// Name for the preview texture
   EGLImageKHR egl_image;              /// The current preview EGL image

   GLuint y_texture;                   /// The Y plane texture
   EGLImageKHR y_egl_image;            /// EGL image for Y plane texture
   GLuint u_texture;                   /// The U plane texture
   EGLImageKHR u_egl_image;            /// EGL image for U plane texture
   GLuint v_texture;                   /// The V plane texture
   EGLImageKHR v_egl_image;            /// EGL image for V plane texture

   MMAL_BUFFER_HEADER_T *preview_buf;  /// MMAL buffer currently bound to texture(s)
   int verbose;                        /// Log FPS
} FleyeState;

int fleye_init(FleyeState *state);
void fleye_destroy(FleyeState *state);
int fleye_start(FleyeState *state);
void fleye_stop(FleyeState *state);
void fleye_set_defaults(FleyeState *state);
int fleye_configure_preview_port(FleyeState *state,
      MMAL_PORT_T *preview_port);
void fleye_display_help();
int fleye_parse_cmdline(FleyeState *state,
      const char *arg1, const char *arg2);

#ifdef __cplusplus
}
#endif

#endif /* RASPITEX_H_ */

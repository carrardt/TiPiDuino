#ifndef fleye_fleyecore_H_
#define fleye_fleyecore_H_

#include <stdio.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "EGL/eglext_brcm.h"
#include "interface/mmal/mmal.h"

#include "fleye/config.h"
#include "fleye/fleyecommonstate.h"

struct ImageProcessingState;
struct FleyeNativeWindow;

/**
 * Contains the internal state and configuration for the GL rendered
 * preview window.
 */
struct FleyeState
{
   // non api specific common parameters
   struct FleyeCommonState common;
   
   MMAL_PORT_T *preview_port;          /// Source port for preview opaque buffers
   MMAL_POOL_T *preview_pool;          /// Pool for storing opaque buffer handles
   MMAL_QUEUE_T *preview_queue;        /// Queue preview buffers to display in order
   VCOS_THREAD_T preview_thread;       /// Preview worker / GL rendering thread

   /* DispmanX info. This might be unused if a custom create_native_window
    * does something else. */
   struct FleyeNativeWindow* fleye_window;
   
   EGLImageKHR egl_image;              /// The current preview EGL image
   MMAL_BUFFER_HEADER_T *preview_buf;  /// MMAL buffer currently bound to texture(s)
   
   /* cpu worker thread */
   VCOS_THREAD_T cpuTrackingThread;

   /* synchronization semaphores between cpu workers and gpu worker */
   VCOS_SEMAPHORE_T start_processing_sem;
   VCOS_SEMAPHORE_T end_processing_sem;

   /* processing and user env state */
   struct UserEnv* user_env; // utiliser une api pour positioner/lire des elements depuis l'exterieur (en C)
   struct ImageProcessingState* ip;
};

typedef struct FleyeState FleyeState;

int fleye_init(FleyeState *state);
void fleye_destroy(FleyeState *state);
int fleye_start(FleyeState *state);
void fleye_stop(FleyeState *state);
void fleye_set_defaults(FleyeState *state);
int fleye_configure_preview_port(FleyeState *state, MMAL_PORT_T *preview_port);
void fleye_display_help();
int fleye_parse_cmdline(FleyeState *state, const char *arg1, const char *arg2);

#endif /* RASPITEX_H_ */

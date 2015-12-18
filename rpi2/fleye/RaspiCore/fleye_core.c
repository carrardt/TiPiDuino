#include "RaspiCLI.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "interface/vcos/vcos.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"

#include "fleye_core.h"
#include "fleye_util.h"
#include "fleye/fleye_c.h"
#include "fleye_window.h"

/**
 * \file GPUTracking.c
 *
 * A simple framework for extending a MMAL application to render buffers via
 * OpenGL.
 *
 * MMAL buffers are often in YUV colour space and in either a planar or
 * tile format which is not supported directly by V3D. Instead of copying
 * the buffer from the GPU and doing a colour space / pixel format conversion
 * the GL_OES_EGL_image_external is used. This allows an EGL image to be
 * created from GPU buffer handle (MMAL opaque buffer handle). The EGL image
 * may then be used to create a texture (glEGLImageTargetTexture2DOES) and
 * drawn by either OpenGL ES 1.0 or 2.0 contexts.
 *
 * Notes:
 * 1) GL_OES_EGL_image_external textures always return pixels in RGBA format.
 *    This is also the case when used from a fragment shader.
 *
 * 2) The driver implementation creates a new RGB_565 buffer and does the color
 *    space conversion from YUV. This happens in GPU memory using the vector
 *    processor.
 *
 * 3) Each EGL external image in use will consume GPU memory for the RGB 565
 *    buffer. In addition, the GL pipeline might require more than one EGL image
 *    to be retained in GPU memory until the drawing commands are flushed.
 *
 *    Typically 128 MB of GPU memory is sufficient for 720p viewfinder and 720p
 *    GL surface. If both the viewfinder and the GL surface are 1080p then
 *    256MB of GPU memory is recommended, otherwise, for non-trivial scenes
 *    the system can run out of GPU memory whilst the camera is running.
 *
 * 4) It is important to make sure that the MMAL opaque buffer is not returned
 *    to MMAL before the GL driver has completed the asynchronous call to
 *    glEGLImageTargetTexture2DOES. Deferring destruction of the EGL image and
 *    the buffer return to MMAL until after eglSwapBuffers is the recommended.
 *
 * See also: http://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
 */

#define DEFAULT_WIDTH   (2592/2)
#define DEFAULT_HEIGHT  (1944/2)

/**
 * Parse a possible command pair - command and parameter
 * @param arg1 Command
 * @param arg2 Parameter (could be NULL)
 * @return How many parameters were used, 0,1,2
 */
int fleye_parse_cmdline(FleyeState *state,
      const char *arg1, const char *arg2)
{
	//printf("fleye_parse_cmdline %s %s\n",arg1,arg2);
	if( strcmp(arg1,"-set")==0 )
	{
		char key[64], value[64];
		sscanf(arg2,"%s %s",key,value);
		fleye_add_optional_value(state->user_env,key,value);
		return 2;
	}
	else if( strcmp(arg1,"-script")==0 )
	{
		fleye_set_processing_script(state->user_env,arg2);
		return 2;
	}

   return 0;
}

/**
 * Display help for command line options
 */
void fleye_display_help()
{
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

/**
 * Checks if there is at least one valid EGL image.
 * @param state RASPITEX STATE
 * @return Zero if successful.
 */
static int check_egl_image(FleyeState *state)
{
   if (state->egl_image == EGL_NO_IMAGE_KHR )
      return -1;
   else
      return 0;
}

/**
 * Draws the next preview frame. If a new preview buffer is available then the
 * preview texture is updated first.
 *
 * @param state RASPITEX STATE
 * @param video_frame MMAL buffer header containing the opaque buffer handle.
 * @return Zero if successful.
 */
static int fleye_draw(FleyeState *state, MMAL_BUFFER_HEADER_T *buf)
{
   int rc = 0;

   /* If buf is non-NULL then there is a new viewfinder frame available
    * from the camera so the texture should be updated.
    *
    * Although it's possible to have multiple textures mapped to different
    * viewfinder frames this can consume a lot of GPU memory for high-resolution
    * viewfinders.
    */
   if (buf)
   {
      /* Update the texture to the new viewfinder image which should */
		 rc = fleyeutil_update_texture(state, (EGLClientBuffer) buf->data);
		 if (rc != 0)
		 {
			fprintf(stderr,"%s: Failed to update RGBX texture %d\n",
				  __PRETTY_FUNCTION__, rc);
			return rc;
		 }

      /* Now return the PREVIOUS MMAL buffer header back to the camera preview. */
      if (state->preview_buf)
      {
         mmal_buffer_header_release(state->preview_buf);
	  }
      state->preview_buf = buf;
   }

   /*  Do the drawing */
   if (check_egl_image(state) == 0)
   {
	  rc = glworker_redraw( & state->common, state->ip );
	  if (rc != 0) return rc;
      update_fps();
   }

   return rc;
}

/**
 * Process preview buffers.
 *
 * Dequeue each available preview buffer in order and call current redraw
 * function. If no new buffers are available then the render function is
 * invoked anyway.
 * @param   state The GL preview window state.
 * @return Zero if successful.
 */
static int preview_process_returned_bufs(FleyeState* state)
{
   MMAL_BUFFER_HEADER_T *buf;
   int new_frame = 0;
   int rc = 0;

   while ((buf = mmal_queue_get(state->preview_queue)) != NULL)
   {
      if (state->common.preview_stop == 0)
      {
         new_frame = 1;
         rc = fleye_draw(state, buf);
         if (rc != 0)
         {
            fprintf(stderr,"%s: Error drawing frame. Stopping.\n", __PRETTY_FUNCTION__);
            state->common.preview_stop = 1;
            return rc;
         }
      }
   }

	// non ! on ne gaspille pas de temps GPU/CPU si aucune image n'est disponible
	// en plus ca peut augmenter la latence de traitement d'une nouvelle image
   /* If there were no new frames then redraw the scene again with the previous
    * texture. Otherwise, go round the loop again to see if any new buffers
    * are returned.
    */
   /*if (! new_frame)
   {
      rc = fleye_draw(state, NULL);
   }*/
   return rc;
}

/** Preview worker thread.
 * Ensures camera preview is supplied with buffers and sends preview frames to GL.
 * @param arg  Pointer to state.
 * @return NULL always.
 */
static void *preview_worker(void *arg)
{
   FleyeState* state = arg;
   MMAL_PORT_T *preview_port = state->preview_port;
   MMAL_BUFFER_HEADER_T *buf;
   MMAL_STATUS_T st;
   int rc;

   printf("%s: port %p\n", __PRETTY_FUNCTION__, preview_port);

   //rc = fleyeutil_create_native_window(state);
   state->fleye_window = create_offscreen_native_window(0,0,state->common.width,state->common.height,state->common.opacity);
   //state->fleye_window = create_native_window(0,0,state->common.width,state->common.height,state->common.opacity);
   if ( state->fleye_window != NULL )
   {
	   fleyeutil_gl_init(state);

	   while (state->common.preview_stop == 0)
	   {
		  /* Send empty buffers to camera preview port */
		  while ((buf = mmal_queue_get(state->preview_pool->queue)) != NULL)
		  {
			 st = mmal_port_send_buffer(preview_port, buf);
			 if (st != MMAL_SUCCESS)
			 {
				fprintf(stderr,"Failed to send buffer to %s\n", preview_port->name);
			 }
		  }
		  /* Process returned buffers */
		  if (preview_process_returned_bufs(state) != 0)
		  {
			 fprintf(stderr,"Preview error. Exiting.\n");
			 state->common.preview_stop = 1;
		  }
	   }
   }
   
   /* Make sure all buffers are returned on exit */
   while ((buf = mmal_queue_get(state->preview_queue)) != NULL)
   {
      mmal_buffer_header_release(buf);
   }

   /* Tear down GL */
   fleyeutil_gl_term(state);
   printf("Exiting preview worker\n");
   return NULL;
}

/**
 * MMAL Callback from camera preview output port.
 * @param port The camera preview port.
 * @param buf The new preview buffer.
 **/
static void preview_output_cb(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf)
{
   FleyeState *state = (FleyeState*) port->userdata;

   if (buf->length == 0)
   {
      printf("%s: zero-length buffer => EOS\n", port->name);
      state->common.preview_stop = 1;
      mmal_buffer_header_release(buf);
   }
   else if (buf->data == NULL)
   {
      printf("%s: zero buffer handle\n", port->name);
      mmal_buffer_header_release(buf);
   }
   else
   {
      /* Enqueue the preview frame for rendering and return to
       * avoid blocking MMAL core.
       */
      mmal_queue_put(state->preview_queue, buf);
   }
}

/* Registers a callback on the camera preview port to receive
 * notifications of new frames.
 * This must be called before rapitex_start and may not be called again
 * without calling fleye_destroy first.
 *
 * @param state Pointer to the GL preview state.
 * @param port  Pointer to the camera preview port
 * @return Zero if successful.
 */
int fleye_configure_preview_port(FleyeState *state,
      MMAL_PORT_T *preview_port)
{
   MMAL_STATUS_T status;
   printf("%s port %p\n", __PRETTY_FUNCTION__, preview_port);

   /* Enable ZERO_COPY mode on the preview port which instructs MMAL to only
    * pass the 4-byte opaque buffer handle instead of the contents of the opaque
    * buffer.
    * The opaque handle is resolved on VideoCore by the GL driver when the EGL
    * image is created.
    */
   status = mmal_port_parameter_set_boolean(preview_port,
         MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
   if (status != MMAL_SUCCESS)
   {
      fprintf(stderr,"Failed to enable zero copy on camera preview port\n");
      goto end;
   }

   status = mmal_port_format_commit(preview_port);
   if (status != MMAL_SUCCESS)
   {
      fprintf(stderr,"camera viewfinder format couldn't be set\n");
      goto end;
   }

   /* For GL a pool of opaque buffer handles must be allocated in the client.
    * These buffers are used to create the EGL images.
    */
   state->preview_port = preview_port;
   preview_port->buffer_num = preview_port->buffer_num_recommended;
   preview_port->buffer_size = preview_port->buffer_size_recommended;

   printf("Creating buffer pool for GL renderer num %d size %d\n",
         preview_port->buffer_num, preview_port->buffer_size);

   /* Pool + queue to hold preview frames */
   state->preview_pool = mmal_port_pool_create(preview_port,
         preview_port->buffer_num, preview_port->buffer_size);

   if (! state->preview_pool)
   {
      fprintf(stderr,"Error allocating pool\n");
      status = MMAL_ENOMEM;
      goto end;
   }

   /* Place filled buffers from the preview port in a queue to render */
   state->preview_queue = mmal_queue_create();
   if (! state->preview_queue)
   {
      fprintf(stderr,"Error allocating queue\n");
      status = MMAL_ENOMEM;
      goto end;
   }

   /* Enable preview port callback */
   preview_port->userdata = (struct MMAL_PORT_USERDATA_T*) state;
   status = mmal_port_enable(preview_port, preview_output_cb);
   if (status != MMAL_SUCCESS)
   {
      fprintf(stderr,"Failed to camera preview port\n");
      goto end;
   }
end:
   return (status == MMAL_SUCCESS ? 0 : -1);
}

/* Initialises GL preview state and creates the dispmanx native window.
 * @param state Pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleye_init(FleyeState *state)
{
   VCOS_STATUS_T status;
   int rc;
   vcos_init();

   return 0;
}

/* Destroys the pools of buffers used by the GL renderer.
 * @param  state Pointer to the GL preview state.
 */
void fleye_destroy(FleyeState *state)
{
   printf("%s\n", __PRETTY_FUNCTION__);
   if (state->preview_pool)
   {
      mmal_pool_destroy(state->preview_pool);
      state->preview_pool = NULL;
   }

   if (state->preview_queue)
   {
      mmal_queue_destroy(state->preview_queue);
      state->preview_queue = NULL;
   }

   destroy_native_window(state->fleye_window);
}

/* Initialise the GL / window state to sensible defaults.
 * Also initialise any rendering parameters e.g. the scene
 *
 * @param state Pointer to the GL preview state.
 * @return Zero if successful.
 */
void fleye_set_defaults(FleyeState *state)
{
   // set to zero
   memset(state, 0, sizeof(*state));

	// initialize only non-zero values
   state->common.display = EGL_NO_DISPLAY;
   state->common.surface = EGL_NO_SURFACE;
   state->common.context = EGL_NO_CONTEXT;
   state->egl_image = EGL_NO_IMAGE_KHR;
   state->common.opacity = 255;
   state->common.width = DEFAULT_WIDTH;
   state->common.height = DEFAULT_HEIGHT;
   
   state->user_env = fleye_create_user_env();
   fleye_set_processing_script(state->user_env,"passthru");
}

/* Stops the rendering loop and destroys MMAL resources
 * @param state  Pointer to the GL preview state.
 */
void fleye_stop(FleyeState *state)
{
   if (! state->common.preview_stop)
   {
      printf("Stopping GL preview\n");
      state->common.preview_stop = 1;
      vcos_thread_join(&state->preview_thread, NULL);
   }
}

/**
 * Starts the worker / GL renderer thread.
 * @pre fleye_init was successful
 * @pre fleye_configure_preview_port was successful
 * @param state Pointer to the GL preview state.
 * @return Zero on success, otherwise, -1 is returned
 * */
int fleye_start(FleyeState *state)
{
   VCOS_STATUS_T status;

   printf("%s\n", __PRETTY_FUNCTION__);
   status = vcos_thread_create(&state->preview_thread, "preview-worker",
         NULL, preview_worker, state);

   if (status != VCOS_SUCCESS)
      fprintf(stderr,"%s: Failed to start worker thread %d\n",
            __PRETTY_FUNCTION__, status);

   return (status == VCOS_SUCCESS ? 0 : -1);
}



/*
 * Note : to get maximum frame rate choose -ex fixedfps
 * */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sysexits.h>

#include "CameraStream.h"

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "RaspiCamControl.h"

// holds a camera stream context
struct s_CameraStream
{
	RASPICAM_CAMERA_PARAMETERS camera_parameters;
	MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
	MMAL_PORT_T *stream_port ;
	MMAL_POOL_T * stream_pool;
	MMAL_QUEUE_T *stream_queue;
	int stream_stop ;
	
	UserBufferCopyFunc buffer_copy_func;
	UserStreamInitializeFunc buffer_process_func;
	void* user_data;
};
typedef struct s_CameraStream CameraStream;


// signal handler. quit app properly when user press ^C
static int UserInterrupt = 0;
static void signal_handler(int signal_number)
{
   if (signal_number == SIGINT)
   {
	   UserInterrupt = 1;
   }
}

static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
      MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
      switch (param->hdr.id) {
         case MMAL_PARAMETER_CAMERA_SETTINGS:
         {
            MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
            printf("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
						settings->exposure,
                        settings->analog_gain.num, settings->analog_gain.den,
                        settings->digital_gain.num, settings->digital_gain.den);
            printf("AWB R=%u/%u, B=%u/%u",
                        settings->awb_red_gain.num, settings->awb_red_gain.den,
                        settings->awb_blue_gain.num, settings->awb_blue_gain.den);
         }
         break;
      }
   }
   else if (buffer->cmd == MMAL_EVENT_ERROR)
   {
      fprintf(stderr,"No data received from sensor. Check all connections, including the Sunny one on the camera board");
   }
   else 
   {
      fprintf(stderr,"Received unexpected camera control callback event, 0x%08x", buffer->cmd);
	}
	
   mmal_buffer_header_release(buffer);
}

/*********************************/
/** Create the camera component **/
/*********************************/
static void create_camera_component(CameraStream* cs, int cameraNum)
{
   MMAL_STATUS_T status = MMAL_SUCCESS;
   MMAL_COMPONENT_T *camera = 0;
   MMAL_ES_FORMAT_T *format;
   MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;

   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
   assert(status==MMAL_SUCCESS);
	
   MMAL_PARAMETER_STEREOSCOPIC_MODE_T stereo_mode = { .mode=MMAL_STEREOSCOPIC_MODE_NONE };
   status = raspicamcontrol_set_stereo_mode(camera->output[0], &stereo_mode);
   status += raspicamcontrol_set_stereo_mode(camera->output[1], &stereo_mode);
   status += raspicamcontrol_set_stereo_mode(camera->output[2], &stereo_mode);
   assert(status==MMAL_SUCCESS);

   MMAL_PARAMETER_INT32_T camera_num =
      {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, cameraNum };
   status = mmal_port_parameter_set(camera->control, &camera_num.hdr);
   assert(status==MMAL_SUCCESS);

   assert( camera->output_num > 0 );
   
   int sensor_mode = 0;
   status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, sensor_mode);
   assert(status==MMAL_SUCCESS);

   preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
   video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
   still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

   // Enable the camera, and tell it its control callback function
   status = mmal_port_enable(camera->control, camera_control_callback);
   assert(status==MMAL_SUCCESS);

   //  set up the camera configuration
   {
      MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
      {
         { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
         .max_stills_w = CAPTURE_WIDTH,
         .max_stills_h = CAPTURE_HEIGHT,
         .stills_yuv422 = 0,
         .one_shot_stills = 1,
         .max_preview_video_w = CAPTURE_WIDTH,
         .max_preview_video_h = CAPTURE_HEIGHT,
         .num_preview_video_frames = 3,
         .stills_capture_circular_buffer_height = 0,
         .fast_preview_resume = 0,
         .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
      };

      status = mmal_port_parameter_set(camera->control, &cam_config.hdr);
	  assert(status==MMAL_SUCCESS);
   }
 
	// set user camera configuration
   raspicamcontrol_set_all_parameters(camera, &cs->camera_parameters);

   // Now set up the port formats
   format = preview_port->format;
   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   if(cs->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
   }
   else if(cs->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 166, 1000 }, {999, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
   }

	// Use a full FOV 4:3 mode
	format->es->video.width = VCOS_ALIGN_UP(CAPTURE_WIDTH, 32);
	format->es->video.height = VCOS_ALIGN_UP(CAPTURE_HEIGHT, 16);
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = CAPTURE_WIDTH;
	format->es->video.crop.height = CAPTURE_HEIGHT;
	format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
	format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

	status = mmal_port_format_commit(preview_port);
	assert(status==MMAL_SUCCESS);

	// Set the same format on the video  port (which we dont use here)
	mmal_format_full_copy(video_port->format, format);
	status = mmal_port_format_commit(video_port);
	assert(status==MMAL_SUCCESS);
	
   // Ensure there are enough buffers to avoid dropping frames
   if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

	// configure still port format
   format = still_port->format;
   if(cs->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(still_port, &fps_range.hdr);
   }
   else if(cs->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 167, 1000 }, {999, 1000}};
        mmal_port_parameter_set(still_port, &fps_range.hdr);
   }
   // Set our stills format on the stills (for encoder) port
   format->encoding = MMAL_ENCODING_OPAQUE;
   format->es->video.width = VCOS_ALIGN_UP(CAPTURE_WIDTH, 32);
   format->es->video.height = VCOS_ALIGN_UP(CAPTURE_HEIGHT, 16);
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = CAPTURE_WIDTH;
   format->es->video.crop.height = CAPTURE_HEIGHT;
   format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
   format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;

   status = mmal_port_format_commit(still_port);
   assert(status==MMAL_SUCCESS);

   /* Ensure there are enough buffers to avoid dropping frames */
   if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   /* Enable component */
   status = mmal_component_enable(camera);
   assert(status==MMAL_SUCCESS);

   cs->camera_component = camera;
}

static void camera_stream_callback(MMAL_PORT_T* port,MMAL_BUFFER_HEADER_T* buf)
{
   CameraStream* cs = (CameraStream*) port->userdata;
   if (buf->length == 0)
   {
      printf("%s: zero-length buffer => EOS\n", port->name);
      cs->stream_stop = 1;
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
      mmal_queue_put(cs->stream_queue, buf);
   }
}

static int configure_camera_stream(CameraStream* cs)
{
   MMAL_STATUS_T status;

   /* Enable ZERO_COPY mode on the preview port which instructs MMAL to only
    * pass the 4-byte opaque buffer handle instead of the contents of the opaque
    * buffer.
    * The opaque handle is resolved on VideoCore by the GL driver when the EGL
    * image is created.
    */
   status = mmal_port_parameter_set_boolean(cs->stream_port,
         MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
   assert(status==MMAL_SUCCESS);

   status = mmal_port_format_commit(cs->stream_port);
   assert(status==MMAL_SUCCESS);

   /* For GL a pool of opaque buffer handles must be allocated in the client.
    * These buffers are used to create the EGL images.
    */
   cs->stream_port->buffer_num = cs->stream_port->buffer_num_recommended;
   cs->stream_port->buffer_size = cs->stream_port->buffer_size_recommended;

   printf("Creating buffer pool for GL renderer num %d size %d\n",
         cs->stream_port->buffer_num, cs->stream_port->buffer_size);

   /* Pool + queue to hold preview frames */
   cs->stream_pool = mmal_port_pool_create(cs->stream_port,
         cs->stream_port->buffer_num, cs->stream_port->buffer_size);
   assert(cs->stream_pool!=NULL);

   /* Place filled buffers from the preview port in a queue to render */
   cs->stream_queue = mmal_queue_create();
   assert(cs->stream_queue!=NULL);

   /* Enable preview port callback */
   cs->stream_port->userdata = (struct MMAL_PORT_USERDATA_T*) cs;
   status = mmal_port_enable(cs->stream_port, camera_stream_callback);
   assert(status==MMAL_SUCCESS);

   return (status == MMAL_SUCCESS ? 0 : -1);
}

static int stream_process_buffer(CameraStream* cs, MMAL_BUFFER_HEADER_T *buf)
{
   if (buf)
   {
	  /* retreive unused previous buffer */
	  MMAL_BUFFER_HEADER_T * prev_buf = (*cs->buffer_copy_func)( buf, cs->user_data );
	  
      /* Now return the PREVIOUS MMAL buffer header back to the camera preview. */
      if (prev_buf)
      {
         mmal_buffer_header_release(prev_buf);
	  }
   }

   /*  Do the processing */
   (*cs->buffer_process_func)( cs->user_data );

   return 0;
}

static int stream_process_returned_bufs(CameraStream* cs)
{
   MMAL_BUFFER_HEADER_T *buf;
   int new_frame = 0;
   int rc = 0;

   while ((buf = mmal_queue_get(cs->stream_queue)) != NULL)
   {
      if (cs->stream_stop == 0)
      {
         new_frame = 1;
         rc = stream_process_buffer(cs,buf);
         if (rc != 0)
         {
            fprintf(stderr,"%s: Error drawing frame. Stopping.\n", __PRETTY_FUNCTION__);
            cs->stream_stop = 1;
            return rc;
         }
      }
   }

   // uncomment if you want more frames per second than the camera delivers
   /*if (! new_frame)
   {
      rc = stream_process_buffer(cs,NULL);
   }*/
   
   return rc;
}

int camera_streamer_init()
{
   /*********************************/
   /** HW interface init           **/
   /*********************************/
   bcm_host_init();
   vcos_init();
   
   return 0;	
}

int camera_stream(int argc, char * argv[],
	int cameraNum,
	UserStreamInitializeFunc user_init_func,
	UserBufferCopyFunc buf_copy_func,
	UserBufferProcessFunc buf_proc_func,
	UserStreamFinalizeFunc user_final_func,
	void* user_data )
{
   MMAL_STATUS_T status = MMAL_SUCCESS;
   MMAL_PORT_T *stream_port = NULL;
   MMAL_BUFFER_HEADER_T *buf = NULL;
   int i;
   CameraStream* camstream = NULL; 
   
   
   /*********************************/
   /** Allocate Camera context     **/
   /*********************************/
   camstream = (CameraStream*) malloc(sizeof(CameraStream));
   memset(camstream,0,sizeof(CameraStream));
   camstream->buffer_copy_func = buf_copy_func;
   camstream->buffer_process_func = buf_proc_func;
   camstream->user_data = user_data;


   /*********************************/
   /** Parse RaspiCam command line **/
   /*********************************/
   raspicamcontrol_set_defaults(&camstream->camera_parameters);
   for(i=1;i<argc;i++)
   {
		if(argv[i][0]=='-')
		{
			if( strcmp(argv[i],"-help")==0 )
			{
				raspicamcontrol_display_help();
				exit(1);
			}
			else
			{
				const char *second_arg = (i + 1 < argc) ? argv[i + 1] : NULL;
				int parms_used = raspicamcontrol_parse_cmdline(&camstream->camera_parameters, &argv[i][1], second_arg);
				i += parms_used - 1;
			}
	    }
	    else
	    {
			fprintf(stderr,"Invalid argument '%s'\n",argv[i]);
			raspicamcontrol_display_help();
			exit(1);
		}
	}


   /*********************************/
   /** Camera streaming setup      **/
   /*********************************/
	create_camera_component(camstream, cameraNum);
	printf("Camera Ok\n");
	
	camstream->stream_port = camstream->camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
	configure_camera_stream(camstream);
	printf("Stream configuration Ok\n");
	
	
   /*********************************/
   /** CTRL-C signal handler       **/
   /*********************************/
   camstream->stream_stop = 0;
   signal(SIGINT, signal_handler);

   /*********************************/
   /** User initialization         **/
   /*********************************/
   if( user_init_func != NULL ) { (*user_init_func)(camstream->user_data); }


   /*********************************/
   /** streaming main loop         **/
   /*********************************/
   printf("Start streaming ...\n");
   while ( !UserInterrupt && !camstream->stream_stop )
   {
	  /* Send empty buffers to camera preview port */
	  while ((buf = mmal_queue_get(camstream->stream_pool->queue)) != NULL)
	  {
		 status = mmal_port_send_buffer(camstream->stream_port, buf);
		 assert(status==MMAL_SUCCESS);
	  }
	  /* Process returned buffers */
	  if (stream_process_returned_bufs(camstream) != 0)
	  {
		 fprintf(stderr,"Preview error. Exiting.\n");
		 camstream->stream_stop = 1;
	  }
   }
   printf("\n... Stop streaming\n");
   
 
   /*********************************/
   /** clean-up                    **/
   /*********************************/
   /* Make sure all buffers are returned on exit */
   while ((buf = mmal_queue_get(camstream->stream_queue)) != NULL)
   {
      mmal_buffer_header_release(buf);
   }
   
	// user finalization func
   if( user_final_func != NULL ) { (*user_final_func)(camstream->user_data); }
	
	// finalize camera component
	mmal_component_destroy(camstream->camera_component);
	
	free(camstream);
	
	return 0;
}

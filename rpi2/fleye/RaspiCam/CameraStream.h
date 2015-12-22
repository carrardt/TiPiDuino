#ifndef __fleye_CameraStream_h
#define __fleye_CameraStream_h

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Stills format information
// 0 implies variable
#define STILLS_FRAME_RATE_NUM 0
#define STILLS_FRAME_RATE_DEN 1

#define PREVIEW_FRAME_RATE_NUM 0
#define PREVIEW_FRAME_RATE_DEN 1

#define FULL_RES_PREVIEW_FRAME_RATE_NUM 0
#define FULL_RES_PREVIEW_FRAME_RATE_DEN 1

#define CAPTURE_WIDTH 1296
#define CAPTURE_HEIGHT 972

#define VIDEO_OUTPUT_BUFFERS_NUM 3

#ifdef __cplusplus
extern "C" {
#endif

#include "interface/mmal/mmal.h"
#include "RaspiCamControl.h"

typedef int(*UserStreamInitializeFunc)(void*);
typedef int(*UserStreamFinalizeFunc)(void*);
typedef int(*UserBufferProcessFunc)(void*);
typedef MMAL_BUFFER_HEADER_T*(*UserBufferCopyFunc)(MMAL_BUFFER_HEADER_T*,void*);

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


extern int camera_streamer_init();

extern int camera_stream(int argc, char * argv[],
	int cameraNum,
	UserStreamInitializeFunc user_init_func,
	UserBufferCopyFunc buf_copy_func,
	UserBufferProcessFunc buf_proc_func,
	UserStreamFinalizeFunc user_final_func,
	void* user_data );


#ifdef __cplusplus
}
#endif


#endif

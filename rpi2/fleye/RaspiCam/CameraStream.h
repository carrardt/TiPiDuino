#ifndef __fleye_CameraStream_h
#define __fleye_CameraStream_h

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Stills format information
// 0 implies variable
#define FRAME_RATE_NUM 0
#define FRAME_RATE_DEN 1

#define VIDEO_OUTPUT_BUFFERS_NUM 3

#ifdef __cplusplus
extern "C" {
#endif

struct MMAL_BUFFER_HEADER_T;

typedef int(*UserStreamInitializeFunc)(void*);
typedef int(*UserStreamFinalizeFunc)(void*);
typedef int(*UserBufferProcessFunc)(void*);
typedef struct MMAL_BUFFER_HEADER_T*(*UserBufferCopyFunc)(struct MMAL_BUFFER_HEADER_T*,void*);

extern int camera_streamer_init();

extern int camera_stream(int argc, char * argv[],
	int cameraNum,
	int captureWidth,
	int captureHeight,
	UserStreamInitializeFunc user_init_func,
	UserBufferCopyFunc buf_copy_func,
	UserBufferProcessFunc buf_proc_func,
	UserStreamFinalizeFunc user_final_func,
	void* user_data )
;


#ifdef __cplusplus
}
#endif


#endif

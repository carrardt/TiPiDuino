#include <stdio.h>
#include <sys/time.h>

#include "CameraStream.h"

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

int user_process(void* user_data)
{
	update_fps();
	return 0;
}
int user_initialize(void* user_data)
{
	printf("Application initialized\n");
	return 0;
}
int user_finalize(void* user_data)
{
	printf("Application terminated\n");
	return 0;
}

//static GLuint cameraTextureId = 0;
//static EGLImageKHR camera_egl_image = EGL_NO_IMAGE_KHR;
struct MMAL_BUFFER_HEADER_T * user_copy_buffer(struct MMAL_BUFFER_HEADER_T *buf, void* user_data)
{
	// hold buffer while we process it
	static struct MMAL_BUFFER_HEADER_T * previous_buffer = NULL;
    struct MMAL_BUFFER_HEADER_T * pb = previous_buffer;
	
	int* bufCount = (int*)user_data;
	++ (*bufCount);
	
   previous_buffer = buf;

   return pb;
}

int main(int argc, char * argv[])
{
	int buffer_count=0;
	camera_stream(argc,argv,0,1296,972,user_initialize,user_copy_buffer,user_process,user_finalize,&buffer_count);
	printf("Buffers processed = %d\n",buffer_count);
}

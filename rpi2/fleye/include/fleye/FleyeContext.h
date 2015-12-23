#ifndef fleye_context_H_
#define fleye_context_H_

#include <stdint.h>
#include <GLES2/gl2.h>

#include <string>
#include <map>

struct ImageProcessingState;
struct FleyeContextInternal;

struct FleyeContext
{
   FleyeContext();
   ~FleyeContext();
   void setIntegerVar(const std::string& key, int value);

	
   /* Display rectangle for the native window */
   int32_t x;                          /// x-offset in pixels
   int32_t y;                          /// y-offset in pixels
   uint32_t width;                      /// width in pixels
   uint32_t height;                     /// height in pixels	

	// camera capture resolution
   uint32_t captureWidth;
   uint32_t captureHeight;
   
   // count processed frames
   uint32_t frameCounter;

   GLuint cameraTextureId;				// GL id of special texture fed with camera
	
	/* contains Dispmanx native window and EGL display/sruface/context */
   struct FleyeRenderWindow* render_window;
   
   /* user env vars */
	std::map<std::string,std::string> vars;
	std::string script;
   
   /* image processing pipeline */
   struct ImageProcessingState* ip;

   struct FleyeContextInternal* priv;
   
   bool verbose;
};

extern int postStartProcessingSem( struct FleyeContext* ctx );
extern int waitStartProcessingSem( struct FleyeContext* ctx );
extern int postEndProcessingSem( struct FleyeContext* ctx );
extern int waitEndProcessingSem( struct FleyeContext* ctx );

#endif

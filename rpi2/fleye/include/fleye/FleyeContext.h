#ifndef fleye_context_H_
#define fleye_context_H_

#include <stdint.h>
#include <GLES2/gl2.h>

struct UserEnv;
struct ImageProcessingState;
struct FleyeContextInternal;

#ifdef __cplusplus
extern "C" {
#endif

struct FleyeContext
{
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
   struct UserEnv* user_env; 
   
   /* image processing pipeline */
   struct ImageProcessingState* ip;

   struct FleyeContextInternal* priv;
};

extern int postStartProcessingSem( struct FleyeContext* ctx );
extern int waitStartProcessingSem( struct FleyeContext* ctx );
extern int postEndProcessingSem( struct FleyeContext* ctx );
extern int waitEndProcessingSem( struct FleyeContext* ctx );

extern void fleye_create_user_env(struct FleyeContext* ctx);
extern void fleye_set_processing_script(struct FleyeContext* ctx, const char* scriptName);
extern const char* fleye_get_processing_script(struct FleyeContext* ctx);
extern const char* fleye_optional_value(struct FleyeContext* ctx, const char* key);
extern void fleye_add_optional_value(struct FleyeContext* ctx, const char* key, const char* value);

#ifdef __cplusplus
}
#endif


#endif

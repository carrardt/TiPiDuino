#ifndef fleye_FleyeCommonState_H_
#define fleye_FleyeCommonState_H_

#include <GLES2/gl2.h>

struct FleyeState;

#ifdef __cplusplus
extern "C" {
#endif

struct FleyeCommonState
{
   struct FleyeState* fleye_state;
   
   uint32_t preview_stop;              /// If zero the worker can continue

   /* Display rectangle for the native window */
   int32_t x;                          /// x-offset in pixels
   int32_t y;                          /// y-offset in pixels
   int32_t width;                      /// width in pixels
   int32_t height;                     /// height in pixels	

   uint32_t frameCounter;

   GLuint cameraTextureId;				// GL id of special texture fed with camera

   int opacity;                        /// Alpha value for display element
   int gl_win_defined;                 /// Use rect from --glwin instead of preview

   int verbose;                        /// Log FPS
};

#ifdef __cplusplus
}
#endif

#endif

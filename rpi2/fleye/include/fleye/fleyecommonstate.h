#ifndef fleye_FleyeCommonState_H_
#define fleye_FleyeCommonState_H_

#include <EGL/egl.h>

struct FleyeState;

#ifdef __cplusplus
extern "C" {
#endif

struct FleyeCommonState
{
   struct FleyeState* fleye_state;
   
   EGLDisplay display;                 /// The current EGL display
   EGLSurface surface;                 /// The current EGL surface
   EGLContext context;                 /// The current EGL context

   uint32_t preview_stop;              /// If zero the worker can continue

   /* Copy of preview window params */
   int32_t preview_x;                  /// x-offset of preview window
   int32_t preview_y;                  /// y-offset of preview window
   int32_t preview_width;              /// preview y-plane width in pixels
   int32_t preview_height;             /// preview y-plane height in pixels

   /* Display rectangle for the native window */
   int32_t x;                          /// x-offset in pixels
   int32_t y;                          /// y-offset in pixels
   int32_t width;                      /// width in pixels
   int32_t height;                     /// height in pixels	

   uint32_t frameCounter;

   int opacity;                        /// Alpha value for display element
   int gl_win_defined;                 /// Use rect from --glwin instead of preview

   int verbose;                        /// Log FPS
};

#ifdef __cplusplus
}
#endif

#endif

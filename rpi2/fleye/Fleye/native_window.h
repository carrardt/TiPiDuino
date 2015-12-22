#ifndef __fleye_window_H_
#define __fleye_window_H_

#include <EGL/egl.h>

extern "C" {
#include "interface/vmcs_host/vc_dispmanx.h"
}

/**
 * Contains the internal state and configuration for a native window
 */
struct FleyeNativeWindow
{
   DISPMANX_DISPLAY_HANDLE_T 	disp;     /// Dispmanx display for GL preview
   DISPMANX_ELEMENT_HANDLE_T 	win_elem;
   DISPMANX_RESOURCE_HANDLE_T 	win_res;
   uint32_t 					win_image_handle;
   DISPMANX_UPDATE_HANDLE_T 	win_update;
   EGL_DISPMANX_WINDOW_T 		win;          /// Dispmanx handle for preview surface   
   EGLNativeWindowType* 		native_window; /// Native window used for EGL surface
   int 							width;
   int 							height;
};

extern struct FleyeNativeWindow* create_offscreen_native_window(int x, int y, int width, int height, int opacity);
extern struct FleyeNativeWindow* create_native_window(int x, int y, int width, int height, int opacity);
extern int read_offscreen_image(struct FleyeNativeWindow* fleye_win, uint8_t* dst_buffer);
extern void destroy_native_window(struct FleyeNativeWindow* fleye_win);


#endif /* fleye_window */

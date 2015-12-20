#ifndef __fleye_rednerwindow_H_
#define __fleye_rednerwindow_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

struct FleyeNativeWindow;

#ifdef __cplusplus
extern "C" {
#endif

struct FleyeRenderWindow
{
	struct FleyeNativeWindow* 	fleye_window;
    EGLDisplay 					display;
    EGLSurface 					surface;
    EGLContext 					context;
    uint8_t * 					read_back_buffer;
};

extern struct FleyeRenderWindow* create_render_window(int x, int y, int width, int height, const EGLint * attribs );
extern struct FleyeRenderWindow* create_offscreen_render_window(int width, int height, const EGLint * attribs,struct FleyeRenderWindow* sharedCtxWin);
extern const uint8_t * read_offscreen_render_window(struct FleyeRenderWindow* renwin, uint32_t* w, uint32_t* h );
extern int destroy_render_window(struct FleyeRenderWindow* renwin);

#ifdef __cplusplus
}
#endif

#endif /* fleye_window */

#ifndef __fleye_rednerwindow_H_
#define __fleye_rednerwindow_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

struct FleyeNativeWindow;

struct FleyeRenderWindow
{    
	FleyeRenderWindow(int x, int y, int width, int height, const EGLint * attribs, FleyeRenderWindow* sharedCtxWin=NULL, bool offscreen=false);
	~FleyeRenderWindow();
	uint8_t* readBack(int &w, int& h);
    
   	FleyeNativeWindow* 	fleye_window;
    EGLDisplay 			display;
    EGLSurface 			surface;
    EGLContext 			context;

  private:

	void create_egl_context(const EGLint * attribs, FleyeRenderWindow* sharedCtxWin);

    uint8_t * 			read_back_buffer;

};

#endif /* fleye_window */

#include <EGL/egl.h>
#include <stdio.h>
#include <assert.h>

#include "fleye/FleyeRenderWindow.h"
#include "native_window.h"

FleyeRenderWindow::FleyeRenderWindow(int x,int y,int width, int height, const EGLint * attribs, FleyeRenderWindow* sharedCtxWin, bool offscreen)
{
	if(offscreen)
	{
		this->fleye_window = create_offscreen_native_window(0,0,width,height,255);
		this->read_back_buffer = new uint8_t[width*height*4];
		memset(this->read_back_buffer,0,width*height*4);
	}
	else 
	{
		this->fleye_window = create_native_window(x,y,width,height,255);
		this->read_back_buffer = 0;
	}
	assert(this->fleye_window!=NULL && "Failed to create native window");
	this->create_egl_context(attribs,sharedCtxWin);
}

FleyeRenderWindow::~FleyeRenderWindow()
{
   /* Terminate EGL */
   eglMakeCurrent(this->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(this->display, this->context);
   eglDestroySurface(this->display, this->surface);
   //eglTerminate(this->display);

	destroy_native_window( this->fleye_window );
	this->fleye_window = NULL;
	
	if(read_back_buffer!=0) delete [] read_back_buffer;
}

uint8_t* FleyeRenderWindow::readBack(int &w, int& h)
{
	if(this->read_back_buffer == NULL)
	{
		w=0;
		h=0;
		return 0;
	}
	read_offscreen_image(this->fleye_window, this->read_back_buffer);
	w = this->fleye_window->width;
	h = this->fleye_window->height;
	return this->read_back_buffer;
}

void FleyeRenderWindow::create_egl_context(const EGLint * attribs, FleyeRenderWindow* sharedCtxWin)
{
   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   EGLConfig config;
   EGLint num_configs;

   printf("%s\n", __PRETTY_FUNCTION__);

   if (this->fleye_window->native_window == NULL)
   {
      fprintf(stderr,"%s: No native window\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (this->display == EGL_NO_DISPLAY)
   {
      fprintf(stderr,"%s: Failed to get EGL display\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (! eglInitialize(this->display, 0, 0))
   {
      fprintf(stderr,"%s: eglInitialize failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (! eglChooseConfig(this->display, attribs, &config,
            1, &num_configs))
   {
      fprintf(stderr,"%s: eglChooseConfig failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->surface = eglCreateWindowSurface(this->display,
         config, this->fleye_window->native_window, NULL);
   if (this->surface == EGL_NO_SURFACE)
   {
      fprintf(stderr,"%s: eglCreateWindowSurface failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->context = eglCreateContext(this->display, config, sharedCtxWin ? sharedCtxWin->context : EGL_NO_CONTEXT, context_attribs);
   if (this->context == EGL_NO_CONTEXT)
   {
      fprintf(stderr,"%s: eglCreateContext failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (!eglMakeCurrent(this->display, this->surface, this->surface, this->context))
   {
      fprintf(stderr,"%s: Failed to activate EGL context\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   eglSwapInterval(this->display,0); // no VSync
}

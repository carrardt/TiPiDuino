#include <stdio.h>

#include "fleye/render_window.h"
#include "native_window.h"

/** Creates the EGL context and window surface for the native window
 * using specified arguments.
 * @param fleye_state  A pointer to the GL preview state. This contains
 *                        the native_window pointer.
 * @param attribs         The config attributes.
 * @param context_attribs The context attributes.
 * @return Zero if successful.
 */
static int fleye_egl_create_context(
	struct FleyeRenderWindow* renwin,
	const EGLint * attribs,
	struct FleyeRenderWindow* sharedCtxWin ) 
{
   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   EGLConfig config;
   EGLint num_configs;

   printf("%s\n", __PRETTY_FUNCTION__);

   if (renwin->fleye_window->native_window == NULL)
   {
      fprintf(stderr,"%s: No native window\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   renwin->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (renwin->display == EGL_NO_DISPLAY)
   {
      fprintf(stderr,"%s: Failed to get EGL display\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   if (! eglInitialize(renwin->display, 0, 0))
   {
      fprintf(stderr,"%s: eglInitialize failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   if (! eglChooseConfig(renwin->display, attribs, &config,
            1, &num_configs))
   {
      fprintf(stderr,"%s: eglChooseConfig failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   renwin->surface = eglCreateWindowSurface(renwin->display,
         config, renwin->fleye_window->native_window, NULL);
   if (renwin->surface == EGL_NO_SURFACE)
   {
      fprintf(stderr,"%s: eglCreateWindowSurface failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   renwin->context = eglCreateContext(renwin->display, config, sharedCtxWin ? sharedCtxWin->context : EGL_NO_CONTEXT, context_attribs);
   if (renwin->context == EGL_NO_CONTEXT)
   {
      fprintf(stderr,"%s: eglCreateContext failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   if (!eglMakeCurrent(renwin->display, renwin->surface, renwin->surface, renwin->context))
   {
      fprintf(stderr,"%s: Failed to activate EGL context\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      return -1;
   }

   eglSwapInterval(renwin->display,0); // no VSync

   return 0;
}

struct FleyeRenderWindow* create_render_window(int x, int y, int width, int height, const EGLint * attribs )
{
	int rc;
	struct FleyeRenderWindow* renwin = (struct FleyeRenderWindow*) malloc( sizeof(struct FleyeRenderWindow) );

    printf("%s(%d,%d,%d,%d,%p)\n", __PRETTY_FUNCTION__,x,y,width,height,attribs);

	memset( renwin , 0 , sizeof(struct FleyeRenderWindow) );
	renwin->fleye_window = create_native_window(x,y,width,height,255);
	if( renwin->fleye_window == NULL )
	{
		fprintf(stderr,"Failed to create native window\n");
		free(renwin);
		return NULL;
	}
	rc = fleye_egl_create_context(renwin,attribs,NULL);
	if( rc != 0 )
	{
		fprintf(stderr,"Failed to create EGL context\n");
		free(renwin);
		return NULL;
	}
	return renwin;
}

struct FleyeRenderWindow* create_offscreen_render_window(
	int width, int height, const EGLint * attribs,
	struct FleyeRenderWindow* sharedCtxWin )
{
	int rc;
	struct FleyeRenderWindow* renwin = (struct FleyeRenderWindow*) malloc( sizeof(struct FleyeRenderWindow) );
	memset( renwin , 0 , sizeof(struct FleyeRenderWindow) );
	renwin->fleye_window = create_offscreen_native_window(0,0,width,height,255);
	if( renwin->fleye_window == NULL )
	{
		fprintf(stderr,"Failed to create native window\n");
		free(renwin);
		return NULL;
	}
	rc = fleye_egl_create_context(renwin,attribs,sharedCtxWin);
	if( rc != 0 )
	{
		fprintf(stderr,"Failed to create EGL context\n");
		free(renwin);
		return NULL;
	}
	renwin->read_back_buffer = malloc(width*height*4);
	memset(renwin->read_back_buffer,0,width*height*4);
	return renwin;
}

const uint8_t * read_offscreen_render_window(struct FleyeRenderWindow* renwin, uint32_t* w, uint32_t* h)
{
	if(renwin->read_back_buffer == NULL) return NULL;
	read_offscreen_image(renwin->fleye_window, renwin->read_back_buffer);
	*w = renwin->fleye_window->width;
	*h = renwin->fleye_window->height;
	return renwin->read_back_buffer;
}

int destroy_render_window(struct FleyeRenderWindow* renwin)
{
   /* Terminate EGL */
   eglMakeCurrent(renwin->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(renwin->display, renwin->context);
   eglDestroySurface(renwin->display, renwin->surface);
   eglTerminate(renwin->display);

	destroy_native_window( renwin->fleye_window );
	renwin->fleye_window = NULL;
	free( renwin );
	return 0;
}

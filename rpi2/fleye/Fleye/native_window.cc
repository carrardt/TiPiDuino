extern "C" {
#include "interface/vmcs_host/vc_dispmanx.h"
}

#include "native_window.h"
#include <stdio.h>

/** Creates a native window for the GL surface using dispmanx
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful, otherwise, -1 is returned.
 */
struct FleyeNativeWindow* create_native_window(int x, int y, int width, int height, int opacity)
{
   VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
   VC_RECT_T src_rect = {0};
   VC_RECT_T dest_rect = {0};
   uint32_t disp_num = 0; // Primary
   uint32_t layer_num = 0;
   //DISPMANX_ELEMENT_HANDLE_T elem;
   //DISPMANX_UPDATE_HANDLE_T update;

	FleyeNativeWindow* fleye_win = (FleyeNativeWindow*) malloc(sizeof(struct FleyeNativeWindow));
	memset(fleye_win,0,sizeof(struct FleyeNativeWindow));

	fleye_win->width = width;
	fleye_win->height = height;

   alpha.opacity = opacity;
   dest_rect.x = x;
   dest_rect.y = y;
   dest_rect.width = width;
   dest_rect.height = height;

   printf("%s(%d,%d,%d,%d,%d)\n", __PRETTY_FUNCTION__,x,y,width,height,opacity);

   src_rect.width = dest_rect.width << 16;
   src_rect.height = dest_rect.height << 16;

   fleye_win->disp = vc_dispmanx_display_open(disp_num);
   if (fleye_win->disp == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to open display handle\n");
      return NULL;
   }

   fleye_win->win_update = vc_dispmanx_update_start(0);
   if (fleye_win->win_update == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to open update handle\n");
      return NULL;
   }

   fleye_win->win_elem = vc_dispmanx_element_add(fleye_win->win_update, fleye_win->disp, 0,
         &dest_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, &alpha, NULL,
         DISPMANX_NO_ROTATE);
   if (fleye_win->win_elem == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to create element handle\n");
      return NULL;
   }

   fleye_win->win.element = fleye_win->win_elem;
   fleye_win->win.width = fleye_win->width;
   fleye_win->win.height = fleye_win->height;
   vc_dispmanx_update_submit_sync(fleye_win->win_update);

   fleye_win->native_window = (EGLNativeWindowType*) &fleye_win->win;

   return fleye_win;
}

struct FleyeNativeWindow* create_offscreen_native_window(int x, int y, int width, int height, int opacity)
{
   VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
   VC_RECT_T src_rect = {0,0,0,0};
   VC_RECT_T dest_rect = {0,0,0,0};

   //alpha.flags = DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_MIX;
   alpha.opacity = opacity;
   
   dest_rect.x = x;
   dest_rect.y = y;
   dest_rect.width = width;
   dest_rect.height = height;
   src_rect.width = dest_rect.width << 16;
   src_rect.height = dest_rect.height << 16;

	FleyeNativeWindow* fleye_win = (FleyeNativeWindow*) malloc(sizeof(struct FleyeNativeWindow));
	memset(fleye_win,0,sizeof(struct FleyeNativeWindow));

	fleye_win->width = width;
	fleye_win->height = height;

   // create destination resource
   fleye_win->win_image_handle = 0;
   fleye_win->win_res = vc_dispmanx_resource_create(
		VC_IMAGE_RGBA32,
		fleye_win->width,
		fleye_win->height, 
		& fleye_win->win_image_handle );
   if ( fleye_win->win_res == 0 )
   {
      fprintf(stderr,"Failed to create resource\n");
      return NULL;
   }

	// open offscreen display
   fleye_win->disp = vc_dispmanx_display_open_offscreen(fleye_win->win_res, DISPMANX_NO_ROTATE);         
   if (fleye_win->disp == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to open offscreen display\n");
      return NULL;
   }

	// create update
   fleye_win->win_update = vc_dispmanx_update_start(0);
   if (fleye_win->win_update == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to open update handle\n");
      return NULL;
   }

   fleye_win->win_elem = vc_dispmanx_element_add(
		fleye_win->win_update,
		fleye_win->disp,
		0,
		&dest_rect,
		0, // no source resource
        &src_rect,
        DISPMANX_PROTECTION_NONE,
        &alpha,
        NULL,
        DISPMANX_NO_ROTATE );
 
   if (fleye_win->win_elem == DISPMANX_NO_HANDLE)
   {
      fprintf(stderr,"Failed to create element handle\n");
      return NULL;
   }

   fleye_win->win.element = fleye_win->win_elem;
   fleye_win->win.width = fleye_win->width;
   fleye_win->win.height = fleye_win->height;
   vc_dispmanx_update_submit_sync(fleye_win->win_update);

   fleye_win->native_window = (EGLNativeWindowType*) &fleye_win->win;

   return fleye_win;
}

int read_offscreen_image(struct FleyeNativeWindow* fleye_win, uint8_t* dst_buffer)
{
   VC_RECT_T dest_rect = {0,0,0,0};
   uint32_t dst_pitch = fleye_win->width * 4;
   dest_rect.x = 0;
   dest_rect.y = 0;
   dest_rect.width = fleye_win->width;
   dest_rect.height = fleye_win->height;
   vc_dispmanx_resource_read_data(fleye_win->win_res, &dest_rect, dst_buffer, dst_pitch);
   return 0;
}

/** Destroys the pools of buffers used by the GL renderer.
 * @param fleye_state A pointer to the GL preview state.
 */
void destroy_native_window(struct FleyeNativeWindow* fleye_win)
{
   printf("%s\n", __PRETTY_FUNCTION__);
   if (fleye_win->disp != DISPMANX_NO_HANDLE)
   {
      vc_dispmanx_display_close(fleye_win->disp);
      fleye_win->disp = DISPMANX_NO_HANDLE;
   }
}

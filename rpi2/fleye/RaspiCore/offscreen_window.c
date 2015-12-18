#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "interface/vmcs_host/vc_dispmanx.h"
#include "fleye/fleye_c.h" // -> c_api.h
#include "fleye/fleyecommonstate.h" // -> commonstate.h
#include "fleye_core.h" 
#include "fleye_window.h"

struct FleyeNativeWindow* create_offscreen_native_window(int x, int y, int width, int height, int opacity)
{
   VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
   VC_RECT_T src_rect = {0,0,0,0};
   VC_RECT_T dest_rect = {0,0,0,0};

   alpha.opacity = opacity;
   dest_rect.x = x;
   dest_rect.y = y;
   dest_rect.width = width;
   dest_rect.height = height;
   src_rect.width = dest_rect.width << 16;
   src_rect.height = dest_rect.height << 16;

	struct FleyeNativeWindow* fleye_win = malloc(sizeof(struct FleyeNativeWindow));
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

int fleye_readback(struct FleyeState* fleye_state, uint8_t* dst_buffer)
{
	return read_offscreen_image( fleye_state->fleye_window, dst_buffer );
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

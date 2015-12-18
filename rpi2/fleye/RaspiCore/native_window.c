#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "interface/vmcs_host/vc_dispmanx.h"
#include "fleye/fleye_c.h" // -> c_api.h
#include "fleye/fleyecommonstate.h" // -> commonstate.h
#include "fleye_core.h" 
#include "fleye_window.h"

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

	struct FleyeNativeWindow* fleye_win = malloc(sizeof(struct FleyeNativeWindow));
	memset(fleye_win,0,sizeof(struct FleyeNativeWindow));

	fleye_win->width = width;
	fleye_win->height = height;

   alpha.opacity = opacity;
   dest_rect.x = x;
   dest_rect.y = y;
   dest_rect.width = width;
   dest_rect.height = height;

   printf("%s: %d,%d,%d,%d %d,%d,0x%x,0x%x\n", __PRETTY_FUNCTION__,
         src_rect.x, src_rect.y, src_rect.width, src_rect.height,
         dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height);

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

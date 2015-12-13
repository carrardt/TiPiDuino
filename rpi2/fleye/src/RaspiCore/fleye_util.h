#ifndef __fleye_UTIL_H_
#define __fleye_UTIL_H_

struct FleyeState;

/* Default GL scene ops functions */
int fleyeutil_create_native_window(struct FleyeState *fleye_state);
int fleyeutil_gl_init_2_0(struct FleyeState *fleye_state);
void fleyeutil_gl_term(struct FleyeState *fleye_state);
void fleyeutil_destroy_native_window(struct FleyeState *fleye_state);
int fleyeutil_create_textures(struct FleyeState *fleye_state);
int fleyeutil_update_texture(struct FleyeState *fleye_state, EGLClientBuffer mm_buf);

#endif /* RASPITEX_UTIL_H_ */

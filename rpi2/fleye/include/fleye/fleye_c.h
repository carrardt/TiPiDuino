#ifndef __fleye_glworker_h
#define __fleye_glworker_h

#include <GLES2/gl2.h>
#include <EGL/egl.h>

struct FleyeState;
struct FleyeCommonState;
struct ImageProcessingState;
struct UserEnv;

#ifdef __cplusplus
extern "C" {
#endif

extern struct FleyeRenderWindow* fleye_get_preview_window(struct FleyeState* fleye_state);
extern struct FleyeRenderWindow* create_render_buffer(struct FleyeState* fleye_state, int width, int height);

extern const EGLint* glworker_egl_config(struct FleyeCommonState* state);
extern int glworker_redraw(struct FleyeCommonState* state, struct ImageProcessingState* ip);
extern struct ImageProcessingState* glworker_init(struct FleyeCommonState* state, struct UserEnv* user_env);

extern struct UserEnv* fleye_create_user_env();

extern void fleye_set_processing_script(struct UserEnv* uenv, const char* scriptName);
extern const char* fleye_get_processing_script(struct UserEnv* uenv);

extern const char* fleye_optional_value(struct UserEnv* env, const char* key);
extern int fleye_add_optional_value(struct UserEnv* env, const char* key, const char* value);

extern void waitStartProcessingSem(struct FleyeState* fleye_state);
extern void postStartProcessingSem(struct FleyeState* fleye_state);
extern void waitEndProcessingSem(struct FleyeState* fleye_state);
extern void postEndProcessingSem(struct FleyeState* fleye_state);

extern void *cpuTrackingWorker(void *arg);

#ifdef __cplusplus
}
#endif

#endif

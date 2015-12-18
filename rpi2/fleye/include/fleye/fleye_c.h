#ifndef __fleye_glworker_h
#define __fleye_glworker_h

#include <GLES2/gl2.h>
#include <EGL/egl.h>

struct FleyeState;
struct FleyeCommonState;
struct ImageProcessingState;
struct UserEnv;
struct FrameBufferObject;
struct ShaderProgram;
struct ShaderPass;
struct GLTexture;
struct CPU_TRACKING_STATE;

#ifdef __cplusplus
extern "C" {
#endif

extern int fleye_readback(struct FleyeState* fleye_state, uint8_t* dst_buffer);

extern const EGLint* glworker_egl_config(struct FleyeCommonState* state);
extern int glworker_redraw(struct FleyeCommonState* state, struct ImageProcessingState* ip);
extern struct ImageProcessingState* glworker_init(struct FleyeCommonState* state, struct UserEnv* user_env, struct CPU_TRACKING_STATE** cpuThreadCtx );
extern struct UserEnv* fleye_create_user_env();
extern void fleye_set_processing_script(struct UserEnv* uenv, const char* scriptName);
extern const char* fleye_get_processing_script(struct UserEnv* uenv);
extern const char* fleye_optional_value(struct UserEnv* env, const char* key);
extern int fleye_add_optional_value(struct UserEnv* env, const char* key, const char* value);
extern GLuint fleye_get_camera_texture_id(struct ImageProcessingState* ip);
extern int create_image_processing(struct ImageProcessingState* ip, struct UserEnv* env, const char* filename);

extern void waitStartProcessingSem(struct FleyeState* fleye_state);
extern void postStartProcessingSem(struct FleyeState* fleye_state);
extern void waitEndProcessingSem(struct FleyeState* fleye_state);
extern void postEndProcessingSem(struct FleyeState* fleye_state);

extern void *cpuTrackingWorker(void *arg);

#ifdef __cplusplus
}
#endif

#endif

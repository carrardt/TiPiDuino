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
struct RASPITEX_Texture;

#ifdef __cplusplus
extern "C" {
#endif

extern const EGLint* glworker_egl_config(struct FleyeCommonState* state);
extern int glworker_redraw(struct FleyeCommonState* state, struct ImageProcessingState* ip);
extern struct ImageProcessingState* glworker_init(struct FleyeCommonState* state, struct UserEnv* user_env);
extern struct UserEnv* fleye_create_user_env();
extern void fleye_set_processing_script(struct UserEnv* uenv, const char* scriptName);
extern const char* fleye_get_processing_script(struct UserEnv* uenv);
extern const char* fleye_optional_value(struct UserEnv* env, const char* key);
extern int fleye_add_optional_value(struct UserEnv* env, const char* key, const char* value);
extern GLuint fleye_get_camera_texture_id(struct ImageProcessingState* ip);
extern int create_image_processing(struct ImageProcessingState* ip, struct UserEnv* env, const char* filename);
extern char* readShader(const char* fileName);
extern int fleyeutil_build_shader_program(struct ShaderProgram *p, const char* vertex_source, const char* fragment_source);
extern int create_image_shader(struct ShaderProgram* shader, const char* vs, const char* fs);
extern struct CompiledShaderCache* get_compiled_shader(struct ShaderPass* shaderPass, struct RASPITEX_Texture ** inputs);
extern int add_fbo(struct ImageProcessingState* ip, const char* name, GLint colorFormat, GLint w, GLint h);
extern struct FrameBufferObject* get_named_fbo(struct ImageProcessingState* ip, const char * name);
extern struct RASPITEX_Texture* get_named_texture(struct ImageProcessingState* ip, const char * name);

extern void *cpuTrackingWorker(void *arg);

#ifdef __cplusplus
}
#endif

#endif

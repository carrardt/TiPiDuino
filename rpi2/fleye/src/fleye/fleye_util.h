#ifndef RASPITEX_UTIL_H_
#define RASPITEX_UTIL_H_

/* Uncomment to enable extra GL error checking */
//#define CHECK_GL_ERRORS
#if defined(CHECK_GL_ERRORS)
#define GLCHK(X) \
do { \
    GLenum err = GL_NO_ERROR; \
    X; \
   while ((err = glGetError())) \
   { \
      fprintf(stderr,"GL error 0x%x in " #X "file %s line %d", err, __FILE__,__LINE__); \
      assert(err == GL_NO_ERROR); \
      exit(err); \
   } \
} \
while(0)
#else
#define GLCHK(X) X
#endif /* CHECK_GL_ERRORS */

struct FleyeState;

/* Default GL scene ops functions */
int fleyeutil_create_native_window(struct FleyeState *fleye_state);
int fleyeutil_gl_init_2_0(struct FleyeState *fleye_state);
void fleyeutil_gl_term(struct FleyeState *fleye_state);
void fleyeutil_destroy_native_window(struct FleyeState *fleye_state);
int fleyeutil_create_textures(struct FleyeState *fleye_state);
int fleyeutil_update_texture(struct FleyeState *fleye_state, EGLClientBuffer mm_buf);

#endif /* RASPITEX_UTIL_H_ */

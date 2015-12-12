#ifndef __fleye_glworker_h
#define __fleye_glworker_h

struct FleyeState;
struct ImageProcessingState;

#ifdef __cplusplus
extern "C" {
#endif

extern const EGLint* glworker_egl_config(struct ImageProcessingState* ip);
extern int glworker_redraw(FleyeState *state);
extern int glworker_init(FleyeState *state);

#ifdef __cplusplus
}
#endif

#endif

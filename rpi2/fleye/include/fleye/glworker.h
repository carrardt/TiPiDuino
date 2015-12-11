#ifndef __fleye_glworker_h
#define __fleye_glworker_h

#ifdef __cplusplus
extern "C" {
#endif

struct FleyeState;
extern int glworker_redraw(FleyeState *state);
extern int glworker_init(FleyeState *state);

#ifdef __cplusplus
}
#endif

#endif

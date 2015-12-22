#ifndef __fleye_glworker_h
#define __fleye_glworker_h

struct FleyeContext;

#ifdef __cplusplus
extern "C" {
#endif

extern int glworker_init(struct FleyeContext* ctx);
extern int glworker_redraw(struct FleyeContext* ctx);

#ifdef __cplusplus
}
#endif


#endif

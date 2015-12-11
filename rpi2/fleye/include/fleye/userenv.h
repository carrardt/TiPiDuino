#ifndef __fleye_UserEnv_H_
#define __fleye_UserEnv_H_

#include "fleye/config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct UserEnv
{
        int n_opt_values;
        char opt_values[MAX_OPT_VALUES][2][32];
};

extern const char* fleye_optional_value(struct UserEnv* env, const char* key);
extern int fleye_add_optional_value(struct UserEnv* env, const char* key, const char* value);

#ifdef __cplusplus
}
#endif

#endif

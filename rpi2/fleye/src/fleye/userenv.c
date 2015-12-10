#include <string.h>
#include "fleye/userenv.h"

const char* fleye_optional_value(struct UserEnv* env, const char* key)
{
	int i;
	for(i=0;i<env->n_opt_values;i++)
	{
		if( strcmp(env->opt_values[i][0],key) == 0 ) return env->opt_values[i][1];
	}
	return "";
}

int fleye_add_optional_value(struct UserEnv* env, const char* key, const char* value)
{
	strcpy(env->opt_values[env->n_opt_values][0],key);
	strcpy(env->opt_values[env->n_opt_values][1],value);
	++ env->n_opt_values;
	return env->n_opt_values ;
}

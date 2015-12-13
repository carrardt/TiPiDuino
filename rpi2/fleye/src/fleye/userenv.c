#include <string.h>
#include "fleye/fleye_c.h"
#include "fleye/config.h"

struct UserEnv
{
        int n_opt_values;
        char opt_values[MAX_OPT_VALUES][2][32];
        char tracking_script[64];
};

struct UserEnv* fleye_create_user_env()
{
	struct UserEnv* uenv = malloc( sizeof(struct UserEnv) );
	memset(uenv,0,sizeof(struct UserEnv));
	return uenv;
}

void fleye_set_processing_script(struct UserEnv* uenv, const char* scriptName)
{
	if( scriptName==0 ) scriptName="";
	strncpy(uenv->tracking_script,scriptName,63);
	printf("Processing script is now '%s'\n",uenv->tracking_script);
}

const char* fleye_get_processing_script(struct UserEnv* uenv)
{
	return uenv->tracking_script;
}

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

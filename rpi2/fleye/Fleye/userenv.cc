#include "fleye/FleyeContext.h"

#include <string>
#include <map>

struct UserEnv
{
	std::map<std::string,std::string> vars;
	std::string script;
};

void fleye_create_user_env(struct FleyeContext* ctx)
{
	ctx->user_env = new UserEnv;
}

void fleye_set_processing_script(struct FleyeContext* ctx, const char* scriptName)
{
	ctx->user_env->script = scriptName;
}

const char* fleye_get_processing_script(struct FleyeContext* ctx)
{
	return ctx->user_env->script.c_str();
}

const char* fleye_optional_value(struct FleyeContext* ctx, const char* key)
{
	return ctx->user_env->vars[key].c_str();
}

void fleye_add_optional_value(struct FleyeContext* ctx, const char* key, const char* value)
{
	ctx->user_env->vars[key] = value;
}

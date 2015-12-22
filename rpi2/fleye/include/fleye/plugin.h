#ifndef __fleye_plugin_H_
#define __fleye_plugin_H_

struct FleyeContext;

typedef void(*PluginSetupFunc)(FleyeContext*);

#define FLEYE_REGISTER_PLUGIN(name) \
extern "C" { void name##_setup(FleyeContext*); void name##_run(FleyeContext*); }

#define FLEYE_REGISTER_GL_DRAW(name) \
extern "C" { void name(CompiledShaderCache*,int); }
	
#endif

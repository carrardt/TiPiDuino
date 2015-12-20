#ifndef __fleye_plugin_H_
#define __fleye_plugin_H_

struct CPU_TRACKING_STATE;
struct CompiledShaderCache;
struct ImageProcessingState;

typedef void(*PluginSetupFunc)(const ImageProcessingState*);

#define FLEYE_REGISTER_PLUGIN(name) \
extern "C" { void name##_setup(const ImageProcessingState*); void name##_run(const ImageProcessingState*,CPU_TRACKING_STATE*); }

#define FLEYE_REGISTER_GL_DRAW(name) \
extern "C" { void name(CompiledShaderCache*,int); }
	
#endif

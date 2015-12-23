#ifndef __fleye_plugin_H_
#define __fleye_plugin_H_

#include <map>
#include <string>

struct FleyeContext;
struct CompiledShader;
struct ProcessingStep;

class FleyePlugin
{
  public:
    FleyePlugin();
	void setupOnce(FleyeContext* ctx);
	std::string name() const ;
	FleyePlugin* registerPlugin(const char* name);
	static FleyePlugin* plugin(FleyeContext* ctx,std::string name);

	virtual void setup(FleyeContext* ctx) {}
	virtual void run(FleyeContext* ctx) {}
	virtual void draw(FleyeContext*,CompiledShader*,int) {}

  private:
	std::string m_name;
	FleyeContext* m_ctx;
	bool m_initialized;
	static std::map<std::string,FleyePlugin*> s_plugins;
};

#define FLEYE_REGISTER_PLUGIN(name) \
FleyePlugin* name##_instance = (new name)->registerPlugin(#name)
	
#endif

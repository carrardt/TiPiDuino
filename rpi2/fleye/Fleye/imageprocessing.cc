#include <stdio.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <json/json.h>

#include "fleye/imageprocessing.h"
#include "fleye/shaderpass.h"
#include "fleye/fbo.h"
#include "fleye/texture.h"
#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "fleye/FleyeRenderWindow.h"

FleyeRenderWindow* ImageProcessingState::getRenderBuffer(const std::string& name) const
{
	std::map<std::string,FrameBufferObject*>::const_iterator it = fbo.find(name);
	if( it != fbo.end() )
	{
		return it->second->render_window;
	}
	else
	{
		return 0;
	}
}

static const EGLint egl_fbo_attribs[] =
{
   EGL_RED_SIZE,   8,
   EGL_GREEN_SIZE, 8,
   EGL_BLUE_SIZE,  8,
   EGL_ALPHA_SIZE, 8,
   EGL_DEPTH_SIZE, 16,
   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
   EGL_NONE
};

static int64_t get_integer_value( FleyeContext* ctx, Json::Value x )
{
	if( x.isNumeric() ) return x.asInt64();
	
	if( x.isString() )
	{
		std::string s = x.asString();
		if( s.empty() ) return 0;
		std::string resolvedStr = s;
		if( s[0] == '$' )
		{
			resolvedStr = ctx->vars[s.substr(1)];
		}
		std::istringstream iss(resolvedStr);
		int64_t i = 0 ;
		iss>>i;
		return i;
	}
	
	return 0;
}

static std::string get_string_value( FleyeContext* ctx, Json::Value x )
{
	//if( ! x.isString() ) return "";
	std::string s = x.asString();
	if( s.empty() ) return s;
	if( s[0] == '$' )
	{
		s = ctx->vars[s.substr(1)];
	}
	return s;
}

static void* dynlib_func_addr(const std::string& plugin, const std::string& funcName)
{
	void * handle = NULL;
	if( ! plugin.empty() )
	{
		std::string libFile = FLEYE_PLUGIN_DIR;
		libFile += "/lib" + plugin + ".so";
		// std::cout<<"load plugin "<<libFile<<"\n";
		handle = dlopen(libFile.c_str(), RTLD_GLOBAL | RTLD_NOW);
		if(handle==NULL)
		{
			std::cerr<<"failed to load plugin "<<plugin<<"\n";
			return 0;
		}
	}
	else
	{
		handle = dlopen(NULL, RTLD_GLOBAL | RTLD_NOW);
	}
	void* proc_addr = dlsym(handle,funcName.c_str());
	if( proc_addr == 0 )
	{
		std::cerr<<"failed to find symbol "<<funcName<<"\n";
	}
	return proc_addr;
}

static std::string readShader(const std::string& shaderName)
{
	std::string fileName = GLSL_SRC_DIR;
	fileName += "/" + shaderName + ".glsl";
	std::ifstream t(fileName);
	if( ! t ){
		std::cerr<<"Can't open file "<<fileName<<"\n";
		return "";
	}
	else {
		return std::string( std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>() );
	}
}

static std::vector<std::string> get_string_array(FleyeContext* env, Json::Value l)
{
	std::vector<std::string> v;
	if( l.isArray() )
	{
		for( const Json::Value& item : l )
		{
			v.push_back( get_string_value(env,item) );
		}
	}
	else if( l.isString() )
	{
		v.push_back( get_string_value(env,l) );
	}
	return v;
}

FrameSet get_frame_set(FleyeContext* env, Json::Value frame)
{
	if( frame.isString() )
	{
		std::string value = get_string_value(env,frame);
		if( value == "odd" ) { return FrameSet(2,1); }
		if( value == "even" ) { return FrameSet(2,0); }
	}
	return FrameSet(); //all frames
}

int read_image_processing_script(FleyeContext* ctx)
{
	// TODO: transferer dans inc_fs et inc_vs
	static const std::string uniforms = 	
		"uniform vec2 step;\n"
		"uniform vec2 size;\n"
		"uniform float iter;\n"
		"uniform float iter2i;\n"
		"uniform vec2 step2i;\n"
		;

	static const std::string vs_attributes = 
		"attribute vec3 vertex;\n"
		;

	static const std::string inc_fs = readShader("inc_fs");

	ctx->ip->cpu_tracking_state.cpuFunc = 0;
	ctx->ip->cpu_tracking_state.nAvailCpuFuncs = 0;
	ctx->ip->cpu_tracking_state.nFinishedCpuFuncs = 0;

	std::string filePath = std::string(FLEYE_SCRIPT_DIR) + "/" + ctx->script + ".json";
	if( ctx->verbose ) { std::cout<<"reading "<<filePath<<"\n\n"; }
	std::ifstream scriptFile(filePath.c_str());
    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( scriptFile, root );
    if ( !parsingSuccessful ){
        // report to the user the failure and their locations in the document.
        std::cerr<< "Failed to parse script\n"<< reader.getFormattedErrorMessages();
        return 1;
    }
    
    // read default user variable values
    const Json::Value envDefaults = root["EnvDefaults"];
    for( auto var : envDefaults.getMemberNames() )
	{
		std::string value = ctx->vars[var];
		if( value.empty() )
		{
			value = get_string_value(ctx,envDefaults[var]);
			if( ctx->verbose ) { std::cout<<"User variable '"<<var<<"' defaults to '"<<value<<"'\n"; }
			ctx->vars[var] = value;
		}
	}
 
    const Json::Value renbufs = root["RenderBuffers"];
	for( auto name : renbufs.getMemberNames() )
	{
		const Json::Value rbuf = renbufs[name];
		int64_t w = get_integer_value(ctx,rbuf.get("width","$WIDTH"));
		int64_t h = get_integer_value(ctx,rbuf.get("height","$HEIGHT"));
		if( ctx->verbose ) { std::cout<<"Add Render buffer '"<<name<<"' : "<<w<<"x"<<h<<"\n"; }
		FrameBufferObject* fbo = new FrameBufferObject;
		fbo->render_window = new FleyeRenderWindow(0,0,w,h,egl_fbo_attribs,ctx->render_window,true);
		assert( fbo->render_window != 0 );
		fbo->width = w;
		fbo->height = h;
		fbo->texture = new GLTexture;
		fbo->texture->format = GL_RGBA;
		ctx->ip->fbo[name] = fbo;
	}

    const Json::Value fbos = root["FrameBufferObjects"];
	for( auto name : fbos.getMemberNames() )
	{
		const Json::Value fboValue = fbos[name];
		int64_t w = get_integer_value(ctx,fboValue.get("width","$WIDTH"));
		int64_t h = get_integer_value(ctx,fboValue.get("height","$HEIGHT"));
		if( ctx->verbose ) { std::cout<<"Add Frame Buffer Object '"<<name<<"' : "<<w<<"x"<<h<<"\n"; }
		FrameBufferObject* fbo = add_fbo(ctx->ip,name,GL_RGBA,w,h);
		fbo->render_window = ctx->render_window;
	}
	
    const Json::Value shadersObject = root["GLShaders"];
    std::map< std::string , ShaderPass* > shadersDB;
	for( auto name : shadersObject.getMemberNames() )
	{
		const Json::Value shader = shadersObject[name];

		// building shader pass content
		ShaderPass* shaderPass = new ShaderPass;
		shaderPass->finalTexture = new GLTexture();
		shadersDB[ name ] = shaderPass;

		// add a texture alias to the shader output.
		// name of the shader can be used as a texture name
		//if( ctx->verbose ) { std::cout<<"create texture alias '"<<name<<"'\n"; }
		ctx->ip->texture[ name ] = shaderPass->finalTexture;
	}
	for( auto name : shadersObject.getMemberNames() )
	{
		if( ctx->verbose ) { std::cout<<"Shader '"<<name<<"'\n"; }
		Json::Value shader = shadersObject[name];
		ShaderPass* shaderPass = shadersDB[ name ];

		// read vertex shader
		shaderPass->vertexSource = vs_attributes+"\n"+uniforms+"\n"+
			readShader( get_string_value(ctx,shader.get("vertex-shader","common_vs")) );
		if( ctx->verbose ) { std::cout<<"\tVertex shader size = "<<shaderPass->vertexSource.size()<<"\n"; }
		
		// read fragment shader
		shaderPass->fragmentSourceWithoutTextures = uniforms+"\n"+inc_fs+"\n" +
			readShader( get_string_value(ctx,shader.get("fragment-shader","passthru_fs")) );
		if( ctx->verbose ) { std::cout<<"\tFramgent shader size = "<<shaderPass->fragmentSourceWithoutTextures.size()<<"\n"; }

		const Json::Value render = shader["rendering"];
		std::string renderPlugin = get_string_value(ctx,render.get("plugin",""));
		std::string renderFunc = get_string_value(ctx,render.get("function","gl_fill"));
		shaderPass->gl_draw = (GLRenderFunctionT) dynlib_func_addr( renderPlugin, renderFunc );
		if( ctx->verbose ) { 
			std::cout<<"\tRender method:";
			if(!renderPlugin.empty()) std::cout<<" plugin="<<renderPlugin;
			std::cout<<" function="<<renderFunc<<" @"<<shaderPass->gl_draw<<"\n";
		}

		const Json::Value textures = shader["textures"];
		for( auto name : textures.getMemberNames() )
		{
			TextureInput texInput;
			texInput.uniformName = name;
			if( ctx->verbose ) { std::cout<<"\tInput '"<<texInput.uniformName<<"' <-"; }
			for( auto textureName : get_string_array(ctx,textures[name]) )
			{
				if( ctx->verbose ) { std::cout<<" "<<textureName; }
				GLTexture* tex = ctx->ip->texture[textureName];
				if( tex != 0 ) { texInput.texPool.push_back( tex ); }
				else { std::cerr<<" texture '"<<textureName<<"' not found\n"; }
			}
			if( ctx->verbose ) { std::cout<<"\n"; }
			shaderPass->inputs.push_back( texInput );
		}
		
		if( ctx->verbose ) { std::cout<<"\tOutput :"; }
		for( auto name : get_string_array(ctx,shader.get("output","DISPLAY")) )
		{
			if( ctx->verbose ) { std::cout<<" "<<name; }
			shaderPass->fboPool.push_back( ctx->ip->fbo[name] );
		}
		if( ctx->verbose ) { std::cout<<"\n"; }

		shaderPass->numberOfPasses = get_integer_value(ctx,shader.get("passes",1));
		if( ctx->verbose ) { std::cout<<"\tPasses : "<<shaderPass->numberOfPasses<<"\n"; }
	}

	if( ctx->verbose ) { std::cout<<"CPU functions :\n"; }
    const Json::Value cpuFuncsObject = root["CPUFunctions"];
	std::map< std::string , CpuPass* > cpuFuncDB;
	for( auto name : cpuFuncsObject.getMemberNames() )
	{
		//std::cout<<"\n+++++ Parsing cpu pass "<<name<<" +++++\n";
		const Json::Value cpuFuncObject = cpuFuncsObject[name];
		std::string plugin = get_string_value(ctx,cpuFuncObject.get("plugin",""));
		std::string setupName = get_string_value(ctx,cpuFuncObject.get("setup",""));
		std::string funcName = get_string_value(ctx,cpuFuncObject.get("run",""));

		// isn't it highly secure ? we're doing graphics anyway, we don't care ;-)
		PluginSetupFunc setup_function = ( PluginSetupFunc ) dynlib_func_addr(plugin,setupName);
		if( setup_function != 0 )
		{ 
			//std::cout<<"initialize plugin ...\n";
			(*setup_function) ( ctx );
		}

		CpuPass* cpu = new CpuPass;
		cpu->exec_thread = get_integer_value(ctx,cpuFuncObject.get("thread-id",0));
		cpu->cpu_processing = (CpuProcessingFunc) dynlib_func_addr(plugin,funcName);
		if( ctx->verbose ) { std::cout<<"\t"<<name<<" : thread="<<cpu->exec_thread<<", function @"<<cpu->cpu_processing<<"\n"; }
		cpuFuncDB[name] = cpu;
	}
	
	if( ctx->verbose ) { std::cout<<"Processing loop :\n"; }
	for( const Json::Value stepValue : root["ProcessingLoop"] )
	{
		ProcessingStep ps;
		std::string name;
		if( stepValue.isString() )
		{
			name = get_string_value(ctx,stepValue);
		}
		else if( stepValue.isObject() )
		{
			name = get_string_value(ctx,stepValue["step"]);
			ps.onFrames = get_frame_set(ctx,stepValue["frames"]);
		}
		ps.shaderPass = shadersDB[name];
		ps.cpuPass = cpuFuncDB[name];
		if( ctx->verbose ) { std::cout<<"\tstep '"<<name<<"' "<<ps.onFrames.modulus<<"/"<<ps.onFrames.value<<"\n"; }
		ctx->ip->processing_step.push_back( ps );
	}
	
	return 0;
}

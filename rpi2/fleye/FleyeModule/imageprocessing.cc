#include <stdio.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <json/json.h>

#include "fleye/imageprocessing.h"
#include "fleye/shaderpass.h"
#include "fleye/fleye_c.h"

GLuint fleye_get_camera_texture_id(struct ImageProcessingState* ip)
{
	return ip->cameraTextureId;
}

static int64_t get_integer_value( struct UserEnv* env, Json::Value x )
{
	if( x.isNumeric() ) return x.asInt64();
	
	if( x.isString() )
	{
		std::string s = x.asString();
		if( s.empty() ) return 0;
		std::string resolvedStr = s;
		if( s[0] == '$' )
		{
			resolvedStr = fleye_optional_value( env, s.c_str()+1 );
		}
		std::istringstream iss(resolvedStr);
		int64_t i;
		iss>>i;
		return i;
	}
	
	return 0;
}

static std::string get_string_value( struct UserEnv* env, Json::Value x )
{
	std::string s = x.asString();
	if( s.empty() ) return s;
	if( s[0] == '$' )
	{
		s = fleye_optional_value( env, s.c_str()+1 );
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
		std::cout<<"load plugin "<<libFile<<"\n";
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

int create_image_processing(struct ImageProcessingState* ip, struct UserEnv* env, const char* filename)
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

	std::string filePath = std::string(FLEYE_SCRIPT_DIR) + "/" + filename + ".json";
	std::cout<<"reading "<<filePath<<"\n\n";
	std::ifstream scriptFile(filePath.c_str());
    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( scriptFile, root );
    if ( !parsingSuccessful ){
        // report to the user the failure and their locations in the document.
        std::cerr<< "Failed to parse script\n"
                   << reader.getFormattedErrorMessages();
        return 1;
    }
    
	ip->cpu_tracking_state.cpuFunc = 0;
	ip->cpu_tracking_state.nAvailCpuFuncs = 0;
	ip->cpu_tracking_state.nFinishedCpuFuncs = 0;

    const Json::Value& fbos = root["FrameBufferObjects"];
	for( auto name : fbos.getMemberNames() )
	{
		auto fbo = fbos[name];
		int64_t w = get_integer_value(env,fbo.get("width","$WIDTH"));
		int64_t h = get_integer_value(env,fbo.get("height","$HEIGHT"));
		std::cout<<"Adding FBO "<<name<<" : "<<w<<"x"<<h<<"\n";
		add_fbo(ip,name.c_str(),GL_RGBA,w,h);
	}
	
    const Json::Value& shadersObject = root["GLShaders"];
    std::map< std::string , ShaderPass* > shadersDB;
	for( auto name : shadersObject.getMemberNames() )
	{
		const Json::Value& shader = shadersObject[name];

		// building shader pass content
		ShaderPass* shaderPass = new ShaderPass;
		shadersDB[ name ] = shaderPass;
		
		shaderPass->finalTexture = new GLTexture();
		shaderPass->finalTexture->format = GL_RGB;
		shaderPass->finalTexture->target = GL_TEXTURE_2D;
		shaderPass->finalTexture->texid = 0;
		// add a texture alias to the shader output.
		// name of the shader can be used as a texture name
		std::cout<<"create texture alias '"<<name<<"'\n";
		ip->texture[ name ] = shaderPass->finalTexture;
	}
	for( auto name : shadersObject.getMemberNames() )
	{
		std::cout<<"\n***** Parsing shader pass "<<name<<" *****\n";
		const Json::Value& shader = shadersObject[name];
		ShaderPass* shaderPass = shadersDB[ name ];
		
		// read vertex shader
		shaderPass->vertexSource = vs_attributes+"\n"+uniforms+"\n"+readShader( get_string_value(env,shader["vertex-shader"]) );
		std::cout<<"VertexSource size = "<<shaderPass->vertexSource.size()<<"\n";
		
		// read fragment shader
		shaderPass->fragmentSourceWithoutTextures = uniforms+"\n"+inc_fs+"\n"+readShader( get_string_value(env,shader["fragment-shader"]) );
		std::cout<<"FramgentSource size = "<<shaderPass->fragmentSourceWithoutTextures.size()<<"\n";

		const Json::Value& render = shader["rendering"];
		std::string renderPlugin = get_string_value(env,render.get("plugin",""));
		std::string renderFunc = get_string_value(env,render.get("function","gl_fill"));
		shaderPass->gl_draw = (GLRenderFunctionT) dynlib_func_addr( renderPlugin, renderFunc );
		std::cout<<"Rendering: plugin="<<renderPlugin<<" function="<<renderFunc<<" @"<<shaderPass->gl_draw<<"\n";

		const Json::Value& textures = shader["textures"];
		for( std::string name : textures.getMemberNames() )
		{
			TextureInput texInput;
			texInput.uniformName = name;
			//std::cout<<"input texture '"<<texInput.uniformName<<"' <-";
			for( const Json::Value& texNameValue : textures[name] )
			{
				std::string textureName = get_string_value(env,texNameValue);
				std::cout<<" "<<textureName;
				GLTexture* tex = ip->texture[textureName];
				if( tex != 0 ) { texInput.texPool.push_back( tex ); }
				else { std::cerr<<" texture '"<<textureName<<"' not found\n"; }
			}
			std::cout<<"\n";
			shaderPass->inputs.push_back( texInput );
		}
		
		for( const Json::Value& nameValue : shader["output"] )
		{
			shaderPass->fboPool.push_back( ip->fbo[get_string_value(env,nameValue)] );
		}

		shaderPass->numberOfPasses = get_integer_value(env,shader.get("passes",1));
	}

    const Json::Value& cpuFuncsObject = root["CPUFunctions"];
	std::map< std::string , CpuPass* > cpuFuncDB;
	for( auto name : cpuFuncsObject.getMemberNames() )
	{
		std::cout<<"+++++ Parsing cpu pass "<<name<<" +++++\n";
		const Json::Value& cpuFuncObject = cpuFuncsObject[name];
		std::string plugin = get_string_value(env,cpuFuncObject.get("plugin",""));
		std::string setupName = get_string_value(env,cpuFuncObject.get("setup",""));
		std::string funcName = get_string_value(env,cpuFuncObject.get("run",""));

		// isn't it highly secure ? we're doing graphics anyway, we don't care ;-)
		void(*setup_function)() = ( void(*)() ) dynlib_func_addr(plugin,setupName);
		if( setup_function != 0 )
		{ 
			std::cout<<"initialize plugin ...\n";
			(*setup_function) ();
		}

		CpuPass* cpu = new CpuPass;
		cpu->exec_thread = get_integer_value(env,cpuFuncObject.get("thread-id",0));
		cpu->cpu_processing = (CpuProcessingFunc) dynlib_func_addr(plugin,funcName);
		
		cpuFuncDB[name] = cpu;
	}
	
	for( const Json::Value& pstepName : root["ProcessingLoop"] )
	{
		ProcessingStep ps;
		std::string name = get_string_value(env,pstepName); 
		ps.shaderPass = shadersDB[name];
		ps.cpuPass = cpuFuncDB[name];
		std::cout<<"add step '"<<name<<"' shaderPass @"<<ps.shaderPass<<", cpuPass @"<<ps.cpuPass<<"\n";
		ip->processing_step.push_back( ps );
	}

#if 0
	int rc;
	FILE* fp;
	char tmp[256];
	sprintf(tmp,"%s/%s.fleye",FLEYE_SCRIPT_DIR,filename);
	fp = fopen(tmp,"rb");
	if( fp == 0 )
	{
		fprintf(stderr,"failed to open processing script %s\n",tmp);
		return -1;
	}
	printf("using processing script %s\n",tmp);

	ip->nProcessingSteps = 0;
	ip->cpu_tracking_state.cpuFunc = 0;
	ip->cpu_tracking_state.nAvailCpuFuncs = 0;
	ip->cpu_tracking_state.nFinishedCpuFuncs = 0;

	while( ip->nProcessingSteps<IMGPROC_MAX_STEPS  && !feof(fp) && fscanf(fp,"%s",tmp)==1 )
	{
		memset( & ip->processing_step[ip->nProcessingSteps] , 0, sizeof(struct ProcessingStep) );

		if( strcasecmp(tmp,"SHADER")==0 )
		{
			char vsFileName[64]={'\0',};
			char fsFileName[64]={'\0',};
			char inputTextureBlock[256]={'\0',};
			char outputFBOBlock[256]={'\0',};
			char drawMethod[128]={'\0',};
			int count = 0;
			char * vs = 0;
			char * fs = 0;
			ShaderPass* shaderPass = & ip->processing_step[ip->nProcessingSteps].shaderPass;
			
			shaderPass->compileCacheSize = 0;
			ip->processing_step[ip->nProcessingSteps].exec_thread = PROCESSING_GPU;

			// one texture will be allocated and named upon the shader
			// texture will always have properties of the last texture the shader has rendered to
			shaderPass->finalTexture = & ip->processing_texture[ip->nTextures++];
			shaderPass->finalTexture->format = GL_RGB;
			shaderPass->finalTexture->target = GL_TEXTURE_2D;
			shaderPass->finalTexture->texid = 0; // will disable corresponding texture unit

			// read shader pass description line
			fscanf(fp,"%s %s %s %s %s %s %s\n",shaderPass->finalTexture->name,vsFileName,fsFileName,drawMethod,inputTextureBlock,outputFBOBlock,tmp);
			printf("SHADER: %s %s %s %s %s %s %s\n",shaderPass->finalTexture->name,vsFileName,fsFileName,drawMethod,inputTextureBlock,outputFBOBlock,tmp);

			// read pass count, possibly a variable ($something)
			if( tmp[0]=='$' ) { count=atoi( fleye_optional_value(env,tmp+1) ); }
			else { count=atoi(tmp); }
			ip->processing_step[ip->nProcessingSteps].numberOfPasses = count;
			
			// assemble vertex and fragment sources
			{
				char* user_vs = 0;
				char* user_fs = 0;
				char* inc_fs = 0;
				char* sep = 0;
				int vs_size=0, fs_size=0;

				user_vs = readShader(vsFileName);
				vs_size = strlen(vs_attributes) + strlen(uniforms) + strlen(user_vs);
				vs = new char [vs_size + 8] ; //malloc( vs_size + 8 );
				sprintf(vs,"%s\n%s\n%s\n",vs_attributes,uniforms,user_vs);
				free(user_vs);
				//printf("Vertex Shader:\n%s",vs);

				user_fs = readShader(fsFileName);
				inc_fs = readShader("inc_fs");
				fs_size = strlen(uniforms) + strlen(inc_fs) + strlen(user_fs) ;
				fs = new char [fs_size + 8] ; //malloc( fs_size + 8 );
				sprintf(fs,"%s\n%s\n%s\n",uniforms,inc_fs,user_fs);
				free(inc_fs);
				free(user_fs);
				//printf("Fragment Shader:\n%s",fs);
			}
			//printf("Compiling shader : %s/%s ...\n",vsFileName,fsFileName);
			//rc = create_image_shader( & ip->processing_step[state->n_processing_steps].gl_shader, vs, fs );
			shaderPass->vertexSource = vs;
			shaderPass->fragmentSourceWithoutTextures = fs;
			
			// load drawing function
			{
				char * drawPlugin[2]={0,0};
				const char* funcName = 0;
				char tmp2[128];
				int n=0;
				void* handle = 0;
				strsplit(drawMethod,':',drawPlugin,2,&n);
				if( n==2 )
				{
					sprintf(tmp2,"%s/lib%s.so",FLEYE_PLUGIN_DIR,drawPlugin[0]);
					handle = dlopen(tmp2, RTLD_GLOBAL | RTLD_NOW);
					if(handle==NULL)
					{
						fprintf(stderr,"failed to load plugin %s\n",tmp2);
						return -1;
					}
					funcName = drawPlugin[1];
				}
				else
				{
					handle = dlopen(NULL, RTLD_GLOBAL | RTLD_NOW);
					funcName = drawPlugin[0];
				}
				ip->processing_step[ip->nProcessingSteps].gl_draw = (GLRenderFunctionT) dlsym(handle,funcName);
				if( ip->processing_step[ip->nProcessingSteps].gl_draw == NULL)
				{
					fprintf(stderr,"can't find function %s\n",funcName);
					return -1;
				}
				printf("resolved function %s to %p\n",funcName,ip->processing_step[ip->nProcessingSteps].gl_draw);
			}
			
			// decode input blocks
			// exemple: tex1=CAMERA,fbo1:tex2=mask_fbo
			if( strcmp(inputTextureBlock,"none") != 0 )
			{
				char* texBlocks[SHADER_MAX_INPUT_TEXTURES];
				int ti;
				strsplit(inputTextureBlock,':',texBlocks, SHADER_MAX_INPUT_TEXTURES, & shaderPass->nInputs);
				for(ti=0;ti<shaderPass->nInputs;ti++)
				{
					char* texInput[2];
					int check2 = 0;
					strsplit( texBlocks[ti], '=', texInput, 2, & check2 );
					if( check2==2 )
					{
						char* texPoolNames[MAX_TEXTURES];
						int pi;
						strcpy( shaderPass->inputs[ti].uniformName, texInput[0] );
						strsplit(texInput[1],',',texPoolNames, MAX_TEXTURES, & shaderPass->inputs[ti].poolSize);
						for(pi=0;pi<shaderPass->inputs[ti].poolSize;pi++)
						{
							shaderPass->inputs[ti].texPool[pi] = get_named_texture(ip,texPoolNames[pi]);
						}
					}
					else
					{
						fprintf(stderr,"syntax error: expected '='\n");
						return rc;
					}
				}
			}
			else
			{
				shaderPass->nInputs = 0;
			}
			
			// decode outputBlock
			{
				char* outputFBONames[MAX_TEXTURES];
				int oi;
				strsplit(outputFBOBlock,',',outputFBONames,MAX_TEXTURES, & shaderPass->fboPoolSize);
				for(oi=0;oi<shaderPass->fboPoolSize;oi++)
				{
					shaderPass->fboPool[oi] = get_named_fbo(ip,outputFBONames[oi]);
				}
			}
			++ ip->nProcessingSteps;
		}
		else if( strcasecmp(tmp,"FBO")==0 )
		{
			char name[TEXTURE_NAME_MAX_LEN];
			char widthStr[64];
			char heightStr[64];
			fscanf(fp,"%s %s %s\n",name,widthStr,heightStr);
			int w = atoi( (widthStr[0]=='$') ? fleye_optional_value(env,widthStr+1) : widthStr );
			int h = atoi( (heightStr[0]=='$') ? fleye_optional_value(env,heightStr+1) : heightStr );
			add_fbo(ip,name,GL_RGBA,w,h);
		}
		// add a TEXTURE keyword to load an image ? might be usefull
		/*else if( strcasecmp(tmp,"TEXTURE")==0 )
		{
			...
		}*/		
		else if( strcasecmp(tmp,"CPU")==0 )
		{
			char tmp2[256];
			ip->processing_step[ip->nProcessingSteps].numberOfPasses = CPU_PROCESSING_PASS;
			fscanf(fp,"%s %d\n",tmp, & ip->processing_step[ip->nProcessingSteps].exec_thread );
			sprintf(tmp2,"%s/lib%s.so",FLEYE_PLUGIN_DIR,tmp);
			printf("loading dynamic library %s ...\n",tmp2);
			void * handle = dlopen(tmp2, RTLD_GLOBAL | RTLD_NOW);
			if(handle==NULL)
			{
				fprintf(stderr,"failed to load plugin %s\n",tmp2);
				return -1;
			}
			sprintf(tmp2,"%s_run",tmp);
			void* funcSym = dlsym(handle,tmp2);
			if( funcSym == 0 )
			{
				fprintf(stderr,"can't find function %s\n",tmp2);
				return -1;
			}
			ip->processing_step[ip->nProcessingSteps].cpu_processing = (CpuProcessingFunc)funcSym ;

			sprintf(tmp2,"%s_setup",tmp);
			void(*init_plugin)() = ( void(*)() ) dlsym(handle,tmp2);
			if( init_plugin != NULL )
			{
				(*init_plugin) ();
			}
			
			++ ip->nProcessingSteps;
		}
		else
		{
			fprintf(stderr,"bad processing step type '%s'\n",tmp);
			return -1;
		}
	}

	fclose(fp);
	printf("processing pipeline has %d steps\n",ip->nProcessingSteps);

	printf("Frame Buffers:\n");
	int i;
	for(i=0;i<ip->nFBO;i++) { printf("\t%s %dx%d\n",ip->processing_fbo[i].texture->name,ip->processing_fbo[i].width,ip->processing_fbo[i].height); }
	printf("Textures:\n");
	for(i=0;i<ip->nTextures;i++) { printf("\t%s %d\n",ip->processing_texture[i].name,ip->processing_texture[i].texid); }

	printf("Processing steps:\n");
	for(i=0;i<ip->nProcessingSteps;i++)
	{
		if(ip->processing_step[i].numberOfPasses == CPU_PROCESSING_PASS)
		{
			printf("\tCPU %p\n",ip->processing_step[i].cpu_processing);
		}
		else
		{
			printf("\tShader %s %d\n",ip->processing_step[i].shaderPass.finalTexture->name,ip->processing_step[i].numberOfPasses);
			int j;
			for(j=0;j<ip->processing_step[i].shaderPass.nInputs;j++)
			{
				printf("\t\t%s <-",ip->processing_step[i].shaderPass.inputs[j].uniformName, ip->processing_step[i].shaderPass.inputs[j].poolSize);
				int k;
				for(k=0;k<ip->processing_step[i].shaderPass.inputs[j].poolSize;k++)
				{
					printf("%s%s",((k>0)?", ":""),ip->processing_step[i].shaderPass.inputs[j].texPool[k]->name);
				}
				printf("\n");
			}
		}
	}
#endif
	return 0;
}


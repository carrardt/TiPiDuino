#include <stdio.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include "fleye/imageprocessing.h"
#include "fleye/fleye_c.h"

GLuint fleye_get_camera_texture_id(struct ImageProcessingState* ip)
{
	return ip->cameraTextureId;
}

void strsplit(char* str, int delim, char** ptrs, int bufsize, int* n)
{
	*n = 0;
	int eos = 0;
	while( !eos && (*n) < bufsize )
	{
		ptrs[*n] = str;
		++ (*n);
		str = strchrnul(str,delim);
		eos = ( *str == '\0' );
		*str = '\0';
		str = str+1;
	}
}

int create_image_processing(struct ImageProcessingState* ip, struct UserEnv* env, const char* filename)
{
	// TODO: transferer dans inc_fs et inc_vs
	const char* uniforms = 	
		"uniform vec2 step;\n"
		"uniform vec2 size;\n"
		"uniform float iter;\n"
		"uniform float iter2i;\n"
		"uniform vec2 step2i;\n"
		;

	const char* vs_attributes = 
		"attribute vec3 vertex;\n"
		;

	int rc;
	FILE* fp;
	char tmp[256];
	sprintf(tmp,"./%s.fleye",filename);
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
					sprintf(tmp2,"./lib%s.so",drawPlugin[0]);
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
			sprintf(tmp2,"./lib%s.so",tmp);
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

	return 0;
}


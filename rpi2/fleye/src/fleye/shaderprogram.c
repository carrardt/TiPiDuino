#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "fleye/shaderprogram.h"
#include "fleye/shaderpass.h"
#include "fleye/fbo.h"
#include "fleye/texture.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


char* readShader(const char* fileName)
{
	char filePath[256];
	snprintf(filePath,255,"%s/%s.glsl",GLSL_SRC_DIR,fileName);
	FILE* fp=fopen(filePath,"rb");
	if(fp==0)
	{
		fprintf(stderr,"Can't open file %s\n",filePath);
		return 0;
	}
	fseek(fp,0,SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char* buf = (char*) malloc(fsize+1);
	fread(buf,fsize,1,fp);
	buf[fsize]='\0';
	fclose(fp);
	return buf;
}

/**
 * Takes a description of shader program, compiles it and gets the locations
 * of uniforms and attributes.
 *
 * @param p The shader program state.
 * @return Zero if successful.
 */
int fleyeutil_build_shader_program(ShaderProgram *p, const char* vertex_source, const char* fragment_source)
{
    GLint status;
    int i = 0;
    char log[1024];
    int logLen = 0;

    assert(p!=NULL);
    assert(vertex_source!=NULL);
    assert(fragment_source!=NULL);

    if (! (p && vertex_source && fragment_source))
        goto fail;

    p->vs = p->fs = 0;

    p->vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(p->vs, 1, &vertex_source, NULL);
    glCompileShader(p->vs);
    glGetShaderiv(p->vs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->vs, sizeof(log), &logLen, log);
        fprintf(stderr,"Program info log %s", log);
        goto fail;
    }

    p->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(p->fs, 1, &fragment_source, NULL);
    glCompileShader(p->fs);

    glGetShaderiv(p->fs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->fs, sizeof(log), &logLen, log);
        fprintf(stderr,"Program info log %s", log);
        goto fail;
    }

    p->program = glCreateProgram();
    glAttachShader(p->program, p->vs);
    glAttachShader(p->program, p->fs);
    glLinkProgram(p->program);
    glGetProgramiv(p->program, GL_LINK_STATUS, &status);
    if (! status)
    {
		char* tmp=0;
		char* str=0;
		char* pendl=0;
		int line=1;
        fprintf(stderr,"Failed to link shader program");
        glGetProgramInfoLog(p->program, sizeof(log), &logLen, log);
        fprintf(stderr,"%s", log);
        
        printf("Vertex shader:\n");
        tmp=strdup(vertex_source);
        str=tmp;
        pendl=0;
        line=1;
        while( (pendl=strchr(str,'\n'))!=0 )
        {
			*pendl = '\0';
			printf("%d: %s\n",line++,str);
			str = pendl+1;
		}
		free(tmp);
		
        printf("Fragment shader:\n");
        tmp=strdup(fragment_source);
        str=tmp;
        pendl=0;
        line=1;
        while( (pendl=strchr(str,'\n'))!=0 )
        {
			*pendl = '\0';
			printf("%d: %s\n",line++,str);
			str = pendl+1;
		}
		free(tmp);

        goto fail;
    }

    for (i = 0; i < SHADER_MAX_ATTRIBUTES; ++i)
    {
        if (! p->attribute_names[i])
            break;
        p->attribute_locations[i] = glGetAttribLocation(p->program, p->attribute_names[i]);
        if (p->attribute_locations[i] == -1)
        {
            fprintf(stderr,"Failed to get location for attribute %s",
                  p->attribute_names[i]);
            goto fail;
        }
        else {
            vcos_log_trace("Attribute for %s is %d",
                  p->attribute_names[i], p->attribute_locations[i]);
        }
    }

    for (i = 0; i < SHADER_MAX_UNIFORMS; ++i)
    {
        if (! p->uniform_names[i])
            break;
        p->uniform_locations[i] = glGetUniformLocation(p->program, p->uniform_names[i]);
        if (p->uniform_locations[i] == -1)
        {
            vcos_log_trace("unused uniform %s", p->uniform_names[i]);
        }
        else {
            vcos_log_trace("Uniform for %s is %d",
                  p->uniform_names[i], p->uniform_locations[i]);
        }
    }

    return 0;

fail:
    fprintf(stderr,"Failed to build shader program");
    if (p)
    {
        glDeleteProgram(p->program);
        glDeleteShader(p->fs);
        glDeleteShader(p->vs);
    }
    return -1;
}

int create_image_shader(ShaderProgram* shader, const char* vs, const char* fs)
{
	int i;
	// generate score values corresponding to color matching of target
	memset(shader,0,sizeof(ShaderProgram));
	
	shader->attribute_names[0] = "vertex";
	
	shader->uniform_names[0] = "step";
	shader->uniform_names[1] = "size";
	shader->uniform_names[2] = "iter";
	shader->uniform_names[3] = "iter2i";
	shader->uniform_names[4] = "step2i";

    int rc = fleyeutil_build_shader_program(shader,vs,fs);    
	return rc;
}

struct CompiledShaderCache* get_compiled_shader(struct ShaderPass* shaderPass,struct RASPITEX_Texture** inputs)
{
	const char* image_external_pragma = "";
	char textureUniformProlog[1024]={'\0',};
	char* fragmentSource = 0;
	int i,rc;
	
	for(i=0;i<shaderPass->compileCacheSize;i++)
	{
		int j,found=1;
		for(j=0;j<shaderPass->nInputs && found;j++)
		{
			if( shaderPass->shaderCahe[i].textureTargets[j] != inputs[j]->target ) found=0;
		}
		if(found)
		{
			//printf("found precompiled shader %d\n",i);
			return & shaderPass->shaderCahe[i];
		}
	}
	if(shaderPass->compileCacheSize>=SHADER_COMPILE_CACHE_SIZE)
	{
		fprintf(stderr,"Shader cache is full");
		return 0;
	}

	struct CompiledShaderCache* compiledShader = & shaderPass->shaderCahe[ shaderPass->compileCacheSize ++ ];
	for(i=0;i<shaderPass->nInputs;i++)
	{
		const char* samplerType = 0;
		//const char* texlookup = "texture2D";
		char uniformDeclare[128];
		switch( inputs[i]->target )
		{
			case GL_TEXTURE_2D:
				samplerType = "sampler2D";
				break;
			case GL_TEXTURE_EXTERNAL_OES:
				samplerType = "samplerExternalOES";
				image_external_pragma = "#extension GL_OES_EGL_image_external : require\n";
				break;
			default:
				fprintf(stderr,"unhandled texture target");
				return 0;
				break;
		}
		compiledShader->textureTargets[i]=inputs[i]->target;
		sprintf(uniformDeclare,"uniform %s %s;\n",samplerType,shaderPass->inputs[i].uniformName);
		strcat(textureUniformProlog,uniformDeclare);
	}
	fragmentSource = malloc( strlen(image_external_pragma) + strlen(shaderPass->fragmentSourceWithoutTextures) + strlen(textureUniformProlog) + 8 );
	sprintf(fragmentSource,"%s\n%s\n%s\n",image_external_pragma,textureUniformProlog,shaderPass->fragmentSourceWithoutTextures);

	// printf("%s FS:\n%s",shaderPass->finalTexture->name,fragmentSource);
	// compile assembled final shader program
	rc = create_image_shader( & compiledShader->shader, shaderPass->vertexSource, fragmentSource );
	free(fragmentSource);
	if( rc != 0)
	{
		fprintf(stderr,"failed to build shader");
		return 0;
	}

	for(i=0;i<shaderPass->nInputs;i++)
	{
		compiledShader->samplerUniformLocations[i] = glGetUniformLocation(compiledShader->shader.program, shaderPass->inputs[i].uniformName);
		// printf("sampler uniform '%s' -> location %d\n",shaderPass->inputs[i].uniformName,compiledShader->samplerUniformLocations[i]);
	}

	return compiledShader;
}

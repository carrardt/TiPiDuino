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
        fprintf(stderr,"Program info log %s\n", log);
        goto fail;
    }

    p->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(p->fs, 1, &fragment_source, NULL);
    glCompileShader(p->fs);

    glGetShaderiv(p->fs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->fs, sizeof(log), &logLen, log);
        fprintf(stderr,"Program info log %s\n", log);
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
        fprintf(stderr,"Failed to link shader program\n");
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
            fprintf(stderr,"Failed to get location for attribute %s\n",
                  p->attribute_names[i]);
            goto fail;
        }
        else {
            printf("Attribute for %s is %d\n",
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
            printf("unused uniform %s\n", p->uniform_names[i]);
        }
        else {
            printf("Uniform for %s is %d\n",
                  p->uniform_names[i], p->uniform_locations[i]);
        }
    }

    return 0;

fail:
    fprintf(stderr,"Failed to build shader program\n");
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

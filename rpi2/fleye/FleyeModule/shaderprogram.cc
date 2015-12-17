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

#include <iostream>
#include <fstream>

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
        return -1;

    p->vs = p->fs = 0;

    p->vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(p->vs, 1, &vertex_source, NULL);
    glCompileShader(p->vs);
    glGetShaderiv(p->vs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->vs, sizeof(log), &logLen, log);
        fprintf(stderr,"Program info log %s\n", log);
        return -1;
    }

    p->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(p->fs, 1, &fragment_source, NULL);
    glCompileShader(p->fs);

    glGetShaderiv(p->fs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->fs, sizeof(log), &logLen, log);
        fprintf(stderr,"Program info log %s\n", log);
        return -1;
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
        std::cerr<<log<<"\n";
        
        printf("Vertex shader:\n");
        tmp=strdup(vertex_source);
        str=tmp;
        pendl=0;
        line=1;
        while( (pendl=strchr(str,'\n'))!=0 )
        {
			*pendl = '\0';
			printf("%03d: %s\n",line++,str);
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
			printf("%03d: %s\n",line++,str);
			str = pendl+1;
		}
		free(tmp);

        return -1;
    }


	// resolve attribute locations
	p->attribute_locations.clear();
	for( auto attrName : p->attribute_names )
	{
		GLint loc = glGetAttribLocation(p->program, attrName.c_str() );
        if ( loc == -1)
        {
            std::cerr<<"Failed to get location for attribute "<<attrName<<"\n";
            return -1;
        }
        std::cout<<"Attribute "<<attrName<<" mapped to location "<<loc<<"\n";
        p->attribute_locations.push_back(loc);
	}
	
	p->uniform_locations.clear();
	for( auto uniformName : p->uniform_names )
	{
		GLint loc = glGetUniformLocation(p->program, uniformName.c_str() );
        if ( loc == -1) { std::cerr<<"unused uniform "<<uniformName<<"\n"; }
        else { std::cout<<"Uniform "<<uniformName<<"mapped to location "<<loc<<"\n"; }
        p->uniform_locations.push_back(loc);
	}
	
    return 0;
}

int create_image_shader(ShaderProgram* shader, const char* vs, const char* fs)
{
	int i;
	// generate score values corresponding to color matching of target
	memset(shader,0,sizeof(ShaderProgram));
	
	shader->attribute_names.resize(1);
	shader->attribute_names[0] = "vertex";
	
	shader->uniform_names.resize(5);
	shader->uniform_names[0] = "step";
	shader->uniform_names[1] = "size";
	shader->uniform_names[2] = "iter";
	shader->uniform_names[3] = "iter2i";
	shader->uniform_names[4] = "step2i";

    int rc = fleyeutil_build_shader_program(shader,vs,fs);    
	return rc;
}

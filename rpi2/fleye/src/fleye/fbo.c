#include <GLES2/gl2.h>
#include <string.h>

#include "fleye/fbo.h"
#include "fleye/imageprocessing.h"
#include "fleye/texture.h"

int add_fbo(struct ImageProcessingState* ip, const char* name, GLint colorFormat, GLint w, GLint h)
{
	RASPITEX_Texture* tex = & ip->processing_texture[ip->nTextures];
	strcpy(tex->name, name);
	tex->format = colorFormat;
	tex->target = GL_TEXTURE_2D;
	tex->texid = 0;
	
	FrameBufferObject* fbo = & ip->processing_fbo[ip->nFBO];
   fbo->width = w;
   fbo->height = h;
   fbo->fb = 0;
   fbo->texture = tex;

   glGenFramebuffers(1, & fbo->fb);

	glGenTextures(1, & tex->texid );
	glBindTexture(tex->target, tex->texid);
	glTexImage2D(tex->target, 0, tex->format, fbo->width, fbo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(tex->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(tex->target, 0);

	/*if( depthFormat != GL_NONE )
	{
	   // Create a texture to hold the depth buffer
		glGenTextures(1, &(fbo->depht_rb) );
		glBindTexture(GL_TEXTURE_2D, fbo->depht_rb);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				fbo->width, fbo->height,
				0, depthFormat, GL_UNSIGNED_SHORT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);        
	}
	else
	{
		fbo->depht_rb = 0;
	}*/

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);

    // Associate the textures with the FBO.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    tex->target, tex->texid, 0);

	// no depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D, /*null texture object*/ 0, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	
    if ( status == GL_FRAMEBUFFER_COMPLETE )
    {
		++ ip->nTextures ;
		++ ip->nFBO ;
		return 0;
	}
    else 
    {
		return 1;
	}
    
/*
glGenTextures(1, (GLuint *) 0x777ada0c);
glBindTexture(GL_TEXTURE_2D, 3);
glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) 0x752d700c);

glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, 3, 0);
 */
}

FrameBufferObject* get_named_fbo(struct ImageProcessingState* ip, const char * name)
{
	int i;
	for(i=0;i<ip->nFBO;i++)
	{
		if( strcasecmp(name,ip->processing_fbo[i].texture->name)==0 )
		{
			return & ip->processing_fbo[i];
		}
	}
	return 0;
}

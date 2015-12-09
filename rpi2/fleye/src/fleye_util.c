/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "fleye_util.h"
#include "RaspiTex.h"
#include <bcm_host.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <string.h>

VCOS_LOG_CAT_T fleye_log_category;

/**
 * \file fleye_util.c
 *
 * Provides default implementations for the fleye_scene_ops functions
 * and general utility functions.
 */

/**
 * Deletes textures and EGL surfaces and context.
 * @param   fleye_state  Pointer to the Raspi
 */
void fleyeutil_gl_term(RASPITEX_STATE *fleye_state)
{
   vcos_log_trace("%s", VCOS_FUNCTION);

   /* Delete OES textures */
   glDeleteTextures(1, &fleye_state->texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->egl_image);
   fleye_state->egl_image = EGL_NO_IMAGE_KHR;

   glDeleteTextures(1, &fleye_state->y_texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->y_egl_image);
   fleye_state->y_egl_image = EGL_NO_IMAGE_KHR;

   glDeleteTextures(1, &fleye_state->u_texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->u_egl_image);
   fleye_state->u_egl_image = EGL_NO_IMAGE_KHR;

   glDeleteTextures(1, &fleye_state->v_texture);
   eglDestroyImageKHR(fleye_state->display, fleye_state->v_egl_image);
   fleye_state->v_egl_image = EGL_NO_IMAGE_KHR;

   /* Terminate EGL */
   eglMakeCurrent(fleye_state->display, EGL_NO_SURFACE,
         EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(fleye_state->display, fleye_state->context);
   eglDestroySurface(fleye_state->display, fleye_state->surface);
   eglTerminate(fleye_state->display);
}

/** Creates a native window for the GL surface using dispmanx
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful, otherwise, -1 is returned.
 */
int fleyeutil_create_native_window(RASPITEX_STATE *fleye_state)
{
   VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
   VC_RECT_T src_rect = {0};
   VC_RECT_T dest_rect = {0};
   uint32_t disp_num = 0; // Primary
   uint32_t layer_num = 0;
   DISPMANX_ELEMENT_HANDLE_T elem;
   DISPMANX_UPDATE_HANDLE_T update;

   alpha.opacity = fleye_state->opacity;
   dest_rect.x = fleye_state->x;
   dest_rect.y = fleye_state->y;
   dest_rect.width = fleye_state->width;
   dest_rect.height = fleye_state->height;

   vcos_log_trace("%s: %d,%d,%d,%d %d,%d,0x%x,0x%x", VCOS_FUNCTION,
         src_rect.x, src_rect.y, src_rect.width, src_rect.height,
         dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height);

   src_rect.width = dest_rect.width << 16;
   src_rect.height = dest_rect.height << 16;

   fleye_state->disp = vc_dispmanx_display_open(disp_num);
   if (fleye_state->disp == DISPMANX_NO_HANDLE)
   {
      vcos_log_error("Failed to open display handle");
      goto error;
   }

   update = vc_dispmanx_update_start(0);
   if (update == DISPMANX_NO_HANDLE)
   {
      vcos_log_error("Failed to open update handle");
      goto error;
   }

   elem = vc_dispmanx_element_add(update, fleye_state->disp, layer_num,
         &dest_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, &alpha, NULL,
         DISPMANX_NO_ROTATE);
   if (elem == DISPMANX_NO_HANDLE)
   {
      vcos_log_error("Failed to create element handle");
      goto error;
   }

   fleye_state->win.element = elem;
   fleye_state->win.width = fleye_state->width;
   fleye_state->win.height = fleye_state->height;
   vc_dispmanx_update_submit_sync(update);

   fleye_state->native_window = (EGLNativeWindowType*) &fleye_state->win;

   return 0;
error:
   return -1;
}

/** Destroys the pools of buffers used by the GL renderer.
 * @param fleye_state A pointer to the GL preview state.
 */
void fleyeutil_destroy_native_window(RASPITEX_STATE *fleye_state)
{
   vcos_log_trace("%s", VCOS_FUNCTION);
   if (fleye_state->disp != DISPMANX_NO_HANDLE)
   {
      vc_dispmanx_display_close(fleye_state->disp);
      fleye_state->disp = DISPMANX_NO_HANDLE;
   }
}

/** Creates the EGL context and window surface for the native window
 * using specified arguments.
 * @param fleye_state  A pointer to the GL preview state. This contains
 *                        the native_window pointer.
 * @param attribs         The config attributes.
 * @param context_attribs The context attributes.
 * @return Zero if successful.
 */
static int fleyeutil_gl_common(RASPITEX_STATE *fleye_state,
      const EGLint attribs[], const EGLint context_attribs[])
{
   EGLConfig config;
   EGLint num_configs;

   vcos_log_trace("%s", VCOS_FUNCTION);

   if (fleye_state->native_window == NULL)
   {
      vcos_log_error("%s: No native window", VCOS_FUNCTION);
      goto error;
   }

   fleye_state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (fleye_state->display == EGL_NO_DISPLAY)
   {
      vcos_log_error("%s: Failed to get EGL display", VCOS_FUNCTION);
      goto error;
   }

   if (! eglInitialize(fleye_state->display, 0, 0))
   {
      vcos_log_error("%s: eglInitialize failed", VCOS_FUNCTION);
      goto error;
   }

   if (! eglChooseConfig(fleye_state->display, attribs, &config,
            1, &num_configs))
   {
      vcos_log_error("%s: eglChooseConfig failed", VCOS_FUNCTION);
      goto error;
   }

   fleye_state->surface = eglCreateWindowSurface(fleye_state->display,
         config, fleye_state->native_window, NULL);
   if (fleye_state->surface == EGL_NO_SURFACE)
   {
      vcos_log_error("%s: eglCreateWindowSurface failed", VCOS_FUNCTION);
      goto error;
   }

   fleye_state->context = eglCreateContext(fleye_state->display,
         config, EGL_NO_CONTEXT, context_attribs);
   if (fleye_state->context == EGL_NO_CONTEXT)
   {
      vcos_log_error("%s: eglCreateContext failed", VCOS_FUNCTION);
      goto error;
   }

   if (!eglMakeCurrent(fleye_state->display, fleye_state->surface,
            fleye_state->surface, fleye_state->context))
   {
      vcos_log_error("%s: Failed to activate EGL context", VCOS_FUNCTION);
      goto error;
   }

   return 0;

error:
   vcos_log_error("%s: EGL error 0x%08x", VCOS_FUNCTION, eglGetError());
   fleye_state->ops.gl_term(fleye_state);
   return -1;
}

/* Creates the RGBA and luma textures with some default parameters
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_create_textures(RASPITEX_STATE *fleye_state)
{
   GLCHK(glGenTextures(1, &fleye_state->y_texture));
   GLCHK(glGenTextures(1, &fleye_state->u_texture));
   GLCHK(glGenTextures(1, &fleye_state->v_texture));
   GLCHK(glGenTextures(1, &fleye_state->texture));
   return 0;
}

/**
 * Creates an OpenGL ES 1.X context.
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_gl_init_1_0(RASPITEX_STATE *fleye_state)
{
   int rc;
   const EGLint* attribs = fleye_state->egl_config_attribs;

   const EGLint default_attribs[] =
   {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 16,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
      EGL_NONE
   };

   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 1,
      EGL_NONE
   };

   if (! attribs)
      attribs = default_attribs;

   rc = fleyeutil_gl_common(fleye_state, attribs, context_attribs);
   if (rc != 0)
      goto end;

   GLCHK(glEnable(GL_TEXTURE_EXTERNAL_OES));
   rc = fleyeutil_create_textures(fleye_state);

end:
   return rc;
}

/**
 * Creates an OpenGL ES 2.X context.
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int fleyeutil_gl_init_2_0(RASPITEX_STATE *fleye_state)
{
   int rc;
   const EGLint* attribs = fleye_state->egl_config_attribs;;

   const EGLint default_attribs[] =
   {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 16,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };

   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   if (! attribs)
      attribs = default_attribs;

   vcos_log_trace("%s", VCOS_FUNCTION);
   rc = fleyeutil_gl_common(fleye_state, attribs, context_attribs);
   if (rc != 0)
      goto end;

   rc = fleyeutil_create_textures(fleye_state);
end:
   return rc;
}

/**
 * Advances the texture and EGL image to the next MMAL buffer.
 *
 * @param display The EGL display.
 * @param target The EGL image target e.g. EGL_IMAGE_BRCM_MULTIMEDIA
 * @param mm_buf The EGL client buffer (mmal opaque buffer) that is used to
 * create the EGL Image for the preview texture.
 * @param egl_image Pointer to the EGL image to update with mm_buf.
 * @param texture Pointer to the texture to update from EGL image.
 * @return Zero if successful.
 */
int fleyeutil_do_update_texture(EGLDisplay display, EGLenum target,
      EGLClientBuffer mm_buf, GLuint *texture, EGLImageKHR *egl_image)
{
   vcos_log_trace("%s: mm_buf %u", VCOS_FUNCTION, (unsigned) mm_buf);
   GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, *texture));
   if (*egl_image != EGL_NO_IMAGE_KHR)
   {
      /* Discard the EGL image for the preview frame */
      eglDestroyImageKHR(display, *egl_image);
      *egl_image = EGL_NO_IMAGE_KHR;
   }

   *egl_image = eglCreateImageKHR(display, EGL_NO_CONTEXT, target, mm_buf, NULL);
   GLCHK(glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, *egl_image));

   return 0;
}

/**
 * Updates the RGBX texture to the specified MMAL buffer.
 * @param fleye_state A pointer to the GL preview state.
 * @param mm_buf The MMAL buffer.
 * @return Zero if successful.
 */
int fleyeutil_update_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf)
{
   return fleyeutil_do_update_texture(fleye_state->display,
         EGL_IMAGE_BRCM_MULTIMEDIA, mm_buf,
         &fleye_state->texture, &fleye_state->egl_image);
}

/**
 * Updates the Y plane texture to the specified MMAL buffer.
 * @param fleye_state A pointer to the GL preview state.
 * @param mm_buf The MMAL buffer.
 * @return Zero if successful.
 */
int fleyeutil_update_y_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf)
{
   return fleyeutil_do_update_texture(fleye_state->display,
         EGL_IMAGE_BRCM_MULTIMEDIA_Y, mm_buf,
         &fleye_state->y_texture, &fleye_state->y_egl_image);
}

/**
 * Updates the U plane texture to the specified MMAL buffer.
 * @param fleye_state A pointer to the GL preview state.
 * @param mm_buf The MMAL buffer.
 * @return Zero if successful.
 */
int fleyeutil_update_u_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf)
{
   return fleyeutil_do_update_texture(fleye_state->display,
         EGL_IMAGE_BRCM_MULTIMEDIA_U, mm_buf,
         &fleye_state->u_texture, &fleye_state->u_egl_image);
}

/**
 * Updates the V plane texture to the specified MMAL buffer.
 * @param fleye_state A pointer to the GL preview state.
 * @param mm_buf The MMAL buffer.
 * @return Zero if successful.
 */
int fleyeutil_update_v_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf)
{
   return fleyeutil_do_update_texture(fleye_state->display,
         EGL_IMAGE_BRCM_MULTIMEDIA_V, mm_buf,
         &fleye_state->v_texture, &fleye_state->v_egl_image);
}

/**
 * Default is a no-op
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero.
 */
int fleyeutil_update_model(RASPITEX_STATE* fleye_state)
{
   (void) fleye_state;
   return 0;
}

/**
 * Default is a no-op
 * @param fleye_state A pointer to the GL preview state.
 * @return Zero.
 */
int fleyeutil_redraw(RASPITEX_STATE* fleye_state)
{
   (void) fleye_state;
   return 0;
}

/**
 * Default is a no-op
 * @param fleye_state A pointer to the GL preview state.
 */
void fleyeutil_close(RASPITEX_STATE* fleye_state)
{
   (void) fleye_state;
}

/**
 * Performs an in-place byte swap from BGRA to RGBA.
 * @param buffer The buffer to modify.
 * @param size Size of the buffer in bytes.
 */
void fleyeutil_brga_to_rgba(uint8_t *buffer, size_t size)
{
   uint8_t* out = buffer;
   uint8_t* end = buffer + size;

   while (out < end)
   {
      uint8_t tmp = out[0];
      out[0] = out[2];
      out[2] = tmp;
      out += 4;
   }
}

/**
 * Uses glReadPixels to grab the current frame-buffer contents
 * and returns the result in a newly allocate buffer along with
 * the its size.
 * Data is returned in BGRA format for TGA output. PPM output doesn't
 * require the channel order swap but would require a vflip. The TGA
 * format also supports alpha. The byte swap is not done in this function
 * to avoid blocking the GL rendering thread.
 * @param state Pointer to the GL preview state.
 * @param buffer Address of pointer to set to pointer to new buffer.
 * @param buffer_size The size of the new buffer in bytes (out param)
 * @return Zero if successful.
 */
int fleyeutil_capture_bgra(RASPITEX_STATE *state,
      uint8_t **buffer, size_t *buffer_size)
{
   const int bytes_per_pixel = 4;

   vcos_log_trace("%s: %dx%d %d", VCOS_FUNCTION,
         state->width, state->height, bytes_per_pixel);

   *buffer_size = state->width * state->height * bytes_per_pixel;
   *buffer = calloc(*buffer_size, 1);
   if (! *buffer)
      goto error;

   glReadPixels(0, 0, state->width, state->height, GL_RGBA,
         GL_UNSIGNED_BYTE, *buffer);
   if (glGetError() != GL_NO_ERROR)
      goto error;

   return 0;

error:
   *buffer_size = 0;
   if (*buffer)
      free(*buffer);
   *buffer = NULL;
   return -1;
}

char* readShader(const char* fileName)
{
	char filePath[256];
	snprintf(filePath,255,"%s/%s.glsl",GLSL_SRC_DIR,fileName);
	FILE* fp=fopen(filePath,"rb");
	if(fp==0)
	{
		vcos_log_error("Can't open file %s\n",filePath);
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
int fleyeutil_build_shader_program(RASPITEXUTIL_SHADER_PROGRAM_T *p, const char* vertex_source, const char* fragment_source)
{
    GLint status;
    int i = 0;
    char log[1024];
    int logLen = 0;
    vcos_assert(p);
    vcos_assert(vertex_source);
    vcos_assert(fragment_source);

    if (! (p && vertex_source && fragment_source))
        goto fail;

    p->vs = p->fs = 0;

    p->vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(p->vs, 1, &vertex_source, NULL);
    glCompileShader(p->vs);
    glGetShaderiv(p->vs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->vs, sizeof(log), &logLen, log);
        vcos_log_error("Program info log %s", log);
        goto fail;
    }

    p->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(p->fs, 1, &fragment_source, NULL);
    glCompileShader(p->fs);

    glGetShaderiv(p->fs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->fs, sizeof(log), &logLen, log);
        vcos_log_error("Program info log %s", log);
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
        vcos_log_error("Failed to link shader program");
        glGetProgramInfoLog(p->program, sizeof(log), &logLen, log);
        vcos_log_error("%s", log);
        
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
            vcos_log_error("Failed to get location for attribute %s",
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
    vcos_log_error("%s: Failed to build shader program", VCOS_FUNCTION);
    if (p)
    {
        glDeleteProgram(p->program);
        glDeleteShader(p->fs);
        glDeleteShader(p->vs);
    }
    return -1;
}

int add_fbo(RASPITEX_STATE *state, const char* name, GLint colorFormat, GLint w, GLint h)
{
	RASPITEX_Texture* tex = & state->processing_texture[state->nTextures];
	strcpy(tex->name, name);
	tex->format = colorFormat;
	tex->target = GL_TEXTURE_2D;
	tex->texid = 0;
	
	RASPITEX_FBO* fbo = & state->processing_fbo[state->nFBO];
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
		++ state->nTextures ;
		++ state->nFBO ;
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

RASPITEX_FBO* get_named_fbo(RASPITEX_STATE *state, const char * name)
{
	int i;
	for(i=0;i<state->nFBO;i++)
	{
		if( strcasecmp(name,state->processing_fbo[i].texture->name)==0 )
		{
			return & state->processing_fbo[i];
		}
	}
	return 0;
}

RASPITEX_Texture* get_named_texture(RASPITEX_STATE *state, const char * name)
{
	int i;
	for(i=0;i<state->nTextures;i++)
	{
		if( strcasecmp(name,state->processing_texture[i].name)==0 )
		{
			return & state->processing_texture[i];
		}
	}
	return 0;
}

int create_image_shader(RASPITEXUTIL_SHADER_PROGRAM_T* shader, const char* vs, const char* fs)
{
	int i;
	// generate score values corresponding to color matching of target
	memset(shader,0,sizeof(RASPITEXUTIL_SHADER_PROGRAM_T));
	
	shader->attribute_names[0] = "vertex";
	
	shader->uniform_names[0] = "step";
	shader->uniform_names[1] = "size";
	shader->uniform_names[2] = "iter";
	shader->uniform_names[3] = "iter2i";
	shader->uniform_names[4] = "step2i";

    int rc = fleyeutil_build_shader_program(shader,vs,fs);    
	return rc;
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


int create_image_processing(RASPITEX_STATE* state, const char* filename)
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
		vcos_log_error("failed to open processing script %s",tmp);
		return -1;
	}
	printf("using processing script %s\n",tmp);

	state->nProcessingSteps = 0;
	state->cpu_tracking_state.cpuFunc = 0;
	state->cpu_tracking_state.nAvailCpuFuncs = 0;
	state->cpu_tracking_state.nFinishedCpuFuncs = 0;

	while( state->nProcessingSteps<IMGPROC_MAX_STEPS  && !feof(fp) && fscanf(fp,"%s",tmp)==1 )
	{
		memset( & state->processing_step[state->nProcessingSteps] , 0, sizeof(ProcessingStep) );

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
			ShaderPass* shaderPass = & state->processing_step[state->nProcessingSteps].shaderPass;
			
			shaderPass->compileCacheSize = 0;
			state->processing_step[state->nProcessingSteps].exec_thread = PROCESSING_GPU;

			// one texture will be allocated and named upon the shader
			// texture will always have properties of the last texture the shader has rendered to
			shaderPass->finalTexture = & state->processing_texture[state->nTextures++];
			shaderPass->finalTexture->format = GL_RGB;
			shaderPass->finalTexture->target = GL_TEXTURE_2D;
			shaderPass->finalTexture->texid = 0; // will disable corresponding texture unit

			// read shader pass description line
			fscanf(fp,"%s %s %s %s %s %s %s\n",shaderPass->finalTexture->name,vsFileName,fsFileName,drawMethod,inputTextureBlock,outputFBOBlock,tmp);
			printf("SHADER: %s %s %s %s %s %s %s\n",shaderPass->finalTexture->name,vsFileName,fsFileName,drawMethod,inputTextureBlock,outputFBOBlock,tmp);

			// read pass count, possibly a variable ($something)
			if( tmp[0]=='$' ) { count=atoi( fleye_optional_value(state,tmp+1) ); }
			else { count=atoi(tmp); }
			state->processing_step[state->nProcessingSteps].numberOfPasses = count;
			
			// assemble vertex and fragment sources
			{
				char* user_vs = 0;
				char* user_fs = 0;
				char* inc_fs = 0;
				char* sep = 0;
				int vs_size=0, fs_size=0;

				user_vs = readShader(vsFileName);
				vs_size = strlen(vs_attributes) + strlen(uniforms) + strlen(user_vs);
				vs = malloc( vs_size + 8 );
				sprintf(vs,"%s\n%s\n%s\n",vs_attributes,uniforms,user_vs);
				free(user_vs);
				//printf("Vertex Shader:\n%s",vs);

				user_fs = readShader(fsFileName);
				inc_fs = readShader("inc_fs");
				fs_size = strlen(uniforms) + strlen(inc_fs) + strlen(user_fs) ;
				fs = malloc( fs_size + 8 );
				sprintf(fs,"%s\n%s\n%s\n",uniforms,inc_fs,user_fs);
				free(inc_fs);
				free(user_fs);
				//printf("Fragment Shader:\n%s",fs);
			}
			//printf("Compiling shader : %s/%s ...\n",vsFileName,fsFileName);
			//rc = create_image_shader( & state->processing_step[state->n_processing_steps].gl_shader, vs, fs );
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
						vcos_log_error("failed to load plugin %s",tmp2);
						return -1;
					}
					funcName = drawPlugin[1];
				}
				else
				{
					handle = dlopen(NULL, RTLD_GLOBAL | RTLD_NOW);
					funcName = drawPlugin[0];
				}
				state->processing_step[state->nProcessingSteps].gl_draw = dlsym(handle,funcName);
				if( state->processing_step[state->nProcessingSteps].gl_draw == NULL)
				{
					vcos_log_error("can't find function %s",drawPlugin[1]);
					return -1;
				}
				printf("resolved function %s to %p\n",funcName,state->processing_step[state->nProcessingSteps].gl_draw);
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
							shaderPass->inputs[ti].texPool[pi] = get_named_texture(state,texPoolNames[pi]);
						}
					}
					else
					{
						vcos_log_error("syntax error: expected '='");
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
					shaderPass->fboPool[oi] = get_named_fbo(state,outputFBONames[oi]);
				}
			}
			++ state->nProcessingSteps;
		}
		else if( strcasecmp(tmp,"FBO")==0 )
		{
			char name[TEXTURE_NAME_MAX_LEN];
			char widthStr[64];
			char heightStr[64];
			fscanf(fp,"%s %s %s\n",name,widthStr,heightStr);
			int w = atoi( (widthStr[0]=='$') ? fleye_optional_value(state,widthStr+1) : widthStr );
			int h = atoi( (heightStr[0]=='$') ? fleye_optional_value(state,heightStr+1) : heightStr );
			add_fbo(state,name,GL_RGBA,w,h);
		}
		// add a TEXTURE keyword to load an image ? might be usefull
		/*else if( strcasecmp(tmp,"TEXTURE")==0 )
		{
			...
		}*/		
		else if( strcasecmp(tmp,"CPU")==0 )
		{
			char tmp2[256];
			state->processing_step[state->nProcessingSteps].numberOfPasses = CPU_PROCESSING_PASS;
			fscanf(fp,"%s %d\n",tmp, & state->processing_step[state->nProcessingSteps].exec_thread );
			sprintf(tmp2,"./lib%s.so",tmp);
			printf("loading dynamic library %s ...\n",tmp2);
			void * handle = dlopen(tmp2, RTLD_GLOBAL | RTLD_NOW);
			if(handle==NULL)
			{
				vcos_log_error("failed to load plugin %s",tmp2);
				return -1;
			}
			sprintf(tmp2,"%s_run",tmp);
			state->processing_step[state->nProcessingSteps].cpu_processing = dlsym(handle,tmp2);
			if( state->processing_step[state->nProcessingSteps].cpu_processing == NULL)
			{
				vcos_log_error("can't find function %s",tmp2);
				return -1;
			}
			sprintf(tmp2,"%s_setup",tmp);
			void(*init_plugin)() = dlsym(handle,tmp2);
			if( init_plugin != NULL )
			{
				(*init_plugin) ();
			}
			
			++ state->nProcessingSteps;
		}
		else
		{
			vcos_log_error("bad processing step type '%s'",tmp);
			return -1;
		}
	}

	fclose(fp);
	printf("processing pipeline has %d steps\n",state->nProcessingSteps);

	printf("Frame Buffers:\n");
	int i;
	for(i=0;i<state->nFBO;i++) { printf("\t%s %dx%d\n",state->processing_fbo[i].texture->name,state->processing_fbo[i].width,state->processing_fbo[i].height); }
	printf("Textures:\n");
	for(i=0;i<state->nTextures;i++) { printf("\t%s %d\n",state->processing_texture[i].name,state->processing_texture[i].texid); }

	printf("Processing steps:\n");
	for(i=0;i<state->nProcessingSteps;i++)
	{
		if(state->processing_step[i].numberOfPasses == CPU_PROCESSING_PASS)
		{
			printf("\tCPU %p\n",state->processing_step[i].cpu_processing);
		}
		else
		{
			printf("\tShader %s %d\n",state->processing_step[i].shaderPass.finalTexture->name,state->processing_step[i].numberOfPasses);
			int j;
			for(j=0;j<state->processing_step[i].shaderPass.nInputs;j++)
			{
				printf("\t\t%s <-",state->processing_step[i].shaderPass.inputs[j].uniformName, state->processing_step[i].shaderPass.inputs[j].poolSize);
				int k;
				for(k=0;k<state->processing_step[i].shaderPass.inputs[j].poolSize;k++)
				{
					printf("%s%s",((k>0)?", ":""),state->processing_step[i].shaderPass.inputs[j].texPool[k]->name);
				}
				printf("\n");
			}
		}
	}


	return 0;
}


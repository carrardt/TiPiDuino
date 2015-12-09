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

#ifndef RASPITEX_UTIL_H_
#define RASPITEX_UTIL_H_

#define VCOS_LOG_CATEGORY (&fleye_log_category)
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "interface/vcos/vcos.h"

extern VCOS_LOG_CAT_T fleye_log_category;

#include "fleye_core.h"
#include "fleye/shaderprogram.h"
#include "fleye/fbo.h"

/* Uncomment to enable extra GL error checking */
//#define CHECK_GL_ERRORS
#if defined(CHECK_GL_ERRORS)
#define GLCHK(X) \
do { \
    GLenum err = GL_NO_ERROR; \
    X; \
   while ((err = glGetError())) \
   { \
      vcos_log_error("GL error 0x%x in " #X "file %s line %d", err, __FILE__,__LINE__); \
      vcos_assert(err == GL_NO_ERROR); \
      exit(err); \
   } \
} \
while(0)
#else
#define GLCHK(X) X
#endif /* CHECK_GL_ERRORS */

/* Default GL scene ops functions */
int fleyeutil_create_native_window(RASPITEX_STATE *fleye_state);
int fleyeutil_gl_init_1_0(RASPITEX_STATE *fleye_state);
int fleyeutil_gl_init_2_0(RASPITEX_STATE *fleye_state);
int fleyeutil_update_model(RASPITEX_STATE* fleye_state);
int fleyeutil_redraw(RASPITEX_STATE* fleye_state);
void fleyeutil_gl_term(RASPITEX_STATE *fleye_state);
void fleyeutil_destroy_native_window(RASPITEX_STATE *fleye_state);
int fleyeutil_create_textures(RASPITEX_STATE *fleye_state);
int fleyeutil_update_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf);
int fleyeutil_update_y_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf);
int fleyeutil_update_u_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf);
int fleyeutil_update_v_texture(RASPITEX_STATE *fleye_state,
      EGLClientBuffer mm_buf);
int fleyeutil_capture_bgra(struct RASPITEX_STATE *state,
      uint8_t **buffer, size_t *buffer_size);
void fleyeutil_close(RASPITEX_STATE* fleye_state);

/* Utility functions */
char* readShader(const char* fileName);
int fleyeutil_build_shader_program(RASPITEXUTIL_SHADER_PROGRAM_T *p, const char* vs, const char* fs);
int create_image_shader(RASPITEXUTIL_SHADER_PROGRAM_T* shader, const char* vs, const char* fs);
void fleyeutil_brga_to_rgba(uint8_t *buffer, size_t size);
int create_fbo(RASPITEX_STATE *state, RASPITEX_FBO* fbo, GLint colorFormat, GLint w, GLint h);
int create_image_processing(RASPITEX_STATE* state, const char* filename);

#endif /* RASPITEX_UTIL_H_ */

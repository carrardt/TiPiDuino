#ifndef fleye_SHADER_PROGRAM_H_
#define fleye_SHADER_PROGRAM_H_

#include <GLES2/gl2.h>
#include "fleye/config.h"

#include <string>
#include <vector>
/**
 * Container for a simple shader program. The uniform and attribute locations
 * are automatically setup by fleye_build_shader_program.
 */
struct ShaderProgram
{
   /// Array of uniform names for fleye_build_shader_program to process
   std::vector<std::string> uniform_names;
   
   /// Array of attribute names for fleye_build_shader_program to process
   std::vector<std::string> attribute_names;

   GLint vs;                        /// Vertex shader handle
   GLint fs;                        /// Fragment shader handle
   GLint program;                   /// Shader program handle

   /// The locations for uniforms defined in uniform_names
   std::vector<GLint> uniform_locations;

   /// The locations for attributes defined in attribute_names
   std::vector<GLint> attribute_locations;
};

int fleyeutil_build_shader_program(ShaderProgram *p, const char* vertex_source, const char* fragment_source);
int create_image_shader(ShaderProgram* shader, const char* vs, const char* fs);

#endif

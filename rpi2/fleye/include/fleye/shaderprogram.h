#ifndef fleye_SHADER_PROGRAM_H_
#define fleye_SHADER_PROGRAM_H_

#include <GLES2/gl2.h>
#include "fleye/config.h"

/**
 * Container for a simple shader program. The uniform and attribute locations
 * are automatically setup by fleye_build_shader_program.
 */
typedef struct ShaderProgram
{
   /// Array of uniform names for fleye_build_shader_program to process
   const char *uniform_names[SHADER_MAX_UNIFORMS];
   /// Array of attribute names for fleye_build_shader_program to process
   const char *attribute_names[SHADER_MAX_ATTRIBUTES];

   GLint vs;                        /// Vertex shader handle
   GLint fs;                        /// Fragment shader handle
   GLint program;                   /// Shader program handle

   /// The locations for uniforms defined in uniform_names
   GLint uniform_locations[SHADER_MAX_UNIFORMS];

   /// The locations for attributes defined in attribute_names
   GLint attribute_locations[SHADER_MAX_ATTRIBUTES];
} ShaderProgram;

#endif

#ifndef __fleye_config_h
#define __fleye_config_h

#ifdef __cplusplus
extern "C" {
#endif

#define RASPITEX_VERSION_MAJOR 1
#define RASPITEX_VERSION_MINOR 0

#define MAX_TEXTURES 16
#define MAX_FBOS 16

#define SHADER_MAX_INPUT_TEXTURES 4
#define SHADER_MAX_OUTPUT_FBOS 4
#define UNIFORM_NAME_MAX_LEN 64
#define TEXTURE_NAME_MAX_LEN 64

#define SHADER_MAX_ATTRIBUTES 16
#define SHADER_MAX_UNIFORMS   16

#define SHADER_COMPILE_CACHE_SIZE 16
#define IMGPROC_MAX_STEPS 16

#define CPU_PROCESSING_PASS 			-1
#define PROCESSING_GPU		    		-1
#define PROCESSING_MAIN_THREAD		    0
#define PROCESSING_ASYNC_THREAD			1

#ifdef __cplusplus
}
#endif

#endif



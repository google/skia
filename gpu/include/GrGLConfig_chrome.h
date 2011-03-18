#ifndef GrGLConfig_chrome_DEFINED
#define GrGLConfig_chrome_DEFINED

#define GR_SUPPORT_GLES2    1

// gl2ext.h will define these extensions macros but Chrome doesn't provide
// prototypes.
#define GL_OES_mapbuffer                      0
#define GL_IMG_multisampled_render_to_texture 0

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GR_GL_FUNC

#define GR_GL_PROC_ADDRESS(X)       &X

// chrome always assumes BGRA
#define GR_GL_32BPP_COLOR_FORMAT    GR_BGRA

// glGetError() forces a sync with gpu process on chrome
#define GR_GL_CHECK_ERROR_START     0

#endif

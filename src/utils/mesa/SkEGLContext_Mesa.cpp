#include "SkEGLContext.h"
#include "SkTypes.h"

#include "GL/osmesa.h"
#include "GL/glu.h"

#define SK_GL_DECL_PROC(T, F) T F ## _func = NULL;
#define SK_GL_GET_PROC(T, F) F ## _func = (T)OSMesaGetProcAddress(#F);
#define SK_GL_GET_EXT_PROC(T, F) F ## _func = (T)OSMesaGetProcAddress(#F "EXT");

SkEGLContext::SkEGLContext() : context(NULL), image(NULL) {
}

SkEGLContext::~SkEGLContext() {
    if (this->image)
        free(this->image);
    
    if (this->context)
        OSMesaDestroyContext(this->context);
}

#if SK_B32_SHIFT < SK_G32_SHIFT &&\
                   SK_G32_SHIFT < SK_R32_SHIFT &&\
                                  SK_R32_SHIFT < SK_A32_SHIFT
    #define SK_OSMESA_COLOR_ORDER OSMESA_BGRA
#elif SK_R32_SHIFT < SK_G32_SHIFT &&\
                     SK_G32_SHIFT < SK_B32_SHIFT &&\
                                    SK_B32_SHIFT < SK_A32_SHIFT
    #define SK_OSMESA_COLOR_ORDER OSMESA_RGBA
#elif SK_A32_SHIFT < SK_R32_SHIFT && \
                     SK_R32_SHIFT < SK_G32_SHIFT && \
                                    SK_G32_SHIFT < SK_B32_SHIFT
    #define SK_OSMESA_COLOR_ORDER OSMESA_ARGB
#else
    //Color order (rgba) SK_R32_SHIFT SK_G32_SHIFT SK_B32_SHIFT SK_A32_SHIFT
    #define SK_OSMESA_COLOR_ORDER OSMESA_RGBA
#endif

bool SkEGLContext::init(const int width, const int height) {
    /* Create an RGBA-mode context */
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
    /* specify Z, stencil, accum sizes */
    OSMesaContext ctx = OSMesaCreateContextExt(SK_OSMESA_COLOR_ORDER, 16, 0, 0, NULL);
#else
    OSMesaContext ctx = OSMesaCreateContext(SK_OSMESA_COLOR_ORDER, NULL);
#endif
    if (!ctx) {
        SkDebugf("OSMesaCreateContext failed!\n");
        return false;
    }
    this->context = ctx;
    
    // Allocate the image buffer
    GLfloat *buffer = (GLfloat *) malloc(width * height * 4 * sizeof(GLfloat));
    if (!buffer) {
        SkDebugf("Alloc image buffer failed!\n");
        return false;
    }
    this->image = buffer;
    
    // Bind the buffer to the context and make it current
    if (!OSMesaMakeCurrent(ctx, buffer, GL_FLOAT, width, height)) {
        SkDebugf("OSMesaMakeCurrent failed!\n");
        return false;
    }
    
    //Setup the framebuffers
    SK_GL_DECL_PROC(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffers)
    SK_GL_DECL_PROC(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebuffer)
    SK_GL_DECL_PROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers)
    SK_GL_DECL_PROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer)
    SK_GL_DECL_PROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage)
    SK_GL_DECL_PROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer)
    SK_GL_DECL_PROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatus)
    
    const GLubyte* glExts = glGetString(GL_EXTENSIONS);
    if (gluCheckExtension(
          reinterpret_cast<const GLubyte*>("GL_ARB_framebuffer_object")
          , glExts))
    {
        SK_GL_GET_PROC(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffers)
        SK_GL_GET_PROC(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebuffer)
        SK_GL_GET_PROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers)
        SK_GL_GET_PROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer)
        SK_GL_GET_PROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage)
        SK_GL_GET_PROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer)
        SK_GL_GET_PROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatus)
        
    //osmesa on mac currently only supports EXT
    } else if (gluCheckExtension(
          reinterpret_cast<const GLubyte*>("GL_EXT_framebuffer_object")
          , glExts))
    {
        SK_GL_GET_EXT_PROC(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffers)
        SK_GL_GET_EXT_PROC(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebuffer)
        SK_GL_GET_EXT_PROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers)
        SK_GL_GET_EXT_PROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer)
        SK_GL_GET_EXT_PROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage)
        SK_GL_GET_EXT_PROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer)
        SK_GL_GET_EXT_PROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatus)
    } else {
      SkDebugf("GL_ARB_framebuffer_object not found.\n");
      return false;
    }
    
    GLuint fboID;
    GLuint cbID;
    GLuint dsID;
    glGenFramebuffers_func(1, &fboID);
    glBindFramebuffer_func(GL_FRAMEBUFFER, fboID);
    
    glGenRenderbuffers_func(1, &cbID);
    glBindRenderbuffer_func(GL_RENDERBUFFER, cbID);
    glRenderbufferStorage_func(GL_RENDERBUFFER, OSMESA_RGBA, width, height);
    glFramebufferRenderbuffer_func(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cbID);
    
    glGenRenderbuffers_func(1, &dsID);
    glBindRenderbuffer_func(GL_RENDERBUFFER_EXT, dsID);
    glRenderbufferStorage_func(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
    glFramebufferRenderbuffer_func(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsID);
    
    glViewport(0, 0, width, height);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    GLenum status = glCheckFramebufferStatus_func(GL_FRAMEBUFFER);
    return GL_FRAMEBUFFER_COMPLETE == status;
}

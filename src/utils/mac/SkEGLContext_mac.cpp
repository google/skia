#include "SkEGLContext.h"
//#include "SkTypes.h"
#include <AGL/agl.h>

SkEGLContext::SkEGLContext() : context(NULL) {
}

SkEGLContext::~SkEGLContext() {
    if (this->context) {
        aglDestroyContext(this->context);
    }
}

bool SkEGLContext::init(int width, int height) {
    GLint major, minor;
    AGLContext ctx;

    aglGetVersion(&major, &minor);
    //SkDebugf("---- agl version %d %d\n", major, minor);

    const GLint pixelAttrs[] = {
        AGL_RGBA,
        AGL_STENCIL_SIZE, 8,
/*
        AGL_SAMPLE_BUFFERS_ARB, 1,
        AGL_MULTISAMPLE,
        AGL_SAMPLES_ARB, 2,
*/
        AGL_ACCELERATED,
        AGL_NONE
    };
    AGLPixelFormat format = aglChoosePixelFormat(NULL, 0, pixelAttrs);
    //AGLPixelFormat format = aglCreatePixelFormat(pixelAttrs);
    //SkDebugf("----- agl format %p\n", format);
    ctx = aglCreateContext(format, NULL);
    //SkDebugf("----- agl context %p\n", ctx);
    aglDestroyPixelFormat(format);

/*
    static const GLint interval = 1;
    aglSetInteger(ctx, AGL_SWAP_INTERVAL, &interval);
*/

    aglSetCurrentContext(ctx);
    this->context = ctx;

    // Now create our FBO render target

    GLuint fboID;
    GLuint cbID;
    GLuint dsID;
    glGenFramebuffersEXT(1, &fboID);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
    glGenRenderbuffers(1, &cbID);
    glBindRenderbuffer(GL_RENDERBUFFER, cbID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cbID);
    glGenRenderbuffers(1, &dsID);
    glBindRenderbuffer(GL_RENDERBUFFER, dsID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsID);
    glViewport(0, 0, width, height);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    return GL_FRAMEBUFFER_COMPLETE == status;
}

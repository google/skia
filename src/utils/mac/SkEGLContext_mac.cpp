
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkEGLContext.h"
//#include "SkTypes.h"
#include <AGL/agl.h>

SkEGLContext::SkEGLContext() 
    : fFBO(0)
    , context(NULL) {
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

    GLuint cbID;
    GLuint dsID;
    glGenFramebuffersEXT(1, &fFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fFBO);
    glGenRenderbuffersEXT(1, &cbID);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, cbID);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER_EXT, cbID);
    glGenRenderbuffersEXT(1, &dsID);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dsID);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, dsID);
    glViewport(0, 0, width, height);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    return GL_FRAMEBUFFER_COMPLETE_EXT == status;
}


/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkGLContext.h"

SkGLContext::SkGLContext()
    : fFBO(0)
    , fGL(NULL) {
}

SkGLContext::~SkGLContext() {
    SkSafeUnref(fGL);
}

bool SkGLContext::init(int width, int height) {
    if (fGL) {
        fGL->unref();
        this->destroyGLContext();
    }

    fGL = this->createGLContext();
    if (fGL) {
        // clear any existing GL erorrs
        GrGLenum error;
        do {
            error = SK_GL(*this, GetError());
        } while (GR_GL_NO_ERROR != error);
        GrGLuint cbID;
        GrGLuint dsID;
        SK_GL(*this, GenFramebuffers(1, &fFBO));
        SK_GL(*this, BindFramebuffer(GR_GL_FRAMEBUFFER, fFBO));
        SK_GL(*this, GenRenderbuffers(1, &cbID));
        SK_GL(*this, BindRenderbuffer(GR_GL_RENDERBUFFER, cbID));
        if (fGL->supportsES2()) {
            SK_GL(*this, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                             GR_GL_RGBA8,
                                             width, height));
        } else {
            SK_GL(*this, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                             GR_GL_RGBA,
                                             width, height));
        }
        SK_GL(*this, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                             GR_GL_COLOR_ATTACHMENT0,
                                             GR_GL_RENDERBUFFER, 
                                             cbID));
        SK_GL(*this, GenRenderbuffers(1, &dsID));
        SK_GL(*this, BindRenderbuffer(GR_GL_RENDERBUFFER, dsID));
        if (fGL->supportsES2()) {
            SK_GL(*this, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                             GR_GL_STENCIL_INDEX8,
                                             width, height));
        } else {
            SK_GL(*this, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                             GR_GL_DEPTH_STENCIL,
                                             width, height));
            SK_GL(*this, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                 GR_GL_DEPTH_ATTACHMENT,
                                                 GR_GL_RENDERBUFFER,
                                                 dsID));
        }
        SK_GL(*this, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                             GR_GL_STENCIL_ATTACHMENT,
                                             GR_GL_RENDERBUFFER,
                                             dsID));
        SK_GL(*this, Viewport(0, 0, width, height));
        SK_GL(*this, ClearStencil(0));
        SK_GL(*this, Clear(GR_GL_STENCIL_BUFFER_BIT));
        
        error = SK_GL(*this, GetError());
        GrGLenum status =
            SK_GL(*this, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));

        if (GR_GL_FRAMEBUFFER_COMPLETE != status ||
            GR_GL_NO_ERROR != error) {
            fFBO = 0;
            fGL->unref();
            fGL = NULL;
            this->destroyGLContext();
            return false;
        } else {
            return true;
        }
    }
    return false;
}

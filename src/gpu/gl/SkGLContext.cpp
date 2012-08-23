
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"
#include "gl/GrGLUtil.h"

SK_DEFINE_INST_COUNT(SkGLContext)

SkGLContext::SkGLContext()
    : fFBO(0)
    , fColorBufferID(0)
    , fDepthStencilBufferID(0)
    , fGL(NULL) {
}

SkGLContext::~SkGLContext() {

    if (fGL) {
        SK_GL(*this, DeleteFramebuffers(1, &fFBO));
        SK_GL(*this, DeleteRenderbuffers(1, &fColorBufferID));
        SK_GL(*this, DeleteRenderbuffers(1, &fDepthStencilBufferID));
    }

    SkSafeUnref(fGL);
}

bool SkGLContext::hasExtension(const char* extensionName) const {
    return GrGLHasExtensionFromString(extensionName, fExtensionString.c_str());
}

bool SkGLContext::init(int width, int height) {
    if (fGL) {
        fGL->unref();
        this->destroyGLContext();
    }

    fGL = this->createGLContext();
    if (fGL) {
        fExtensionString =
            reinterpret_cast<const char*>(SK_GL(*this,
                                                 GetString(GR_GL_EXTENSIONS)));
        const char* versionStr =
            reinterpret_cast<const char*>(SK_GL(*this,
                                                GetString(GR_GL_VERSION)));
        GrGLVersion version = GrGLGetVersionFromString(versionStr);

        // clear any existing GL erorrs
        GrGLenum error;
        do {
            error = SK_GL(*this, GetError());
        } while (GR_GL_NO_ERROR != error);

        GrGLBinding bindingInUse = GrGLGetBindingInUse(this->gl());

        SK_GL(*this, GenFramebuffers(1, &fFBO));
        SK_GL(*this, BindFramebuffer(GR_GL_FRAMEBUFFER, fFBO));
        SK_GL(*this, GenRenderbuffers(1, &fColorBufferID));
        SK_GL(*this, BindRenderbuffer(GR_GL_RENDERBUFFER, fColorBufferID));
        if (kES2_GrGLBinding == bindingInUse) {
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
                                             fColorBufferID));
        SK_GL(*this, GenRenderbuffers(1, &fDepthStencilBufferID));
        SK_GL(*this, BindRenderbuffer(GR_GL_RENDERBUFFER, fDepthStencilBufferID));

        // Some drivers that support packed depth stencil will only succeed
        // in binding a packed format an FBO. However, we can't rely on packed
        // depth stencil being available.
        bool supportsPackedDepthStencil;
        if (kES2_GrGLBinding == bindingInUse) {
            supportsPackedDepthStencil =
                    this->hasExtension("GL_OES_packed_depth_stencil");
        } else {
            supportsPackedDepthStencil = version >= GR_GL_VER(3,0) ||
                    this->hasExtension("GL_EXT_packed_depth_stencil") ||
                    this->hasExtension("GL_ARB_framebuffer_object");
        }

        if (supportsPackedDepthStencil) {
            // ES2 requires sized internal formats for RenderbufferStorage
            // On Desktop we let the driver decide.
            GrGLenum format = kES2_GrGLBinding == bindingInUse ?
                                    GR_GL_DEPTH24_STENCIL8 :
                                    GR_GL_DEPTH_STENCIL;
            SK_GL(*this, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                             format,
                                             width, height));
            SK_GL(*this, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                 GR_GL_DEPTH_ATTACHMENT,
                                                 GR_GL_RENDERBUFFER,
                                                 fDepthStencilBufferID));
        } else {
            GrGLenum format = kES2_GrGLBinding == bindingInUse ?
                                    GR_GL_STENCIL_INDEX8 :
                                    GR_GL_STENCIL_INDEX;
            SK_GL(*this, RenderbufferStorage(GR_GL_RENDERBUFFER,
                                             format,
                                             width, height));
        }
        SK_GL(*this, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                             GR_GL_STENCIL_ATTACHMENT,
                                             GR_GL_RENDERBUFFER,
                                             fDepthStencilBufferID));
        SK_GL(*this, Viewport(0, 0, width, height));
        SK_GL(*this, ClearStencil(0));
        SK_GL(*this, Clear(GR_GL_STENCIL_BUFFER_BIT));

        error = SK_GL(*this, GetError());
        GrGLenum status =
            SK_GL(*this, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));

        if (GR_GL_FRAMEBUFFER_COMPLETE != status ||
            GR_GL_NO_ERROR != error) {
            fFBO = 0;
            fColorBufferID = 0;
            fDepthStencilBufferID = 0;
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

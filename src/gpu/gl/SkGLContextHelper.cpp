
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContextHelper.h"
#include "GrGLUtil.h"

SkGLContextHelper::SkGLContextHelper()
    : fFBO(0)
    , fColorBufferID(0)
    , fDepthStencilBufferID(0)
    , fGL(NULL) {
}

SkGLContextHelper::~SkGLContextHelper() {

    if (fGL) {
        // TODO: determine why DeleteFramebuffers is generating a GL error in tests
        SK_GL_NOERRCHECK(*this, DeleteFramebuffers(1, &fFBO));
        SK_GL_NOERRCHECK(*this, DeleteRenderbuffers(1, &fColorBufferID));
        SK_GL_NOERRCHECK(*this, DeleteRenderbuffers(1, &fDepthStencilBufferID));
    }

    SkSafeUnref(fGL);
}

bool SkGLContextHelper::init(int width, int height) {
    if (fGL) {
        fGL->unref();
        this->destroyGLContext();
    }

    fGL = this->createGLContext();
    if (fGL) {
        const GrGLubyte* temp;

        if (!fGL->validate()) {
            fGL = NULL;
            this->destroyGLContext();
            return false;
        }

        SK_GL_RET(*this, temp, GetString(GR_GL_VERSION));
        const char* versionStr = reinterpret_cast<const char*>(temp);
        GrGLVersion version = GrGLGetVersionFromString(versionStr);

        // clear any existing GL erorrs
        GrGLenum error;
        do {
            SK_GL_RET(*this, error, GetError());
        } while (GR_GL_NO_ERROR != error);

        SK_GL(*this, GenFramebuffers(1, &fFBO));
        SK_GL(*this, BindFramebuffer(GR_GL_FRAMEBUFFER, fFBO));
        SK_GL(*this, GenRenderbuffers(1, &fColorBufferID));
        SK_GL(*this, BindRenderbuffer(GR_GL_RENDERBUFFER, fColorBufferID));
        if (kGLES_GrGLStandard == this->gl()->fStandard) {
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
        if (kGLES_GrGLStandard == this->gl()->fStandard) {
            supportsPackedDepthStencil = version >= GR_GL_VER(3,0) ||
                                         this->hasExtension("GL_OES_packed_depth_stencil");
        } else {
            supportsPackedDepthStencil = version >= GR_GL_VER(3,0) ||
                                         this->hasExtension("GL_EXT_packed_depth_stencil") ||
                                         this->hasExtension("GL_ARB_framebuffer_object");
        }

        if (supportsPackedDepthStencil) {
            // ES2 requires sized internal formats for RenderbufferStorage
            // On Desktop we let the driver decide.
            GrGLenum format = kGLES_GrGLStandard == this->gl()->fStandard ?
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
            GrGLenum format = kGLES_GrGLStandard == this->gl()->fStandard ? GR_GL_STENCIL_INDEX8 :
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

        SK_GL_RET(*this, error, GetError());
        GrGLenum status;
        SK_GL_RET(*this, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));

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

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DawnGLWindowContext.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "common/SwapChainUtils.h"

#define GL(X) GR_GL_CALL(fGLInterface.get(), X)

namespace sk_app {

class SwapChainImplGL {
public:
    typedef void WSIContext;
    static DawnSwapChainImplementation Create(sk_sp<const GrGLInterface> glInterface) {
        auto impl = new SwapChainImplGL(glInterface);
        return CreateSwapChainImplementation<SwapChainImplGL>(impl);
    }

    void Init(WSIContext* ctx) {
        GL(GenTextures(1, &fBackTexture));
        GL(GenFramebuffers(1, &fBackFBO));
    }

    SwapChainImplGL(sk_sp<const GrGLInterface> glInterface)
        : fGLInterface(glInterface) {
    }

    ~SwapChainImplGL() {
        GL(DeleteTextures(1, &fBackTexture));
        GL(DeleteFramebuffers(1, &fBackFBO));
    }

    DawnSwapChainError Configure(DawnTextureFormat format, DawnTextureUsageBit,
            uint32_t width, uint32_t height) {
        if (format != DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
            return "unsupported format";
        }
        SkASSERT(width > 0);
        SkASSERT(height > 0);
        if (width == fWidth && height == fHeight) {
            return DAWN_SWAP_CHAIN_NO_ERROR;
        }

        fWidth = width;
        fHeight = height;

        GL(DeleteTextures(1, &fBackTexture));
        GL(GenTextures(1, &fBackTexture));
        GL(BindTexture(GR_GL_TEXTURE_2D, fBackTexture));
        // Reallocate the texture
        GL(TexStorage2D(GR_GL_TEXTURE_2D, 1, GR_GL_RGBA8, width, height));

        GL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, fBackFBO));
        GL(FramebufferTexture2D(GR_GL_READ_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                GR_GL_TEXTURE_2D, fBackTexture, 0));

        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    DawnSwapChainError GetNextTexture(DawnSwapChainNextTexture* nextTexture) {
        nextTexture->texture.ptr = reinterpret_cast<void*>(static_cast<size_t>(fBackTexture));
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    DawnSwapChainError Present() {
        GL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, fBackFBO));
        GL(BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, 0));
        GL(BlitFramebuffer(0, 0, fWidth, fHeight, 0, 0, fWidth, fHeight,
                GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }
private:
    sk_sp<const GrGLInterface> fGLInterface;
    uint32_t fWidth = 0;
    uint32_t fHeight = 0;
    GrGLuint fBackFBO = 0;
    GrGLuint fBackTexture = 0;
};

DawnGLWindowContext::DawnGLWindowContext(const DisplayParams& params)
    : DawnWindowContext(params, dawn::TextureFormat::R8G8B8A8Unorm) {
}

DawnGLWindowContext::~DawnGLWindowContext() {
}

DawnSwapChainImplementation DawnGLWindowContext::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    return SwapChainImplGL::Create(interface);
}

}   //namespace sk_app

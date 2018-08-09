/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "NXTGLWindowContext.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "common/SwapChainUtils.h"

#define GL(X) GR_GL_CALL(fGLInterface.get(), X)

namespace sk_app {

class SwapChainImplGL {
public:
    typedef void WSIContext;
    static dawnSwapChainImplementation Create(sk_sp<const GrGLInterface> glInterface) {
        auto impl = new SwapChainImplGL(glInterface);
        return CreateSwapChainImplementation<SwapChainImplGL>(impl);
    }

    void Init(WSIContext* ctx) {
        GL(GenTextures(1, &fBackTexture));
        GL(BindTexture(GR_GL_TEXTURE_2D, fBackTexture));
        GL(TexImage2D(GR_GL_TEXTURE_2D, 0, GR_GL_RGBA8, 0, 0, 0,
                      GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, nullptr));

        GL(GenFramebuffers(1, &fBackFBO));
        GL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, fBackFBO));
        GL(FramebufferTexture2D(GR_GL_READ_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                GR_GL_TEXTURE_2D, fBackTexture, 0));
    }

    SwapChainImplGL(sk_sp<const GrGLInterface> glInterface)
        : fGLInterface(glInterface) {
    }

    ~SwapChainImplGL() {
        GL(DeleteTextures(1, &fBackTexture));
        GL(DeleteFramebuffers(1, &fBackFBO));
    }

    dawnSwapChainError Configure(nxtTextureFormat format, nxtTextureUsageBit,
            uint32_t width, uint32_t height) {
        if (format != NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
            return "unsupported format";
        }
        SkASSERT(width > 0);
        SkASSERT(height > 0);
        fWidth = width;
        fHeight = height;

        GL(BindTexture(GR_GL_TEXTURE_2D, fBackTexture));
        // Reallocate the texture
        GL(TexImage2D(GR_GL_TEXTURE_2D, 0, GR_GL_RGBA8, width, height, 0,
                GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, nullptr));

        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawnSwapChainError GetNextTexture(dawnSwapChainNextTexture* nextTexture) {
        nextTexture->texture.ptr = reinterpret_cast<void*>(static_cast<size_t>(fBackTexture));
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawnSwapChainError Present() {
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

NXTGLWindowContext::NXTGLWindowContext(const DisplayParams& params)
    : NXTWindowContext(params, dawn::TextureFormat::R8G8B8A8Unorm) {
}

NXTGLWindowContext::~NXTGLWindowContext() {
}

dawnSwapChainImplementation NXTGLWindowContext::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    return SwapChainImplGL::Create(interface);
}

}   //namespace sk_app

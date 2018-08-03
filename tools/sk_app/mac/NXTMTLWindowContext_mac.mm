/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../NXTWindowContext.h"
#include "SDL_syswm.h"
#include "WindowContextFactory_mac.h"
#include "common/SwapChainUtils.h"
#include "nxt/GrNXTBackendContext.h"
#include "nxt/nxtcpp.h"
#include "nxt/nxt_wsi.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

namespace backend {
    namespace metal {
        void Init(id<MTLDevice> metalDevice, nxtProcTable* procs, nxtDevice* device);
    }
}

namespace sk_app {

using sk_app::window_context_factory::MacWindowInfo;

class NXTMTLWindowContext : public NXTWindowContext {
public:
    NXTMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params);
    ~NXTMTLWindowContext() override;
    sk_sp<GrNXTBackendContext> onInitializeContext() override;
    void onDestroyContext() override;
    nxtSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override;
private:
    SDL_Window*          fWindow;
    id<MTLDevice>        fMTLDevice;
    CAMetalLayer*        fLayer;
    NSAutoreleasePool*   fAutoreleasePool = nil;
};

class SwapChainImplMTL {
public:
    typedef void WSIContext;
    static nxtSwapChainImplementation Create(id<MTLDevice> device, CAMetalLayer* layer) {
        auto impl = new SwapChainImplMTL(device, layer);
        return CreateSwapChainImplementation<SwapChainImplMTL>(impl);
    }

    void Init(WSIContext* ctx) {
    }

    SwapChainImplMTL(id<MTLDevice> device, CAMetalLayer* layer)
      : fQueue([device newCommandQueue])
      , fLayer(layer) {
    }

    ~SwapChainImplMTL() {
    }

    nxtSwapChainError Configure(nxtTextureFormat format, nxtTextureUsageBit,
            uint32_t width, uint32_t height) {
        if (format != NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
            return "unsupported format";
        }
        SkASSERT(width > 0);
        SkASSERT(height > 0);
        fWidth = width;
        fHeight = height;

        return NXT_SWAP_CHAIN_NO_ERROR;
    }

    nxtSwapChainError GetNextTexture(nxtSwapChainNextTexture* nextTexture) {
        fCurrentDrawable = [fLayer nextDrawable];

        nextTexture->texture.ptr = reinterpret_cast<void*>(fCurrentDrawable.texture);

        return NXT_SWAP_CHAIN_NO_ERROR;
    }

    nxtSwapChainError Present() {
        id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
        [commandBuffer presentDrawable: fCurrentDrawable];
        [commandBuffer commit];
        return NXT_SWAP_CHAIN_NO_ERROR;
    }
private:
    uint32_t             fWidth = 0;
    uint32_t             fHeight = 0;
    id<MTLCommandQueue>  fQueue;
    CAMetalLayer*        fLayer;
    id<CAMetalDrawable>  fCurrentDrawable = nil;
};

NXTMTLWindowContext::NXTMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params)
    : NXTWindowContext(params, nxt::TextureFormat::B8G8R8A8Unorm)
    , fWindow(info.fWindow)
    , fMTLDevice(MTLCreateSystemDefaultDevice()) {
    int width, height;
    SDL_GetWindowSize(fWindow, &width, &height);
    this->initializeContext(width, height);
}

NXTMTLWindowContext::~NXTMTLWindowContext() {
    this->destroyContext();
}

nxtSwapChainImplementation NXTMTLWindowContext::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    return SwapChainImplMTL::Create(fMTLDevice, fLayer);
}

sk_sp<GrNXTBackendContext> NXTMTLWindowContext::onInitializeContext() {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(fWindow, &info)) {
        return nullptr;
    }
    fAutoreleasePool = [[NSAutoreleasePool alloc] init];
    NSWindow* window = info.info.cocoa.window;
    NSView* contentView = [window contentView];

    CGSize size;
    size.width = width();
    size.height = height();

    fLayer = [CAMetalLayer layer];
    [fLayer setDevice:fMTLDevice];
    [fLayer setPixelFormat: MTLPixelFormatBGRA8Unorm];
    [fLayer setFramebufferOnly: YES];
    [fLayer setDrawableSize: size];
    [fLayer setColorspace: CGColorSpaceCreateDeviceRGB()];

    [contentView setWantsLayer: YES];
    [contentView setLayer: fLayer];

    nxtDevice backendDevice;
    nxtProcTable backendProcs;

    backend::metal::Init(fMTLDevice, &backendProcs, &backendDevice);
    nxtSetProcs(&backendProcs);
    nxtQueue backendQueue = nxtDeviceCreateQueue(backendDevice);
    sk_sp<GrNXTBackendContext> ctx(new GrNXTBackendContext(backendDevice, backendQueue));

    return ctx;
}

void NXTMTLWindowContext::onDestroyContext() {
    [fAutoreleasePool release];
}

void NXTMTLWindowContext::onSwapBuffers() {
    [fAutoreleasePool drain];
    fAutoreleasePool = [[NSAutoreleasePool alloc] init];
}

namespace window_context_factory {

WindowContext* NewNXTMTLForMac(const MacWindowInfo& winInfo, const DisplayParams& params) {
    WindowContext* ctx = new NXTMTLWindowContext(winInfo, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../DawnWindowContext.h"
#include "WindowContextFactory_mac.h"
#include "common/SwapChainUtils.h"
#include "dawn/dawncpp.h"
#include "dawn/dawn_wsi.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/MetalBackend.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

namespace sk_app {

using sk_app::window_context_factory::MacWindowInfo;

class DawnMTLWindowContext : public DawnWindowContext {
public:
    DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params);
    ~DawnMTLWindowContext() override;
    dawn::Device onInitializeContext() override;
    void onDestroyContext() override;
    dawnSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override;
private:
    NSView*              fMainView;
    id<MTLDevice>        fMTLDevice;
    CAMetalLayer*        fLayer;
    NSAutoreleasePool*   fAutoreleasePool = nil;
    std::unique_ptr<dawn_native::Instance> fInstance;
};

class SwapChainImplMTL {
public:
    typedef void WSIContext;
    static dawnSwapChainImplementation Create(id<MTLDevice> device, CAMetalLayer* layer) {
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

    dawnSwapChainError Configure(dawnTextureFormat format, dawnTextureUsageBit,
            uint32_t width, uint32_t height) {
        if (format != DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
            return "unsupported format";
        }
        SkASSERT(width > 0);
        SkASSERT(height > 0);

        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawnSwapChainError GetNextTexture(dawnSwapChainNextTexture* nextTexture) {
        fCurrentDrawable = [fLayer nextDrawable];

        nextTexture->texture.ptr = reinterpret_cast<void*>(fCurrentDrawable.texture);

        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawnSwapChainError Present() {
        id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
        [commandBuffer presentDrawable: fCurrentDrawable];
        [commandBuffer commit];
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }
private:
    id<MTLCommandQueue>  fQueue;
    CAMetalLayer*        fLayer;
    id<CAMetalDrawable>  fCurrentDrawable = nil;
};

DawnMTLWindowContext::DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params)
    : DawnWindowContext(params, dawn::TextureFormat::B8G8R8A8Unorm)
    , fMainView(info.fMainView)
    , fMTLDevice(MTLCreateSystemDefaultDevice()) {
    CGSize size = fMainView.bounds.size;
    this->initializeContext(size.width, size.height);
}

DawnMTLWindowContext::~DawnMTLWindowContext() {
    this->destroyContext();
}

dawnSwapChainImplementation DawnMTLWindowContext::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    return SwapChainImplMTL::Create(fMTLDevice, fLayer);
}

dawn::Device DawnMTLWindowContext::onInitializeContext() {
    fAutoreleasePool = [[NSAutoreleasePool alloc] init];
    CGSize size;
    size.width = width();
    size.height = height();

    fLayer = [CAMetalLayer layer];
    [fLayer setDevice:fMTLDevice];
    [fLayer setPixelFormat: MTLPixelFormatBGRA8Unorm];
    [fLayer setFramebufferOnly: YES];
    [fLayer setDrawableSize: size];
    [fLayer setColorspace: CGColorSpaceCreateDeviceRGB()];

    [fMainView setWantsLayer: YES];
    [fMainView setLayer: fLayer];

    dawnDevice backendDevice = nullptr;
    dawnProcTable backendProcs;

    fInstance = std::make_unique<dawn_native::Instance>();
    fInstance->DiscoverDefaultAdapters();

    std::vector<dawn_native::Adapter> adapters = fInstance->GetAdapters();
    for (dawn_native::Adapter adapter : adapters) {
        if (adapter.GetBackendType() == dawn_native::BackendType::Metal) {
            backendDevice = adapter.CreateDevice();
            break;
        }
    }
    if (!backendDevice) {
        return nullptr;
    }

    backendProcs = dawn_native::GetProcs();
    dawnSetProcs(&backendProcs);
    return backendDevice;
}

void DawnMTLWindowContext::onDestroyContext() {
    [fAutoreleasePool release];
}

void DawnMTLWindowContext::onSwapBuffers() {
    [fAutoreleasePool drain];
    fAutoreleasePool = [[NSAutoreleasePool alloc] init];
}

namespace window_context_factory {

WindowContext* NewDawnMTLForMac(const MacWindowInfo& winInfo, const DisplayParams& params) {
    WindowContext* ctx = new DawnMTLWindowContext(winInfo, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

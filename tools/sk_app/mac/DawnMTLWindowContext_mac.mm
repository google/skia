/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/DawnWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"
#include "dawn/webgpu_cpp.h"
#include "dawn/dawn_wsi.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/MetalBackend.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

namespace sk_app {

using sk_app::window_context_factory::MacWindowInfo;

template <typename T>
DawnSwapChainImplementation CreateSwapChainImplementation(T* swapChain) {
    DawnSwapChainImplementation impl = {};
    impl.userData = swapChain;
    impl.Init = [](void* userData, void* wsiContext) {
        auto* ctx = static_cast<typename T::WSIContext*>(wsiContext);
        reinterpret_cast<T*>(userData)->Init(ctx);
    };
    impl.Destroy = [](void* userData) { delete reinterpret_cast<T*>(userData); };
    impl.Configure = [](void* userData, WGPUTextureFormat format, WGPUTextureUsage allowedUsage,
                        uint32_t width, uint32_t height) {
        return static_cast<T*>(userData)->Configure(format, allowedUsage, width, height);
    };
    impl.GetNextTexture = [](void* userData, DawnSwapChainNextTexture* nextTexture) {
        return static_cast<T*>(userData)->GetNextTexture(nextTexture);
    };
    impl.Present = [](void* userData) { return static_cast<T*>(userData)->Present(); };
    return impl;
}

class DawnMTLWindowContext : public DawnWindowContext {
public:
    DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params);
    ~DawnMTLWindowContext() override;
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override;
    DawnSwapChainImplementation createSwapChainImplementation(int width, int height,
                                                              const DisplayParams& params) override;
    void onSwapBuffers() override;
private:
    NSView*              fMainView;
    id<MTLDevice>        fMTLDevice;
    CAMetalLayer*        fLayer;
};

class SwapChainImplMTL {
public:
    typedef void WSIContext;
    static DawnSwapChainImplementation Create(id<MTLDevice> device, CAMetalLayer* layer) {
        auto impl = new SwapChainImplMTL(device, layer);
        return CreateSwapChainImplementation<SwapChainImplMTL>(impl);
    }

    void Init(WSIContext* ctx) {}

    SwapChainImplMTL(id<MTLDevice> device, CAMetalLayer* layer)
      : fQueue([device newCommandQueue])
      , fLayer(layer) {}

    ~SwapChainImplMTL() {}

    DawnSwapChainError Configure(WGPUTextureFormat format, WGPUTextureUsage,
            uint32_t width, uint32_t height) {
        if (format != WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm) {
            return "unsupported format";
        }
        SkASSERT(width > 0);
        SkASSERT(height > 0);

        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    DawnSwapChainError GetNextTexture(DawnSwapChainNextTexture* nextTexture) {
        fCurrentDrawable = [fLayer nextDrawable];

        nextTexture->texture.ptr = reinterpret_cast<void*>(fCurrentDrawable.texture);

        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    DawnSwapChainError Present() {
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
    : DawnWindowContext(params, wgpu::TextureFormat::BGRA8Unorm)
    , fMainView(info.fMainView) {
    CGSize size = fMainView.bounds.size;
    this->initializeContext(size.width, size.height);
}

DawnMTLWindowContext::~DawnMTLWindowContext() {
    this->destroyContext();
}

DawnSwapChainImplementation DawnMTLWindowContext::createSwapChainImplementation(
        int width, int height, const DisplayParams& params) {
    return SwapChainImplMTL::Create(fMTLDevice, fLayer);
}

wgpu::Device DawnMTLWindowContext::onInitializeContext() {
    wgpu::Device device = this->createDevice(wgpu::BackendType::Metal);
    if (!device) {
        return nullptr;
    }

    fMTLDevice = dawn_native::metal::GetMetalDevice(device.Get());

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

    return device;
}

void DawnMTLWindowContext::onDestroyContext() {
}

void DawnMTLWindowContext::onSwapBuffers() {
}

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeDawnMTLForMac(const MacWindowInfo& winInfo,
                                                 const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new DawnMTLWindowContext(winInfo, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

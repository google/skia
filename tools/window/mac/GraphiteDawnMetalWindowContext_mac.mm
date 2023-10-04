/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteDawnWindowContext.h"
#include "tools/window/mac/WindowContextFactory_mac.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAConstraintLayoutManager.h>
#import <QuartzCore/CAMetalLayer.h>

using skwindow::DisplayParams;
using skwindow::MacWindowInfo;
using skwindow::internal::GraphiteDawnWindowContext;

namespace {

class GraphiteDawnMetalWindowContext_mac : public GraphiteDawnWindowContext {
public:
    GraphiteDawnMetalWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~GraphiteDawnMetalWindowContext_mac() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void resize(int w, int h) override;

private:
    bool resizeInternal();

    NSView*              fMainView;
    CAMetalLayer*        fMetalLayer;
};

GraphiteDawnMetalWindowContext_mac::GraphiteDawnMetalWindowContext_mac(const MacWindowInfo& info,
                                                                       const DisplayParams& params)
    : GraphiteDawnWindowContext(params, wgpu::TextureFormat::BGRA8Unorm)
    , fMainView(info.fMainView) {

    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    CGSize backingSize = fMainView.bounds.size;
    this->initializeContext(backingSize.width * backingScaleFactor,
                            backingSize.height * backingScaleFactor);
}

GraphiteDawnMetalWindowContext_mac::~GraphiteDawnMetalWindowContext_mac() {
    this->destroyContext();
}

bool GraphiteDawnMetalWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    auto device = createDevice(wgpu::BackendType::Metal);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    // Create a CAMetalLayer that covers the whole window that will be passed to
    // CreateSurface.
    fMetalLayer = [CAMetalLayer layer];
    BOOL useVsync = fDisplayParams.fDisableVsync ? NO : YES;
    fMetalLayer.displaySyncEnabled = useVsync;
    fMainView.wantsLayer = YES;
    fMainView.layer = fMetalLayer;

    // Adjust fMetalLayer size based on window size.
    this->resizeInternal();

    wgpu::SurfaceDescriptorFromMetalLayer surfaceChainedDesc;
    surfaceChainedDesc.layer = fMetalLayer;
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;

    auto surface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    if (!surface) {
        SkASSERT(false);
        return false;
    }


    fDevice = std::move(device);
    fSurface = std::move(surface);
    fSwapChain = this->createSwapChain();

    return true;
}

void GraphiteDawnMetalWindowContext_mac::onDestroyContext() {
    fMetalLayer = nil;
    fMainView.layer = nil;
    fMainView.wantsLayer = NO;
}

void GraphiteDawnMetalWindowContext_mac::resize(int w, int h) {
    if (!this->resizeInternal()) {
        return;
    }
    fSwapChain = this->createSwapChain();
}

bool GraphiteDawnMetalWindowContext_mac::resizeInternal() {
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    CGSize backingSize = fMainView.bounds.size;
    backingSize.width *= backingScaleFactor;
    backingSize.height *= backingScaleFactor;

    fMetalLayer.drawableSize = backingSize;
    fMetalLayer.contentsScale = backingScaleFactor;

    if (fWidth == backingSize.width && fHeight == backingSize.height) {
        return false;
    }

    fWidth = backingSize.width;
    fHeight = backingSize.height;
    return true;
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteDawnMetalForMac(const MacWindowInfo& info,
                                                           const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GraphiteDawnMetalWindowContext_mac(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/DawnWindowContext.h"
#include "tools/window/DisplayParams.h"
#include "tools/window/mac/WindowContextFactory_mac.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"

#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

using skwindow::DisplayParams;
using skwindow::MacWindowInfo;
using skwindow::internal::DawnWindowContext;

namespace {

class DawnMTLWindowContext : public DawnWindowContext {
public:
    DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params);
    ~DawnMTLWindowContext() override;
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override;
    void resize(int width, int height) override;
private:
    NSView*              fMainView;
    CAMetalLayer*        fLayer;
};

DawnMTLWindowContext::DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params)
    : DawnWindowContext(params, wgpu::TextureFormat::BGRA8Unorm)
    , fMainView(info.fMainView) {
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    CGSize size = fMainView.bounds.size;
    size.width *= backingScaleFactor;
    size.height *= backingScaleFactor;
    this->initializeContext(size.width, size.height);
}

DawnMTLWindowContext::~DawnMTLWindowContext() {
    this->destroyContext();
}

wgpu::Device DawnMTLWindowContext::onInitializeContext() {
    fLayer = [CAMetalLayer layer];
    [fLayer setFramebufferOnly: YES];
    [fLayer setColorspace: CGColorSpaceCreateDeviceRGB()];
    [fLayer setContentsScale: skwindow::GetBackingScaleFactor(fMainView)];
    [fLayer setContentsGravity: kCAGravityTopLeft];
    [fLayer setAutoresizingMask: kCALayerHeightSizable | kCALayerWidthSizable];

    [fMainView setWantsLayer: YES];
    [fMainView setLayer: fLayer];

    wgpu::SurfaceDescriptorFromMetalLayer layerDesc;
    layerDesc.layer = fLayer;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &layerDesc;

    fDawnSurface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    SkASSERT(fDawnSurface);

    return this->createDevice(wgpu::BackendType::Metal);
}

void DawnMTLWindowContext::onDestroyContext() {
}

void DawnMTLWindowContext::resize(int w, int h) {
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    CGSize size = fMainView.bounds.size;
    size.width *= backingScaleFactor;
    size.height *= backingScaleFactor;

    fLayer.drawableSize = size;
    fLayer.contentsScale = backingScaleFactor;

    DawnWindowContext::resize(size.width, size.height);
}

} // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeDawnMTLForMac(const MacWindowInfo& winInfo,
                                                 const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new DawnMTLWindowContext(winInfo, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

} // namespace skwindow

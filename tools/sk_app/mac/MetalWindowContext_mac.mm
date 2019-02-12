
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../MetalWindowContext.h"
#include "WindowContextFactory_mac.h"

#import <MetalKit/MetalKit.h>

#include <Cocoa/Cocoa.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::MetalWindowContext;

namespace {

class MetalWindowContext_mac : public MetalWindowContext {
public:
    MetalWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~MetalWindowContext_mac() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;

private:
    NSWindow*              fNSWindow;

    typedef MetalWindowContext INHERITED;
};

MetalWindowContext_mac::MetalWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fNSWindow(info.fNSWindow) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

MetalWindowContext_mac::~MetalWindowContext_mac() {
    this->destroyContext();
}

bool MetalWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fNSWindow);

    // create mtkview
    fMTKView = [[MTKView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1) device:fDevice];
    if (nil == fMTKView) {
        return false;
    }

    fMTKView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (![fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
            return false;
        }
    }
    fMTKView.sampleCount = fDisplayParams.fMSAASampleCount;

    // attach view to window
    [fNSWindow setContentView:fMTKView];

    fSampleCount = [fMTKView sampleCount];
    fStencilBits = 8;

    CGSize backingSize = [fMTKView drawableSize];
    fWidth = backingSize.width;
    fHeight = backingSize.height;

    return true;
}

void MetalWindowContext_mac::onDestroyContext() {
    [fNSWindow setContentView:nil];
    [fMTKView release];
    fMTKView = nil;
}

}  // anonymous namespace


namespace sk_app {
namespace window_context_factory {

WindowContext* NewMetalForMac(const MacWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new MetalWindowContext_mac(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

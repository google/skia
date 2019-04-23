
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/MetalWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"

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
    NSView*              fMainView;

    typedef MetalWindowContext INHERITED;
};

MetalWindowContext_mac::MetalWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fMainView(info.fMainView) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

MetalWindowContext_mac::~MetalWindowContext_mac() {
    this->destroyContext();
}

bool MetalWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    // create mtkview
    NSRect rect = fMainView.bounds;
    fMTKView = [[MTKView alloc] initWithFrame:rect device:fDevice];
    if (nil == fMTKView) {
        return false;
    }

    fMTKView.autoResizeDrawable = NO;
    fMTKView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    fMTKView.drawableSize = rect.size;

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (![fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
            return false;
        }
    }
    fMTKView.sampleCount = fDisplayParams.fMSAASampleCount;

    // attach Metal view to main view
    [fMTKView setTranslatesAutoresizingMaskIntoConstraints:NO];

    [fMainView addSubview:fMTKView];
    NSDictionary *views = NSDictionaryOfVariableBindings(fMTKView);

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[fMTKView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[fMTKView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    fSampleCount = [fMTKView sampleCount];
    fStencilBits = 8;

    CGSize backingSize = [fMTKView drawableSize];
    fWidth = backingSize.width;
    fHeight = backingSize.height;

    return true;
}

void MetalWindowContext_mac::onDestroyContext() {
    [fMTKView removeFromSuperview];
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

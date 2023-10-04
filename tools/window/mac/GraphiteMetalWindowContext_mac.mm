/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteMetalWindowContext.h"
#include "tools/window/mac/WindowContextFactory_mac.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAConstraintLayoutManager.h>

using skwindow::DisplayParams;
using skwindow::MacWindowInfo;
using skwindow::internal::GraphiteMetalWindowContext;

namespace {

class GraphiteMetalWindowContext_mac : public GraphiteMetalWindowContext {
public:
    GraphiteMetalWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~GraphiteMetalWindowContext_mac() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    NSView*              fMainView;
};

GraphiteMetalWindowContext_mac::GraphiteMetalWindowContext_mac(const MacWindowInfo& info,
                                                               const DisplayParams& params)
    : GraphiteMetalWindowContext(params)
    , fMainView(info.fMainView) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GraphiteMetalWindowContext_mac::~GraphiteMetalWindowContext_mac() {
    this->destroyContext();
}

bool GraphiteMetalWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    fMetalLayer = [CAMetalLayer layer];
    fMetalLayer.device = fDevice.get();
    fMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    // resize ignores the passed values and uses the fMainView directly.
    this->resize(0, 0);

    BOOL useVsync = fDisplayParams.fDisableVsync ? NO : YES;
    fMetalLayer.displaySyncEnabled = useVsync;
    fMetalLayer.layoutManager = [CAConstraintLayoutManager layoutManager];
    fMetalLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
    fMetalLayer.contentsGravity = kCAGravityTopLeft;
    fMetalLayer.magnificationFilter = kCAFilterNearest;
    fMetalLayer.framebufferOnly = false;
    NSColorSpace* cs = fMainView.window.colorSpace;
    fMetalLayer.colorspace = cs.CGColorSpace;

    fMainView.layer = fMetalLayer;
    fMainView.wantsLayer = YES;

    return true;
}

void GraphiteMetalWindowContext_mac::onDestroyContext() {}

void GraphiteMetalWindowContext_mac::resize(int w, int h) {
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    CGSize backingSize = fMainView.bounds.size;
    backingSize.width *= backingScaleFactor;
    backingSize.height *= backingScaleFactor;

    fMetalLayer.drawableSize = backingSize;
    fMetalLayer.contentsScale = backingScaleFactor;

    fWidth = backingSize.width;
    fHeight = backingSize.height;
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteMetalForMac(const MacWindowInfo& info,
                                                       const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GraphiteMetalWindowContext_mac(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

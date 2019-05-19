/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/MetalWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"

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

    void resize(int w, int h) override;

private:
    NSView*              fMainView;

    typedef MetalWindowContext INHERITED;
};

MetalWindowContext_mac::MetalWindowContext_mac(const MacWindowInfo& info,
                                               const DisplayParams& params)
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

    NSRect frameRect = [fMainView frame];
    fMetalLayer.drawableSize = frameRect.size;
    fMetalLayer.frame = frameRect;

    fMainView.layer = fMetalLayer;
    fMainView.wantsLayer = YES;

    fWidth = frameRect.size.width;
    fHeight = frameRect.size.height;

    return true;
}

void MetalWindowContext_mac::onDestroyContext() {
    fMainView.layer = nil;
    fMainView.wantsLayer = NO;
}

void MetalWindowContext_mac::resize(int w, int h) {
    fMetalLayer.drawableSize = fMainView.frame.size;
    fMetalLayer.frame = fMainView.frame;
    fWidth = w;
    fHeight = h;
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

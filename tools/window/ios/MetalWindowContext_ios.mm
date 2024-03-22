/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/MetalWindowContext.h"
#include "tools/window/ios/WindowContextFactory_ios.h"

#import <Metal/Metal.h>
#import <UIKit/UIKit.h>

using skwindow::DisplayParams;
using skwindow::IOSWindowInfo;
using skwindow::internal::MetalWindowContext;

@interface MetalView : MainView
@end

@implementation MetalView
+ (Class) layerClass {
    return [CAMetalLayer class];
}
@end

namespace {

class MetalWindowContext_ios : public MetalWindowContext {
public:
    MetalWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~MetalWindowContext_ios() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    sk_app::Window_ios*  fWindow;
    UIViewController*    fViewController;
    MetalView*           fMetalView;
};

MetalWindowContext_ios::MetalWindowContext_ios(const IOSWindowInfo& info,
                                               const DisplayParams& params)
    : MetalWindowContext(params)
    , fWindow(info.fWindow)
    , fViewController(info.fViewController) {

    // iOS test apps currently ignore MSAA settings.

    this->initializeContext();
}

MetalWindowContext_ios::~MetalWindowContext_ios() {
    this->destroyContext();
    [fMetalView removeFromSuperview];
    [fMetalView release];
}

bool MetalWindowContext_ios::onInitializeContext() {
    SkASSERT(fWindow != nil);
    SkASSERT(fViewController != nil);

    CGRect frameRect = [fViewController.view frame];
    fMetalView = [[[MetalView alloc] initWithFrame:frameRect] initWithWindow:fWindow];
    [fViewController.view addSubview:fMetalView];

    fMetalLayer = (CAMetalLayer*)fMetalView.layer;
    fMetalLayer.device = fDevice.get();
    fMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    fMetalLayer.drawableSize = frameRect.size;
    fMetalLayer.frame = frameRect;

    fMetalLayer.contentsGravity = kCAGravityTopLeft;

    fWidth = frameRect.size.width;
    fHeight = frameRect.size.height;

    return true;
}

void MetalWindowContext_ios::onDestroyContext() {}

void MetalWindowContext_ios::resize(int w, int h) {
    fMetalLayer.drawableSize = fMetalView.frame.size;
    fMetalLayer.frame = fMetalView.frame;
    fWidth = w;
    fHeight = h;
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeMetalForIOS(const IOSWindowInfo& info,
                                               const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new MetalWindowContext_ios(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

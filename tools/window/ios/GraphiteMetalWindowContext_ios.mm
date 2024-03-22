/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteMetalWindowContext.h"
#include "tools/window/ios/WindowContextFactory_ios.h"

#import <Metal/Metal.h>
#import <UIKit/UIKit.h>

using skwindow::DisplayParams;
using skwindow::IOSWindowInfo;
using skwindow::internal::GraphiteMetalWindowContext;

@interface GraphiteMetalView : MainView
@end

@implementation GraphiteMetalView
+ (Class) layerClass {
    return [CAMetalLayer class];
}
@end

namespace {

class GraphiteMetalWindowContext_ios : public GraphiteMetalWindowContext {
public:
    GraphiteMetalWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~GraphiteMetalWindowContext_ios() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    sk_app::Window_ios*  fWindow;
    UIViewController*    fViewController;
    GraphiteMetalView*   fMetalView;
};

GraphiteMetalWindowContext_ios::GraphiteMetalWindowContext_ios(const IOSWindowInfo& info,
                                                               const DisplayParams& params)
    : GraphiteMetalWindowContext(params)
    , fWindow(info.fWindow)
    , fViewController(info.fViewController) {

    // iOS test apps currently ignore MSAA settings.

    this->initializeContext();
}

GraphiteMetalWindowContext_ios::~GraphiteMetalWindowContext_ios() {
    this->destroyContext();
    [fMetalView removeFromSuperview];
    [fMetalView release];
}

bool GraphiteMetalWindowContext_ios::onInitializeContext() {
    SkASSERT(fWindow != nil);
    SkASSERT(fViewController != nil);

    CGRect frameRect = [fViewController.view frame];
    fMetalView = [[[GraphiteMetalView alloc] initWithFrame:frameRect] initWithWindow:fWindow];
    [fViewController.view addSubview:fMetalView];

    fMetalLayer = (CAMetalLayer*)fMetalView.layer;
    fMetalLayer.device = fDevice.get();
    fMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    fMetalLayer.drawableSize = frameRect.size;
    fMetalLayer.frame = frameRect;
    fMetalLayer.framebufferOnly = false;

    fMetalLayer.contentsGravity = kCAGravityTopLeft;

    fWidth = frameRect.size.width;
    fHeight = frameRect.size.height;

    return true;
}

void GraphiteMetalWindowContext_ios::onDestroyContext() {}

void GraphiteMetalWindowContext_ios::resize(int w, int h) {
    fMetalLayer.drawableSize = fMetalView.frame.size;
    fMetalLayer.frame = fMetalView.frame;
    fWidth = w;
    fHeight = h;
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteMetalForIOS(const IOSWindowInfo& info,
                                                       const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GraphiteMetalWindowContext_ios(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

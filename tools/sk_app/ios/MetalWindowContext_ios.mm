/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/MetalWindowContext.h"
#include "tools/sk_app/ios/WindowContextFactory_ios.h"

#import <Metal/Metal.h>
#import <UIKit/UIKit.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::IOSWindowInfo;
using sk_app::MetalWindowContext;

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

    typedef MetalWindowContext INHERITED;
};

MetalWindowContext_ios::MetalWindowContext_ios(const IOSWindowInfo& info,
                                               const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fViewController(info.fViewController) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

MetalWindowContext_ios::~MetalWindowContext_ios() {
    this->destroyContext();
    [fMetalView removeFromSuperview];
    [fMetalView release];
}

bool MetalWindowContext_ios::onInitializeContext() {
    SkASSERT(nil != fWindow);
    SkASSERT(nil != fViewController);

    CGRect frameRect = [fViewController.view frame];
    fMetalView = [[[MetalView alloc] initWithFrame:frameRect] initWithWindow:fWindow];
    [fViewController.view addSubview:fMetalView];

    fMetalLayer = (CAMetalLayer*)fMetalView.layer;
    fMetalLayer.device = fDevice;
    fMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    fMetalLayer.drawableSize = frameRect.size;
    fMetalLayer.frame = frameRect;

    // TODO: need solution for iOS
    // BOOL useVsync = fDisplayParams.fDisableVsync ? NO : YES;
    // fMetalLayer.displaySyncEnabled = useVsync;
    fMetalLayer.contentsGravity = kCAGravityTopLeft;

    fWidth = frameRect.size.width;
    fHeight = frameRect.size.height;

    return true;
}

void MetalWindowContext_ios::onDestroyContext() {}

void MetalWindowContext_ios::resize(int w, int h) {
    // TODO: handle rotation
    fMetalLayer.drawableSize = fMetalView.frame.size;
    fMetalLayer.frame = fMetalView.frame;
    fWidth = w;
    fHeight = h;
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeMetalForIOS(const IOSWindowInfo& info,
                                               const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new MetalWindowContext_ios(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

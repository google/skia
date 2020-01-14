// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// This is an example of a minimal iOS application that uses Skia to draw to
// a Metal drawable.

// Much of this code is copied from the default application created by XCode.

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

////////////////////////////////////////////////////////////////////////////////

@interface AppViewDelegate : NSObject <MTKViewDelegate>
@property (assign, nonatomic) GrContext* grContext;  // non-owning pointer.
@property (assign, nonatomic) id<MTLCommandQueue> metalQueue;
@end

@implementation AppViewDelegate {
    SkPaint fPaint;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if (![self grContext] || !view ||
        MTLPixelFormatDepth32Float_Stencil8 != [view depthStencilPixelFormat] ||
        MTLPixelFormatBGRA8Unorm != [view colorPixelFormat]) {
        NSLog(@"error: misconfigured MTKView");
        return;
    }
    // Do as much as possible before creating surface.
    if (!fPaint.getShader()) {
        const SkColor4f colors[2] = {SkColors::kBlack, SkColors::kWhite};
        const SkPoint points[2] = {{0, -1024}, {0, 1024}};
        fPaint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, nullptr, 2,
                                                      SkTileMode::kClamp, 0, nullptr));
    }
    float rotation = (float)(180 * 1e-9 * SkTime::GetNSecs());

    // Create surface:
    constexpr SkColorType colorType = kBGRA_8888_SkColorType;  // MTLPixelFormatBGRA8Unorm
    sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatBGRA8Unorm
    constexpr GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    int sampleCount = (int)[view sampleCount];
    sk_sp<SkSurface> surface = SkSurface::MakeFromMTKView(
            [self grContext], (__bridge GrMTLHandle)view, origin,
            sampleCount, colorType, colorSpace, &surfaceProps);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(surface->width() * 0.5f, surface->height() * 0.5f);
    canvas->rotate(rotation);
    canvas->drawPaint(fPaint);

    // Must flush *and* present for this to work!
    surface->flush();
    surface = nullptr;

    id<MTLCommandBuffer> commandBuffer = [[self metalQueue] commandBuffer];
    [commandBuffer presentDrawable:[view currentDrawable]];
    [commandBuffer commit];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    // change anything on size change?
}
@end

////////////////////////////////////////////////////////////////////////////////

@interface AppViewController : UIViewController
@property (strong, nonatomic) id<MTLDevice> metalDevice;
@property (strong, nonatomic) id<MTLCommandQueue> metalQueue;
@end

@implementation AppViewController {
    sk_sp<GrContext> fGrContext;
}

- (void)loadView {
    [self setView:[[MTKView alloc] initWithFrame:[[UIScreen mainScreen] bounds] device:nil]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    if (!fGrContext) {
        [self setMetalDevice:MTLCreateSystemDefaultDevice()];
        [self setMetalQueue:[[self metalDevice] newCommandQueue]];
        fGrContext = GrContext::MakeMetal((__bridge void*)[self metalDevice],
                                          (__bridge void*)[self metalQueue],
                                          GrContextOptions());
    }
    if (![self view] || ![self metalDevice]) {
        NSLog(@"Metal is not supported on this device");
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
        return;
    }
    MTKView* mtkView = (MTKView*)[self view];
    [mtkView setDevice:[self metalDevice]];
    [mtkView setBackgroundColor:[UIColor blackColor]];
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth32Float_Stencil8];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setSampleCount:1];
    AppViewDelegate* viewDelegate = [[AppViewDelegate alloc] init];
    [viewDelegate setGrContext:fGrContext.get()];
    [viewDelegate setMetalQueue:[self metalQueue]];
    [viewDelegate mtkView:mtkView drawableSizeWillChange:[mtkView bounds].size];
    [mtkView setDelegate:viewDelegate];
}
@end

////////////////////////////////////////////////////////////////////////////////

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate
- (BOOL)application:(UIApplication *)app didFinishLaunchingWithOptions:(NSDictionary*)opts {
    // Override point for customization after application launch.
    [self setWindow:[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]];
    [[self window] setFrame:[[UIScreen mainScreen] bounds]];
    [[self window] setRootViewController:[[AppViewController alloc] init]];
    [[self window] makeKeyAndVisible];
    return YES;
}
@end

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

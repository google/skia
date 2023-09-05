// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// This is an example of a minimal iOS application that uses Skia to draw to
// a Metal drawable.

// Much of this code is copied from the default application created by XCode.

#include "tools/skottie_ios_app/SkMetalViewBridge.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/base/SkTime.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

////////////////////////////////////////////////////////////////////////////////

static void config_paint(SkPaint* paint) {
    if (!paint->getShader()) {
        const SkColor4f colors[2] = {SkColors::kBlack, SkColors::kWhite};
        const SkPoint points[2] = {{0, -1024}, {0, 1024}};
        paint->setShader(SkGradientShader::MakeLinear(points, colors, nullptr, nullptr, 2,
                                                      SkTileMode::kClamp, 0, nullptr));
    }
}

static void draw_example(SkSurface* surface, const SkPaint& paint, double rotation) {
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(surface->width() * 0.5f, surface->height() * 0.5f);
    canvas->rotate(rotation);
    canvas->drawPaint(paint);
}

////////////////////////////////////////////////////////////////////////////////

@interface AppViewDelegate : NSObject <MTKViewDelegate>
@property (assign, nonatomic) GrDirectContext* grContext;  // non-owning pointer.
@property (assign, nonatomic) id<MTLCommandQueue> metalQueue;
@end

@implementation AppViewDelegate {
    SkPaint fPaint;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if (![self grContext] || !view) {
        return;
    }
    // Do as much as possible before creating surface.
    config_paint(&fPaint);
    float rotation = (float)(180 * 1e-9 * SkTime::GetNSecs());

    // Create surface:
    sk_sp<SkSurface> surface = SkMtkViewToSurface(view, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }

    draw_example(surface.get(), fPaint, rotation);

    // Must flush *and* present for this to work!
    skgpu::ganesh::Flush(surface);
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
    GrContextHolder fGrContext;
}

- (void)loadView {
    [self setView:[[MTKView alloc] initWithFrame:[[UIScreen mainScreen] bounds] device:nil]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    if (!fGrContext) {
        [self setMetalDevice:MTLCreateSystemDefaultDevice()];
        [self setMetalQueue:[[self metalDevice] newCommandQueue]];
        fGrContext = SkMetalDeviceToGrContext([self metalDevice], [self metalQueue]);
    }
    if (![self view] || ![self metalDevice]) {
        NSLog(@"Metal is not supported on this device");
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
        return;
    }
    MTKView* mtkView = (MTKView*)[self view];
    [mtkView setDevice:[self metalDevice]];
    [mtkView setBackgroundColor:[UIColor blackColor]];
    SkMtkViewConfigForSkia(mtkView);
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

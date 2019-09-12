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

static sk_sp<SkSurface> to_surface(MTKView* mtkView, GrContext* grContext) {
    if (![[mtkView currentDrawable] texture] ||
        !grContext ||
        MTLPixelFormatDepth32Float_Stencil8 != [mtkView depthStencilPixelFormat] ||
        MTLPixelFormatBGRA8Unorm != [mtkView colorPixelFormat]) {
        return nullptr;
    }
    const SkColorType colorType = kBGRA_8888_SkColorType;  // MTLPixelFormatBGRA8Unorm
    sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatBGRA8Unorm
    const GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    int sampleCount = (int)[mtkView sampleCount];
    CGSize size = [mtkView drawableSize];
    int width  = (int)size.width;
    int height = (int)size.height;

    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)([[mtkView currentDrawable] texture]));
    if (sampleCount == 1) {
        GrBackendRenderTarget backendRT(width, height, 1, fbInfo);
        return SkSurface::MakeFromBackendRenderTarget(grContext, backendRT, origin,
                                                      colorType, colorSpace, &surfaceProps);
    } else {
        GrBackendTexture backendTexture(width, height, GrMipMapped::kNo, fbInfo);
        return SkSurface::MakeFromBackendTexture(grContext, backendTexture, origin, sampleCount,
                                                 colorType, colorSpace, &surfaceProps);
    }
}

static sk_sp<GrContext> to_context(id<MTLDevice> device, const GrContextOptions& opts) {
    return GrContext::MakeMetal((void*)device,
                                (void*)[device newCommandQueue], opts);
}

static void configure_mtk_view(MTKView* mtkView) {
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth32Float_Stencil8];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setSampleCount:1];
}

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
@property (assign) GrContext* grContext;  // non-owning pointer.
@end

@implementation AppViewDelegate {
    SkPaint fPaint;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if (![self grContext] || !view) {
        return;
    }
    // Do as much as possible before calling to_surface()
    config_paint(&fPaint);
    float rotation = (float)(180 * 1e-9 * SkTime::GetNSecs());

    // Create surface:
    sk_sp<SkSurface> surface = to_surface(view, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }

    draw_example(surface.get(), fPaint, rotation);

    // Must flush *and* present for this to work!
    surface->flush();
    surface = nullptr;
    [[view currentDrawable] present];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    // change anything on size change?
}
@end

////////////////////////////////////////////////////////////////////////////////

@interface AppViewController : UIViewController
@property (strong) id<MTLDevice> metalDevice;
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
        GrContextOptions grContextOptions;  // set different options here.
        fGrContext = to_context([self metalDevice], grContextOptions);
    }
    if (![self view] || ![self metalDevice]) {
        NSLog(@"Metal is not supported on this device");
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
        return;
    }
    MTKView* mtkView = (MTKView*)[self view];
    [mtkView setDevice:[self metalDevice]];
    [mtkView setBackgroundColor:[UIColor blackColor]];
    configure_mtk_view(mtkView);
    AppViewDelegate* viewDelegate = [[AppViewDelegate alloc] init];
    [viewDelegate setGrContext:fGrContext.get()];
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

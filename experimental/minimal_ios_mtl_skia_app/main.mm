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

#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/utils/SkottieUtils.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkSurface> to_surface(MTKView* mtkView, GrContext* grContext) {
    if (!grContext || mtkView == nil) {
        return nullptr;
    }
    id<CAMetalDrawable> drawable = mtkView.currentDrawable;
    CGSize size = mtkView.drawableSize;
    int sampleCount = (int)mtkView.sampleCount;
    int width = (int)size.width;
    int height = (int)size.height;
    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(drawable.texture));
    sk_sp<SkColorSpace> colorSpace = nullptr;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    if (sampleCount == 1) {
        GrBackendRenderTarget backendRT(width, height, 1, fbInfo);
        return SkSurface::MakeFromBackendRenderTarget(grContext, backendRT,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      kBGRA_8888_SkColorType,
                                                      colorSpace, &surfaceProps);
    } else {
        GrBackendTexture backendTexture(width, height, GrMipMapped::kNo, fbInfo);
        return SkSurface::MakeFromBackendTexture(
                grContext, backendTexture, kTopLeft_GrSurfaceOrigin, sampleCount,
                kBGRA_8888_SkColorType, colorSpace, &surfaceProps);
    }
}

static sk_sp<GrContext> to_context(id<MTLDevice> device, const GrContextOptions& opts) {
    if (device == nil) {
        return nullptr;
    }
    id<MTLCommandQueue> queue = [device newCommandQueue];
    return GrContext::MakeMetal((__bridge void*)device, (__bridge void*)queue, opts);
}

static void configure_mtk_view(MTKView* mtkView) {
    mtkView.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    mtkView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    mtkView.sampleCount = 1;
}

////////////////////////////////////////////////////////////////////////////////

@interface AppViewDelegate : NSObject <MTKViewDelegate> {
@public
    sk_sp<skottie::Animation> fAnimation;
    GrContext* fGrContext;
    double fStartTime;
    float fScale;
    SkPoint fOffset;
}
@end

@implementation AppViewDelegate
- (void)drawInMTKView:(nonnull MTKView *)view {
    if (!fGrContext) {
        NSLog(@"error: no context");
        return;
    }
    if (0 == fScale) {
        [self mtkView:view drawableSizeWillChange:view.drawableSize];
    }
    fAnimation->seekFrameTime(std::fmod(1e-9 * (SkTime::GetNSecs() - fStartTime), 
                                        fAnimation->duration()), nullptr);
    sk_sp<SkSurface> surface = to_surface(view, fGrContext);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    {
        SkCanvas* canvas = surface->getCanvas();
        canvas->translate(fOffset.x(), fOffset.y());
        canvas->scale(fScale, fScale);
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawRect(SkRect::MakeSize(fAnimation->size()), SkPaint(SkColors::kWhite));
        fAnimation->render(canvas);
    }
    surface->flush();
    [view.currentDrawable present];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    if (fAnimation) {
        const SkSize& animSize = fAnimation->size();
        fScale = std::min(size.width  / animSize.width(),
                          size.height / animSize.height());
        fOffset = {((float)size.width  - animSize.width()  * fScale) * 0.5f,
                   ((float)size.height - animSize.height() * fScale) * 0.5f};
    }
}
@end

////////////////////////////////////////////////////////////////////////////////

@interface AppViewController : UIViewController {
    id<MTLDevice> fMtlDevice;
    sk_sp<GrContext> fGrContext;
}

@end
@implementation AppViewController
- (void)loadView {
    CGRect rect = [[UIScreen mainScreen] bounds];
    rect.size.height = rect.size.height * 3 / 4;
    self.view = [[MTKView alloc] initWithFrame:rect device:nil];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    if (!fGrContext) {
        fMtlDevice = MTLCreateSystemDefaultDevice();
        if(!fMtlDevice) {
            NSLog(@"Metal is not supported on this device");
            return;
        }
        GrContextOptions grContextOptions;  // set different options here.
        fGrContext = to_context(fMtlDevice, grContextOptions);
    }
    MTKView* mtkView = (MTKView*)self.view;
    mtkView.device = fMtlDevice;
    mtkView.backgroundColor = UIColor.blackColor;
    configure_mtk_view(mtkView);
    AppViewDelegate* viewDelegate = [[AppViewDelegate alloc] init];
    {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* lottie = [mainBundle pathForResource:@"lottie" ofType:@"json"];
        NSData *content = [NSData dataWithContentsOfFile:lottie];
        skottie::Animation::Builder builder;
        viewDelegate->fAnimation = builder.make((const char*)content.bytes, content.length);
        if (!viewDelegate->fAnimation) {
            return;
        }
        viewDelegate->fStartTime = SkTime::GetNSecs();
        viewDelegate->fScale = 0;
    }
    viewDelegate->fGrContext = fGrContext.get();
    [viewDelegate mtkView:mtkView drawableSizeWillChange:mtkView.bounds.size];
    mtkView.delegate = viewDelegate;
}
@end

////////////////////////////////////////////////////////////////////////////////

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.frame = [UIScreen mainScreen].bounds;
    self.window.rootViewController = [[AppViewController alloc] init];
    [self.window makeKeyAndVisible];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive
    // state. This can occur for certain types of temporary interruptions (such
    // as an incoming phone call or SMS message) or when the user quits the
    // application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate
    // graphics rendering callbacks. Games should use this method to pause the
    // game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate
    // timers, and store enough application state information to restore your
    // application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called
    // instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active
    // state; here you can undo many of the changes made on entering the
    // background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the
    // application was inactive. If the application was previously in the
    // background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if
    // appropriate. See also applicationDidEnterBackground:.
}
@end

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

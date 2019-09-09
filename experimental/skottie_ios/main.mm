// Copyright 2019 Google LLC.
// Use of this source cofcee is governed by a BSD-style license that can be found in the LICENSE file.

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

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkSurface> to_surface(MTKView* mtkView, GrContext* grContext) {
    if (!mtkView.currentDrawable.texture || !grContext || !mtkView) {
        return nullptr;
    }
    SkASSERT(MTLPixelFormatDepth32Float_Stencil8 == mtkView.depthStencilPixelFormat);
    SkASSERT(MTLPixelFormatBGRA8Unorm == mtkView.colorPixelFormat);
    const SkColorType colorType = kBGRA_8888_SkColorType;  // MTLPixelFormatBGRA8Unorm
    sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatBGRA8Unorm
    const GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    int sampleCount = (int)mtkView.sampleCount;
    int width  = (int)mtkView.drawableSize.width;
    int height = (int)mtkView.drawableSize.height;

    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(mtkView.currentDrawable.texture));
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
    SkASSERT(device);
    id<MTLCommandQueue> queue = [device newCommandQueue];
    return GrContext::MakeMetal((__bridge void*)device, (__bridge void*)queue, opts);
}

static void configure_mtk_view(MTKView* mtkView) {
    mtkView.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    mtkView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    mtkView.sampleCount = 1;
}

////////////////////////////////////////////////////////////////////////////////

@interface SkottieViewDelegate : NSObject <MTKViewDelegate> {
@public
    sk_sp<skottie::Animation> fAnimation;
    GrContext* fGrContext;
    double fStartTime;
    double fTime;
    float fScale;
    SkPoint fOffset;
    bool fPaused;
}
@end

@implementation SkottieViewDelegate
- (void)drawInMTKView:(nonnull MTKView*)view {
    if (!view.currentDrawable || !view.currentDrawable.texture) {
        return;
    }
    if (!fGrContext) {
        NSLog(@"error: no context");
        return;
    }
    if (0 == fScale) {
        [self mtkView:view drawableSizeWillChange:view.drawableSize];
    }
    if (!fPaused) {
        fTime = SkTime::GetNSecs();
        fAnimation->seekFrameTime(std::fmod(1e-9 * (fTime - fStartTime),
                                            fAnimation->duration()), nullptr);
    }
    sk_sp<SkSurface> surface = to_surface(view, fGrContext);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(fOffset.x(), fOffset.y());
    canvas->scale(fScale, fScale);
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeSize(fAnimation->size()), SkPaint(SkColors::kWhite));
    fAnimation->render(canvas);
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

static SkottieViewDelegate* create_skottie_view_delegate(NSString* path) {
    NSData *content = [NSData dataWithContentsOfFile:path];
    if (!content) {
        NSLog(@"'%@' not found", path);
        return nil;
    }
    skottie::Animation::Builder builder;
    SkottieViewDelegate* viewDelegate = [[SkottieViewDelegate alloc] init];
    viewDelegate->fAnimation = builder.make((const char*)content.bytes, content.length);
    if (!viewDelegate->fAnimation) {
        return nil;
    }
    viewDelegate->fStartTime = SkTime::GetNSecs();
    viewDelegate->fTime = viewDelegate->fStartTime;
    viewDelegate->fPaused = false;
    viewDelegate->fScale = 0;
    return viewDelegate;
}


@interface AppViewController : UIViewController
@end

@implementation AppViewController {
    id<MTLDevice> fMtlDevice;
    sk_sp<GrContext> fGrContext;
    std::vector<std::pair<MTKView*, SkottieViewDelegate*>> fDelegates;
}

- (void)loadView {
    self.view = [[UIView alloc] init];
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

    CGRect statusBarFrame = [[UIApplication sharedApplication] statusBarFrame];
    CGRect mainScreenBounds = [[UIScreen mainScreen] bounds];
    CGRect scrollViewBounds = {{0, statusBarFrame.size.height},
                               {mainScreenBounds.size.width,
                                mainScreenBounds.size.height - statusBarFrame.size.height}};

    UIStackView* stack = [[UIStackView alloc] initWithFrame:scrollViewBounds];
    stack.axis = UILayoutConstraintAxisVertical;
    stack.distribution = UIStackViewDistributionEqualSpacing;

    float screenWidth = [[UIScreen mainScreen] bounds].size.width;
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSArray<NSString*>* paths = [mainBundle pathsForResourcesOfType:@"json"
                                            inDirectory:@"skottie"];
    float totalHeight = 2;
    for (NSUInteger i = 0; i < paths.count; ++i) {
        SkottieViewDelegate* viewDelegate = create_skottie_view_delegate(paths[i]);
        if (!viewDelegate) {
            continue;
        }
        float height = screenWidth * viewDelegate->fAnimation->size().height()
                                   / viewDelegate->fAnimation->size().width();
        CGSize size = {screenWidth, height};
        viewDelegate->fGrContext = fGrContext.get();
        MTKView* mtkView = [[MTKView alloc] initWithFrame:{{0, 0}, size} device:fMtlDevice];
        configure_mtk_view(mtkView);
        mtkView.delegate = viewDelegate;
        mtkView.preferredFramesPerSecond = 30;
        [mtkView.heightAnchor constraintEqualToConstant:height].active = true;
        [mtkView.widthAnchor constraintEqualToConstant:screenWidth].active = true;
        [stack addArrangedSubview:mtkView];
        totalHeight += height + 2;
        fDelegates.push_back({mtkView, viewDelegate});
    }
    UIView* bar = [[UIView alloc] initWithFrame:statusBarFrame];
    bar.backgroundColor = [UIColor whiteColor];

    UIScrollView* scrollView = [[UIScrollView alloc] initWithFrame:scrollViewBounds];

    [stack setFrame:{{0, 0}, {screenWidth, totalHeight}}];
    [scrollView setContentSize:{screenWidth, totalHeight}];
    [scrollView addSubview:stack];

    [self.view setBounds:mainScreenBounds];
    [self.view addSubview:bar];
    [self.view addSubview:scrollView];

    UITapGestureRecognizer* tapGestureRecognizer = [[UITapGestureRecognizer alloc] init];
    [tapGestureRecognizer addTarget:self action:@selector(handleTap:)];
    [self.view addGestureRecognizer:tapGestureRecognizer];
}

- (void)handleTap:(UIGestureRecognizer*)sender {
    if (sender.state == UIGestureRecognizerStateEnded) {
        for (auto pair : self->fDelegates) {
            SkottieViewDelegate* delegate = pair.second;
            MTKView* mtkView = pair.first;        
            delegate->fPaused = !delegate->fPaused;
            if (!delegate->fPaused) {
                delegate->fStartTime += (SkTime::GetNSecs() - delegate->fTime);
            }
            [mtkView setEnableSetNeedsDisplay:delegate->fPaused];
            [mtkView setPaused:delegate->fPaused];
        }
    }
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

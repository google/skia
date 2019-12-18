// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"

#include "tools/skottie_ios_app/SkottieViewController.h"
#include "tools/skottie_ios_app/GrContextHolder.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

#if SK_SUPPORT_GPU && defined(SK_METAL)
    #include "tools/skottie_ios_app/SkMetalViewBridge.h"
    #include "tools/skottie_ios_app/SkiaMtkView.h"
    using SkiaView = SkiaMtkView;
#elif SK_SUPPORT_GPU && defined(SK_GL)
    #include "tools/skottie_ios_app/SkiaGLView.h"
    using SkiaView = SkiaGLView;
#else
    #include "tools/skottie_ios_app/SkiaUIView.h"
    using SkiaView = SkiaUIView;
#endif

#define LOG_ABORT(X) NSLog(@ X); SK_ABORT(X);

// SkiaUIViewFactory = UIView*(SkottieViewController*, CGRect);
template <typename SkiaUIViewFactory>
static UIStackView* make_skottie_stack(CGFloat width,
                                       SkiaUIViewFactory skiaViewFactory) {
    UIStackView* stack = [[UIStackView alloc] init];
    [stack setAxis:UILayoutConstraintAxisVertical];
    [stack setDistribution:UIStackViewDistributionEqualSpacing];

    NSBundle* mainBundle = [NSBundle mainBundle];
    NSArray<NSString*>* paths = [mainBundle pathsForResourcesOfType:@"json"
                                            inDirectory:@"data"];
    constexpr CGFloat kSpacing = 2;
    CGFloat totalHeight = kSpacing;
    for (NSUInteger i = 0; i < [paths count]; ++i) {
        NSString* path = [paths objectAtIndex:i];
        NSData* content = [NSData dataWithContentsOfFile:path];
        if (!content) {
            NSLog(@"'%@' not found", path);
            continue;
        }
        SkottieViewController* controller = [[SkottieViewController alloc] init];
        if (![controller loadAnimation:content]) {
            continue;
        }
        CGSize animSize = [controller size];
        CGFloat height = animSize.width ? (width * animSize.height / animSize.width) : 0;
        CGRect frame = {{0, 0}, {width, height}};
        UIView* skiaView = skiaViewFactory(controller, frame);
        [skiaView setFrame:frame];
        [[[skiaView heightAnchor] constraintEqualToConstant:height] setActive:true];
        [[[skiaView widthAnchor] constraintEqualToConstant:width] setActive:true];
        [skiaView setNeedsDisplay];
        [stack addArrangedSubview:skiaView];
        totalHeight += height + kSpacing;
    }
    [stack setFrame:{{0, 0}, {width, totalHeight}}];
    [stack setNeedsDisplay];
    return stack;
}

@interface AppViewController : UIViewController
    @property (strong) id<MTLDevice> metalDevice;
    @property (strong) id<MTLCommandQueue> metalQueue;
    @property (strong) EAGLContext* eaglContext;
    @property (strong) UIStackView* stackView;
@end

@implementation AppViewController {
    GrContextHolder fGrContext;
}

- (void)loadView {
    [self setView:[[UIView alloc] init]];
}

- (void)configureContexts {
    #if SK_SUPPORT_GPU && defined(SK_METAL)
    if (!fGrContext) {
        [self setMetalDevice:MTLCreateSystemDefaultDevice()];
        if(![self metalDevice]) {
            LOG_ABORT("Metal is not supported on this device");
        }
        [self setMetalQueue:[[self metalDevice] newCommandQueue]];
        fGrContext = SkMetalDeviceToGrContext([self metalDevice], [self metalQueue]);
        if (!fGrContext) {
            LOG_ABORT("GrContext::MakeMetal failed");
        }
    }
    #elif SK_SUPPORT_GPU && defined(SK_GL)
    if (!fGrContext) {
        [self setEaglContext:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]];
        if (![self eaglContext]) {
            NSLog(@"Falling back to GLES2.\n");
            [self setEaglContext:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]];
        }
        if (![self eaglContext]) {
            LOG_ABORT("[[EAGLContext alloc] initWithAPI:...] failed");
        }
        EAGLContext* oldContext = [EAGLContext currentContext];
        [EAGLContext setCurrentContext:[self eaglContext]];
        fGrContext = SkMakeGLContext();
        [EAGLContext setCurrentContext:oldContext];
        if (!fGrContext) {
            LOG_ABORT("GrContext::MakeGL failed");
        }
    }
    #endif
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self configureContexts];
    #if SK_SUPPORT_GPU && defined(SK_METAL)
    auto skiaViewFactory = [device=[self metalDevice],
                            queue=[self metalQueue],
                            grContext=fGrContext.get()](SkiaViewController* vc,
                                                        CGRect frame) -> UIView* {
        SkiaMtkView* skiaView = [[SkiaMtkView alloc] initWithFrame:frame];
        [skiaView setDevice:device];
        [skiaView setQueue:queue];
        [skiaView setGrContext:grContext];
        [skiaView setPreferredFramesPerSecond:30];
        [skiaView setController:vc];
        return skiaView;
    };
    #elif SK_SUPPORT_GPU && defined(SK_GL)
    auto skiaViewFactory = [grContext=fGrContext.get(),
                            ctx=[self eaglContext]](SkiaViewController* vc,
                                                    CGRect frame) -> UIView* {
        SkASSERT(grContext);
        SkASSERT(ctx);
        SkiaGLView* skiaView = [[SkiaGLView alloc] initWithFrame:frame context:ctx];
        [skiaView setGrContext:grContext];
        [skiaView setController:vc];
        return skiaView;
    };
    #else
    auto skiaViewFactory = [](SkiaViewController* controller, CGRect frame) -> UIView* {
        SkiaUIView* skiaView = [[SkiaUIView alloc] initWithFrame:frame];
        [skiaView setController:controller];
        return skiaView;
    };
    #endif

    CGFloat screenWidth = [[UIScreen mainScreen] bounds].size.width;
    [self setStackView:make_skottie_stack(screenWidth, skiaViewFactory)];

    CGFloat statusBarHeight = [[UIApplication sharedApplication] statusBarFrame].size.height;
    CGSize mainScreenSize = [[UIScreen mainScreen] bounds].size;
    CGRect scrollViewBounds = {{0, statusBarHeight},
                               {mainScreenSize.width, mainScreenSize.height - statusBarHeight}};
    UIScrollView* scrollView = [[UIScrollView alloc] initWithFrame:scrollViewBounds];
    [scrollView setContentSize:[[self stackView] frame].size];
    [scrollView addSubview:[self stackView]];
    [scrollView setBackgroundColor:[UIColor blackColor]];
    [scrollView setNeedsDisplay];

    UIView* mainView = [self view];
    [mainView setBounds:{{0, 0}, mainScreenSize}];
    [mainView setBackgroundColor:[UIColor whiteColor]];
    [mainView addSubview:scrollView];
    [mainView setNeedsDisplay];

    UITapGestureRecognizer* tapGestureRecognizer = [[UITapGestureRecognizer alloc] init];
    [tapGestureRecognizer addTarget:self action:@selector(handleTap:)];
    [mainView addGestureRecognizer:tapGestureRecognizer];
}

- (void)handleTap:(UIGestureRecognizer*)sender {
    if ([sender state] != UIGestureRecognizerStateEnded) {
        return;
    }
    NSArray<UIView*>* subviews = [[self stackView] subviews];
    for (NSUInteger i = 0; i < [subviews count]; ++i) {
        UIView* uIView = [subviews objectAtIndex:i];
        if ([uIView isKindOfClass:[SkiaView class]]) {
            if (SkiaViewController* controller = [(SkiaView*)uIView controller]) {
                [controller togglePaused];
                [uIView setNeedsDisplay];
            }
        }
    }
}
@end

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)ops {
    [self setWindow:[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]];
    [[self window] setRootViewController:[[AppViewController alloc] init]];
    [[self window] makeKeyAndVisible];
    return YES;
}
@end

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

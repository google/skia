// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"

#include "tools/skottie_ios_app/GrContextHolder.h"
#include "tools/skottie_ios_app/SkottieViewController.h"

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

////////////////////////////////////////////////////////////////////////////////

@interface SkiaContext : NSObject
    @property (strong) id<MTLDevice> metalDevice;
    @property (strong) id<MTLCommandQueue> metalQueue;
    @property (strong) EAGLContext* eaglContext;
    - (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame;
    - (id)init;
@end

@implementation SkiaContext {
    GrContextHolder fGrContext;
}

- (id)init {
    self = [super init];
    #define LOG_ABORT(X) NSLog(@ X); SK_ABORT(X);
    #if SK_SUPPORT_GPU && defined(SK_METAL)
    [self setMetalDevice:MTLCreateSystemDefaultDevice()];
    if(![self metalDevice]) {
        LOG_ABORT("Metal is not supported on this device");
    }
    [self setMetalQueue:[[self metalDevice] newCommandQueue]];
    fGrContext = SkMetalDeviceToGrContext([self metalDevice], [self metalQueue]);
    if (!fGrContext) {
        LOG_ABORT("GrContext::MakeMetal failed");
    }
    #elif SK_SUPPORT_GPU && defined(SK_GL)
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
    #endif
    #undef LOG_ABORT
    return self;
}

- (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame {
    #if SK_SUPPORT_GPU && defined(SK_METAL)
    SkiaMtkView* skiaView = [[SkiaMtkView alloc] initWithFrame:frame];
    [skiaView setDevice:[self metalDevice]];
    [skiaView setQueue:[self metalQueue]];
    [skiaView setGrContext:fGrContext.get()];
    [skiaView setPreferredFramesPerSecond:30];
    [skiaView setController:vc];
    return skiaView;
    #elif SK_SUPPORT_GPU && defined(SK_GL)
    SkiaGLView* skiaView = [[SkiaGLView alloc] initWithFrame:frame context:[self eaglContext]];
    [skiaView setGrContext:fGrContext.get()];
    [skiaView setController:vc];
    return skiaView;
    #else
    SkiaUIView* skiaView = [[SkiaUIView alloc] initWithFrame:frame];
    [skiaView setController:controller];
    return skiaView;
    #endif
}
@end

////////////////////////////////////////////////////////////////////////////////

@interface AppViewController : UIViewController
    @property (strong) SkiaContext* skiaContext;
@end

@implementation AppViewController

- (void)loadView {
    [self setView:[[UIView alloc] init]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    if (![self skiaContext]) {
        [self setSkiaContext:[[SkiaContext alloc] init]];
    }
    CGFloat screenWidth = [[UIScreen mainScreen] bounds].size.width;

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
        CGFloat height = animSize.width ? (screenWidth * animSize.height / animSize.width) : 0;
        CGRect frame = {{0, 0}, {screenWidth, height}};
        UIView* skiaView = [[self skiaContext] makeViewWithController:controller withFrame:frame];
        [[[skiaView heightAnchor] constraintEqualToConstant:height] setActive:true];
        [[[skiaView widthAnchor] constraintEqualToConstant:screenWidth] setActive:true];
        [skiaView setNeedsDisplay];
        [stack addArrangedSubview:skiaView];
        totalHeight += height + kSpacing;
    }
    [stack setFrame:{{0, 0}, {screenWidth, totalHeight}}];
    [stack setNeedsDisplay];

    [self setStackView:stack];

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

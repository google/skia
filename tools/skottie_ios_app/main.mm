// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"

#ifdef SK_METAL
    #include "tools/skottie_ios_app/SkMetalViewBridge.h"
    #include "tools/skottie_ios_app/SkottieMtkView.h"
    #include "tools/skottie_ios_app/GrContextHolder.h"

    #import <Metal/Metal.h>
    #import <MetalKit/MetalKit.h>
#elif SK_GL
    #include "tools/skottie_ios_app/GrContextHolder.h"
    #include "tools/skottie_ios_app/SkMakeGLContext.h"
    #include "tools/skottie_ios_app/SkottieGLView.h"
#else
    #include "tools/skottie_ios_app/SkottieUIView.h"
#endif

#import <UIKit/UIKit.h>

template<typename T> static inline T* objc_cast(id v) {
    return [v isKindOfClass:[T class]] ? static_cast<T*>(v) : nil;
}

struct SkottieViewFactory {
    #ifdef SK_METAL
    id<MTLDevice> metalDevice;
    id<MTLCommandQueue> metalQueue;
    GrContext* grContext;
    UIView* operator()(SkottieViewController* controller) const {
        SkottieMtkView* skottieView = [[SkottieMtkView alloc] init];
        [skottieView setDevice:metalDevice];
        [skottieView setQueue:metalQueue];
        [skottieView setGrContext:grContext];
        SkMtkViewConfigForSkia(skottieView);
        [skottieView setPreferredFramesPerSecond:30];
        [skottieView setController:controller];
        return skottieView;
    }
    #elif SK_GL
    const GrGLInterface* grGLInterface;
    GrContext* grContext;
    UIView* operator()(SkottieViewController* controller) const {
        SkottieGLView* skottieView = [[SkottieGLView alloc] init];
        [skottieView setGrGLInterface:grGLInterface];
        [skottieView setGrContext:grContext];
        [skottieView setController:controller];
        return skottieView;
    }
#else
    UIView* operator()(SkottieViewController* controller) const {
        SkottieUIView* skottieView = [[SkottieUIView alloc] init];
        [skottieView setController:controller];
        return skottieView;
    }
    #endif
};


static UIStackView* make_skottie_stack(CGFloat width,
                                       const SkottieViewFactory& skottieViewFactory) {
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
        UIView* skottieView = skottieViewFactory(controller);
        CGSize animSize = [controller size];
        CGFloat height = animSize.width ? (width * animSize.height / animSize.width) : 0;
        [skottieView setFrame:{{0, 0}, {width, height}}];
        [[[skottieView heightAnchor] constraintEqualToConstant:height] setActive:true];
        [[[skottieView widthAnchor] constraintEqualToConstant:width] setActive:true];
        [skottieView setNeedsDisplay];
        [stack addArrangedSubview:skottieView];
        totalHeight += height + kSpacing;
    }
    [stack setFrame:{{0, 0}, {width, totalHeight}}];
    return stack;
}

@interface AppViewController : UIViewController
    #ifdef SK_METAL
    @property (strong) id<MTLDevice> metalDevice;
    @property (strong) id<MTLCommandQueue> metalQueue;
    #endif
    @property (strong) UIStackView* stackView;
@end

@implementation AppViewController {
    #ifdef SK_METAL
    GrContextHolder fGrContext;
    #elif SK_GL
    GrGLInterfaceHolder grGLInterfaceHolder;
    GrContextHolder grContextHolder;
    #endif
}

- (void)loadView {
    [self setView:[[UIView alloc] init]];
}

- (void)viewDidLoad {
    #ifdef SK_METAL
    [super viewDidLoad];
    if (!fGrContext) {
        [self setMetalDevice:MTLCreateSystemDefaultDevice()];
        if(![self metalDevice]) {
            NSLog(@"Metal is not supported on this device");
            return;
        }
        [self setMetalQueue:[[self metalDevice] newCommandQueue]];
        fGrContext = SkMetalDeviceToGrContext([self metalDevice], [self metalQueue]);
    }

    SkottieViewFactory viewFactory{[self metalDevice], [self metalQueue], fGrContext.get()};
    #elif SK_GL
    GrGLInterfaceHolder grGLInterfaceHolder = SkMakeGLInterface();
    GrContextHolder grContextHolder = SkMakeGLContext(grGLInterfaceHolder.get());
    SkottieViewFactory viewFactory{grGLInterfaceHolder.get(), grContextHolder.get()};
    #else
    SkottieViewFactory viewFactory;
    #endif
    [self setStackView:make_skottie_stack([[UIScreen mainScreen] bounds].size.width, viewFactory)];

    CGFloat statusBarHeight = [[UIApplication sharedApplication] statusBarFrame].size.height;
    CGSize mainScreenSize = [[UIScreen mainScreen] bounds].size;
    CGRect scrollViewBounds = {{0, statusBarHeight},
                               {mainScreenSize.width, mainScreenSize.height - statusBarHeight}};
    UIScrollView* scrollView = [[UIScrollView alloc] initWithFrame:scrollViewBounds];
    [scrollView setContentSize:[[self stackView] frame].size];
    [scrollView addSubview:[self stackView]];
    [scrollView setBackgroundColor:[UIColor blackColor]];

    UIView* mainView = [self view];
    [mainView setBounds:{{0, 0}, mainScreenSize}];
    [mainView setBackgroundColor:[UIColor whiteColor]];
    [mainView addSubview:scrollView];

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
        UIView* subview = [subviews objectAtIndex:i];
        #ifdef SK_METAL
        if (SkottieMtkView* skottieView = objc_cast<SkottieMtkView>(subview)) {
            BOOL paused = [[skottieView controller] togglePaused];
            [skottieView setEnableSetNeedsDisplay:paused];
            [skottieView setPaused:paused];
        }
        #elif SK_GL
        if (SkottieGLView* skottieView = objc_cast<SkottieGLView>(subview)) {
            [[skottieView controller] togglePaused];
        }
        #else
        if (SkottieMtkView* skottieView = objc_cast<SkottieUIView>(subview)) {
            [[skottieView controller] togglePaused];
        }
        #endif
        [subview setNeedsDisplay];
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

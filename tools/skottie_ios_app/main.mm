// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"

#ifdef SK_METAL
    #include "tools/skottie_ios_app/SkMetalViewBridge.h"
    #include "tools/skottie_ios_app/SkottieMtkView.h"

    #import <Metal/Metal.h>
    #import <MetalKit/MetalKit.h>
#else
    #include "tools/skottie_ios_app/SkottieUIView.h"
#endif

#import <UIKit/UIKit.h>


#ifdef SK_METAL
static UIStackView* make_skottie_stack(CGFloat width,
                                       id<MTLDevice> metalDevice,
                                       id<MTLCommandQueue> metalQueue,
                                       GrContext* grContext) {
#else
static UIStackView* make_skottie_stack(CGFloat width) {
#endif
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
        #ifdef SK_METAL
        SkottieMtkView* skottieView = [[SkottieMtkView alloc] init];
        #else
        SkottieUIView* skottieView = [[SkottieUIView alloc] init];
        #endif

        if (![skottieView loadAnimation:content]) {
            continue;
        }
        #ifdef SK_METAL
        [skottieView setDevice:metalDevice];
        [skottieView setQueue:metalQueue];
        [skottieView setGrContext:grContext];
        SkMtkViewConfigForSkia(skottieView);
        [skottieView setPreferredFramesPerSecond:30];
        #endif
        CGSize animSize = [skottieView size];
        CGFloat height = animSize.width ? (width * animSize.height / animSize.width) : 0;
        [skottieView setFrame:{{0, 0}, {width, height}}];
        [[[skottieView heightAnchor] constraintEqualToConstant:height] setActive:true];
        [[[skottieView widthAnchor] constraintEqualToConstant:width] setActive:true];
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
    [self setStackView:make_skottie_stack([[UIScreen mainScreen] bounds].size.width,
                                          [self metalDevice], [self metalQueue], fGrContext.get())];
    #else
    [self setStackView:make_skottie_stack([[UIScreen mainScreen] bounds].size.width)];
    #endif

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
        if (![subview isKindOfClass:[SkottieMtkView class]]) {
            continue;
        }
        SkottieMtkView* skottieView = (SkottieMtkView*)subview;
        #else
        if (![subview isKindOfClass:[SkottieUIView class]]) {
            continue;
        }
        SkottieUIView* skottieView = (SkottieUIView*)subview;
        #endif
        BOOL paused = [skottieView togglePaused];
        #ifdef SK_METAL
        [skottieView setEnableSetNeedsDisplay:paused];
        [skottieView setPaused:paused];
        #endif
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

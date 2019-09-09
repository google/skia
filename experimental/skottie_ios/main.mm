// Copyright 2019 Google LLC.
// Use of this source cofcee is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/skottie_ios/SkottieViewDelegate.h"
#include "experimental/skottie_ios/SkMetalViewBridge.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

@interface AppViewController : UIViewController
    @property (strong) id<MTLDevice> metalDevice;
    @property (strong) UIStackView* stackView;
@end

@implementation AppViewController {
    sk_sp<GrContext> fGrContext;
}

- (void)dealloc {
    fGrContext = nullptr;
    [super dealloc];
}

- (void)loadView {
    [self setView:[[UIView alloc] init]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    UIView* mainView = [self view];
    if (!fGrContext) {
        [self setMetalDevice:MTLCreateSystemDefaultDevice()];
        if(![self metalDevice]) {
            NSLog(@"Metal is not supported on this device");
            return;
        }
        GrContextOptions grContextOptions;  // set different options here.
        fGrContext = SkMetalDeviceToGrContext([self metalDevice], grContextOptions);
    }

    CGFloat statusBarHeight = [[UIApplication sharedApplication] statusBarFrame].size.height;
    CGSize mainScreenSize = [[UIScreen mainScreen] bounds].size;
    CGFloat screenWidth = mainScreenSize.width;

    UIStackView* stack = [[UIStackView alloc] init];
    [stack setAxis:UILayoutConstraintAxisVertical];
    [stack setDistribution:UIStackViewDistributionEqualSpacing];

    NSBundle* mainBundle = [NSBundle mainBundle];
    NSArray<NSString*>* paths = [mainBundle pathsForResourcesOfType:@"json"
                                            inDirectory:@"skottie"];
    constexpr CGFloat kSpacing = 2;
    CGFloat totalHeight = kSpacing;
    for (NSUInteger i = 0; i < [paths count]; ++i) {
        NSString* path = [paths objectAtIndex:i];
        NSData* content = [NSData dataWithContentsOfFile:path];
        if (!content) {
            NSLog(@"'%@' not found", path);
            continue;
        }
        SkottieViewDelegate* viewDelegate = [[SkottieViewDelegate alloc] init];
        if (![viewDelegate loadAnimation:content]) {
            continue;
        }
        [viewDelegate setGrContext:(fGrContext.get())];
        CGSize animSize = viewDelegate.size;
        CGFloat height = screenWidth * animSize.height / animSize.width;
        CGSize size = {screenWidth, height};
        MTKView* mtkView = [[MTKView alloc] initWithFrame:{{0, 0}, size} device:[self metalDevice]];
        SkMtkViewConfigForSkia(mtkView);
        [mtkView setDelegate:viewDelegate];
        [mtkView setPreferredFramesPerSecond:30];
        [[[mtkView heightAnchor] constraintEqualToConstant:height] setActive:true];
        [[[mtkView widthAnchor] constraintEqualToConstant:screenWidth] setActive:true];
        [stack addArrangedSubview:mtkView];
        totalHeight += height + kSpacing;
    }

    CGRect scrollViewBounds = {{0, statusBarHeight},
                               {mainScreenSize.width, mainScreenSize.height - statusBarHeight}};
    UIScrollView* scrollView = [[UIScrollView alloc] initWithFrame:scrollViewBounds];

    [stack setFrame:{{0, 0}, {screenWidth, totalHeight}}];
    [scrollView setContentSize:{screenWidth, totalHeight}];
    [scrollView addSubview:stack];
    [scrollView setBackgroundColor:[UIColor blackColor]];

    [self setStackView:stack];

    [mainView setBounds:{{0, 0}, mainScreenSize}];
    [mainView setBackgroundColor:[UIColor whiteColor]];
    [mainView addSubview:scrollView];

    UITapGestureRecognizer* tapGestureRecognizer = [[UITapGestureRecognizer alloc] init];
    [tapGestureRecognizer addTarget:self action:@selector(handleTap:)];
    [mainView addGestureRecognizer:tapGestureRecognizer];
}

- (void)handleTap:(UIGestureRecognizer*)sender {
    if (![sender state] == UIGestureRecognizerStateEnded) {
        return;
    }
    NSArray<UIView*>* subviews = [[self stackView] subviews];
    for (NSUInteger i = 0; i < [subviews count]; ++i) {
        UIView* subview = [subviews objectAtIndex:i];
        if (![subview isKindOfClass:[MTKView class]]) {
            continue;
        }
        MTKView* mtkView = (MTKView*)subview;
        id<MTKViewDelegate> mtkViewDelegate = [mtkView delegate];
        if (mtkViewDelegate && [mtkViewDelegate isKindOfClass:[SkottieViewDelegate class]]) {
            BOOL paused = [mtkViewDelegate togglePaused];
            [mtkView setEnableSetNeedsDisplay:paused];
            [mtkView setPaused:paused];
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

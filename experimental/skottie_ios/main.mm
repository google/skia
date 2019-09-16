// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/skottie_ios/SkMetalViewBridge.h"
#include "experimental/skottie_ios/SkottieMtkView.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

static UIStackView* make_skottie_stack(CGFloat width,
                                       id<MTLDevice> metalDevice,
                                       GrContext* grContext) {
    UIStackView* stack = [[UIStackView alloc] init];
    [stack setAxis:UILayoutConstraintAxisVertical];
    [stack setDistribution:UIStackViewDistributionEqualSpacing];

    NSBundle* mainBundle = [NSBundle mainBundle];
    NSArray<NSString*>* paths = [mainBundle pathsForResourcesOfType:@"json"
                                            inDirectory:nil];
    constexpr CGFloat kSpacing = 2;
    CGFloat totalHeight = kSpacing;
    for (NSUInteger i = 0; i < [paths count]; ++i) {
        NSString* path = [paths objectAtIndex:i];
        NSData* content = [NSData dataWithContentsOfFile:path];
        if (!content) {
            NSLog(@"'%@' not found", path);
            continue;
        }
        SkottieMtkView* skottieView = [[SkottieMtkView alloc] init];
        if (![skottieView loadAnimation:content]) {
            continue;
        }
        [skottieView setDevice:metalDevice];
        [skottieView setGrContext:grContext];
        SkMtkViewConfigForSkia(skottieView);
        CGSize animSize = [skottieView size];
        CGFloat height = animSize.width ? (width * animSize.height / animSize.width) : 0;
        [skottieView setFrame:{{0, 0}, {width, height}}];
        [skottieView setPreferredFramesPerSecond:30];
        [[[skottieView heightAnchor] constraintEqualToConstant:height] setActive:true];
        [[[skottieView widthAnchor] constraintEqualToConstant:width] setActive:true];
        [stack addArrangedSubview:skottieView];
        totalHeight += height + kSpacing;
    }
    [stack setFrame:{{0, 0}, {width, totalHeight}}];
    return stack;
}

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
    if (!fGrContext) {
        [self setMetalDevice:MTLCreateSystemDefaultDevice()];
        if(![self metalDevice]) {
            NSLog(@"Metal is not supported on this device");
            return;
        }
        GrContextOptions grContextOptions;  // set different options here.
        fGrContext = SkMetalDeviceToGrContext([self metalDevice], grContextOptions);
    }

    [self setStackView:make_skottie_stack([[UIScreen mainScreen] bounds].size.width,
                                          [self metalDevice], fGrContext.get())];

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
    if (![sender state] == UIGestureRecognizerStateEnded) {
        return;
    }
    NSArray<UIView*>* subviews = [[self stackView] subviews];
    for (NSUInteger i = 0; i < [subviews count]; ++i) {
        UIView* subview = [subviews objectAtIndex:i];
        if (![subview isKindOfClass:[SkottieMtkView class]]) {
            continue;
        }
        SkottieMtkView* skottieView = (SkottieMtkView*)subview;
        BOOL paused = [skottieView togglePaused];
        [skottieView setEnableSetNeedsDisplay:paused];
        [skottieView setPaused:paused];
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

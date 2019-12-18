// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaContext.h"
#include "tools/skottie_ios_app/SkottieViewController.h"

#include "include/core/SkTypes.h"

#import <UIKit/UIKit.h>

#include <cstdlib>

@interface AppViewController : UIViewController
    @property (strong) SkiaContext* skiaContext;
    @property (strong) UIStackView* stackView;
@end

@implementation AppViewController

- (void)loadView {
    [self setView:[[UIView alloc] init]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    if (![self skiaContext]) {
        #if (SK_SUPPORT_GPU && defined(SK_METAL) && !defined(SK_BUILD_FOR_GOOGLE3))
        [self setSkiaContext:MakeSkiaMetalContext()];
        #elif (SK_SUPPORT_GPU && defined(SK_GL) && !defined(SK_BUILD_FOR_GOOGLE3))
        [self setSkiaContext:MakeSkiaGLContext()];
        #else
        [self setSkiaContext:MakeSkiaUIContext()];
        #endif
        if (![self skiaContext]) {
            NSLog(@"abort: failed to make skia context.");
            std::abort();
        }
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

    CGFloat statusBarHeight = [[UIApplication sharedApplication] statusBarFrame].size.height;
    CGSize mainScreenSize = [[UIScreen mainScreen] bounds].size;
    CGRect scrollViewBounds = {{0, statusBarHeight},
                               {mainScreenSize.width, mainScreenSize.height - statusBarHeight}};
    UIScrollView* scrollView = [[UIScrollView alloc] initWithFrame:scrollViewBounds];
    [scrollView setContentSize:[stack frame].size];
    [scrollView addSubview:stack];
    [scrollView setBackgroundColor:[UIColor blackColor]];
    [scrollView setNeedsDisplay];

    [self setStackView:stack];

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
        if (SkiaViewController* controller = [[self skiaContext] getViewController:uIView]) {
            [controller togglePaused];
            [uIView setNeedsDisplay];
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

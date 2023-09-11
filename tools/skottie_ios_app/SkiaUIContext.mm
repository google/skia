// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaContext.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSize.h"
#include "include/utils/mac/SkCGUtils.h"
#include "src/base/SkTime.h"

#import <UIKit/UIKit.h>

// A UIView that uses a CPU-backed SkSurface to draw.
@interface SkiaUIView : UIView
    @property (strong) SkiaViewController* controller;

    // Override of the UIView interface.
    - (void)drawRect:(CGRect)rect;
@end

@implementation SkiaUIView {
    SkBitmap fBackBuffer;
}

- (void)drawRect:(CGRect)rect {
    SkiaViewController* viewController = [self controller];
    static constexpr double kFrameRate = 1.0 / 30.0;
    double next = [viewController isPaused] ? 0 : kFrameRate + SkTime::GetNSecs() * 1e-9;
    [super drawRect:rect];
    CGSize size = [self bounds].size;
    SkISize iSize = {(int)size.width, (int)size.height};
    if (fBackBuffer.drawsNothing() || iSize != fBackBuffer.dimensions()) {
        fBackBuffer.allocN32Pixels(iSize.fWidth, iSize.fHeight);
    }
    fBackBuffer.eraseColor(SK_ColorTRANSPARENT);
    {
        SkCanvas canvas(fBackBuffer);
        [viewController draw:rect toCanvas:&canvas atSize:size];
    }
    SkCGDrawBitmap(UIGraphicsGetCurrentContext(), fBackBuffer, 0, 0);
    if (next) {
        [NSTimer scheduledTimerWithTimeInterval:std::max(0.0, next - SkTime::GetNSecs() * 1e-9)
                 target:self
                 selector:@selector(setNeedsDisplay)
                 userInfo:nil
                 repeats:NO];
    }
}
@end

@interface SkiaUIContext : SkiaContext
    - (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame;
    - (SkiaViewController*) getViewController:(UIView*)view;
@end

@implementation SkiaUIContext
- (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame {
    SkiaUIView* skiaView = [[SkiaUIView alloc] initWithFrame:frame];
    [skiaView setController:vc];
    return skiaView;
}
- (SkiaViewController*) getViewController:(UIView*)view {
    return [view isKindOfClass:[SkiaUIView class]] ? [(SkiaUIView*)view controller] : nil;
}
@end

SkiaContext* MakeSkiaUIContext() { return [[SkiaUIContext alloc] init]; }

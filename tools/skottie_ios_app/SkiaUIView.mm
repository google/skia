// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaUIView.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkTime.h"
#include "include/utils/mac/SkCGUtils.h"

#include "modules/skottie/include/Skottie.h"

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

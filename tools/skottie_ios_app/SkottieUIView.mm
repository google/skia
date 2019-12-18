// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkottieUIView.h"

#include "tools/skottie_ios_app/SkAnimationDraw.h"
#include "tools/skottie_ios_app/SkTimeKeeper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/utils/mac/SkCGUtils.h"

#include "modules/skottie/include/Skottie.h"

@implementation SkottieUIView
- (BOOL)loadAnimation:(NSData*) data {
    SkottieViewController* vc = [[SkottieViewController alloc] init];
    if (![vc loadAnimation:data]) {
        return false;
    }
    [self setController:vc];
    return true;
}

- (void)setStopAtEnd:(BOOL)stop{
    SkiaViewController* vc = [self controller];
    if (vc && [vc isKindOfClass:[SkottieViewController class]]) {
        [(SkottieViewController*)vc setStopAtEnd:stop];
        [self setNeedsDisplay];
    }
}

- (void)seek:(float)seconds {
    SkiaViewController* vc = [self controller];
    if (vc && [vc isKindOfClass:[SkottieViewController class]]) {
        [(SkottieViewController*)vc seek:seconds];
        [self setNeedsDisplay];
    }
}

- (CGSize)size {
    SkiaViewController* vc = [self controller];
    if (vc && [vc isKindOfClass:[SkottieViewController class]]) {
        return [(SkottieViewController*)vc size];
    }
    return CGSize{0, 0};
}

- (BOOL)togglePaused {
    SkiaViewController* vc = [self controller];
    if (vc && [vc isKindOfClass:[SkottieViewController class]]) {
        [self setNeedsDisplay];
        [(SkottieViewController*)vc togglePaused];
    }
    return [vc isPaused];
}

- (BOOL)isPaused {
    return [[self controller] isPaused];
}
@end

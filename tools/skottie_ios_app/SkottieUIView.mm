// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkottieUIView.h"

#include "tools/skottie_ios_app/SkottieViewController.h"

template <typename T, typename U>
T* objc_cast(U* p) { return [p isKindOfClass:[T class]] ? (T*)p : nullptr; }

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
    if (SkottieViewController* vc = objc_cast<SkottieViewController>([self controller])) {
        [vc setStopAtEnd:stop];
        [self setNeedsDisplay];
    }
}

- (void)seek:(float)seconds {
    if (SkottieViewController* vc = objc_cast<SkottieViewController>([self controller])) {
        [vc seek:seconds];
        [self setNeedsDisplay];
    }
}

- (CGSize)size {
    if (SkottieViewController* vc = objc_cast<SkottieViewController>([self controller])) {
        return [vc size];
    }
    return CGSize{0, 0};
}

- (BOOL)togglePaused {
    if (SkottieViewController* vc = objc_cast<SkottieViewController>([self controller])) {
        [self setNeedsDisplay];
        [vc togglePaused];
    }
    return [self isPaused];
}

- (BOOL)isPaused {
    return [[self controller] isPaused];
}
@end

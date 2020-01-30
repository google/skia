// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieViewController_DEFINED
#define SkottieViewController_DEFINED

#include "tools/skottie_ios_app/SkiaViewController.h"

#import <UIKit/UIKit.h>

// An abstraction of the Skottie module into Obective-C.
@interface SkottieViewController : SkiaViewController
- (void)draw:(CGRect)rect toCanvas:(SkCanvas*)canvas atSize:(CGSize)size;

// Return the current paused state.
- (bool)isPaused;

// When set, pauses at end of loop.
- (void)setStopAtEnd:(bool)stop;

// Load an animation from a Lottie JSON file.  Returns Yes on success.
- (bool)loadAnimation:(NSData*)d;

// Jump to the specified location in the animation.
- (void)seek:(float)seconds;

// Toggle paused mode.  Return paused state.
- (bool)togglePaused;

// Return the default size of the Lottie animation.
- (CGSize)size;

// Return the length of the animation loop.
- (float)animationDurationSeconds;

// Return the current position in the animation in seconds (between zero and
// animationDurationSeconds).
- (float)currentTime;
@end

#endif  // SkottieViewController_DEFINED

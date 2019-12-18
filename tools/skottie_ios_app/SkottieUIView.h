// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieUIView_DEFINED
#define SkottieUIView_DEFINED

#import <UIKit/UIKit.h>

#include "tools/skottie_ios_app/SkiaUIView.h"

@interface SkottieUIView : SkiaUIView

// Load an animation from a Lottie JSON file.  Returns Yes on success.
- (BOOL)loadAnimation:(NSData*)d;

// Jump to the specified location in the animation.
- (void)seek:(float)seconds;

// Toggle paused mode.  Return paused state.
- (BOOL)togglePaused;

// Return the current paused state.
- (BOOL)isPaused;

// Return the default size of the Lottie animation.
- (CGSize)size;
@end

#endif  // SkottieUIView_DEFINED

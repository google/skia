// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkiaViewController_DEFINED
#define SkiaViewController_DEFINED

class SkCanvas;

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h> 

@interface SkiaViewController : NSObject
- (void)draw:(CGRect)rect toCanvas:(SkCanvas*)canvas atSize:(CGSize)size;

// Return the current paused state.
- (bool)isPaused;
@end
#endif  // SkiaViewController_DEFINED

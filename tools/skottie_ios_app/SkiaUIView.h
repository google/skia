// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkiaUIView_DEFINED
#define SkiaUIView_DEFINED

#import <UIKit/UIKit.h>

#include "tools/skottie_ios_app/SkiaViewController.h"

@interface SkiaUIView : UIView
@property (strong) SkiaViewController* controller;

// Override of the UIView interface.
- (void)drawRect:(CGRect)rect;
@end

#endif  // SkiaUIView_DEFINED

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieUIView_DEFINED
#define SkottieUIView_DEFINED

#import <UIKit/UIKit.h>

#include "tools/skottie_ios_app/SkottieViewController.h"

@interface SkottieUIView : UIView
@property (strong) SkottieViewController* controller;

// Override of the UIView interface.
- (void)drawRect:(CGRect)rect;
@end

#endif  // SkottieUIView_DEFINED

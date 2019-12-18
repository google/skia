// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieGLView_DEFINED
#define SkottieGLView_DEFINED

#include "tools/skottie_ios_app/SkottieViewController.h"

#include <CoreFoundation/CoreFoundation.h>
#import <UIKit/UIKit.h>

class GrContext;
struct GrGLInterface;

@interface SkottieGLView : UIView
@property (strong) SkottieViewController* controller;
@property (assign) const GrGLInterface* grGLInterface;
@property (assign) GrContext* grContext;

// Override of the UIView interface.
- (void)drawRect:(CGRect)rect;
@end

#endif  // SkottieGLView_DEFINED

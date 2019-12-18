// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkiaGLView_DEFINED
#define SkiaGLView_DEFINED

#include "tools/skottie_ios_app/SkiaViewController.h"

#include <CoreFoundation/CoreFoundation.h>
#import <UIKit/UIKit.h>

class GrContext;
struct GrGLInterface;

@interface SkiaGLView : UIView
@property (strong) SkiaViewController* controller;
@property (assign) const GrGLInterface* grGLInterface;
@property (assign) GrContext* grContext;

// Override of the UIView interface.
- (void)drawRect:(CGRect)rect;
@end

#endif  // SkiaGLView_DEFINED

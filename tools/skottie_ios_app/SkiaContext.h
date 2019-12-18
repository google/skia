// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkiaContext_DEFINED
#define SkiaContext_DEFINED

#include "tools/skottie_ios_app/SkottieViewController.h"

#import <UIKit/UIKit.h>

@interface SkiaContext : NSObject
    - (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame;
    - (SkiaViewController*) getViewController:(UIView*)view;
@end

SkiaContext* MakeSkiaMetalContext();

SkiaContext* MakeSkiaGLContext();

SkiaContext* MakeSkiaUIContext();
#endif  // SkiaContext_DEFINED

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkiaMtkView_DEFINED
#define SkiaMtkView_DEFINED

#include "tools/skottie_ios_app/SkiaViewController.h"

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

class GrContext;

// A UIView that uses a Metal-backed SkSurface to draw.
@interface SkiaMtkView : MTKView
    @property (strong) SkiaViewController* controller;

    // Override of the MTKView interface.  Uses Skia+Metal to draw.
    - (void)drawRect:(CGRect)rect;

    // Required initializer.
    - (instancetype)initWithFrame:(CGRect)frameRect
                    device:(id<MTLDevice>)device
                    queue:(id<MTLCommandQueue>)queue
                    grDevice:(GrContext*)grContext;
@end

#endif  // SkiaMtkView_DEFINED

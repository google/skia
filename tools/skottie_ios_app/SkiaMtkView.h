// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkiaMtkView_DEFINED
#define SkiaMtkView_DEFINED

#include "tools/skottie_ios_app/SkiaViewController.h"

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

class GrContext;

@interface SkiaMtkView : MTKView

@property (strong) SkiaViewController* controller;

// Must be set to a Metal-backed GrContext in order to draw.
// e.g.: use SkMetalDeviceToGrContext().
@property (assign) GrContext* grContext;  // non-owning pointer.

// Must be set to a valid MTLCommandQueue. Will be used to present.
@property (assign) id<MTLCommandQueue> queue;  // non-owning pointer.

// Override of the MTKView interface.  Uses Skia+Metal to draw.
- (void)drawRect:(CGRect)rect;
@end

#endif  // SkiaMtkView_DEFINED

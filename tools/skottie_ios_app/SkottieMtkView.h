// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieMtkView_DEFINED
#define SkottieMtkView_DEFINED

#include "tools/skottie_ios_app/SkottieViewController.h"

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

class GrContext;

@interface SkottieMtkView : MTKView

@property (strong) SkottieViewController* controller;

// Must be set to a Metal-backed GrContext in order to draw.
// e.g.: use SkMetalDeviceToGrContext().
@property (assign) GrContext* grContext;  // non-owning pointer.

// Must be set to a valid MTLCommandQueue. Will be used to present.
@property (assign) id<MTLCommandQueue> queue;  // non-owning pointer.

// Override of the MTKView interface.  Uses Skia+Skottie+Metal to draw.
- (void)drawRect:(CGRect)rect;
@end

#endif  // SkottieMtkView_DEFINED

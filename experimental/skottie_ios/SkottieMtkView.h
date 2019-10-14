// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieMtkView_DEFINED
#define SkottieMtkView_DEFINED

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

class GrContext;

@interface SkottieMtkView : MTKView

// Must be set to a Metal-backed GrContext in order to draw.
// e.g.: use SkMetalDeviceToGrContext().
@property (assign) GrContext* grContext;  // non-owning pointer.

// Must be set to a valid MTLCommandQueue. Will be used to present.
@property (assign) id<MTLCommandQueue> queue;  // non-owning pointer.

// When set, pauses at end of loop.
@property (assign) BOOL stopAtEnd;

// Override of the MTKView interface.  Uses Skia+Skottie+Metal to draw.
- (void)drawRect:(CGRect)rect;

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

// Return the length of the animation loop.
- (float)animationDurationSeconds;

// Return the current position in the animation in seconds (between zero and
// animationDurationSeconds).
- (float)currentTime;
@end

#endif  // SkottieMtkView_DEFINED

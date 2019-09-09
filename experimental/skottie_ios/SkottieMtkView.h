// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieMtkView_DEFINED
#define SkottieMtkView_DEFINED

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

class GrContext;

@interface SkottieMtkView : MTKView
@property (assign) GrContext* grContext;  // non-owning pointer.
- (void)drawRect:(CGRect)rect;
- (BOOL)loadAnimation:(NSData*)d;
- (CGSize)size;
- (BOOL)togglePaused;
@end

#endif  // SkottieMtkView_DEFINED

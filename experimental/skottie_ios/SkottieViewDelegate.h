// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkottieViewDelegate_DEFINED
#define SkottieViewDelegate_DEFINED

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

class GrContext;

@interface SkottieViewDelegate : NSObject <MTKViewDelegate>
@property (assign) GrContext* grContext;  // non-owning pointer.
- (BOOL)loadAnimation:(NSData*)d;
- (CGSize)size;
- (BOOL)togglePaused;
@end
#endif  // SkottieViewDelegate_DEFINED

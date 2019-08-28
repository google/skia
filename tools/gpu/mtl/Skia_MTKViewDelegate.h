// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#import <MetalKit/MetalKit.h>

class SkiaView;
struct GrContextOptions;

@interface Skia_MTKViewDelegate : NSObject<MTKViewDelegate>
- (nonnull instancetype)init:(nonnull MTKView *)view
                        withOpts:(nullable const GrContextOptions*)opts
                        withView:(nonnull SkiaView*)delegate;
@end

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#import <MetalKit/MetalKit.h>

namespace skui { class ViewLayer; }
struct GrContextOptions;

@interface SkMTKViewDelegate : NSObject<MTKViewDelegate>
- (nonnull instancetype)init:(nonnull MTKView *)view
                        withOpts:(nullable const GrContextOptions*)opts
                        withView:(nonnull skui::ViewLayer*)delegate;
@end

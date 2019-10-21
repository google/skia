// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkMetalViewBridge_DEFINED
#define SkMetalViewBridge_DEFINED

#import <MetalKit/MetalKit.h>

class SkSurface;
class GrContext;
class GrContextOptions;
template <typename T> class sk_sp;

sk_sp<SkSurface> SkMtkViewToSurface(MTKView*, GrContext*);

sk_sp<GrContext> SkMetalDeviceToGrContext(id<MTLDevice>, id<MTLCommandQueue>, const GrContextOptions&);

void SkMtkViewConfigForSkia(MTKView*);

#endif  // SkMetalViewBridge_DEFINED

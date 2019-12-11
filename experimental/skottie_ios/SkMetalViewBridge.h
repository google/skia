// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkMetalViewBridge_DEFINED
#define SkMetalViewBridge_DEFINED

#import <MetalKit/MetalKit.h>

#include <memory>

class SkSurface;
class GrContext;
template <typename T> class sk_sp;

sk_sp<SkSurface> SkMtkViewToSurface(MTKView*, GrContext*);

struct GrContextRelease { void operator()(GrContext*); };

using GrContextHolder = std::unique_ptr<GrContext, GrContextRelease>;

GrContextHolder SkMetalDeviceToGrContext(id<MTLDevice>, id<MTLCommandQueue>);

void SkMtkViewConfigForSkia(MTKView*);

#endif  // SkMetalViewBridge_DEFINED

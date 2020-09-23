// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkMetalViewBridge_DEFINED
#define SkMetalViewBridge_DEFINED

#include "tools/skottie_ios_app/GrContextHolder.h"

#import <MetalKit/MetalKit.h>

#include <memory>

class GrRecordingContext;
class SkSurface;
template <typename T> class sk_sp;

sk_sp<SkSurface> SkMtkViewToSurface(MTKView*, GrRecordingContext*);

GrContextHolder SkMetalDeviceToGrContext(id<MTLDevice>, id<MTLCommandQueue>);

void SkMtkViewConfigForSkia(MTKView*);

#endif  // SkMetalViewBridge_DEFINED

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlRenderPipeline_DEFINED
#define skgpu_MtlRenderPipeline_DEFINED

#include "experimental/graphite/src/RenderPipeline.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class RenderPipeline final : public skgpu::RenderPipeline {
public:
    RenderPipeline();
    ~RenderPipeline() override {}

private:
};

} // namespace skgpu::mtl

#endif // skgpu_MtlRenderPipeline_DEFINED

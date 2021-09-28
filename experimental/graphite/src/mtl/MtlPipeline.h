/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlPipeline_DEFINED
#define skgpu_MtlPipeline_DEFINED

#include "experimental/graphite/src/Pipeline.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Pipeline final : public skgpu::Pipeline {
public:
    Pipeline();
    ~Pipeline() override {}

private:
};

} // namespace skgpu::mtl

#endif // skgpu_MtlPipeline_DEFINED

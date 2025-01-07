/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Sampler.h"

namespace skgpu::graphite {

Sampler::Sampler(const SharedContext* sharedContext)
        : Resource(sharedContext,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0) {}

Sampler::~Sampler() = default;

} // namespace skgpu::graphite

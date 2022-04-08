/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const Gpu* gpu) : Resource(gpu, Ownership::kOwned) {
}

GraphicsPipeline::~GraphicsPipeline() {
}

} // namespace skgpu::graphite

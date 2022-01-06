/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/GraphicsPipeline.h"

namespace skgpu {

GraphicsPipeline::GraphicsPipeline(const Gpu* gpu) : Resource(gpu) {
}

GraphicsPipeline::~GraphicsPipeline() {
}

} // namespace skgpu

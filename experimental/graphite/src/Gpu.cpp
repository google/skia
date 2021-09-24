/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Gpu.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/ResourceProvider.h"

namespace skgpu {

Gpu::Gpu() {
    // subclasses create their own subclassed resource provider
}

Gpu::~Gpu() {
}

} // namespace skgpu

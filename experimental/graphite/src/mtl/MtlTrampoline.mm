/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlTrampoline.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"

namespace skgpu::graphite {
sk_sp<skgpu::graphite::Gpu> MtlTrampoline::MakeGpu(const MtlBackendContext& backendContext) {
    return MtlGpu::Make(backendContext);
}

}  // namespace skgpu::graphite

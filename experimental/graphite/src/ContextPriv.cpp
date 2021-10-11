/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextPriv.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/Gpu.h"

namespace skgpu {

const Caps* ContextPriv::caps() {
    return fContext->fGpu->caps();
}

} // namespace skgpu

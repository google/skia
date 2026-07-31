/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/RasterContext.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "src/core/SkSharedContext.h"

namespace SkContexts {

// Creates a context for SkContext with only Software Rasterization
std::unique_ptr<SkContext> MakeRaster(const SkContextOptions& options) {
    return nullptr;
}

}  // namespace SkContexts

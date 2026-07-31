/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RasterContext_DEFINED
#define RasterContext_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

class SkContext;
struct SkContextOptions;

namespace SkContexts {

// Creates a context for SkContext with only Software Rasterization
std::unique_ptr<SkContext> MakeRaster(const SkContextOptions& options);

}  // namespace SkContexts

#endif  // RasterContext_DEFINED

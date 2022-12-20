/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlUtils_DEFINED
#define skgpu_graphite_MtlUtils_DEFINED

#include "include/core/SkTypes.h"
#include <memory>

namespace skgpu { struct MtlBackendContext; }

namespace skgpu::graphite {

class Context;
struct ContextOptions;

std::unique_ptr<Context> MakeMetalContext(const MtlBackendContext&, const ContextOptions&);

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlUtils_DEFINED

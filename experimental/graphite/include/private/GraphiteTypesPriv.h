/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypesPriv_DEFINED
#define skgpu_GraphiteTypesPriv_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

/**
 * Is the Texture renderable or not
 */
enum class Renderable : bool {
    kNo = false,
    kYes = true,
};

enum class DepthStencilType {
    kDepthOnly,
    kStencilOnly,
    kDepthStencil,
};

} // namespace skgpu

#endif // skgpu_GraphiteTypesPriv_DEFINED

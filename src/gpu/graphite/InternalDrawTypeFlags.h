/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_InternalDrawTypeFlags_DEFINED
#define skgpu_graphite_InternalDrawTypeFlags_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

namespace skgpu::graphite {

/*
 * This enum extends the DrawTypeFlags enum to include 'drawTypes' that are only needed internal
 * to Graphite.
 */
enum InternalDrawTypeFlags : uint16_t {
    // Corresponds to the CoverageMaskRenderStep. Used when creating precompile Pipelines for
    // blur-filtering and raster/compute path atlasing.
    kCoverageMask  = DrawTypeFlags::kLast << 1,

    // Corresponds to the AnalyticBlurRenderStep. It can perform an analytic blur for rects,
    // rrects, and circles.
    kAnalyticBlur = DrawTypeFlags::kLast << 2,

    kLastInternal = kAnalyticBlur,
};
static_assert(kLastInternal <= (1 << 15), "DrawTypeFlags do not fit in 16 bits");

} // namespace skgpu::graphite

#endif // skgpu_graphite_InternalDrawTypeFlags_DEFINED

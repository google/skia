/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptionsPriv_DEFINED
#define skgpu_graphite_ContextOptionsPriv_DEFINED

#include "include/private/base/SkMath.h"

namespace skgpu::graphite {

/**
 * Used to include or exclude a specific path rendering technique for testing purposes.
 */
enum class PathRendererStrategy {
    /**
     * Graphite selects the best path rendering technique for each shape. This is the default
     * behavior.
     */
    kDefault,

    /**
     * All paths are rasterized into coverage masks using a GPU compute approach. This method
     * always uses analytic anti-aliasing.
     */
    kComputeAnalyticAA,

    /**
     * All paths are rasterized into coverage masks using a GPU compute approach. This method
     * supports 16 and 8 sample multi-sampled anti-aliasing.
     */
    kComputeMSAA16,
    kComputeMSAA8,

    /**
     * All paths are rasterized into coverage masks using the CPU raster backend.
     */
    kRasterAA,

    /**
     * Render paths using tessellation and stencil-and-cover.
     */
    kTessellation,
};

/**
 * Private options that are only meant for testing within Skia's tools.
 */
struct ContextOptionsPriv {

    int  fMaxTextureSizeOverride = SK_MaxS32;

    /**
     * Maximum width and height of internal texture atlases.
     */
    int  fMaxTextureAtlasSize = 2048;

    /**
     * If true, will store a pointer in Recorder that points back to the Context
     * that created it. Used by readPixels() and other methods that normally require a Context.
     */
    bool fStoreContextRefInRecorder = false;

    PathRendererStrategy fPathRendererStrategy = PathRendererStrategy::kDefault;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptionsPriv_DEFINED

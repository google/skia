/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextOptions_DEFINED
#define GrContextOptions_DEFINED

#include "SkTypes.h"

struct GrContextOptions {
    GrContextOptions() {}

    // Suppress prints for the GrContext.
    bool fSuppressPrints = false;

    /** Overrides: These options override feature detection using backend API queries. These
        overrides can only reduce the feature set or limits, never increase them beyond the
        detected values. */

    int  fMaxTextureSizeOverride = SK_MaxS32;

    /** If non-zero, overrides the maximum size of a tile for sw-backed images and bitmaps rendered
        by SkGpuDevice. */
    int  fMaxTileSizeOverride = 0;
    bool fSuppressDualSourceBlending = false;

    /** the threshold in bytes above which we will use a buffer mapping API to map vertex and index
        buffers to CPU memory in order to update them.  A value of -1 means the GrContext should
        deduce the optimal value for this platform. */
    int  fBufferMapThreshold = -1;

    /** some gpus have problems with partial writes of the rendertarget */
    bool fUseDrawInsteadOfPartialRenderTargetWrite = false;

    /** The GrContext operates in immediate mode. It will issue all draws to the backend API
        immediately. Intended to ease debugging. */
    bool fImmediateMode = false;

    /** For debugging purposes turn each GrBatch's bounds into a clip rect. This is used to
        verify that the clip bounds are conservative. */
    bool fClipBatchToBounds = false;

    /** For debugging purposes draw a wireframe device bounds rect for each GrBatch. The wire
        frame rect is draw before the GrBatch in order to visualize batches that draw outside
        of their dev bounds. */
    bool fDrawBatchBounds = false;

    /** For debugging, override the default maximum look-back or look-ahead window for GrBatch
        combining. */
    int fMaxBatchLookback = -1;
    int fMaxBatchLookahead = -1;

    /** Force us to do all swizzling manually in the shader and don't rely on extensions to do
        swizzling. */
    bool fUseShaderSwizzling = false;

    /** Construct mipmaps manually, via repeated downsampling draw-calls. This is used when
        the driver's implementation (glGenerateMipmap) contains bugs. This requires mipmap
        level and LOD control (ie desktop or ES3). */
    bool fDoManualMipmapping = false;

    /** Enable instanced rendering as long as all required functionality is supported by the HW.
        Instanced rendering is still experimental at this point and disabled by default. */
    bool fEnableInstancedRendering = false;

    /** Disables distance field rendering for paths. Distance field computation can be expensive
        and yields no benefit if a path is not rendered multiple times with different transforms */
    bool fDisableDistanceFieldPaths = false;

    /**
     * If true this allows path mask textures to be cached. This is only really useful if paths
     * are commonly rendered at the same scale and fractional translation.
     */
    bool fAllowPathMaskCaching = false;

    /**
     * Force all path draws to go through through the sw-rasterize-to-texture code path (assuming
     * the path is not recognized as a simpler shape (e.g. a rrect). This is intended for testing
     * purposes.
     */
    bool fForceSWPathMasks = false;
};

#endif

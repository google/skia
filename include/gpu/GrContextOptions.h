/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextOptions_DEFINED
#define GrContextOptions_DEFINED

#include "SkTypes.h"
#include "GrTypes.h"

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

    /** For debugging, override the default maximum look-back or look-ahead window for GrOp
        combining. */
    int fMaxOpCombineLookback = -1;
    int fMaxOpCombineLookahead = -1;

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

    /**
     * If true this allows path mask textures to be cached. This is only really useful if paths
     * are commonly rendered at the same scale and fractional translation.
     */
    bool fAllowPathMaskCaching = false;

    /**
     * If true, sRGB support will not be enabled unless sRGB decoding can be disabled (via an
     * extension). If mixed use of "legacy" mode and sRGB/color-correct mode is not required, this
     * can be set to false, which will significantly expand the number of devices that qualify for
     * sRGB support.
     */
    bool fRequireDecodeDisableForSRGB = true;

    /**
     * If true, the GPU will not be used to perform YUV -> RGB conversion when generating
     * textures from codec-backed images.
     */
    bool fDisableGpuYUVConversion = false;

    /**
     * If true, the caps will never report driver support for path rendering.
     */
    bool fSuppressPathRendering = false;

    /**
     * Allows the client to include or exclude specific GPU path renderers.
     */
    enum class GpuPathRenderers {
        kNone              = 0, // Always use sofware masks.
        kDashLine          = 1 << 0,
        kStencilAndCover   = 1 << 1,
        kMSAA              = 1 << 2,
        kAAHairline        = 1 << 3,
        kAAConvex          = 1 << 4,
        kAALinearizing     = 1 << 5,
        kSmall             = 1 << 6,
        kTessellating      = 1 << 7,
        kDefault           = 1 << 8,

        kAll               = kDefault | (kDefault - 1),

        // For legacy. To be removed when updated in Android.
        kDistanceField     = kSmall
    };

    GpuPathRenderers fGpuPathRenderers = GpuPathRenderers::kAll;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrContextOptions::GpuPathRenderers)

#endif

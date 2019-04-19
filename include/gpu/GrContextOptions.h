/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextOptions_DEFINED
#define GrContextOptions_DEFINED

#include "SkData.h"
#include "SkTypes.h"
#include "GrTypes.h"
#include "../private/GrTypesPriv.h"
#include "GrDriverBugWorkarounds.h"

#include <vector>

class SkExecutor;

#if SK_SUPPORT_GPU
struct SK_API GrContextOptions {
    enum class Enable {
        /** Forces an option to be disabled. */
        kNo,
        /** Forces an option to be enabled. */
        kYes,
        /**
         * Uses Skia's default behavior, which may use runtime properties (e.g. driver version).
         */
        kDefault
    };

    /**
     * Abstract class which stores Skia data in a cache that persists between sessions. Currently,
     * Skia stores compiled shader binaries (only when glProgramBinary / glGetProgramBinary are
     * supported) when provided a persistent cache, but this may extend to other data in the future.
     */
    class SK_API PersistentCache {
    public:
        virtual ~PersistentCache() {}

        /**
         * Returns the data for the key if it exists in the cache, otherwise returns null.
         */
        virtual sk_sp<SkData> load(const SkData& key) = 0;

        virtual void store(const SkData& key, const SkData& data) = 0;
    };

    GrContextOptions() {}

    // Suppress prints for the GrContext.
    bool fSuppressPrints = false;

    /** Overrides: These options override feature detection using backend API queries. These
        overrides can only reduce the feature set or limits, never increase them beyond the
        detected values. */

    int  fMaxTextureSizeOverride = SK_MaxS32;

    /** the threshold in bytes above which we will use a buffer mapping API to map vertex and index
        buffers to CPU memory in order to update them.  A value of -1 means the GrContext should
        deduce the optimal value for this platform. */
    int  fBufferMapThreshold = -1;

    /**
     * Executor to handle threaded work within Ganesh. If this is nullptr, then all work will be
     * done serially on the main thread. To have worker threads assist with various tasks, set this
     * to a valid SkExecutor instance. Currently, used for software path rendering, but may be used
     * for other tasks.
     */
    SkExecutor* fExecutor = nullptr;

    /** Construct mipmaps manually, via repeated downsampling draw-calls. This is used when
        the driver's implementation (glGenerateMipmap) contains bugs. This requires mipmap
        level and LOD control (ie desktop or ES3). */
    bool fDoManualMipmapping = false;

    /**
     * Disables the use of coverage counting shortcuts to render paths. Coverage counting can cause
     * artifacts along shared edges if care isn't taken to ensure both contours wind in the same
     * direction.
     */
    // FIXME: Once this is removed from Chrome and Android, rename to fEnable"".
    bool fDisableCoverageCountingPaths = true;

    /**
     * Disables distance field rendering for paths. Distance field computation can be expensive,
     * and yields no benefit if a path is not rendered multiple times with different transforms.
     */
    bool fDisableDistanceFieldPaths = false;

    /**
     * If true this allows path mask textures to be cached. This is only really useful if paths
     * are commonly rendered at the same scale and fractional translation.
     */
    bool fAllowPathMaskCaching = true;

    /**
     * If true, the GPU will not be used to perform YUV -> RGB conversion when generating
     * textures from codec-backed images.
     */
    bool fDisableGpuYUVConversion = false;

    /**
     * The maximum size of cache textures used for Skia's Glyph cache.
     */
    size_t fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;

    /**
     * Below this threshold size in device space distance field fonts won't be used. Distance field
     * fonts don't support hinting which is more important at smaller sizes. A negative value means
     * use the default threshold.
     */
    float fMinDistanceFieldFontSize = -1.f;

    /**
     * Above this threshold size in device space glyphs are drawn as individual paths. A negative
     * value means use the default threshold.
     */
    float fGlyphsAsPathsFontSize = -1.f;

    /**
     * Can the glyph atlas use multiple textures. If allowed, the each texture's size is bound by
     * fGlypheCacheTextureMaximumBytes.
     */
    Enable fAllowMultipleGlyphCacheTextures = Enable::kDefault;

    /**
     * Bugs on certain drivers cause stencil buffers to leak. This flag causes Skia to avoid
     * allocating stencil buffers and use alternate rasterization paths, avoiding the leak.
     */
    bool fAvoidStencilBuffers = false;

    /**
     * If true, texture fetches from mip-mapped textures will be biased to read larger MIP levels.
     * This has the effect of sharpening those textures, at the cost of some aliasing, and possible
     * performance impact.
     */
    bool fSharpenMipmappedTextures = false;

    /**
     * Enables driver workaround to use draws instead of HW clears, e.g. glClear on the GL backend.
     */
    Enable fUseDrawInsteadOfClear = Enable::kDefault;

    /**
     * Allow Ganesh to sort the opLists prior to allocating resources. This is an optional
     * behavior but, eventually, this will just be what is done and will not be optional.
     */
    Enable fSortRenderTargets = Enable::kDefault;

    /**
     * Allow Ganesh to more aggressively reorder operations. This is an optional
     * behavior that is only relevant when 'fSortRenderTargets' is enabled.
     * Eventually this will just be what is done and will not be optional.
     */
    Enable fReduceOpListSplitting = Enable::kDefault;

    /**
     * Some ES3 contexts report the ES2 external image extension, but not the ES3 version.
     * If support for external images is critical, enabling this option will cause Ganesh to limit
     * shaders to the ES2 shading language in that situation.
     */
    bool fPreferExternalImagesOverES3 = false;

    /**
     * Disables correctness workarounds that are enabled for particular GPUs, OSes, or drivers.
     * This does not affect code path choices that are made for perfomance reasons nor does it
     * override other GrContextOption settings.
     */
    bool fDisableDriverCorrectnessWorkarounds = false;

    /**
     * Cache in which to store compiled shader binaries between runs.
     */
    PersistentCache* fPersistentCache = nullptr;

    /**
     * This affects the usage of the PersistentCache. If this is set to true GLSL shader strings
     * rather than GL program binaries will be cached. It is intended to be used when the driver's
     * binary loading/storing is believed to have bugs. Caching GLSL strings still saves a
     * significant amount of CPU work when a GL program is created.
     */
     bool fDisallowGLSLBinaryCaching = false;

#if GR_TEST_UTILS
    /**
     * Private options that are only meant for testing within Skia's tools.
     */

    /**
     * If non-zero, overrides the maximum size of a tile for sw-backed images and bitmaps rendered
     * by SkGpuDevice.
     */
    int  fMaxTileSizeOverride = 0;

    /**
     * Prevents use of dual source blending, to test that all xfer modes work correctly without it.
     */
    bool fSuppressDualSourceBlending = false;

    /**
     * If true, the caps will never report driver support for path rendering.
     */
    bool fSuppressPathRendering = false;

    /**
     * If true, the caps will never support geometry shaders.
     */
    bool fSuppressGeometryShaders = false;

    /**
     * Render everything in wireframe
     */
    bool fWireframeMode = false;

    /**
     * Include or exclude specific GPU path renderers.
     */
    GpuPathRenderers fGpuPathRenderers = GpuPathRenderers::kAll;
#endif

#if SK_SUPPORT_ATLAS_TEXT
    /**
     * Controls whether distance field glyph vertices always have 3 components even when the view
     * matrix does not have perspective.
     */
    Enable fDistanceFieldGlyphVerticesAlwaysHaveW = Enable::kDefault;
#endif

    GrDriverBugWorkarounds fDriverBugWorkarounds;
};
#else
struct GrContextOptions {
    struct PersistentCache {};
};
#endif

#endif

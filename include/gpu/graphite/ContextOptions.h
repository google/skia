/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptions_DEFINED
#define skgpu_graphite_ContextOptions_DEFINED

#include "include/private/base/SkAPI.h"
#include "include/private/base/SkMath.h"

namespace skgpu { class ShaderErrorHandler; }

namespace skgpu::graphite {

struct SK_API ContextOptions {
    ContextOptions() {}

    /**
     * Disables correctness workarounds that are enabled for particular GPUs, OSes, or drivers.
     * This does not affect code path choices that are made for perfomance reasons nor does it
     * override other ContextOption settings.
     */
    bool fDisableDriverCorrectnessWorkarounds = false;

    /**
     * If present, use this object to report shader compilation failures. If not, report failures
     * via SkDebugf and assert.
     */
    skgpu::ShaderErrorHandler* fShaderErrorHandler = nullptr;

    /**
     * Specifies the number of samples Graphite should use when performing internal draws with MSAA
     * (hardware capabilities permitting).
     *
     * If <= 1, Graphite will disable internal code paths that use multisampling.
     */
    int fInternalMultisampleCount = 4;

    /**
     * Will the client make sure to only ever be executing one thread that uses the Context and all
     * derived classes (e.g. Recorders, Recordings, etc.) at a time. If so we can possibly make some
     * objects (e.g. VulkanMemoryAllocator) not thread safe to improve single thread performance.
     */
    bool fClientWillExternallySynchronizeAllThreads = false;

    /**
     * The maximum size of cache textures used for Skia's Glyph cache.
     */
    size_t fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;

    /**
     * Below this threshold size in device space distance field fonts won't be used. Distance field
     * fonts don't support hinting which is more important at smaller sizes.
     */
    float fMinDistanceFieldFontSize = 18;

    /**
     * Above this threshold size in device space glyphs are drawn as individual paths.
     */
#if defined(SK_BUILD_FOR_ANDROID)
    float fGlyphsAsPathsFontSize = 384;
#elif defined(SK_BUILD_FOR_MAC)
    float fGlyphsAsPathsFontSize = 256;
#else
    float fGlyphsAsPathsFontSize = 324;
#endif

    /**
     * Can the glyph atlas use multiple textures. If allowed, the each texture's size is bound by
     * fGlypheCacheTextureMaximumBytes.
     */
    bool fAllowMultipleGlyphCacheTextures = true;
    bool fSupportBilerpFromGlyphAtlas = false;

    /**
     * In the Dawn backend, controls SkSL compilation to native code. When false, we emit SPIR-V and
     * rely on Tint's SPIR-V Reader. When true, we emit native WGSL.
     * TODO(b/40044196): once WGSL is stable, remove this flag and always emit WGSL.
     */
    bool fEnableWGSL = true;

    static constexpr size_t kDefaultContextBudget = 256 * (1 << 20);
    /**
     * What is the budget for GPU resources allocated and held by the Context.
     */
    size_t fGpuBudgetInBytes = kDefaultContextBudget;

#if defined(GRAPHITE_TEST_UTILS)
    /**
     * Private options that are only meant for testing within Skia's tools.
     */

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
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptions

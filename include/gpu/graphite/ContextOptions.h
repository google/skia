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

struct ContextOptionsPriv;

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
     * This option is only legal with the Dawn backend. It makes it so that the Context will never
     * call Device::Tick() in native or rely on Asyncify to yield in Emscripten. Note that disables
     * any CPU/GPU synchronization or any ability for Context to wait for GPU work to complete.
     * This means SyncToCpu::kYes is disallowed as a parameter to Context::submit.
     *
     * In native the client can use wgpu::Device::Tick to allow the GPU to make forward progress
     * that Context can detect. In Emscripten the client must allow the main thread loop to run
     * in order for Context to detect GPU progress.
     *
     * The client must guarantee that GPU work has compeleted before destroying Context as Context
     * cannot await the work completion in its destructor. Context reports whether it is awaiting
     * GPU work completion via Context::hasUnfinishedGpuWork().
     */
    bool fNeverYieldToWebGPU = false;

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
     * Disable caching of glyph uploads at the start of each Recording. These can add additional
     * overhead and are only necessary if Recordings are replayed or played out of order.
     */
    bool fDisableCachedGlyphUploads = false;

    static constexpr size_t kDefaultContextBudget = 256 * (1 << 20);
    /**
     * What is the budget for GPU resources allocated and held by the Context.
     */
    size_t fGpuBudgetInBytes = kDefaultContextBudget;

    /**
     * Private options that are only meant for testing within Skia's tools.
     */
    ContextOptionsPriv* fOptionsPriv = nullptr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptions

/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptions_DEFINED
#define skgpu_graphite_ContextOptions_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkMath.h"

#include <optional>

class SkData;
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
     * The maximum size of textures used for Skia's PathAtlas caches.
     */
    int fMaxPathAtlasTextureSize = 8192;  // oversized, PathAtlas will likely be smaller

    /**
     * Can the glyph and path atlases use multiple textures. If allowed, each texture's size is
     * bound by fGlyphCacheTextureMaximumBytes and fMaxPathAtlasTextureSize, respectively.
     */
    bool fAllowMultipleAtlasTextures = true;
    bool fSupportBilerpFromGlyphAtlas = false;

    /**
     * For the moment, if Recordings are replayed in the order they are recorded, then
     * Graphite can make certain assumptions that allow for better performance. Otherwise
     * we have to flush some caches at the start of each Recording to ensure that they can
     * be played back properly.
     */
    bool fRequireOrderedRecordings = false;

    static constexpr size_t kDefaultContextBudget = 256 * (1 << 20);
    /**
     * What is the budget for GPU resources allocated and held by the Context.
     */
    size_t fGpuBudgetInBytes = kDefaultContextBudget;

    /**
     * Whether labels will be set on backend resources.
     */
#if defined(SK_DEBUG)
    bool fSetBackendLabels = true;
#else
    bool fSetBackendLabels = false;
#endif

    /**
     * If Skia is creating a default VMA allocator for the Vulkan backend this value will be used
     * for the preferredLargeHeapBlockSize. If the value is not set, then Skia will use an
     * inernally defined default size.
     *
     * However, it is highly discouraged to have Skia make a default allocator (and support for
     * doing so will be removed soon,  b/321962001). Instead clients should create their own
     * allocator to pass into Skia where they can fine tune this value themeselves.
     */
    std::optional<uint64_t> fVulkanVMALargeHeapBlockSize;

    /** Client-provided context that is passed to client-provided PipelineCallback. */
    using PipelineCallbackContext = void*;
    /**  Client-provided callback that is called whenever Graphite encounters a new Pipeline. */
    using PipelineCallback = void (*)(PipelineCallbackContext context, sk_sp<SkData> pipelineData);

    /**
     *  These two members allow a client to register a callback that will be invoked
     *  whenever Graphite encounters a new Pipeline. The callback will be passed an
     *  sk_sp<SkData> that a client can take ownership of and serialize. The SkData
     *  contains all the information Graphite requires to recreate the Pipeline at
     *  a later date. The SkData is versioned however, so must be regenerated and
     *  re-serialized when it becomes out of date.
     */
    PipelineCallbackContext fPipelineCallbackContext = nullptr;
    PipelineCallback fPipelineCallback = nullptr;

    /**
     * Private options that are only meant for testing within Skia's tools.
     */
    ContextOptionsPriv* fOptionsPriv = nullptr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptions

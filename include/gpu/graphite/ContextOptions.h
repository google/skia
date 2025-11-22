/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptions_DEFINED
#define skgpu_graphite_ContextOptions_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkMath.h"

#include <optional>
#include <string>

class SkData;
class SkExecutor;
class SkRuntimeEffect;
namespace skgpu { class ShaderErrorHandler; }

namespace skgpu::graphite {

struct ContextOptionsPriv;
class PersistentPipelineStorage;

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
    uint8_t fInternalMultisampleCount = 4;

    /**
     * If set, this specifies the max width/height of MSAA textures that Graphite should use for
     * internal draws. Graphite might have to break the drawing region into multiple tiles to
     * satisfy the size constraint.
     * Note: this option will be ignored if the backend doesn't support it, or if a more optimal HW
     * feature is available.
     */
    std::optional<SkISize> fInternalMSAATileSize = std::nullopt;

    /**
     * If set, paths that are smaller than this size (in device space) will avoid MSAA techniques,
     * even if MSAA is otherwise enabled via `fInternalMultisampleCount`. This should be smaller
     * than `fGlyphsAsPathsFontSize` or large glyphs will not correctly avoid higher memory
     * overhead.
     */
    float fMinimumPathSizeForMSAA = 0;

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
     * For the moment, if Recordings from the same Recorder are replayed in the order they are
     * recorded, then Graphite can make certain assumptions that allow for better performance.
     * Otherwise we have to flush some caches at the start of each Recording to ensure that they can
     * be played back properly.
     *
     * This is the default ordering requirement for a Recorder. It can be overridden by
     * setting the same field on the RecorderOptions passed to makeRecorder.
     *
     * Regardless of this value or a per-Recorder's setting, Recordings from separate Recorders can
     * always be inserted in any order but it is the application's responsible to ensure that any
     * implicit dependencies between the Recorders are respected (e.g. rendering to an SkSurface
     * in one Recorder and sampling from that SkSurface's SkImage view on another Recorder).
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
     * internally defined default size.
     *
     * However, it is highly discouraged to have Skia make a default allocator (and support for
     * doing so will be removed soon,  b/321962001). Instead clients should create their own
     * allocator to pass into Skia where they can fine tune this value themeselves.
     */
    std::optional<uint64_t> fVulkanVMALargeHeapBlockSize;

    /**
     * Client-provided context that is passed to the client-provided PipelineCachingCallback
     * and the (deprecated) PipelineCallback.
     */
    using PipelineCallbackContext = void*;

    PipelineCallbackContext fPipelineCallbackContext = nullptr;

    enum class PipelineCacheOp {
        kAddingPipeline,
        kPipelineFound,
    };

    using PipelineCachingCallback = void (*)(PipelineCallbackContext context,
                                             PipelineCacheOp op,
                                             const std::string& label,
                                             uint32_t uniqueKeyHash,
                                             bool fromPrecompile,
                                             sk_sp<SkData> pipelineData);

    /**
     * This member variable allows a client to register a callback that will be invoked
     * whenever Graphite either adds a Pipeline to its cache (kAddingPipeline op) or finds an
     * existing Pipeline in its cache (kPipelineFound op). Together this allows clients
     * to determine the frequency of a given Pipeline's use and which precompiled Pipelines
     * are unused. The callback is also passed:
     *    a human-readable label that describes the Pipeline
     *    a 32-bit hash code that can be used rather than rehashing the provided data
     *    a Boolean indicating if the Pipeline had been generated via Precompilation
     * Additionally, for kAddingPipeline ops:
     *    an SkData version of the Pipeline that a client can take ownership of and serialize.
     *    Not all Pipelines can be serialized, however, and nullptr will be passed in such cases.
     *
     * When provided, the SkData contains all the information Graphite requires to recreate
     * the Pipeline at a later date, but it is versioned so recreation can fail if it's
     * incompatible with a newer version of Skia.
     */
    PipelineCachingCallback fPipelineCachingCallback = nullptr;

    /**
     * Deprecated version of the Pipeline callback. This callback is only invoked for
     * PipelineCacheOp::kAddingPipeline ops and when the key is serializable. It is ignored
     * if fPipelineCachingCallback is set.
     */
    using PipelineCallback = void (*)(PipelineCallbackContext context, sk_sp<SkData> pipelineData);

    PipelineCallback fPipelineCallback = nullptr;

    /**
     * The runtime effects provided here will be registered as user-defined *known* runtime
     * effects and will be given a stable key. Such runtime effects can then be used in
     * serialized pipeline keys (c.f. PrecompileContext::precompile).
     *
     * Graphite will take a ref on the provided runtime effects and they will persist for as long
     * as the Context exists. Rather than recreating new SkRuntimeEffects using the same SkSL,
     * clients should use the existing SkRuntimeEffects provided here.
     *
     * Warning: Registering runtime effects here does obligate users to clear out their caches
     * of serialized pipeline keys if the provided runtime effects ever change in a meaningful way.
     * This includes adding, removing or reordering the effects provided here.
     */
    SkSpan<sk_sp<SkRuntimeEffect>> fUserDefinedKnownRuntimeEffects;

    /**
     * Executor to handle threaded work within Graphite. If this is nullptr, then all work will be
     * done serially on the main thread. To have worker threads assist with various tasks, set this
     * to a valid SkExecutor instance. Currently, used for Pipeline compilation, but may be used
     * for other tasks. It is up to the client to ensure the SkExecutor remains valid throughout
     * the lifetime of the Context.
     */
    SkExecutor* fExecutor = nullptr;

    /**
     * Allows Graphite to store Pipeline data across Context lifetimes. It is up to the
     * client to ensure the PersistentPipelineStorage object remains valid throughout the lifetime
     * of the Context(s).
     */
    PersistentPipelineStorage* fPersistentPipelineStorage = nullptr;

    /**
     * An experimental flag in development. Behavior and performance is subject to change.
     *
     * Enables the use of startCapture and endCapture functions. Calling these APIs will capture all
     * draw calls and surface creation from Recorders spawned from the Context.
     */
     bool fEnableCapture = false;

    /**
     * Private options that are only meant for testing within Skia's tools.
     */
    ContextOptionsPriv* fOptionsPriv = nullptr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptions

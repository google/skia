/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineData_DEFINED
#define skgpu_graphite_PipelineData_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorData.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#include <type_traits>
#include <variant>

namespace skgpu::graphite {

class Uniform;

/**
 * Wraps an SkSpan<const char> and provides ==/!= operators and a hash function based on the
 * bit contents of the spans. It is assumed that the bytes are aligned to match some uniform
 * interface declaration that will consume this data once it's copied to the GPU.
 */
class UniformDataBlock {
public:
    constexpr UniformDataBlock(const UniformDataBlock&) = default;
    constexpr UniformDataBlock() = default;

    static UniformDataBlock Make(UniformDataBlock toClone, SkArenaAlloc* arena) {
        const char* copy = arena->makeArrayCopy<char>(toClone.fData);
        return UniformDataBlock(SkSpan(copy, toClone.size()));
    }

    // Wraps the finished accumulated uniform data within the manager's underlying storage.
    static UniformDataBlock Wrap(UniformManager* uniforms) {
        return UniformDataBlock(uniforms->finish());
    }

    constexpr UniformDataBlock& operator=(const UniformDataBlock&) = default;

    explicit operator bool() const { return !this->empty(); }
    bool empty() const { return fData.empty(); }

    const char* data() const { return fData.data(); }
    size_t size() const { return fData.size(); }

    bool operator==(UniformDataBlock that) const {
        return this->size() == that.size() &&
               (this->data() == that.data() || // Shortcuts the memcmp if the spans are the same
                memcmp(this->data(), that.data(), this->size()) == 0);
    }
    bool operator!=(UniformDataBlock that) const { return !(*this == that); }

    struct Hash {
        uint32_t operator()(UniformDataBlock block) const {
            return SkChecksum::Hash32(block.fData.data(), block.fData.size_bytes());
        }
    };

private:
    // To ensure that the underlying data is actually aligned properly, UniformDataBlocks can
    // only be created publicly by copying an existing block or wrapping data accumulated by a
    // UniformManager (or transitively a PipelineDataGatherer).
    constexpr UniformDataBlock(SkSpan<const char> data) : fData(data) {}

    SkSpan<const char> fData;
};

/**
 * Wraps an SkSpan<const SampledTexture> (list of pairs of TextureProxy and SamplerDesc) and
 * provides ==/!= operators and a hash function based on the proxy addresses and sampler desc
 * bit representation.
 */
class TextureDataBlock {
public:
    using SampledTexture = std::pair<sk_sp<TextureProxy>, SamplerDesc>;

    constexpr TextureDataBlock(const TextureDataBlock&) = default;
    constexpr TextureDataBlock() = default;

    static TextureDataBlock Make(TextureDataBlock toClone, SkArenaAlloc* arena) {
        SampledTexture* copy = arena->makeArrayCopy<SampledTexture>(toClone.fTextures);
        return TextureDataBlock(SkSpan(copy, toClone.numTextures()));
    }

    // TODO(b/330864257): Once Device::drawCoverageMask() can keep its texture proxy alive without
    // creating a temporary TextureDataBlock this constructor can go away.
    explicit TextureDataBlock(const SampledTexture& texture) : fTextures(&texture, 1) {}

    constexpr TextureDataBlock& operator=(const TextureDataBlock&) = default;

    explicit operator bool() const { return !this->empty(); }
    bool empty() const { return fTextures.empty(); }

    int numTextures() const { return SkTo<int>(fTextures.size()); }
    const SampledTexture& texture(int index) const { return fTextures[index]; }

    bool operator==(TextureDataBlock other) const {
        if (fTextures.size() != other.fTextures.size()) {
            return false;
        }
        if (fTextures.data() == other.fTextures.data()) {
            return true; // shortcut for the same span
        }

        for (size_t i = 0; i < fTextures.size(); ++i) {
            if (fTextures[i] != other.fTextures[i]) {
                return false;
            }
        }

        return true;
    }
    bool operator!=(TextureDataBlock other) const { return !(*this == other);  }

    struct Hash {
        uint32_t operator()(TextureDataBlock block) const {
            uint32_t hash = 0;

            for (auto& d : block.fTextures) {
                SamplerDesc samplerKey = std::get<1>(d);
                hash = SkChecksum::Hash32(&samplerKey, sizeof(samplerKey), hash);

                // Because the lifetime of the TextureDataCache is for just one Recording and the
                // TextureDataBlocks hold refs on their proxies, we can just use the proxy's pointer
                // for the hash here.
                uintptr_t proxy = reinterpret_cast<uintptr_t>(std::get<0>(d).get());
                hash = SkChecksum::Hash32(&proxy, sizeof(proxy), hash);
            }

            return hash;
        }
    };

private:
    friend class PipelineDataGatherer;

    // Initial TextureDataBlocks must come from a PipelineDataGatherer
    constexpr TextureDataBlock(SkSpan<const SampledTexture> textures) : fTextures(textures) {}

    SkSpan<const SampledTexture> fTextures;
};

template <typename K,                  // Initial type inserted into the map from K->Index
          typename V = K,              // Value type stored in contiguous memory (map from Index->V)
          typename S = std::monostate, // Storage with persist(K) -> K function if not monostate
          typename H = SkGoodHash>     // Hash function applied to K
class DenseBiMap {
public:
    using Index = uint32_t;
    static constexpr Index kInvalidIndex = 4096;// 1 << SkNextLog2_portable(DrawList::kMaxRenderSteps);

    Index insert(K data) {
        Index* index = fDataToIndex.find(data);
        if (!index) {
            // First time we've seen this piece of data.
            SkASSERT(SkToU32(fIndexToData.size()) < kInvalidIndex);
            // Persist it in storage if S is not monostate.
            if constexpr (!std::is_same_v<S, std::monostate>) {
                data = fStorage.persist(data);
            } else {
                static_assert(std::is_trivially_copyable<K>::value);
            }

            index = fDataToIndex.set(data, static_cast<Index>(fIndexToData.size()));
            fIndexToData.emplace_back(data); // constructs V in place from single K argument
        }
        return *index;
    }

    const V& lookup(Index index) const { return fIndexToData[index]; }

    V& lookup(Index index) { return fIndexToData[index]; }

    skia_private::TArray<V>&& detach() { return std::move(fIndexToData); }

    const skia_private::TArray<V>& get() const { return fIndexToData; }

    void reset() {
        fIndexToData.clear();
        fDataToIndex.reset();
        if constexpr (!std::is_same_v<S, std::monostate>) {
            fStorage.~S();
            new (&fStorage) S();
        }
    }

    int count() const { return fIndexToData.size(); }

    template <typename SV = S>
    typename std::enable_if<!std::is_same_v<SV, std::monostate>, const SV&>::type
    storage() const { return fStorage; }

    template <typename SV = S>
    typename std::enable_if<!std::is_same_v<SV, std::monostate>, SV&>::type
    storage() { return fStorage; }

private:
    skia_private::THashMap<K, Index, H> fDataToIndex;
    skia_private::TArray<V> fIndexToData;

    S fStorage;
};

// A GraphicsPipelineCache is used to de-duplicate GraphicsPipelineDescs when they will all be
// used with the same as-yet-undeterminied RenderPassDesc. Once collected the indices can then be
// used to map to the resolved GraphicsPipelines after resources have been prepared.
using GraphicsPipelineCache = DenseBiMap<GraphicsPipelineDesc>;

// A UniformDataCache is used to deduplicate uniform data blocks uploaded to uniform / storage
// buffers for a DrawPass pipeline.
class UniformDataCache {
public:
    // Tracks uniform data on the CPU and then its transition to storage in a GPU buffer (UBO or
    // SSBO).
    struct Entry {
        UniformDataBlock fCpuData;
        BindBufferInfo fBufferBinding;

        // Can only be initialized with CPU data.
        Entry(UniformDataBlock cpuData) : fCpuData(cpuData) {}
    };

private:
    struct UniformCopier {
        UniformDataBlock persist(UniformDataBlock data) {
            return UniformDataBlock::Make(data, &fArena);
        }
        SkArenaAlloc fArena{0};
    };
    using UniformDataMap =
            DenseBiMap<UniformDataBlock, Entry, UniformCopier, UniformDataBlock::Hash>;

    UniformDataMap fUniforms;

public:
    using Index = UniformDataMap::Index;
    static constexpr Index kInvalidIndex = UniformDataMap::kInvalidIndex;

    UniformDataCache() = default;

    void reset() { fUniforms.reset(); }

    Index insert(UniformDataBlock dataBlock) { return fUniforms.insert(dataBlock); }

    const Entry& lookup(Index index) const { return fUniforms.lookup(index); }

    Entry& lookup(Index index) { return fUniforms.lookup(index); }

#if defined(GPU_TEST_UTILS)
    int count() { return fUniforms.count(); }
#endif
};

// A TextureDataCache is used to deduplicate sets of texture bindings and collect the list of
// unique texture proxies that are referenced by all inserted bindings.
class TextureDataCache {
    struct TakeTextureRef {
        TextureProxy* persist(TextureProxy* proxy) {
            // This ref will be adopted by the sk_sp() value stored in the TextureProxyCache.
            proxy->ref();
            return proxy;
        }
    };
    using TextureProxyCache = DenseBiMap<TextureProxy*, sk_sp<TextureProxy>, TakeTextureRef>;

    struct TextureCopier {
        TextureDataBlock persist(TextureDataBlock textures) {
            // Insert every referenced texture into fUniqueTextures to hand off to DrawPass.
            for (int i = 0; i < textures.numTextures(); ++i) {
                (void) fUniqueTextures.insert(textures.texture(i).first.get());
            }

            // Confirm that we're getting the right value back.
#if defined(SK_DEBUG)
            auto t = TextureDataBlock::Make(textures, &fArena);
            SkASSERT(textures == t);
            return t;
#else
            return TextureDataBlock::Make(textures, &fArena);
#endif
        }

        SkArenaAlloc fArena{0};
        TextureProxyCache fUniqueTextures;
    };
    using TextureDataMap = DenseBiMap<TextureDataBlock,
                                      TextureDataBlock,
                                      TextureCopier,
                                      TextureDataBlock::Hash>;
    TextureDataMap fTextures;

public:
    using Index = TextureDataMap::Index;
    static constexpr Index kInvalidIndex = TextureDataMap::kInvalidIndex;

    TextureDataCache() = default;

    void reset() { fTextures.reset(); }

    Index insert(TextureDataBlock dataBlock) { return fTextures.insert(dataBlock); }

    TextureDataBlock lookup(Index index) const { return fTextures.lookup(index); }

    skia_private::TArray<sk_sp<TextureProxy>> detachTextures() {
        return fTextures.storage().fUniqueTextures.detach();
    }

    const skia_private::TArray<TextureDataBlock>& getBindings() const {
        return fTextures.get();
    }

#if defined(GPU_TEST_UTILS)
    int bindingCount() { return fTextures.count(); }
    int uniqueTextureCount() { return fTextures.storage().fUniqueTextures.count(); }
#endif
};

class PipelineDataGatherer {
public:
    PipelineDataGatherer(Layout layout) : fUniformManager(layout) {}

    // Fully resets uniforms and textures, but does not reset gradient storage.
    void resetForDraw() {
        fUniformManager.reset();
        fTextures.clear();
        fPaintTextureCount = 0;
    }

#if defined(SK_DEBUG)
    // Check that the gatherer has been reset to its initial state prior to collecting new data.
    void checkReset() const {
        SkASSERT(fTextures.empty());
        SkASSERT(fUniformManager.isReset());
        SkASSERT(fPaintTextureCount == 0);
    }

    void checkRewind() const {
        SkASSERT(fTextures.size() == fPaintTextureCount);
        SkASSERT(fUniformManager.isReset());
    }
#endif // SK_DEBUG

    // All accumulated gradient data or empty if no draw requires this feature.
    SkSpan<const float> gradientBufferData() const { return fGradientStorage; }

    // Mark the end of extracting paint uniforms and textures from the current draw's PaintParams.
    UniformDataBlock endPaintData() {
        // Save the end of the paint textures for rewind(), but return the current state of the
        // uniforms.
        // TODO: Once paint and renderstep uniforms are combined, endPaintData() will return void
        // and this will just save the state of the UniformManager to rewind to.
        fPaintTextureCount = fTextures.size();
        return UniformDataBlock::Wrap(&fUniformManager);
    }

    // Mark the end of extract uniforms and textures from the RenderStep that will be combined with
    // the already extracted paint data.
    //
    // The returned TextureDataBlock represents the list of sampled textures to be bound for the
    // GPU draw call. If `performsShading` is true, this will be the combined set of textures for
    // both paint and render step. If `performsShading` is false, the TextureDataBlock represents
    // just the step's required textures.
    //
    // TODO: For now, the returned UniformDataBlock is always just the render step's uniforms since
    // the paint's uniforms are returned by endPaintData(). Once uniform data is combined then the
    // returned UniformDataBlock will follow the same pattern as the TextureDataBlock.
    std::pair<UniformDataBlock, TextureDataBlock> endRenderStepData(bool performsShading) {
        SkSpan<const TextureDataBlock::SampledTexture> textures{fTextures};
        if (!performsShading) {
            textures = textures.subspan(fPaintTextureCount);
        }
        return {UniformDataBlock::Wrap(&fUniformManager), TextureDataBlock(textures)};
    }

    // Rewind the PipelineDataGatherer to collect new uniforms and textures for another RenderStep
    // that depends on the already extracted PaintParams uniforms and textures.
    void rewindForRenderStep() {
        fTextures.resize_back(fPaintTextureCount);
        // TODO: Eventually this will not reset the uniform manager, but set its current byte offset
        // and required alignment to what was saved in endPaintData().
        fUniformManager.reset();
    }

    // Append a sampled texture that will be bound with a sampler matching `samplerDesc`. Textures
    // are bound in the order that they are added to the gatherer.
    void add(sk_sp<TextureProxy> proxy, const SamplerDesc& samplerDesc) {
        fTextures.push_back({std::move(proxy), samplerDesc});
    }

    // Mimic the type-safe API available in UniformManager
    template <typename T> void write(const T& t) { fUniformManager.write(t); }
    template <typename T> void writeHalf(const T& t) { fUniformManager.writeHalf(t); }
    template <typename T> void writeArray(SkSpan<const T> t) { fUniformManager.writeArray(t); }
    template <typename T> void writeHalfArray(SkSpan<const T> t) {
        fUniformManager.writeHalfArray(t);
    }

    void write(const Uniform& u, const void* data) { fUniformManager.write(u, data); }

    void writePaintColor(const SkPMColor4f& color) { fUniformManager.writePaintColor(color); }

    void beginStruct(int baseAligment) { fUniformManager.beginStruct(baseAligment); }
    void endStruct() { fUniformManager.endStruct(); }

    // Checks if data already exists for the requested gradient shader, and returns a nullptr
    // and the offset the data begins at. If it doesn't exist, it allocates the data for the
    // required number of stops and caches the start index, returning the data pointer
    // and index offset the data will begin at.
    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader) {
        int* existingOfffset = fGradientOffsetCache.find(shader);
        if (existingOfffset) {
            return std::make_pair(nullptr, *existingOfffset);
        }

        auto dataPair = this->allocateFloatData(numStops * 5);
        fGradientOffsetCache.set(shader, dataPair.second);

        return dataPair;
    }

private:
    // Allocates the data for the requested number of bytes and returns the
    // pointer and buffer index offset the data will begin at.
    std::pair<float*, int> allocateFloatData(int size) {
        int lastSize = fGradientStorage.size();
        fGradientStorage.resize(lastSize + size);
        float* startPtr = fGradientStorage.begin() + lastSize;

        return std::make_pair(startPtr, lastSize);
    }

    SkDEBUGCODE(friend class UniformExpectationsValidator;)

    // Uniforms and textures are reset between draws but the PipelineDataGatherer is responsible
    // for combining one set of extracted "paint" uniforms+textures with N "renderstep" uniforms
    // and textures.
    // TODO: Right now paint uniforms and renderstep uniforms are bound separately so rewind() only
    // applies to the textures.
    UniformManager fUniformManager;
    skia_private::TArray<TextureDataBlock::SampledTexture> fTextures;
    int fPaintTextureCount = 0;

    // NOTE: This storage aggregates all data required by all draws within a DrawPass so that its
    // storage buffer can be bound once and accessed at random. It is not reset between draws like
    // the regular uniform manager or texture list.
    SkTDArray<float>  fGradientStorage;
    // Storing the address of the shader as a proxy for comparing the colors and offsets arrays to
    // keep lookup fast.
    skia_private::THashMap<const SkGradientBaseShader*, int> fGradientOffsetCache;
};

#ifdef SK_DEBUG
class UniformExpectationsValidator {
public:
    UniformExpectationsValidator(PipelineDataGatherer* gatherer,
                                 SkSpan<const Uniform> expectedUniforms,
                                 bool isSubstruct=false)
            : fGatherer(gatherer) {
        fGatherer->fUniformManager.setExpectedUniforms(expectedUniforms, isSubstruct);
    }

    ~UniformExpectationsValidator() {
        fGatherer->fUniformManager.doneWithExpectedUniforms();
    }

private:
    PipelineDataGatherer* fGatherer;

    UniformExpectationsValidator(UniformExpectationsValidator &&) = delete;
    UniformExpectationsValidator(const UniformExpectationsValidator &) = delete;
    UniformExpectationsValidator &operator=(UniformExpectationsValidator &&) = delete;
    UniformExpectationsValidator &operator=(const UniformExpectationsValidator &) = delete;
};
#endif // SK_DEBUG

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineData_DEFINED

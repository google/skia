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
#include "include/private/base/SkTDArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorData.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#include <optional>
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

    static UniformDataBlock Wrap(UniformManager* uniforms, size_t subspanOffset) {
        return UniformDataBlock(uniforms->finish(subspanOffset));
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
    // 1 << SkNextLog2_portable(DrawList::kMaxRenderSteps);
    static constexpr Index kInvalidIndex = 4096;

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

    bool contains(K data) const { return SkToBool(fDataToIndex.find(data)); }

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

    bool hasTexture(const TextureProxy* texture) const {
        // The template for TextureProxyCache uses `TextureProxy*` because `sk_sp` does not
        // take a const pointer; this contains() check just uses the address and doesn't do
        // anything that actually requires it to be non-const.
        return fTextures.storage().fUniqueTextures.contains(
                const_cast<TextureProxy*>(texture));
    }

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

    // Fully resets both uniforms (paint and renderstep)and textures
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
    }
#endif // SK_DEBUG

    // Mark the end of extracting paint uniforms and textures. This finalizes the paint uniform
    // data, pads it correctly, and records its size.
    UniformDataBlock endPaintData() {
        fPaintTextureCount = fTextures.size();
        auto paintData = UniformDataBlock::Wrap(&fUniformManager, /*subspanOffset=*/0);
        fUniformManager.markNewOffset(); // No-op if already assigned
        return paintData;
    }

    // Mark the end of extracting uniforms and textures from a RenderStep.
    std::pair<UniformDataBlock, TextureDataBlock> endRenderStepData(bool performsShading) {
        SkSpan<const TextureDataBlock::SampledTexture> textures{fTextures};
        if (!performsShading) {
            textures = textures.subspan(fPaintTextureCount);
        }
        // By this point, the fStartingOffset of the uniform manager will have advanced to the end
        // of the paint uniforms, so calling wrap now returns the renderstep uniforms.
        return {UniformDataBlock::Wrap(&fUniformManager), TextureDataBlock(textures)};
    }

    // Rewind to collect data for another RenderStep using the same paint data.
    void rewindForRenderStep() {
        fTextures.resize_back(fPaintTextureCount);
        fUniformManager.rewindToOffset();
    }

    // Append a sampled texture.
    void add(sk_sp<TextureProxy> proxy, const SamplerDesc& samplerDesc) {
        fTextures.push_back({std::move(proxy), samplerDesc});
    }

    void tryShrinkCapacity() {
        SkDEBUGCODE(this->checkReset());
        fUniformManager.tryShrinkCapacity();
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

    SkDEBUGCODE(UniformManager* uniformManager() { return &fUniformManager; })
private:
    SkDEBUGCODE(friend class UniformExpectationsValidator;)

    UniformManager fUniformManager;
    skia_private::TArray<TextureDataBlock::SampledTexture> fTextures;

    int fPaintTextureCount = 0;
};

#ifdef SK_DEBUG
class UniformExpectationsValidator {
public:
    UniformExpectationsValidator(PipelineDataGatherer* gatherer,
                                 SkSpan<const Uniform> expectedUniforms,
                                 bool isSubstruct = false)
            : fManager(gatherer->uniformManager()) {
        fManager->setExpectedUniforms(expectedUniforms, isSubstruct);
    }

    ~UniformExpectationsValidator() {
        fManager->doneWithExpectedUniforms();
    }

private:
    UniformManager* fManager;

    UniformExpectationsValidator(UniformExpectationsValidator&&) = delete;
    UniformExpectationsValidator(const UniformExpectationsValidator&) = delete;
    UniformExpectationsValidator& operator=(UniformExpectationsValidator&&) = delete;
    UniformExpectationsValidator& operator=(const UniformExpectationsValidator&) = delete;
};
#endif // SK_DEBUG

/**
 * Aggregates gradient color and stop information into a single buffer to be bound once for a
 * DrawPass. It de-duplicates gradient data by caching based on the SkGradientBaseShader pointer.
 */
class FloatStorageManager : public SkRefCnt {
public:
    FloatStorageManager() = default;

    void reset() {
        fGradientStorage.clear();
        fGradientOffsetCache.reset();
    }

    // Checks if data already exists for the requested gradient shader. If so, it returns
    // a nullptr and the existing offset. If not, it allocates space, caches the offset,
    // and returns a pointer to the start of the new data and the calculated offset.
    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader) {
        SkASSERT(!this->isFinalized());
        int* existingOffset = fGradientOffsetCache.find(shader->uniqueID());
        if (existingOffset) {
            return {nullptr, *existingOffset};
        }
        auto [ptr, offset] = this->allocateFloatData(numStops * 5); // 4 for color, 1 for offset
        fGradientOffsetCache.set(shader->uniqueID(), offset);

        return {ptr, offset};
    }

    bool finalize(DrawBufferManager* bufferMgr) {
        SkASSERT(!this->isFinalized());
        if (!fGradientStorage.empty()) {
            auto [writer, bufferInfo, _] =
                    bufferMgr->getMappedStorageBuffer(fGradientStorage.size(), sizeof(float));
            if (writer) {
                writer.write(fGradientStorage.data(), fGradientStorage.size_bytes());
                fBufferInfo = bufferInfo;
                this->reset();
            } else {
                return false;
            }
        } else {
            fBufferInfo = BindBufferInfo();
        }
        return true;
    }

    BindBufferInfo getBufferInfo() { return fBufferInfo.value(); }
    bool hasData() const { return fBufferInfo.has_value() &&
                                  fBufferInfo.value().fBuffer != nullptr; }
    SkDEBUGCODE(bool isFinalized() const { return fBufferInfo.has_value(); })
private:
    // Allocates space for a given number of floats and returns a pointer to the start
    // of the new allocation and its offset from the beginning of the buffer.
    std::pair<float*, int> allocateFloatData(int floatCount) {
        int currentSize = fGradientStorage.size();
        fGradientStorage.resize(currentSize + floatCount);
        float* startPtr = fGradientStorage.begin() + currentSize;

        return {startPtr, currentSize};
    }

    // NOTE: This storage aggregates all data required by all draws within a DrawPass so that its
    // storage buffer can be bound once and accessed at random.
    SkTDArray<float> fGradientStorage;

    // We use the shader's unique ID as a key to de-duplicate gradient data.
    skia_private::THashMap<uint32_t, int> fGradientOffsetCache;

    std::optional<BindBufferInfo> fBufferInfo = std::nullopt;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineData_DEFINED

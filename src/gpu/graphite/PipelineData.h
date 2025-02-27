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

// Add a block of data to the cache and return a stable pointer to the contents (assuming that a
// resettable gatherer had accumulated the input data pointer).
//
// If an identical block of data is already in the cache, that existing pointer is returned, making
// pointer comparison suitable when comparing data blocks retrieved from the cache.
//
// T must define a Hash struct function, an operator==, and a static Make(T, SkArenaAlloc*)
// factory that's used to copy the data into an arena allocation owned by the PipelineDataCache.
template<typename T>
class PipelineDataCache {
public:
    PipelineDataCache() = default;

    T insert(T dataBlock) {
        const T* existing = fData.find(dataBlock);
        if (existing) {
            return *existing;
        } else {
            // Need to make a copy of dataBlock into the arena
            T copy = T::Make(dataBlock, &fArena);
            fData.add(copy);
            return copy;
        }
    }

    // The number of unique T objects in the cache
    int count() const {
        return fData.count();
    }

    // Call fn on every item in the set.  You may not mutate anything.
    template <typename Fn>  // f(T), f(const T&)
    void foreach(Fn&& fn) const {
        fData.foreach(fn);
    }

private:
    skia_private::THashSet<T, typename T::Hash> fData;
    // Holds the data that is pointed to by the span keys in fData
    SkArenaAlloc fArena{0};
};

// A TextureDataCache only lives for a single Recording. When a Recording is snapped it is pulled
// off of the Recorder and goes with the Recording as a record of the required Textures and
// Samplers.
using TextureDataCache = PipelineDataCache<TextureDataBlock>;

// A UniformDataCache is used to deduplicate uniform data blocks uploaded to uniform / storage
// buffers for a DrawPass pipeline.
// TODO: This is just a combination of PipelineDataCache and DrawPass's DenseBiMap, ideally we can
// merge those two classes rather than defining this new class.
class UniformDataCache {
public:
    using Index = uint32_t;
    static constexpr Index kInvalidIndex{1 << SkNextLog2_portable(DrawList::kMaxRenderSteps)};

    // Tracks uniform data on the CPU and then its transition to storage in a GPU buffer (UBO or
    // SSBO).
    struct Entry {
        UniformDataBlock fCpuData;
        BindBufferInfo fBufferBinding;

        // Can only be initialized with CPU data.
        Entry(UniformDataBlock cpuData) : fCpuData(cpuData) {}
    };

    UniformDataCache() = default;

    Index insert(const UniformDataBlock& dataBlock) {
        Index* index = fDataToIndex.find(dataBlock);
        if (!index) {
            // Need to make a copy of dataBlock into the arena
            UniformDataBlock copy = UniformDataBlock::Make(dataBlock, &fArena);
            SkASSERT(SkToU32(fIndexToData.size()) < kInvalidIndex);
            index = fDataToIndex.set(copy, static_cast<Index>(fIndexToData.size()));
            fIndexToData.push_back(Entry{copy});
        }
        return *index;
    }

    const Entry& lookup(Index index) const {
        SkASSERT(index < kInvalidIndex);
        return fIndexToData[index];
    }

    Entry& lookup(Index index) {
        SkASSERT(index < kInvalidIndex);
        return fIndexToData[index];
    }

#if defined(GPU_TEST_UTILS)
    int count() { return fIndexToData.size(); }
#endif

private:
    skia_private::THashMap<UniformDataBlock, Index, UniformDataBlock::Hash> fDataToIndex;
    skia_private::TArray<Entry> fIndexToData;

    // Holds the de-duplicated data.
    SkArenaAlloc fArena{0};
};

// The PipelineDataGatherer is just used to collect information for a given PaintParams object.
//   The UniformData is added to a cache and uniquified. Only that unique ID is passed around.
//   The TextureData is also added to a cache and uniquified. Only that ID is passed around.

// TODO: The current plan for fixing uniform padding is for the PipelineDataGatherer to hold a
// persistent uniformManager. A stretch goal for this system would be for this combination
// to accumulate all the uniforms and then rearrange them to minimize padding. This would,
// obviously, vastly complicate uniform accumulation.
class PipelineDataGatherer {
public:
    PipelineDataGatherer(Layout layout) : fUniformManager(layout) {}

    void resetWithNewLayout(Layout layout) {
        fUniformManager.resetWithNewLayout(layout);
        fTextures.clear();
    }

#if defined(SK_DEBUG)
    // Check that the gatherer has been reset to its initial state prior to collecting new data.
    void checkReset() const {
        SkASSERT(fTextures.empty());
        SkASSERT(fUniformManager.isReset());
    }
#endif // SK_DEBUG

    void add(sk_sp<TextureProxy> proxy, const SamplerDesc& samplerDesc) {
        fTextures.push_back({std::move(proxy), samplerDesc});
    }
    bool hasTextures() const { return !fTextures.empty(); }

    TextureDataBlock textureDataBlock() { return TextureDataBlock(SkSpan(fTextures)); }

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

    bool hasUniforms() const { return fUniformManager.size(); }

    bool hasGradientBufferData() const { return !fGradientStorage.empty(); }

    SkSpan<const float> gradientBufferData() const { return fGradientStorage; }

    // Returns the uniform data written so far. Will automatically pad the end of the data as needed
    // to the overall required alignment, and so should only be called when all writing is done.
    UniformDataBlock finishUniformDataBlock() { return UniformDataBlock::Wrap(&fUniformManager); }

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

    UniformManager fUniformManager;
    skia_private::TArray<TextureDataBlock::SampledTexture> fTextures;

    SkTDArray<float>  fGradientStorage;
    // Storing the address of the shader as a proxy for comparing
    // the colors and offsets arrays to keep lookup fast.
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

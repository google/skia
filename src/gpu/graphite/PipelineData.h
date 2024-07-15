/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineData_DEFINED
#define skgpu_graphite_PipelineData_DEFINED

#include <vector>
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkColorData.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

class SkArenaAlloc;

namespace skgpu::graphite {

class Uniform;

class UniformDataBlock {
public:
    static UniformDataBlock* Make(const UniformDataBlock&, SkArenaAlloc*);

    UniformDataBlock(SkSpan<const char> data) : fData(data) {}
    UniformDataBlock() = default;

    const char* data() const { return fData.data(); }
    size_t size() const { return fData.size(); }

    uint32_t hash() const;

    bool operator==(const UniformDataBlock& that) const {
        return fData.size() == that.fData.size() &&
               !memcmp(fData.data(), that.fData.data(), fData.size());
    }
    bool operator!=(const UniformDataBlock& that) const { return !(*this == that); }

private:
    SkSpan<const char> fData;
};

class TextureDataBlock {
public:
    using SampledTexture = std::pair<sk_sp<TextureProxy>, SamplerDesc>;

    static TextureDataBlock* Make(const TextureDataBlock&, SkArenaAlloc*);
    TextureDataBlock() = default;

    bool empty() const { return fTextureData.empty(); }
    int numTextures() const { return SkTo<int>(fTextureData.size()); }
    const SampledTexture& texture(int index) const { return fTextureData[index]; }

    bool operator==(const TextureDataBlock&) const;
    bool operator!=(const TextureDataBlock& other) const { return !(*this == other);  }
    uint32_t hash() const;

    void add(sk_sp<TextureProxy> proxy, const SamplerDesc& samplerDesc) {
        fTextureData.push_back({std::move(proxy), samplerDesc});
    }

    void reset() {
        fTextureData.clear();
    }

private:
    // TODO: Move this into a SkSpan that's managed by the gatherer or copied into the arena.
    std::vector<SampledTexture> fTextureData;
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
    PipelineDataGatherer(Layout layout);

    void resetWithNewLayout(Layout layout);

    // Check that the gatherer has been reset to its initial state prior to collecting new data.
    SkDEBUGCODE(void checkReset();)

    void add(sk_sp<TextureProxy> proxy, const SamplerDesc& samplerDesc) {
        fTextureDataBlock.add(std::move(proxy), samplerDesc);
    }
    bool hasTextures() const { return !fTextureDataBlock.empty(); }

    const TextureDataBlock& textureDataBlock() { return fTextureDataBlock; }

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
    UniformDataBlock finishUniformDataBlock() { return UniformDataBlock(fUniformManager.finish()); }

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

    TextureDataBlock  fTextureDataBlock;
    UniformManager    fUniformManager;

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

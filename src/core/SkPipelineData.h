/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipelineData_DEFINED
#define SkPipelineData_DEFINED

#include <vector>
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkColorData.h"
#include "src/core/SkEnumBitMask.h"
#include "src/core/SkUniform.h"

#ifdef SK_GRAPHITE_ENABLED
#include "include/private/SkVx.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#endif

class SkArenaAlloc;
class SkUniform;

namespace skgpu::graphite {
enum class SnippetRequirementFlags : uint32_t;
}

class SkUniformDataBlock {
public:
    static SkUniformDataBlock* Make(const SkUniformDataBlock&, SkArenaAlloc*);

    SkUniformDataBlock(SkSpan<const char> data) : fData(data) {}
    SkUniformDataBlock() = default;

    const char* data() const { return fData.data(); }
    size_t size() const { return fData.size(); }

    uint32_t hash() const;

    bool operator==(const SkUniformDataBlock& that) const {
        return fData.size() == that.fData.size() &&
               !memcmp(fData.data(), that.fData.data(), fData.size());
    }
    bool operator!=(const SkUniformDataBlock& that) const { return !(*this == that); }

private:
    SkSpan<const char> fData;
};

#ifdef SK_GRAPHITE_ENABLED
class SkTextureDataBlock {
public:
    using SampledTexture = std::pair<sk_sp<skgpu::graphite::TextureProxy>,
                                     skgpu::graphite::SamplerDesc>;

    static SkTextureDataBlock* Make(const SkTextureDataBlock&, SkArenaAlloc*);
    SkTextureDataBlock() = default;

    bool empty() const { return fTextureData.empty(); }
    int numTextures() const { return SkTo<int>(fTextureData.size()); }
    const SampledTexture& texture(int index) const { return fTextureData[index]; }

    bool operator==(const SkTextureDataBlock&) const;
    bool operator!=(const SkTextureDataBlock& other) const { return !(*this == other);  }
    uint32_t hash() const;

    void add(const SkSamplingOptions& sampling,
             const SkTileMode tileModes[2],
             sk_sp<skgpu::graphite::TextureProxy> proxy) {
        fTextureData.push_back({std::move(proxy), {sampling, {tileModes[0], tileModes[1]}}});
    }

    void reset() {
        fTextureData.clear();
    }

private:
    // TODO: Move this into a SkSpan that's managed by the gatherer or copied into the arena.
    std::vector<SampledTexture> fTextureData;
};
#endif // SK_GRAPHITE_ENABLED

// The PipelineDataGatherer is just used to collect information for a given PaintParams object.
//   The UniformData is added to a cache and uniquified. Only that unique ID is passed around.
//   The TextureData is also added to a cache and uniquified. Only that ID is passed around.

// TODO: The current plan for fixing uniform padding is for the SkPipelineDataGatherer to hold a
// persistent uniformManager. A stretch goal for this system would be for this combination
// to accumulate all the uniforms and then rearrange them to minimize padding. This would,
// obviously, vastly complicate uniform accumulation.
class SkPipelineDataGatherer {
public:
#ifdef SK_GRAPHITE_ENABLED
    SkPipelineDataGatherer(skgpu::graphite::Layout layout);
#endif

    void reset();
    // Check that the gatherer has been reset to its initial state prior to collecting new data.
    SkDEBUGCODE(void checkReset();)

#ifdef SK_GRAPHITE_ENABLED
    void add(const SkSamplingOptions& sampling,
             const SkTileMode tileModes[2],
             sk_sp<skgpu::graphite::TextureProxy> proxy) {
        fTextureDataBlock.add(sampling, tileModes, std::move(proxy));
    }
    bool hasTextures() const { return !fTextureDataBlock.empty(); }
#endif // SK_GRAPHITE_ENABLED

    void addFlags(SkEnumBitMask<skgpu::graphite::SnippetRequirementFlags> flags);
    bool needsLocalCoords() const;

#ifdef SK_GRAPHITE_ENABLED
    const SkTextureDataBlock& textureDataBlock() { return fTextureDataBlock; }

    void write(const SkM44& mat) { fUniformManager.write(mat); }
    void write(const SkPMColor4f& premulColor) { fUniformManager.write(premulColor); }
    void write(const SkRect& rect) { fUniformManager.write(rect); }
    void write(SkPoint point) { fUniformManager.write(point); }
    void write(float f) { fUniformManager.write(f); }
    void write(int i) { fUniformManager.write(i); }
    void write(skvx::float2 v) { fUniformManager.write(v); }
    void write(skvx::float4 v) { fUniformManager.write(v); }

    void write(SkSLType t, const void* data) { fUniformManager.write(t, data); }
    void write(const SkUniform& u, const uint8_t* data) { fUniformManager.write(u, data); }

    void writeArray(SkSpan<const SkColor4f> colors) { fUniformManager.writeArray(colors); }
    void writeArray(SkSpan<const float> floats) { fUniformManager.writeArray(floats); }

    bool hasUniforms() const { return fUniformManager.size(); }

    // Returns the uniform data written so far. Will automatically pad the end of the data as needed
    // to the overall required alignment, and so should only be called when all writing is done.
    SkUniformDataBlock finishUniformDataBlock() { return fUniformManager.finishUniformDataBlock(); }

private:
#ifdef SK_DEBUG
    friend class UniformExpectationsValidator;

    void setExpectedUniforms(SkSpan<const SkUniform> expectedUniforms) {
        fUniformManager.setExpectedUniforms(expectedUniforms);
    }
    void doneWithExpectedUniforms() { fUniformManager.doneWithExpectedUniforms(); }
#endif // SK_DEBUG

    SkTextureDataBlock                     fTextureDataBlock;
    skgpu::graphite::UniformManager        fUniformManager;
    SkEnumBitMask<skgpu::graphite::SnippetRequirementFlags> fSnippetRequirementFlags;
#endif // SK_GRAPHITE_ENABLED
};

#if defined(SK_DEBUG) && defined(SK_GRAPHITE_ENABLED)
class UniformExpectationsValidator {
public:
    UniformExpectationsValidator(SkPipelineDataGatherer *gatherer,
                                 SkSpan<const SkUniform> expectedUniforms)
            : fGatherer(gatherer) {
        fGatherer->setExpectedUniforms(expectedUniforms);
    }

    ~UniformExpectationsValidator() {
        fGatherer->doneWithExpectedUniforms();
    }

private:
    SkPipelineDataGatherer *fGatherer;

    UniformExpectationsValidator(UniformExpectationsValidator &&) = delete;
    UniformExpectationsValidator(const UniformExpectationsValidator &) = delete;
    UniformExpectationsValidator &operator=(UniformExpectationsValidator &&) = delete;
    UniformExpectationsValidator &operator=(const UniformExpectationsValidator &) = delete;
};
#endif // SK_DEBUG && SK_GRAPHITE_ENABLED

#endif // SkPipelineData_DEFINED

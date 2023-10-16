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
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"

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

    void add(const SkSamplingOptions& sampling,
             const SkTileMode tileModes[2],
             sk_sp<TextureProxy> proxy) {
        fTextureData.push_back({std::move(proxy), SamplerDesc{sampling, tileModes}});
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

    void add(const SkSamplingOptions& sampling,
             const SkTileMode tileModes[2],
             sk_sp<TextureProxy> proxy) {
        fTextureDataBlock.add(sampling, tileModes, std::move(proxy));
    }
    bool hasTextures() const { return !fTextureDataBlock.empty(); }

    const TextureDataBlock& textureDataBlock() { return fTextureDataBlock; }

    void write(const SkM44& mat) { fUniformManager.write(mat); }
    void write(const SkMatrix& mat) { fUniformManager.write(mat); }
    void write(const SkPMColor4f& premulColor) { fUniformManager.write(premulColor); }
    void writePaintColor(const SkPMColor4f& premulColor) {
        fUniformManager.writePaintColor(premulColor);
    }
    void write(const SkRect& rect) { fUniformManager.write(rect); }
    void write(const SkV2& v) { fUniformManager.write(v); }
    void write(const SkV4& v) { fUniformManager.write(v); }
    void write(const SkSize& size) { fUniformManager.write(size); }
    void write(const SkPoint& point) { fUniformManager.write(point); }
    void write(const SkPoint3& point3) { fUniformManager.write(point3); }
    void write(float f) { fUniformManager.write(f); }
    void write(int i) { fUniformManager.write(i); }

    void write(SkSLType t, const void* data) { fUniformManager.write(t, data); }
    void write(const Uniform& u, const uint8_t* data) { fUniformManager.write(u, data); }

    void writeArray(SkSLType t, const void* data, int n) { fUniformManager.writeArray(t, data, n); }
    void writeArray(SkSpan<const SkColor4f> colors) { fUniformManager.writeArray(colors); }
    void writeArray(SkSpan<const SkPMColor4f> colors) { fUniformManager.writeArray(colors); }
    void writeArray(SkSpan<const float> floats) { fUniformManager.writeArray(floats); }

    void writeHalf(float f) { fUniformManager.writeHalf(f); }
    void writeHalf(const SkMatrix& mat) { fUniformManager.writeHalf(mat); }
    void writeHalf(const SkM44& mat) { fUniformManager.writeHalf(mat); }
    void writeHalf(const SkColor4f& unpremulColor) { fUniformManager.writeHalf(unpremulColor); }
    void writeHalfArray(SkSpan<const float> floats) { fUniformManager.writeHalfArray(floats); }

    bool hasUniforms() const { return fUniformManager.size(); }

    // Returns the uniform data written so far. Will automatically pad the end of the data as needed
    // to the overall required alignment, and so should only be called when all writing is done.
    UniformDataBlock finishUniformDataBlock() { return fUniformManager.finishUniformDataBlock(); }

private:
#ifdef SK_DEBUG
    friend class UniformExpectationsValidator;

    void setExpectedUniforms(SkSpan<const Uniform> expectedUniforms);
    void doneWithExpectedUniforms() { fUniformManager.doneWithExpectedUniforms(); }
#endif // SK_DEBUG

    TextureDataBlock                       fTextureDataBlock;
    UniformManager                         fUniformManager;
};

#ifdef SK_DEBUG
class UniformExpectationsValidator {
public:
    UniformExpectationsValidator(PipelineDataGatherer *gatherer,
                                 SkSpan<const Uniform> expectedUniforms);

    ~UniformExpectationsValidator() {
        fGatherer->doneWithExpectedUniforms();
    }

private:
    PipelineDataGatherer *fGatherer;

    UniformExpectationsValidator(UniformExpectationsValidator &&) = delete;
    UniformExpectationsValidator(const UniformExpectationsValidator &) = delete;
    UniformExpectationsValidator &operator=(UniformExpectationsValidator &&) = delete;
    UniformExpectationsValidator &operator=(const UniformExpectationsValidator &) = delete;
};
#endif // SK_DEBUG

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineData_DEFINED

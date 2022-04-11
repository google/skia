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

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/VectorTypes.h"
#endif

class SkArenaAlloc;
class SkUniform;

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

// We would like to store just a "const SkUniformDataBlock*" in the UniformDataCache but, until
// the TextureDataCache is switched over to storing its data in an arena, whatever is held in
// the cache must interoperate w/ std::unique_ptr (i.e., have a get() function).
// TODO: remove this class
class SkUniformDataBlockPassThrough {
public:
    SkUniformDataBlockPassThrough() = default;
    SkUniformDataBlockPassThrough(SkUniformDataBlock* udb) : fUDB(udb) {}

    SkUniformDataBlock* get() const { return fUDB; }

private:
    SkUniformDataBlock* fUDB = nullptr;
};

#ifdef SK_GRAPHITE_ENABLED
class SkTextureDataBlock {
public:
    struct TextureInfo {
        bool operator==(const TextureInfo&) const;
        bool operator!=(const TextureInfo& other) const { return !(*this == other);  }

        uint32_t samplerKey() const;

        sk_sp<skgpu::graphite::TextureProxy> fProxy;
        SkSamplingOptions                    fSamplingOptions;
        SkTileMode                           fTileModes[2];
    };

    static std::unique_ptr<SkTextureDataBlock> Make(const SkTextureDataBlock&, SkArenaAlloc*);
    SkTextureDataBlock() = default;

    bool empty() const { return fTextureData.empty(); }
    int numTextures() const { return SkTo<int>(fTextureData.size()); }
    const TextureInfo& texture(int index) const { return fTextureData[index]; }

    bool operator==(const SkTextureDataBlock&) const;
    bool operator!=(const SkTextureDataBlock& other) const { return !(*this == other);  }
    uint32_t hash() const;

    void add(const SkSamplingOptions& sampling,
             const SkTileMode tileModes[2],
             sk_sp<skgpu::graphite::TextureProxy> proxy) {
        fTextureData.push_back({std::move(proxy), sampling, {tileModes[0], tileModes[1]}});
    }

    void reset() {
        fTextureData.clear();
    }

private:
    std::vector<TextureInfo> fTextureData;
};
#endif // SK_GRAPHITE_ENABLED

// The PipelineDataGatherer is just used to collect information for a given PaintParams object.
//   The UniformData is added to a cache and uniquified. Only that unique ID is passed around.
//   The TextureData is also added to a cache and uniquified. Only that ID is passed around.
//   The BlendInfo is ultimately stored in the SkShaderCodeDictionary next to its associated
//       PaintParamsKey

// TODO: The current plan for fixing uniform padding is for the SkPipelineDataGatherer to hold a
// persistent uniformManager. A stretch goal for this system would be for this combination
// to accumulate all the uniforms and then rearrange them to minimize padding. This would,
// obviously, vastly complicate uniform accumulation.
class SkPipelineDataGatherer {
public:
#ifdef SK_GRAPHITE_ENABLED
    struct BlendInfo {
        bool operator==(const BlendInfo& other) const {
            return fEquation == other.fEquation &&
                   fSrcBlend == other.fSrcBlend &&
                   fDstBlend == other.fDstBlend &&
                   fBlendConstant == other.fBlendConstant &&
                   fWritesColor == other.fWritesColor;
        }

        skgpu::BlendEquation fEquation = skgpu::BlendEquation::kAdd;
        skgpu::BlendCoeff    fSrcBlend = skgpu::BlendCoeff::kOne;
        skgpu::BlendCoeff    fDstBlend = skgpu::BlendCoeff::kZero;
        SkPMColor4f          fBlendConstant = SK_PMColor4fTRANSPARENT;
        bool                 fWritesColor = true;
    };
#endif

#ifdef SK_GRAPHITE_ENABLED
    SkPipelineDataGatherer(skgpu::graphite::Layout layout) : fUniformManager(layout) {}
#endif

    void reset();
    // Check that the gatherer has been reset to its initial state prior to collecting new data.
    SkDEBUGCODE(void checkReset();)

#ifdef SK_GRAPHITE_ENABLED
    void setBlendInfo(const SkPipelineDataGatherer::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const BlendInfo& blendInfo() const { return fBlendInfo; }

    void add(const SkSamplingOptions& sampling,
             const SkTileMode tileModes[2],
             sk_sp<skgpu::graphite::TextureProxy> proxy) {
        fTextureDataBlock.add(sampling, tileModes, std::move(proxy));
    }
    bool hasTextures() const { return !fTextureDataBlock.empty(); }

    const SkTextureDataBlock& textureDataBlock() { return fTextureDataBlock; }

    void write(const SkColor4f* colors, int numColors) { fUniformManager.write(colors, numColors); }
    void write(const SkPMColor4f& premulColor) { fUniformManager.write(&premulColor, 1); }
    void write(const SkRect& rect) { fUniformManager.write(rect); }
    void write(SkPoint point) { fUniformManager.write(point); }
    void write(const float* floats, int count) { fUniformManager.write(floats, count); }
    void write(float f) { fUniformManager.write(&f, 1); }
    void write(int i) { fUniformManager.write(i); }
    void write(skgpu::graphite::float2 v) { fUniformManager.write(v); }

    bool hasUniforms() const { return fUniformManager.size(); }

    SkUniformDataBlock peekUniformData() const { return fUniformManager.peekData(); }

private:
#ifdef SK_DEBUG
    friend class UniformExpectationsValidator;

    void setExpectedUniforms(SkSpan<const SkUniform> expectedUniforms) {
        fUniformManager.setExpectedUniforms(expectedUniforms);
    }
    void doneWithExpectedUniforms() { fUniformManager.doneWithExpectedUniforms(); }
#endif // SK_DEBUG

    SkTextureDataBlock              fTextureDataBlock;
    BlendInfo                       fBlendInfo;
    skgpu::graphite::UniformManager fUniformManager;
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

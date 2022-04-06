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
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/VectorTypes.h"
#include "src/gpu/Blend.h"
#endif

class SkArenaAlloc;
class SkUniform;

class SkUniformDataBlock {
public:
    static std::unique_ptr<SkUniformDataBlock> Make(const SkUniformDataBlock&, SkArenaAlloc*);

    SkUniformDataBlock(SkSpan<const char> data, bool ownMem) : fData(data), fOwnMem(ownMem) {}
    SkUniformDataBlock() = default;
    ~SkUniformDataBlock() {
        if (fOwnMem) {
            delete [] fData.data();
        }
    }

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

    // This is only required until the uniform data is stored in the arena. Once there this
    // class will never delete the data referenced w/in the span
    bool fOwnMem = false;
};

#ifdef SK_GRAPHITE_ENABLED
class SkTextureDataBlock {
public:
    struct TextureInfo {
        bool operator==(const TextureInfo&) const;
        bool operator!=(const TextureInfo& other) const { return !(*this == other);  }

        uint32_t samplerKey() const;

        sk_sp<skgpu::TextureProxy> fProxy;
        SkSamplingOptions          fSamplingOptions;
        SkTileMode                 fTileModes[2];
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
             sk_sp<skgpu::TextureProxy> proxy) {
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
    SkPipelineDataGatherer(skgpu::Layout layout) : fUniformManager(layout) {}
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
             sk_sp<skgpu::TextureProxy> proxy) {
        fTextureDataBlock.add(sampling, tileModes, std::move(proxy));
    }
    bool hasTextures() const { return !fTextureDataBlock.empty(); }

    const SkTextureDataBlock& textureDataBlock() { return fTextureDataBlock; }

#ifdef SK_DEBUG
    void setExpectedUniforms(SkSpan<const SkUniform> expectedUniforms) {
        fUniformManager.setExpectedUniforms(expectedUniforms);
    }
    void doneWithExpectedUniforms() { fUniformManager.doneWithExpectedUniforms(); }
#endif // SK_DEBUG

    void write(const SkColor4f* colors, int numColors) { fUniformManager.write(colors, numColors); }
    void write(const SkPMColor4f& premulColor) { fUniformManager.write(&premulColor, 1); }
    void write(const SkRect& rect) { fUniformManager.write(rect); }
    void write(SkPoint point) { fUniformManager.write(point); }
    void write(const float* floats, int count) { fUniformManager.write(floats, count); }
    void write(float something) { fUniformManager.write(&something, 1); }
    void write(int something) { fUniformManager.write(something); }
    void write(skgpu::float2 something) { fUniformManager.write(something); }

    bool hasUniforms() const { return fUniformManager.size(); }

    SkUniformDataBlock peekUniformData() const { return fUniformManager.peekData(); }

private:
    SkTextureDataBlock    fTextureDataBlock;
    BlendInfo             fBlendInfo;
    skgpu::UniformManager fUniformManager;
#endif // SK_GRAPHITE_ENABLED
};

#endif // SkPipelineData_DEFINED

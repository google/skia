/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipelineData_DEFINED
#define SkPipelineData_DEFINED

#include <vector>
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkColorData.h"
#include "src/core/SkUniformData.h"

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/UniformManager.h"
#include "src/gpu/Blend.h"
#endif

class SkUniformDataBlock {
public:
    SkUniformDataBlock() = default;
    SkUniformDataBlock(sk_sp<SkUniformData> initial) {
        SkASSERT(initial && initial->dataSize());
        fUniformData.push_back(std::move(initial));
    }

    bool empty() const { return fUniformData.empty(); }
    size_t totalUniformSize() const;  // TODO: cache this?

    bool operator==(const SkUniformDataBlock&) const;
    bool operator!=(const SkUniformDataBlock& other) const { return !(*this == other);  }
    uint32_t hash() const;

    using container = std::vector<sk_sp<SkUniformData>>;
    using iterator = container::iterator;

    inline iterator begin() noexcept { return fUniformData.begin(); }
    inline iterator end() noexcept { return fUniformData.end(); }

    void add(sk_sp<SkUniformData> ud) {
        fUniformData.push_back(std::move(ud));
    }

    void reset() {
        fUniformData.clear();
    }

private:
    // TODO: SkUniformData should be held uniquely
    std::vector<sk_sp<SkUniformData>> fUniformData;
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

    SkTextureDataBlock() = default;

    bool empty() const { return fTextureData.empty(); }
    int numTextures() const { return SkTo<int>(fTextureData.size()); }
    const TextureInfo& texture(int index) { return fTextureData[index]; }

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
    SkPipelineDataGatherer(skgpu::Layout layout) : fLayout(layout) {}
#endif

    void reset();
    // Check that the gatherer has been reset to its initial state prior to collecting new data.
    SkDEBUGCODE(void checkReset();)

#ifdef SK_GRAPHITE_ENABLED
    skgpu::Layout layout() const { return fLayout; }

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
#endif

    void add(sk_sp<SkUniformData>);
    bool hasUniforms() const { return !fUniformDataBlock.empty(); }

    SkUniformDataBlock& uniformDataBlock() { return fUniformDataBlock; }

private:
    SkUniformDataBlock fUniformDataBlock;

#ifdef SK_GRAPHITE_ENABLED
    SkTextureDataBlock fTextureDataBlock;
    BlendInfo        fBlendInfo;
    skgpu::Layout    fLayout;
#endif
};

#endif // SkPipelineData_DEFINED

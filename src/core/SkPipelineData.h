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
#include "src/gpu/Blend.h"
#endif

// TODO: The current plan for fixing uniform padding is for the SkPipelineData to hold a
// persistent uniformManager. A stretch goal for this system would be for this combination
// to accumulate all the uniforms and then rearrange them to minimize padding. This would,
// obviously, vastly complicate uniform accumulation.
class SkPipelineData {
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

    SkPipelineData() = default;
    SkPipelineData(sk_sp<SkUniformData> initial);

#ifdef SK_GRAPHITE_ENABLED
    void setBlendInfo(const SkPipelineData::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const BlendInfo& blendInfo() const { return fBlendInfo; }

    void addImage(const SkSamplingOptions&, const SkTileMode[2], sk_sp<skgpu::TextureProxy>);
#endif

    void add(sk_sp<SkUniformData>);

    bool hasUniforms() const { return !fUniformData.empty(); }
    size_t totalUniformSize() const;  // TODO: cache this?
    int numUniforms() const;         // TODO: cache this?

    bool operator==(const SkPipelineData&) const;
    bool operator!=(const SkPipelineData& other) const { return !(*this == other);  }
    size_t hash() const;

    using container = std::vector<sk_sp<SkUniformData>>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    inline iterator begin() noexcept { return fUniformData.begin(); }
    inline const_iterator cbegin() const noexcept { return fUniformData.cbegin(); }
    inline iterator end() noexcept { return fUniformData.end(); }
    inline const_iterator cend() const noexcept { return fUniformData.cend(); }

private:
    // TODO: SkUniformData should be held uniquely
    std::vector<sk_sp<SkUniformData>> fUniformData;

#ifdef SK_GRAPHITE_ENABLED
    struct TextureInfo {
        sk_sp<skgpu::TextureProxy> fProxy;
        SkSamplingOptions          fSamplingOptions;
        SkTileMode                 fTileModes[2];
    };

    std::vector<TextureInfo> fProxies;
    BlendInfo fBlendInfo;
#endif
};

#endif // SkPipelineData_DEFINED

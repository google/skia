/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/d3d/GrD3DBackendSurface.h"

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/d3d/GrD3DTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/d3d/GrD3DBackendSurfacePriv.h"
#include "src/gpu/ganesh/d3d/GrD3DResourceState.h"
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"

class GrD3DBackendFormatData final : public GrBackendFormatData {
public:
    GrD3DBackendFormatData(DXGI_FORMAT format) : fFormat(format) {}

    DXGI_FORMAT asDxgiFormat() const { return fFormat; }

private:
    SkTextureCompressionType compressionType() const override {
        switch (fFormat) {
            case DXGI_FORMAT_BC1_UNORM:
                return SkTextureCompressionType::kBC1_RGBA8_UNORM;
            default:
                return SkTextureCompressionType::kNone;
        }
    }

    size_t bytesPerBlock() const override { return GrDxgiFormatBytesPerBlock(fFormat); }

    int stencilBits() const override { return GrDxgiFormatStencilBits(fFormat); }

    uint32_t channelMask() const override { return GrDxgiFormatChannels(fFormat); }

    GrColorFormatDesc desc() const override { return GrDxgiFormatDesc(fFormat); }

    bool equal(const GrBackendFormatData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kDirect3D);
        if (auto otherD3D = static_cast<const GrD3DBackendFormatData*>(that)) {
            return fFormat == otherD3D->fFormat;
        }
        return false;
    }

    std::string toString() const override {
#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
        return GrDxgiFormatToStr(fFormat);
#else
        return "";
#endif
    }

    void copyTo(AnyFormatData& formatData) const override {
        formatData.emplace<GrD3DBackendFormatData>(fFormat);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kDirect3D; }
#endif

    DXGI_FORMAT fFormat;
};

static const GrD3DBackendFormatData* get_and_cast_data(const GrBackendFormat& format) {
    auto data = GrBackendSurfacePriv::GetBackendData(format);
    SkASSERT(!data || data->type() == GrBackendApi::kDirect3D);
    return static_cast<const GrD3DBackendFormatData*>(data);
}

namespace GrBackendFormats {

GrBackendFormat MakeD3D(DXGI_FORMAT format) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D, GrBackendApi::kDirect3D, GrD3DBackendFormatData(format));
}

DXGI_FORMAT AsDxgiFormat(const GrBackendFormat& format) {
    if (!format.isValid() || format.backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendFormat\n");
        return DXGI_FORMAT_UNKNOWN;
    }
    const GrD3DBackendFormatData* data = get_and_cast_data(format);
    SkASSERT(data);
    return data->asDxgiFormat();
}

}  // namespace GrBackendFormats

class GrD3DBackendTextureData final : public GrBackendTextureData {
public:
    GrD3DBackendTextureData(const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state)
            : fInfo(info, std::move(state)) {}

    GrD3DTextureResourceInfo snapTextureResourceInfo() const {
        return fInfo.snapTextureResourceInfo();
    }

    void setResourceState(GrD3DResourceStateEnum state) { fInfo.setResourceState(state); }

    sk_sp<GrD3DResourceState> getResourceState() const { return fInfo.getResourceState(); }

private:
    void copyTo(AnyTextureData& textureData) const override {
        textureData.emplace<GrD3DBackendTextureData>(this->snapTextureResourceInfo(),
                                                     this->getResourceState());
    }

    bool isProtected() const override { return false; }

    bool equal(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kDirect3D);
        if (auto otherD3D = static_cast<const GrD3DBackendTextureData*>(that)) {
            return fInfo == otherD3D->fInfo;
        }
        return false;
    }

    bool isSameTexture(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kDirect3D);
        if (auto otherD3D = static_cast<const GrD3DBackendTextureData*>(that)) {
            return fInfo.snapTextureResourceInfo().fResource ==
                   otherD3D->snapTextureResourceInfo().fResource;
        }
        return false;
    }

    GrBackendFormat getBackendFormat() const override {
        auto d3dInfo = this->snapTextureResourceInfo();
        return GrBackendFormats::MakeD3D(d3dInfo.fFormat);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kDirect3D; }
#endif

    GrD3DBackendSurfaceInfo fInfo;
};

static const GrD3DBackendTextureData* get_and_cast_data(const GrBackendTexture& texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kDirect3D);
    return static_cast<const GrD3DBackendTextureData*>(data);
}

static GrD3DBackendTextureData* get_and_cast_data(GrBackendTexture* texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kDirect3D);
    return static_cast<GrD3DBackendTextureData*>(data);
}

namespace GrBackendTextures {

GrBackendTexture MakeD3D(int width,
                         int height,
                         const GrD3DTextureResourceInfo& d3dInfo,
                         std::string_view label) {
    GrD3DBackendTextureData data(
            d3dInfo,
            sk_sp<GrD3DResourceState>(new GrD3DResourceState(
                    static_cast<D3D12_RESOURCE_STATES>(d3dInfo.fResourceState))));
    return GrBackendSurfacePriv::MakeGrBackendTexture(width,
                                                      height,
                                                      label,
                                                      skgpu::Mipmapped(d3dInfo.fLevelCount > 1),
                                                      GrBackendApi::kDirect3D,
                                                      GrTextureType::k2D,
                                                      data);
}

GrBackendTexture MakeD3D(int width,
                         int height,
                         const GrD3DTextureResourceInfo& d3dInfo,
                         sk_sp<GrD3DResourceState> state,
                         std::string_view label) {
    return GrBackendSurfacePriv::MakeGrBackendTexture(
            width,
            height,
            label,
            skgpu::Mipmapped(d3dInfo.fLevelCount > 1),
            GrBackendApi::kDirect3D,
            GrTextureType::k2D,
            GrD3DBackendTextureData(d3dInfo, std::move(state)));
}

GrD3DTextureResourceInfo GetD3DTextureResourceInfo(const GrBackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendTexture\n");
        return {};
    }
    const GrD3DBackendTextureData* data = get_and_cast_data(tex);
    SkASSERT(data);
    return data->snapTextureResourceInfo();
}

void SetD3DResourceState(GrBackendTexture* tex, GrD3DResourceStateEnum state) {
    SkASSERT(tex);
    if (!tex->isValid() || tex->backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendTexture\n");
        return;
    }
    GrD3DBackendTextureData* data = get_and_cast_data(tex);
    SkASSERT(data);
    data->setResourceState(state);
}

sk_sp<GrD3DResourceState> GetD3DResourceState(const GrBackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendTexture\n");
        return nullptr;
    }
    const GrD3DBackendTextureData* data = get_and_cast_data(tex);
    SkASSERT(data);
    return data->getResourceState();
}

}  // namespace GrBackendTextures

class GrD3DBackendRenderTargetData final : public GrBackendRenderTargetData {
public:
    GrD3DBackendRenderTargetData(const GrD3DTextureResourceInfo& info,
                                 sk_sp<GrD3DResourceState> state)
            : fInfo(info, std::move(state)) {}

    GrD3DTextureResourceInfo snapTextureResourceInfo() const {
        return fInfo.snapTextureResourceInfo();
    }

    void setResourceState(GrD3DResourceStateEnum state) { fInfo.setResourceState(state); }

    sk_sp<GrD3DResourceState> getResourceState() const { return fInfo.getResourceState(); }

private:
    void copyTo(AnyRenderTargetData& rtData) const override {
        rtData.emplace<GrD3DBackendRenderTargetData>(this->snapTextureResourceInfo(),
                                                     this->getResourceState());
    }

    bool isProtected() const override { return false; }

    bool equal(const GrBackendRenderTargetData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kDirect3D);
        if (auto otherD3D = static_cast<const GrD3DBackendRenderTargetData*>(that)) {
            return fInfo == otherD3D->fInfo;
        }
        return false;
    }

    GrBackendFormat getBackendFormat() const override {
        auto d3dInfo = this->snapTextureResourceInfo();
        return GrBackendFormats::MakeD3D(d3dInfo.fFormat);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kDirect3D; }
#endif

    GrD3DBackendSurfaceInfo fInfo;
};

static const GrD3DBackendRenderTargetData* get_and_cast_data(const GrBackendRenderTarget& texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kDirect3D);
    return static_cast<const GrD3DBackendRenderTargetData*>(data);
}

static GrD3DBackendRenderTargetData* get_and_cast_data(GrBackendRenderTarget* texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kDirect3D);
    return static_cast<GrD3DBackendRenderTargetData*>(data);
}

namespace GrBackendRenderTargets {

GrBackendRenderTarget MakeD3D(int width, int height, const GrD3DTextureResourceInfo& d3dInfo) {
    GrD3DBackendRenderTargetData data(
            d3dInfo,
            sk_sp<GrD3DResourceState>(new GrD3DResourceState(
                    static_cast<D3D12_RESOURCE_STATES>(d3dInfo.fResourceState))));
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(width,
                                                           height,
                                                           std::max(1U, d3dInfo.fSampleCount),
                                                           0, /*stencilBits*/
                                                           GrBackendApi::kDirect3D,
                                                           false, /*framebufferOnly*/
                                                           data);
}

GrBackendRenderTarget MakeD3D(int width,
                              int height,
                              const GrD3DTextureResourceInfo& d3dInfo,
                              sk_sp<GrD3DResourceState> state) {
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(
            width,
            height,
            std::max(1U, d3dInfo.fSampleCount),
            0, /*stencilBits*/
            GrBackendApi::kDirect3D,
            false, /*framebufferOnly*/
            GrD3DBackendRenderTargetData(d3dInfo, std::move(state)));
}

GrD3DTextureResourceInfo GetD3DTextureResourceInfo(const GrBackendRenderTarget& rt) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendRenderTarget\n");
        return {};
    }
    const GrD3DBackendRenderTargetData* data = get_and_cast_data(rt);
    SkASSERT(data);
    return data->snapTextureResourceInfo();
}

void SetD3DResourceState(GrBackendRenderTarget* rt, GrD3DResourceStateEnum state) {
    SkASSERT(rt);
    if (!rt->isValid() || rt->backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendRenderTarget\n");
        return;
    }
    GrD3DBackendRenderTargetData* data = get_and_cast_data(rt);
    SkASSERT(data);
    data->setResourceState(state);
}

sk_sp<GrD3DResourceState> GetD3DResourceState(const GrBackendRenderTarget& rt) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kDirect3D) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendRenderTarget\n");
        return nullptr;
    }
    const GrD3DBackendRenderTargetData* data = get_and_cast_data(rt);
    SkASSERT(data);
    return data->getResourceState();
}

}  // namespace GrBackendRenderTargets

/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mtl/GrMtlBackendSurface.h"

#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/mtl/GrMtlCppUtil.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

class GrMtlBackendFormatData final : public GrBackendFormatData {
public:
    GrMtlBackendFormatData(GrMTLPixelFormat format) : fFormat((MTLPixelFormat)format) {}

    GrMTLPixelFormat asMtlFormat() const { return (GrMTLPixelFormat)fFormat; }

private:
    SkTextureCompressionType compressionType() const override {
        return GrMtlFormatToCompressionType(fFormat);
    }

    size_t bytesPerBlock() const override { return skgpu::MtlFormatBytesPerBlock(fFormat); }

    int stencilBits() const override { return GrMtlFormatStencilBits(fFormat); }

    uint32_t channelMask() const override { return skgpu::MtlFormatChannels(fFormat); }

    GrColorFormatDesc desc() const override { return GrMtlFormatDesc(fFormat); }

    bool equal(const GrBackendFormatData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMetal);
        if (auto otherMtl = static_cast<const GrMtlBackendFormatData*>(that)) {
            return fFormat == otherMtl->fFormat;
        }
        return false;
    }

    std::string toString() const override {
#if defined(SK_DEBUG) || GR_TEST_UTILS
        return skgpu::MtlFormatToString(fFormat);
#else
        return "";
#endif
    }

    void copyTo(AnyFormatData& formatData) const override {
        formatData.emplace<GrMtlBackendFormatData>(fFormat);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMetal; }
#endif

    MTLPixelFormat fFormat;
};

static const GrMtlBackendFormatData* get_and_cast_data(const GrBackendFormat& format) {
    auto data = GrBackendSurfacePriv::GetBackendData(format);
    SkASSERT(!data || data->type() == GrBackendApi::kMetal);
    return static_cast<const GrMtlBackendFormatData*>(data);
}

namespace GrBackendFormats {

GrBackendFormat MakeMtl(GrMTLPixelFormat format) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D, GrBackendApi::kMetal, GrMtlBackendFormatData(format));
}

GrMTLPixelFormat AsMtlFormat(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kMetal) {
        const GrMtlBackendFormatData* data = get_and_cast_data(format);
        SkASSERT(data);
        return data->asMtlFormat();
    }
    // MTLPixelFormatInvalid == 0
    return GrMTLPixelFormat(0);
}

}  // namespace GrBackendFormats

class GrMtlBackendTextureData final : public GrBackendTextureData {
public:
    GrMtlBackendTextureData(const GrMtlTextureInfo& info) : fTexInfo(info) {}

    const GrMtlTextureInfo& info() const { return fTexInfo; }

private:
    void copyTo(AnyTextureData& textureData) const override {
        textureData.emplace<GrMtlBackendTextureData>(fTexInfo);
    }

    bool isProtected() const override { return false; }

    bool equal(const GrBackendTextureData* that) const override {
        return this->isSameTexture(that);
    }

    bool isSameTexture(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMetal);
        if (auto otherMtl = static_cast<const GrMtlBackendTextureData*>(that)) {
            return fTexInfo == otherMtl->fTexInfo;
        }
        return false;
    }

    GrBackendFormat getBackendFormat() const override {
        return GrBackendFormats::MakeMtl(GrGetMTLPixelFormatFromMtlTextureInfo(fTexInfo));
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMetal; }
#endif

    GrMtlTextureInfo fTexInfo;
};

static const GrMtlBackendTextureData* get_and_cast_data(const GrBackendTexture& texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kMetal);
    return static_cast<const GrMtlBackendTextureData*>(data);
}

namespace GrBackendTextures {

GrBackendTexture MakeMtl(int width,
                         int height,
                         skgpu::Mipmapped mipmapped,
                         const GrMtlTextureInfo& mtlInfo,
                         std::string_view label) {
    return GrBackendSurfacePriv::MakeGrBackendTexture(width,
                                                      height,
                                                      label,
                                                      mipmapped,
                                                      GrBackendApi::kMetal,
                                                      GrTextureType::k2D,
                                                      GrMtlBackendTextureData(mtlInfo));
}

bool GetMtlTextureInfo(const GrBackendTexture& tex, GrMtlTextureInfo* outInfo) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kMetal) {
        return false;
    }
    const GrMtlBackendTextureData* data = get_and_cast_data(tex);
    SkASSERT(data);
    *outInfo = data->info();
    return true;
}

}  // namespace GrBackendTextures

class GrMtlBackendRenderTargetData final : public GrBackendRenderTargetData {
public:
    GrMtlBackendRenderTargetData(const GrMtlTextureInfo& info) : fTexInfo(info) {}

    const GrMtlTextureInfo& info() const { return fTexInfo; }

private:
    GrBackendFormat getBackendFormat() const override {
        return GrBackendFormats::MakeMtl(GrGetMTLPixelFormatFromMtlTextureInfo(fTexInfo));
    }

    bool isProtected() const override { return false; }

    bool equal(const GrBackendRenderTargetData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMetal);
        if (auto otherMtl = static_cast<const GrMtlBackendRenderTargetData*>(that)) {
            return fTexInfo == otherMtl->fTexInfo;
        }
        return false;
    }

    void copyTo(AnyRenderTargetData& rtData) const override {
        rtData.emplace<GrMtlBackendRenderTargetData>(fTexInfo);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMetal; }
#endif

    GrMtlTextureInfo fTexInfo;
};

static const GrMtlBackendRenderTargetData* get_and_cast_data(const GrBackendRenderTarget& rt) {
    auto data = GrBackendSurfacePriv::GetBackendData(rt);
    SkASSERT(!data || data->type() == GrBackendApi::kMetal);
    return static_cast<const GrMtlBackendRenderTargetData*>(data);
}

namespace GrBackendRenderTargets {

GrBackendRenderTarget MakeMtl(int width, int height, const GrMtlTextureInfo& mtlInfo) {
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(
            width,
            height,
            std::max(1, GrMtlTextureInfoSampleCount(mtlInfo)),
            /*stencilBits=*/0,
            GrBackendApi::kMetal,
            /*framebufferOnly=*/false,  // TODO: set this from mtlInfo.fTexture->framebufferOnly
            GrMtlBackendRenderTargetData(mtlInfo));
}

bool GetMtlTextureInfo(const GrBackendRenderTarget& rt, GrMtlTextureInfo* outInfo) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kMetal) {
        return false;
    }
    const GrMtlBackendRenderTargetData* data = get_and_cast_data(rt);
    SkASSERT(data);
    *outInfo = data->info();
    return true;
}

}  // namespace GrBackendRenderTargets

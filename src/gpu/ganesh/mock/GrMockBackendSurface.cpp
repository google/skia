/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mock/GrMockBackendSurface.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/GrUtil.h"

#include <algorithm>

class GrMockBackendFormatData final : public GrBackendFormatData {
public:
    GrMockBackendFormatData(GrColorType colorType,
                            SkTextureCompressionType compression,
                            bool isStencilFormat)
            : fColorType(colorType)
            , fCompressionType(compression)
            , fIsStencilFormat(isStencilFormat) {
        SkASSERT(this->validate());
    }

    GrColorType colorType() const { return fColorType; }
    SkTextureCompressionType compressionType() const override { return fCompressionType; }
    bool isStencilFormat() const { return fIsStencilFormat; }

private:
    size_t bytesPerBlock() const override {
        if (fCompressionType != SkTextureCompressionType::kNone) {
            return SkCompressedDataSize(fCompressionType, {1, 1}, nullptr, false);
        }
        if (fIsStencilFormat) {
            return 4;
        }
        return GrColorTypeBytesPerPixel(fColorType);
    }

    int stencilBits() const override { return fIsStencilFormat ? 8 : 0; }

    uint32_t channelMask() const override { return GrColorTypeChannelFlags(fColorType); }

    GrColorFormatDesc desc() const override { return GrGetColorTypeDesc(fColorType); }

    bool equal(const GrBackendFormatData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMock);
        if (auto otherMock = static_cast<const GrMockBackendFormatData*>(that)) {
            return fColorType == otherMock->fColorType &&
                   fCompressionType == otherMock->fCompressionType &&
                   fIsStencilFormat == otherMock->fIsStencilFormat;
        }
        return false;
    }

    std::string toString() const override {
#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
        std::string str = GrColorTypeToStr(fColorType);
        str += "-";
        str += skgpu::CompressionTypeToStr(fCompressionType);
        if (fIsStencilFormat) {
            str += "-stencil";
        }
        return str;
#else
        return "";
#endif
    }

    void copyTo(AnyFormatData& formatData) const override {
        formatData.emplace<GrMockBackendFormatData>(fColorType, fCompressionType, fIsStencilFormat);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMock; }
#endif

    bool validate() const {
        int trueStates = 0;
        if (fCompressionType != SkTextureCompressionType::kNone) {
            trueStates++;
        }
        if (fColorType != GrColorType::kUnknown) {
            trueStates++;
        }
        if (fIsStencilFormat) {
            trueStates++;
        }
        return trueStates == 1;
    }

    GrColorType fColorType;
    SkTextureCompressionType fCompressionType;
    bool fIsStencilFormat;
};

static const GrMockBackendFormatData* get_and_cast_data(const GrBackendFormat& format) {
    auto data = GrBackendSurfacePriv::GetBackendData(format);
    SkASSERT(!data || data->type() == GrBackendApi::kMock);
    return static_cast<const GrMockBackendFormatData*>(data);
}

namespace GrBackendFormats {

GrBackendFormat MakeMockColorType(GrColorType colorType) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D,
            GrBackendApi::kMock,
            GrMockBackendFormatData(colorType, SkTextureCompressionType::kNone, false));
}

GrBackendFormat MakeMockCompressionType(SkTextureCompressionType compression) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D,
            GrBackendApi::kMock,
            GrMockBackendFormatData(GrColorType::kUnknown, compression, false));
}

GrBackendFormat MakeMockStencilFormat() {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D,
            GrBackendApi::kMock,
            GrMockBackendFormatData(GrColorType::kUnknown, SkTextureCompressionType::kNone, true));
}

GrColorType AsMockColorType(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kMock) {
        return get_and_cast_data(format)->colorType();
    }
    return GrColorType::kUnknown;
}

SkTextureCompressionType AsMockCompressionType(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kMock) {
        return get_and_cast_data(format)->compressionType();
    }
    return SkTextureCompressionType::kNone;
}

bool IsMockStencilFormat(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kMock) {
        return get_and_cast_data(format)->isStencilFormat();
    }
    return false;
}

}  // namespace GrBackendFormats

class GrMockBackendTextureData final : public GrBackendTextureData {
public:
    GrMockBackendTextureData(const GrMockTextureInfo& info) : fInfo(info) {}

    const GrMockTextureInfo& info() const { return fInfo; }

private:
    void copyTo(AnyTextureData& textureData) const override {
        textureData.emplace<GrMockBackendTextureData>(fInfo);
    }

    bool isProtected() const override { return fInfo.isProtected(); }

    bool equal(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMock);
        if (auto otherMock = static_cast<const GrMockBackendTextureData*>(that)) {
            return fInfo == otherMock->fInfo;
        }
        return false;
    }

    bool isSameTexture(const GrBackendTextureData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMock);
        if (auto otherMock = static_cast<const GrMockBackendTextureData*>(that)) {
            return fInfo.id() == otherMock->fInfo.id();
        }
        return false;
    }

    GrBackendFormat getBackendFormat() const override { return fInfo.getBackendFormat(); }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMock; }
#endif

    GrMockTextureInfo fInfo;
};

static const GrMockBackendTextureData* get_and_cast_data(const GrBackendTexture& texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kMock);
    return static_cast<const GrMockBackendTextureData*>(data);
}

namespace GrBackendTextures {

GrBackendTexture MakeMock(int width,
                          int height,
                          skgpu::Mipmapped mipmapped,
                          const GrMockTextureInfo& mockInfo,
                          std::string_view label) {
    return GrBackendSurfacePriv::MakeGrBackendTexture(width,
                                                      height,
                                                      label,
                                                      mipmapped,
                                                      GrBackendApi::kMock,
                                                      GrTextureType::k2D,
                                                      GrMockBackendTextureData(mockInfo));
}

GrMockTextureInfo GetMockTextureInfo(const GrBackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kMock) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendTexture\n");
        return {};
    }
    return get_and_cast_data(tex)->info();
}

}  // namespace GrBackendTextures

class GrMockBackendRenderTargetData final : public GrBackendRenderTargetData {
public:
    GrMockBackendRenderTargetData(const GrMockRenderTargetInfo& info) : fInfo(info) {}

    const GrMockRenderTargetInfo& info() const { return fInfo; }

private:
    void copyTo(AnyRenderTargetData& rtData) const override {
        rtData.emplace<GrMockBackendRenderTargetData>(fInfo);
    }

    bool isProtected() const override { return fInfo.isProtected(); }

    bool equal(const GrBackendRenderTargetData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMock);
        if (auto otherMock = static_cast<const GrMockBackendRenderTargetData*>(that)) {
            return fInfo == otherMock->fInfo;
        }
        return false;
    }

    GrBackendFormat getBackendFormat() const override { return fInfo.getBackendFormat(); }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMock; }
#endif

    GrMockRenderTargetInfo fInfo;
};

static const GrMockBackendRenderTargetData* get_and_cast_data(const GrBackendRenderTarget& rt) {
    auto data = GrBackendSurfacePriv::GetBackendData(rt);
    SkASSERT(!data || data->type() == GrBackendApi::kMock);
    return static_cast<const GrMockBackendRenderTargetData*>(data);
}

namespace GrBackendRenderTargets {

GrBackendRenderTarget MakeMock(int width,
                               int height,
                               int sampleCnt,
                               int stencilBits,
                               const GrMockRenderTargetInfo& mockInfo) {
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(width,
                                                           height,
                                                           std::max(1, sampleCnt),
                                                           stencilBits,
                                                           GrBackendApi::kMock,
                                                           false, /*framebufferOnly*/
                                                           GrMockBackendRenderTargetData(mockInfo));
}

GrMockRenderTargetInfo GetMockRenderTargetInfo(const GrBackendRenderTarget& rt) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kMock) {
        SkDEBUGFAIL("Mismatching backend or uninitialized GrBackendRenderTarget\n");
        return {};
    }
    return get_and_cast_data(rt)->info();
}

}  // namespace GrBackendRenderTargets

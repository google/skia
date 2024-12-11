/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurfacePriv_DEFINED
#define GrBackendSurfacePriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/MutableTextureState.h"  // IWYU pragma: keep
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

enum class GrBackendApi : unsigned int;
enum class SkTextureCompressionType;

namespace skgpu {
enum class Mipmapped : bool;
}

class GrBackendFormatData {
public:
    virtual ~GrBackendFormatData();
    virtual SkTextureCompressionType compressionType() const = 0;
    virtual size_t bytesPerBlock() const = 0;
    virtual int stencilBits() const = 0;
    virtual bool equal(const GrBackendFormatData* that) const = 0;
#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendFormatData() = default;
    GrBackendFormatData(const GrBackendFormatData&) = default;

    using AnyFormatData = GrBackendFormat::AnyFormatData;

private:
    friend class GrBackendFormat;
    virtual uint32_t channelMask() const = 0;
    virtual GrColorFormatDesc desc() const = 0;
    virtual std::string toString() const = 0;
    virtual void copyTo(AnyFormatData&) const = 0;

    // Vulkan-only API:
    virtual void makeTexture2D() {}
};

class GrBackendTextureData {
public:
    virtual ~GrBackendTextureData();
#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendTextureData() = default;
    GrBackendTextureData(const GrBackendTextureData&) = default;

    using AnyTextureData = GrBackendTexture::AnyTextureData;

private:
    friend class GrBackendTexture;
    virtual bool isProtected() const = 0;
    virtual bool equal(const GrBackendTextureData* that) const = 0;
    virtual bool isSameTexture(const GrBackendTextureData*) const = 0;
    virtual GrBackendFormat getBackendFormat() const = 0;
    virtual void copyTo(AnyTextureData&) const = 0;

    // Vulkan-only API:
    virtual sk_sp<skgpu::MutableTextureState> getMutableState() const { return nullptr; }
    virtual void setMutableState(const skgpu::MutableTextureState&) {}
};

class GrBackendRenderTargetData {
public:
    virtual ~GrBackendRenderTargetData();
#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendRenderTargetData() = default;
    GrBackendRenderTargetData(const GrBackendRenderTargetData&) = default;

    using AnyRenderTargetData = GrBackendRenderTarget::AnyRenderTargetData;

private:
    friend class GrBackendRenderTarget;
    virtual GrBackendFormat getBackendFormat() const = 0;
    virtual bool isProtected() const = 0;
    virtual bool equal(const GrBackendRenderTargetData* that) const = 0;
    virtual void copyTo(AnyRenderTargetData&) const = 0;

    // Vulkan-only API:
    virtual sk_sp<skgpu::MutableTextureState> getMutableState() const { return nullptr; }
    virtual void setMutableState(const skgpu::MutableTextureState&) {}
};

class GrBackendSurfacePriv final {
public:
    template <typename FormatData>
    static GrBackendFormat MakeGrBackendFormat(GrTextureType textureType,
                                               GrBackendApi api,
                                               const FormatData& data) {
        return GrBackendFormat(textureType, api, data);
    }

    static const GrBackendFormatData* GetBackendData(const GrBackendFormat& format) {
        return format.fFormatData.get();
    }

    template <typename TextureData>
    static GrBackendTexture MakeGrBackendTexture(int width,
                                                 int height,
                                                 std::string_view label,
                                                 skgpu::Mipmapped mipped,
                                                 GrBackendApi backend,
                                                 GrTextureType texture,
                                                 const TextureData& data) {
        return GrBackendTexture(width, height, label, mipped, backend, texture, data);
    }

    static const GrBackendTextureData* GetBackendData(const GrBackendTexture& tex) {
        return tex.fTextureData.get();
    }

    static GrBackendTextureData* GetBackendData(GrBackendTexture* tex) {
        SkASSERT(tex);
        return tex->fTextureData.get();
    }

    template <typename RenderTargetData>
    static GrBackendRenderTarget MakeGrBackendRenderTarget(int width,
                                                           int height,
                                                           int sampleCnt,
                                                           int stencilBits,
                                                           GrBackendApi backend,
                                                           bool framebufferOnly,
                                                           const RenderTargetData& data) {
        return GrBackendRenderTarget(
                width, height, sampleCnt, stencilBits, backend, framebufferOnly, data);
    }

    static const GrBackendRenderTargetData* GetBackendData(const GrBackendRenderTarget& rt) {
        return rt.fRTData.get();
    }

    static GrBackendRenderTargetData* GetBackendData(GrBackendRenderTarget* rt) {
        return rt->fRTData.get();
    }
};

#endif

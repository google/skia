/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurfacePriv_DEFINED
#define GrBackendSurfacePriv_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

enum class GrBackendApi : unsigned int;
enum class SkTextureCompressionType;
namespace skgpu { enum class Mipmapped : bool; }

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

private:
    friend class GrBackendFormat;
    virtual uint32_t channelMask() const = 0;
    virtual GrColorFormatDesc desc() const = 0;
    virtual std::string toString() const = 0;
    virtual GrBackendFormatData* copy() const = 0;
};

class GrBackendTextureData {
public:
    virtual ~GrBackendTextureData();
#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendTextureData() = default;

private:
    friend class GrBackendTexture;
    virtual bool isProtected() const = 0;
    virtual bool equal(const GrBackendTextureData* that) const = 0;
    virtual bool isSameTexture(const GrBackendTextureData*) const = 0;
    virtual GrBackendFormat getBackendFormat() const = 0;
    virtual GrBackendTextureData* copy() const = 0;
};

class GrBackendRenderTargetData {
public:
    virtual ~GrBackendRenderTargetData();
#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendRenderTargetData() = default;

private:
    friend class GrBackendRenderTarget;
    virtual bool isValid() const = 0;
    virtual GrBackendFormat getBackendFormat() const = 0;
    virtual bool isProtected() const = 0;
    virtual bool equal(const GrBackendRenderTargetData* that) const = 0;
    virtual GrBackendRenderTargetData* copy() const = 0;
};

class GrBackendSurfacePriv final {
public:
    static GrBackendFormat MakeGrBackendFormat(GrTextureType textureType,
                                               GrBackendApi api,
                                               std::unique_ptr<const GrBackendFormatData> data) {
        return GrBackendFormat(textureType, api, std::move(data));
    }

    static const GrBackendFormatData* GetBackendData(const GrBackendFormat& format) {
        return format.fFormatData.get();
    }

    static GrBackendTexture MakeGrBackendTexture(int width,
                                                 int height,
                                                 std::string_view label,
                                                 skgpu::Mipmapped mipped,
                                                 GrBackendApi backend,
                                                 GrTextureType texture,
                                                 std::unique_ptr<GrBackendTextureData> data) {
        return GrBackendTexture(width, height, label, mipped, backend, texture, std::move(data));
    }

    static GrBackendTextureData* GetBackendData(const GrBackendTexture& tex) {
        return tex.fTextureData.get();
    }

    static GrBackendTextureData* GetBackendData(GrBackendTexture* tex) {
        SkASSERT(tex);
        return tex->fTextureData.get();
    }

    static GrBackendRenderTarget MakeGrBackendRenderTarget(
            int width,
            int height,
            int sampleCnt,
            int stencilBits,
            GrBackendApi backend,
            bool framebufferOnly,
            std::unique_ptr<GrBackendRenderTargetData> data) {
        return GrBackendRenderTarget(
                width, height, sampleCnt, stencilBits, backend, framebufferOnly, std::move(data));
    }

    static const GrBackendRenderTargetData* GetBackendData(const GrBackendRenderTarget& rt) {
        return rt.fRTData.get();
    }
};

#endif

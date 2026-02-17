/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/GrBackendSurface.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/MutableTextureState.h"  // IWYU pragma: keep
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/GrUtil.h"

#include <algorithm>
#include <new>

GrBackendFormat::GrBackendFormat() : fValid(false) {}
GrBackendFormat::~GrBackendFormat() = default;

GrBackendFormat::GrBackendFormat(const GrBackendFormat& that)
        : fBackend(that.fBackend)
        , fValid(that.fValid)
        , fTextureType(that.fTextureType) {
    if (!fValid) {
        return;
    }

    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            fFormatData.reset();
            that.fFormatData->copyTo(fFormatData);
            break;  // fFormatData is sufficient
        case GrBackendApi::kMock:
            fMock = that.fMock;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
}

GrBackendFormat& GrBackendFormat::operator=(const GrBackendFormat& that) {
    if (this != &that) {
        this->~GrBackendFormat();
        new (this) GrBackendFormat(that);
    }
    return *this;
}

GrBackendFormat::GrBackendFormat(GrColorType colorType, SkTextureCompressionType compression,
                                 bool isStencilFormat)
        : fBackend(GrBackendApi::kMock)
        , fValid(true)
        , fTextureType(GrTextureType::k2D) {
    fMock.fColorType = colorType;
    fMock.fCompressionType = compression;
    fMock.fIsStencilFormat = isStencilFormat;
    SkASSERT(this->validateMock());
}

uint32_t GrBackendFormat::channelMask() const {
    if (!this->isValid()) {
        return 0;
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fFormatData->channelMask();
        case GrBackendApi::kMock:
            return GrColorTypeChannelFlags(fMock.fColorType);

        default:
            return 0;
    }
}

GrColorFormatDesc GrBackendFormat::desc() const {
    if (!this->isValid()) {
        return GrColorFormatDesc::MakeInvalid();
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fFormatData->desc();
        case GrBackendApi::kMock:
            return GrGetColorTypeDesc(fMock.fColorType);

        default:
            return GrColorFormatDesc::MakeInvalid();
    }
}

#ifdef SK_DEBUG
bool GrBackendFormat::validateMock() const {
    int trueStates = 0;
    if (fMock.fCompressionType != SkTextureCompressionType::kNone) {
        trueStates++;
    }
    if (fMock.fColorType != GrColorType::kUnknown) {
        trueStates++;
    }
    if (fMock.fIsStencilFormat) {
        trueStates++;
    }
    return trueStates == 1;
}
#endif

GrColorType GrBackendFormat::asMockColorType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fColorType;
    }

    return GrColorType::kUnknown;
}

SkTextureCompressionType GrBackendFormat::asMockCompressionType() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fCompressionType;
    }

    return SkTextureCompressionType::kNone;
}

bool GrBackendFormat::isMockStencilFormat() const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        SkASSERT(this->validateMock());
        return fMock.fIsStencilFormat;
    }

    return false;
}

GrBackendFormat GrBackendFormat::makeTexture2D() const {
    GrBackendFormat copy = *this;
    // TODO(b/293490566): Remove this kVulkan check once mock backend is using fFormatData.
    if (fBackend == GrBackendApi::kVulkan) {
        copy.fFormatData->makeTexture2D();
    }
    copy.fTextureType = GrTextureType::k2D;
    return copy;
}

GrBackendFormat GrBackendFormat::MakeMock(GrColorType colorType,
                                          SkTextureCompressionType compression,
                                          bool isStencilFormat) {
    return GrBackendFormat(colorType, compression, isStencilFormat);
}

bool GrBackendFormat::operator==(const GrBackendFormat& that) const {
    // Invalid GrBackendFormats are never equal to anything.
    if (!fValid || !that.fValid) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fFormatData->equal(that.fFormatData.get());
        case GrBackendApi::kMock:
            return fMock.fColorType == that.fMock.fColorType &&
                   fMock.fCompressionType == that.fMock.fCompressionType;
        default:
            SK_ABORT("Unknown GrBackend");
    }
}

#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
#include "include/core/SkString.h"

SkString GrBackendFormat::toStr() const {
    SkString str;

    if (!fValid) {
        str.append("invalid");
        return str;
    }

    str.appendf("%s-", GrBackendApiToStr(fBackend));

    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            str.append(fFormatData->toString());
            break;
        case GrBackendApi::kMock:
            str.append(GrColorTypeToStr(fMock.fColorType));
            str.appendf("-");
            str.append(skgpu::CompressionTypeToStr(fMock.fCompressionType));
            break;
        case GrBackendApi::kUnsupported:
            break;
    }

    return str;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
GrBackendTexture::GrBackendTexture() : fIsValid(false) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   skgpu::Mipmapped mipmapped,
                                   const GrMockTextureInfo& mockInfo,
                                   std::string_view label)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fLabel(label)
        , fMipmapped(mipmapped)
        , fBackend(GrBackendApi::kMock)
        , fTextureType(GrTextureType::k2D)
        , fMockInfo(mockInfo) {}

GrBackendTexture::~GrBackendTexture() = default;

GrBackendTexture::GrBackendTexture(const GrBackendTexture& that) : fIsValid(false) {
    *this = that;
}

GrBackendTexture& GrBackendTexture::operator=(const GrBackendTexture& that) {
    if (this == &that) {
        return *this;
    }

    fTextureData.reset();
    if (!that.isValid()) {
        fIsValid = false;
        return *this;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fMipmapped = that.fMipmapped;
    fBackend = that.fBackend;
    fTextureType = that.fTextureType;

    switch (that.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            that.fTextureData->copyTo(fTextureData);
            break;
        case GrBackendApi::kMock:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = true;
    return *this;
}

sk_sp<skgpu::MutableTextureState> GrBackendTexture::getMutableState() const {
    return fTextureData->getMutableState();
}

bool GrBackendTexture::getMockTextureInfo(GrMockTextureInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

void GrBackendTexture::setMutableState(const skgpu::MutableTextureState& state) {
    fTextureData->setMutableState(state);
}

bool GrBackendTexture::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
    if (this->backend() == GrBackendApi::kOpenGL || this->backend() == GrBackendApi::kVulkan) {
        return fTextureData->isProtected();
    }
    if (this->backend() == GrBackendApi::kMock) {
        return fMockInfo.isProtected();
    }

    return false;
}

bool GrBackendTexture::isSameTexture(const GrBackendTexture& that) {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }
    if (fBackend != that.fBackend) {
        return false;
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fTextureData->isSameTexture(that.fTextureData.get());
        case GrBackendApi::kMock:
            return fMockInfo.id() == that.fMockInfo.id();
        default:
            return false;
    }
}

GrBackendFormat GrBackendTexture::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fTextureData->getBackendFormat();
        case GrBackendApi::kMock:
            return fMockInfo.getBackendFormat();
        default:
            return GrBackendFormat();
    }
}

#if defined(GPU_TEST_UTILS)
bool GrBackendTexture::TestingOnly_Equals(const GrBackendTexture& t0, const GrBackendTexture& t1) {
    if (!t0.isValid() || !t1.isValid()) {
        return false; // two invalid backend textures are not considered equal
    }

    if (t0.fWidth != t1.fWidth ||
        t0.fHeight != t1.fHeight ||
        t0.fMipmapped != t1.fMipmapped ||
        t0.fBackend != t1.fBackend) {
        return false;
    }

    switch (t0.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return t0.fTextureData->equal(t1.fTextureData.get());
        case GrBackendApi::kMock:
            return t0.fMockInfo == t1.fMockInfo;
        default:
            return false;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

GrBackendRenderTarget::GrBackendRenderTarget() : fIsValid(false) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrMockRenderTargetInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(std::max(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fBackend(GrBackendApi::kMock)
        , fMockInfo(mockInfo) {}

GrBackendRenderTarget::~GrBackendRenderTarget() = default;

GrBackendRenderTarget::GrBackendRenderTarget(const GrBackendRenderTarget& that) : fIsValid(false) {
    *this = that;
}

GrBackendRenderTarget& GrBackendRenderTarget::operator=(const GrBackendRenderTarget& that) {
    if (this == &that) {
        return *this;
    }

    fRTData.reset();
    if (!that.isValid()) {
        fIsValid = false;
        return *this;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fSampleCnt = that.fSampleCnt;
    fStencilBits = that.fStencilBits;
    fBackend = that.fBackend;

    switch (that.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            that.fRTData->copyTo(fRTData);
            break;
        case GrBackendApi::kMock:
            fMockInfo = that.fMockInfo;
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = true;
    return *this;
}

sk_sp<skgpu::MutableTextureState> GrBackendRenderTarget::getMutableState() const {
    return fRTData->getMutableState();
}

GrBackendFormat GrBackendRenderTarget::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fRTData->getBackendFormat();
        case GrBackendApi::kMock:
            return fMockInfo.getBackendFormat();
        default:
            return GrBackendFormat();
    }
}

bool GrBackendRenderTarget::getMockRenderTargetInfo(GrMockRenderTargetInfo* outInfo) const {
    if (this->isValid() && GrBackendApi::kMock == fBackend) {
        *outInfo = fMockInfo;
        return true;
    }
    return false;
}

void GrBackendRenderTarget::setMutableState(const skgpu::MutableTextureState& state) {
    fRTData->setMutableState(state);
}

bool GrBackendRenderTarget::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
    if (this->backend() == GrBackendApi::kOpenGL || this->backend() == GrBackendApi::kVulkan) {
        return fRTData->isProtected();
    }
    if (this->backend() == GrBackendApi::kMock) {
        return fMockInfo.isProtected();
    }

    return false;
}

#if defined(GPU_TEST_UTILS)
bool GrBackendRenderTarget::TestingOnly_Equals(const GrBackendRenderTarget& r0,
                                               const GrBackendRenderTarget& r1) {
    if (!r0.isValid() || !r1.isValid()) {
        return false; // two invalid backend rendertargets are not considered equal
    }

    if (r0.fWidth != r1.fWidth ||
        r0.fHeight != r1.fHeight ||
        r0.fSampleCnt != r1.fSampleCnt ||
        r0.fStencilBits != r1.fStencilBits ||
        r0.fBackend != r1.fBackend) {
        return false;
    }

    switch (r0.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return r0.fRTData->equal(r1.fRTData.get());
        case GrBackendApi::kMock:
            return r0.fMockInfo == r1.fMockInfo;
        default:
            return false;
    }

    SkASSERT(0);
    return false;
}
#endif

GrBackendFormatData::~GrBackendFormatData() {}
GrBackendTextureData::~GrBackendTextureData() {}
GrBackendRenderTargetData::~GrBackendRenderTargetData() {}

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

#include "src/gpu/graphite/TextureInfoPriv.h"

#ifdef SK_DAWN
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"
#endif

namespace skgpu::graphite {

TextureInfo::TextureInfo(){};
TextureInfo::~TextureInfo() = default;

#ifdef SK_DAWN
TextureInfo::TextureInfo(const DawnTextureInfo& dawnInfo)
        : fBackend(BackendApi::kDawn)
        , fValid(true)
        , fSampleCount(dawnInfo.fSampleCount)
        , fMipmapped(dawnInfo.fMipmapped)
        , fProtected(Protected::kNo)
        , fDawnSpec(dawnInfo) {}
#endif

TextureInfo::TextureInfo(const TextureInfo& that)
        : fBackend(that.fBackend)
        , fValid(that.fValid)
        , fSampleCount(that.fSampleCount)
        , fMipmapped(that.fMipmapped)
        , fProtected(that.fProtected) {
    if (!fValid) {
        return;
    }

    switch (that.backend()) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            fTextureInfoData.reset();
            that.fTextureInfoData->copyTo(fTextureInfoData);
            break;
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnSpec = that.fDawnSpec;
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }
}

TextureInfo& TextureInfo::operator=(const TextureInfo& that) {
    if (this != &that) {
        this->~TextureInfo();
        new (this) TextureInfo(that);
    }
    return *this;
}

bool TextureInfo::operator==(const TextureInfo& that) const {
    if (!this->isValid() && !that.isValid()) {
        return true;
    }
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fMipmapped != that.fMipmapped ||
        fProtected != that.fProtected) {
        return false;
    }

    switch (fBackend) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->equal(that.fTextureInfoData.get());
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return fDawnSpec == that.fDawnSpec;
#endif
        default:
            return false;
    }
}

bool TextureInfo::isCompatible(const TextureInfo& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fMipmapped != that.fMipmapped ||
        fProtected != that.fProtected) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    switch (fBackend) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->isCompatible(that.fTextureInfoData.get());
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return fDawnSpec.isCompatible(that.fDawnSpec);
#endif
        default:
            return false;
    }
}

#ifdef SK_DAWN
bool TextureInfo::getDawnTextureInfo(DawnTextureInfo* info) const {
    if (!this->isValid() || fBackend != BackendApi::kDawn) {
        return false;
    }
    *info = DawnTextureSpecToTextureInfo(fDawnSpec, fSampleCount, fMipmapped);
    return true;
}
#endif

SkString TextureInfo::toString() const {
    SkString ret;
    switch (fBackend) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            ret = fTextureInfoData->toString();
            break;
#ifdef SK_DAWN
        case BackendApi::kDawn:
            ret.appendf("Dawn(%s,", fDawnSpec.toString().c_str());
            break;
#endif
        case BackendApi::kMock:
            ret += "Mock(";
            break;
        default:
            ret += "Invalid(";
            break;
    }
    ret.appendf("bytesPerPixel=%zu,sampleCount=%u,mipmapped=%d,protected=%d)",
                this->bytesPerPixel(),
                fSampleCount,
                static_cast<int>(fMipmapped),
                static_cast<int>(fProtected));
    return ret;
}

SkString TextureInfo::toRPAttachmentString() const {
    // For renderpass attachments, the string will contain the view format and sample count only
    switch (fBackend) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->toRPAttachmentString(fSampleCount);
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return SkStringPrintf("Dawn(f=%u,s=%u)",
                                  static_cast<unsigned int>(fDawnSpec.fViewFormat),
                                  fSampleCount);
#endif
        case BackendApi::kMock:
            return SkStringPrintf("Mock(s=%u)", fSampleCount);
        default:
            return SkString("Invalid");
    }
}

size_t TextureInfo::bytesPerPixel() const {
    if (!this->isValid()) {
        return 0;
    }

    switch (fBackend) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->bytesPerPixel();
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return DawnFormatBytesPerBlock(this->dawnTextureSpec().getViewFormat());
#endif
        default:
            return 0;
    }
}

SkTextureCompressionType TextureInfo::compressionType() const {
    if (!this->isValid()) {
        return SkTextureCompressionType::kNone;
    }

    switch (fBackend) {
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->compressionType();
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return DawnFormatToCompressionType(this->dawnTextureSpec().getViewFormat());
#endif
        default:
            return SkTextureCompressionType::kNone;
    }
}

TextureInfoData::~TextureInfoData(){};

} // namespace skgpu::graphite

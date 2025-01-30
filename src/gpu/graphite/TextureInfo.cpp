/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

TextureInfo::TextureInfo(){};
TextureInfo::~TextureInfo() = default;

static inline void assert_is_supported_backend(const BackendApi& backend) {
    SkASSERT(backend == BackendApi::kDawn ||
             backend == BackendApi::kMetal ||
             backend == BackendApi::kVulkan);
}

TextureInfo::TextureInfo(const TextureInfo& that)
        : fBackend(that.fBackend)
        , fValid(that.fValid)
        , fSampleCount(that.fSampleCount)
        , fMipmapped(that.fMipmapped)
        , fProtected(that.fProtected) {
    if (!fValid) {
        return;
    }

    assert_is_supported_backend(fBackend);
    fTextureInfoData.reset();
    that.fTextureInfoData->copyTo(fTextureInfoData);
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
    assert_is_supported_backend(fBackend);
    return fTextureInfoData->equal(that.fTextureInfoData.get());
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
    assert_is_supported_backend(fBackend);
    return fTextureInfoData->isCompatible(that.fTextureInfoData.get());
}

SkString TextureInfo::toString() const {
    if (!fValid) {
        return SkString("{}");
    }

    SkString ret;
    switch (fBackend) {
        case BackendApi::kDawn:
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            ret = fTextureInfoData->toString();
            break;
        case BackendApi::kMock:
            ret += "Mock(";
            break;
        case BackendApi::kUnsupported:
            ret += "Unsupported(";
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
    if (!fValid) {
        return SkString("{}");
    }

    // For renderpass attachments, the string will contain the view format and sample count only
    switch (fBackend) {
        case BackendApi::kDawn:
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->toRPAttachmentString(fSampleCount);
        case BackendApi::kMock:
            return SkStringPrintf("Mock(s=%u)", fSampleCount);
        case BackendApi::kUnsupported:
            return SkString("Unsupported");
    }
    SkUNREACHABLE;
}

size_t TextureInfo::bytesPerPixel() const {
    if (!this->isValid()) {
        return 0;
    }

    switch (fBackend) {
        case BackendApi::kDawn:
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->bytesPerPixel();
        default:
            return 0;
    }
}

SkTextureCompressionType TextureInfo::compressionType() const {
    if (!this->isValid()) {
        return SkTextureCompressionType::kNone;
    }

    switch (fBackend) {
        case BackendApi::kDawn:
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->compressionType();
        default:
            return SkTextureCompressionType::kNone;
    }
}

bool TextureInfo::isMemoryless() const {
    if (!this->isValid()) {
        return false;
    }

    switch (fBackend) {
        case BackendApi::kDawn:
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureInfoData->isMemoryless();
        default:
            return false;
    }
}

TextureInfoData::~TextureInfoData(){};

bool TextureInfoPriv::Serialize(SkWStream* stream,
                                const TextureInfo& textureInfo) {
    if (!stream->write32(
            SkSetFourByteTag(static_cast<uint8_t>(textureInfo.backend()),
                             static_cast<uint8_t>(textureInfo.isValid()),
                             static_cast<uint8_t>(textureInfo.mipmapped()),
                             static_cast<uint8_t>(textureInfo.isProtected())))) {
        return false;
    }

    if (!stream->write32(textureInfo.numSamples())) {
        return false;
    }

    if (!textureInfo.isValid()) {
        // We don't bother serializing the textureInfoData is it isn't valid
        return true;
    }

    return textureInfo.fTextureInfoData.get()->serialize(stream);
}

bool TextureInfoPriv::Deserialize(const Caps* caps,
                                  SkStream* stream,
                                  TextureInfo* textureInfo) {
    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return false;
    }

    BackendApi backendAPI = static_cast<BackendApi>(0xF & (tag >> 24));
    bool isValid = SkToBool(0xF & (tag >> 16));
    Mipmapped mipmapped = static_cast<Mipmapped>(0xF & (tag >> 8));
    Protected isProtected = static_cast<Protected>(0xF & tag);

    uint32_t sampleCount;
    if (!stream->readU32(&sampleCount)) {
        return false;
    }

    if (!isValid) {
        // It is okay to have a serialized, invalid TextureInfo
        return true;
    }

    return caps->deserializeTextureInfo(stream, backendAPI, mipmapped, isProtected,
                                        sampleCount, textureInfo);
}

} // namespace skgpu::graphite

/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkGraphics.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkNextID.h"
#include "src/image/SkImageGeneratorPriv.h"

#include <memory>
#include <optional>
#include <utility>

SkImageGenerator::SkImageGenerator(const SkImageInfo& info, uint32_t uniqueID)
    : fInfo(info)
    , fUniqueID(kNeedNewImageUniqueID == uniqueID ? SkNextID::ImageID() : uniqueID)
{}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    if (kUnknown_SkColorType == info.colorType()) {
        return false;
    }
    if (nullptr == pixels) {
        return false;
    }
    if (rowBytes < info.minRowBytes()) {
        return false;
    }

    Options defaultOpts;
    return this->onGetPixels(info, pixels, rowBytes, defaultOpts);
}

bool SkImageGenerator::queryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                                     SkYUVAPixmapInfo* yuvaPixmapInfo) const {
    SkASSERT(yuvaPixmapInfo);

    return this->onQueryYUVAInfo(supportedDataTypes, yuvaPixmapInfo) &&
           yuvaPixmapInfo->isSupported(supportedDataTypes);
}

bool SkImageGenerator::getYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) {
    return this->onGetYUVAPlanes(yuvaPixmaps);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkGraphics::ImageGeneratorFromEncodedDataFactory gFactory;

SkGraphics::ImageGeneratorFromEncodedDataFactory
SkGraphics::SetImageGeneratorFromEncodedDataFactory(ImageGeneratorFromEncodedDataFactory factory)
{
    ImageGeneratorFromEncodedDataFactory prev = gFactory;
    gFactory = factory;
    return prev;
}

namespace SkImageGenerators {

std::unique_ptr<SkImageGenerator> MakeFromEncoded(sk_sp<SkData> data,
                                                  std::optional<SkAlphaType> at) {
    if (!data || at == kOpaque_SkAlphaType) {
        return nullptr;
    }
    if (gFactory) {
        if (std::unique_ptr<SkImageGenerator> generator = gFactory(data)) {
            return generator;
        }
    }
    return MakeFromEncodedImpl(std::move(data), at);
}

}  // namespace SkImageGenerators

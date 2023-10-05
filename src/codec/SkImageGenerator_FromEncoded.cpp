/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/image/SkImageGeneratorPriv.h"

#include <memory>
#include <optional>
#include <utility>

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
    return SkCodecImageGenerator::MakeFromEncodedCodec(std::move(data), at);
}

}  // namespace SkImageGenerators

namespace SkImages {

sk_sp<SkImage> DeferredFromEncodedData(sk_sp<SkData> encoded,
                                       std::optional<SkAlphaType> alphaType) {
    if (nullptr == encoded || encoded->isEmpty()) {
        return nullptr;
    }
    return DeferredFromGenerator(SkImageGenerators::MakeFromEncoded(std::move(encoded), alphaType));
}

}  // namespace SkImages

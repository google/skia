/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"

#include "src/image/SkImageGeneratorPriv.h"

#include <optional>
#include <utility>

class SkMatrix;
class SkPaint;
enum SkAlphaType : int;
struct SkISize;

namespace SkImages {

sk_sp<SkImage> DeferredFromEncodedData(sk_sp<SkData> encoded,
                                       std::optional<SkAlphaType> alphaType) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    return DeferredFromGenerator(SkImageGenerator::MakeFromEncoded(std::move(encoded), alphaType));
}

sk_sp<SkImage> DeferredFromPicture(sk_sp<SkPicture> picture,
                                   const SkISize& dimensions,
                                   const SkMatrix* matrix,
                                   const SkPaint* paint,
                                   BitDepth bitDepth,
                                   sk_sp<SkColorSpace> colorSpace) {
    return DeferredFromPicture(std::move(picture), dimensions, matrix, paint, bitDepth,
                               std::move(colorSpace), {});
}

sk_sp<SkImage> DeferredFromPicture(sk_sp<SkPicture> picture,
                                   const SkISize& dimensions,
                                   const SkMatrix* matrix,
                                   const SkPaint* paint,
                                   BitDepth bitDepth,
                                   sk_sp<SkColorSpace> colorSpace,
                                   SkSurfaceProps props) {
    return DeferredFromGenerator(SkImageGenerators::MakeFromPicture(
            dimensions, std::move(picture), matrix, paint, bitDepth, std::move(colorSpace), props));
}

}  // namespace SkImages

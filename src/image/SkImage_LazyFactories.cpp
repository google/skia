/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "src/image/SkImage_Picture.h"

#include <utility>

class SkMatrix;
class SkPaint;
struct SkISize;

namespace SkImages {

sk_sp<SkImage> DeferredFromPicture(sk_sp<SkPicture> picture,
                                   const SkISize& dimensions,
                                   const SkMatrix* matrix,
                                   const SkPaint* paint,
                                   BitDepth bitDepth,
                                   sk_sp<SkColorSpace> colorSpace) {
    return SkImage_Picture::Make(std::move(picture), dimensions, matrix, paint, bitDepth,
                                 std::move(colorSpace), {});
}

sk_sp<SkImage> DeferredFromPicture(sk_sp<SkPicture> picture,
                                   const SkISize& dimensions,
                                   const SkMatrix* matrix,
                                   const SkPaint* paint,
                                   BitDepth bitDepth,
                                   sk_sp<SkColorSpace> colorSpace,
                                   SkSurfaceProps props) {
    return SkImage_Picture::Make(std::move(picture), dimensions, matrix, paint, bitDepth,
                                 std::move(colorSpace), props);
}

}  // namespace SkImages

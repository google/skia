/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Picture.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkMutex.h"
#include "src/base/SkTLazy.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "src/image/SkImage_Lazy.h"
#include "src/image/SkPictureImageGenerator.h"

#include <memory>
#include <utility>

class SkMatrix;
class SkPaint;
struct SkISize;

sk_sp<SkImage> SkImage_Picture::Make(sk_sp<SkPicture> picture, const SkISize& dimensions,
                                     const SkMatrix* matrix, const SkPaint* paint,
                                     SkImages::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                                     SkSurfaceProps props) {
    auto gen = SkImageGenerators::MakeFromPicture(dimensions, std::move(picture), matrix, paint,
                                                  bitDepth, std::move(colorSpace), props);

    SkImage_Lazy::Validator validator(
            SharedGenerator::Make(std::move(gen)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Picture>(&validator) : nullptr;
}

const SkSurfaceProps* SkImage_Picture::props() const {
    auto pictureIG = static_cast<SkPictureImageGenerator*>(this->generator()->fGenerator.get());
    return &pictureIG->fProps;
}

void SkImage_Picture::replay(SkCanvas* canvas) const {
    auto sharedGenerator = this->generator();
    SkAutoMutexExclusive mutex(sharedGenerator->fMutex);

    auto pictureIG = static_cast<SkPictureImageGenerator*>(sharedGenerator->fGenerator.get());
    canvas->clear(SkColors::kTransparent);
    canvas->drawPicture(pictureIG->fPicture,
                        &pictureIG->fMatrix,
                        pictureIG->fPaint.getMaybeNull());
}

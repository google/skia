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
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTFitsIn.h"
#include "src/base/SkTLazy.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "src/image/SkImage_Lazy.h"
#include "src/image/SkPictureImageGenerator.h"

#include <cstring>
#include <memory>
#include <utility>

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

sk_sp<SkImage> SkImage_Picture::onMakeSubset(GrDirectContext*, const SkIRect& subset) const {
    auto sharedGenerator = this->generator();
    auto pictureIG = static_cast<SkPictureImageGenerator*>(sharedGenerator->fGenerator.get());

    SkMatrix matrix = pictureIG->fMatrix;
    matrix.postTranslate(-subset.left(), -subset.top());
    SkImages::BitDepth bitDepth =
            this->colorType() == kRGBA_F16_SkColorType ? SkImages::BitDepth::kF16
                                                       : SkImages::BitDepth::kU8;

    return SkImage_Picture::Make(pictureIG->fPicture, subset.size(),
                                 &matrix, pictureIG->fPaint.getMaybeNull(),
                                 bitDepth, this->refColorSpace(), pictureIG->fProps);
}

sk_sp<SkImage> SkImage_Picture::onMakeSubset(skgpu::graphite::Recorder*,
                                             const SkIRect& subset,
                                             RequiredProperties) const {
    // The Ganesh version doesn't make use of GrDirectContext so we can use it to
    // generate our initial subset. In addition, requesting mipmaps doesn't make
    // much sense in this case so we ignore the props.
    return this->onMakeSubset(nullptr, subset);
}

bool SkImage_Picture::getImageKeyValues(
        uint32_t keyValues[SkTiledImageUtils::kNumImageKeyValues]) const {

    auto sharedGenerator = this->generator();
    SkAutoMutexExclusive mutex(sharedGenerator->fMutex);

    auto pictureIG = static_cast<SkPictureImageGenerator*>(sharedGenerator->fGenerator.get());
    if (pictureIG->fPaint.getMaybeNull()) {
        // A full paint complicates the potential key too much.
        return false;
    }

    const SkImageInfo& ii = sharedGenerator->getInfo();
    if (!ii.colorSpace()->isSRGB()) {
        // We only return key values if the colorSpace is sRGB.
        return false;
    }

    const SkMatrix& m = pictureIG->fMatrix;
    if (!m.isIdentity() && !m.isTranslate()) {
        // To keep the key small we only cache simple (<= translation) matrices
        return false;
    }

    bool isU8 = ii.colorType() != kRGBA_F16_SkColorType;
    uint32_t pixelGeometry = this->props()->pixelGeometry();
    uint32_t surfacePropFlags = this->props()->flags();
    int width = ii.width();
    int height = ii.height();
    float transX = m.getTranslateX();
    float transY = m.getTranslateY();

    SkASSERT(pixelGeometry <= 4);
    SkASSERT(surfacePropFlags < 8);
    SkASSERT(SkTFitsIn<uint32_t>(width));
    SkASSERT(SkTFitsIn<uint32_t>(height));
    SkASSERT(sizeof(float) == sizeof(uint32_t));

    // The 0th slot usually holds either the SkBitmap's ID or the image's. In those two cases
    // slot #1 is zero so we can reuse the 0th slot here.
    keyValues[0] = (isU8 ? 0x1 : 0x0) |     // 1 bit
                   (pixelGeometry << 1) |   // 3 bits
                   (surfacePropFlags << 4); // 3 bits
    keyValues[1] = pictureIG->fPicture->uniqueID();
    SkASSERT(keyValues[1] != 0);    // Double check we don't collide w/ bitmap or image keys
    keyValues[2] = width;
    keyValues[3] = height;
    memcpy(&keyValues[4], &transX, sizeof(uint32_t));
    memcpy(&keyValues[5], &transY, sizeof(uint32_t));
    return true;
}

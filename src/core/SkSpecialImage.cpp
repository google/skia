/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "src/core/SkSpecialImage.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkNextID.h"
#include "src/image/SkImage_Base.h"

// Currently, the raster imagefilters can only handle certain imageinfos. Call this to know if
// a given info is supported.
static bool valid_for_imagefilters(const SkImageInfo& info) {
    // no support for other swizzles/depths yet
    return info.colorType() == kN32_SkColorType;
}

SkSpecialImage::SkSpecialImage(const SkIRect& subset,
                               uint32_t uniqueID,
                               const SkColorInfo& colorInfo,
                               const SkSurfaceProps& props)
    : fSubset(subset)
    , fUniqueID(kNeedNewImageUniqueID_SpecialImage == uniqueID ? SkNextID::ImageID() : uniqueID)
    , fColorInfo(colorInfo)
    , fProps(props) {
}

void SkSpecialImage::draw(SkCanvas* canvas,
                          SkScalar x, SkScalar y,
                          const SkSamplingOptions& sampling,
                          const SkPaint* paint, bool strict) const {
    SkRect dst = SkRect::MakeXYWH(x, y, this->subset().width(), this->subset().height());

    canvas->drawImageRect(this->asImage(), SkRect::Make(this->subset()), dst,
                          sampling, paint, strict ? SkCanvas::kStrict_SrcRectConstraint
                                                  : SkCanvas::kFast_SrcRectConstraint);
}

sk_sp<SkShader> SkSpecialImage::asShader(SkTileMode tileMode,
                                         const SkSamplingOptions& sampling,
                                         const SkMatrix& lm) const {
    return this->onAsShader(tileMode, sampling, lm);
}

sk_sp<SkShader> SkSpecialImage::asShader(const SkSamplingOptions& sampling) const {
    return this->asShader(sampling, SkMatrix::I());
}

sk_sp<SkShader> SkSpecialImage::asShader(const SkSamplingOptions& sampling,
                                         const SkMatrix& lm) const {
    return this->asShader(SkTileMode::kClamp, sampling, lm);
}

class SkSpecialImage_Raster final : public SkSpecialImage {
public:
    SkSpecialImage_Raster(const SkIRect& subset, const SkBitmap& bm, const SkSurfaceProps& props)
            : SkSpecialImage(subset, bm.getGenerationID(), bm.info().colorInfo(), props)
            , fBitmap(bm) {
        SkASSERT(bm.pixelRef());
        SkASSERT(fBitmap.getPixels());
    }

    bool getROPixels(SkBitmap* bm) const {
        return fBitmap.extractSubset(bm, this->subset());
    }

    size_t getSize() const override { return fBitmap.computeByteSize(); }

    sk_sp<SkImage> asImage() const override { return fBitmap.asImage(); }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        // No need to extract subset, onGetROPixels handles that when needed
        return SkSpecialImages::MakeFromRaster(subset, fBitmap, this->props());
    }

    sk_sp<SkShader> onAsShader(SkTileMode tileMode,
                               const SkSamplingOptions& sampling,
                               const SkMatrix& lm) const override {
        // TODO(skbug.com/12784): SkImage::makeShader() doesn't support a subset yet, but SkBitmap
        // supports subset views so create the shader from the subset bitmap instead of fBitmap.
        SkBitmap subsetBM;
        if (!this->getROPixels(&subsetBM)) {
            return nullptr;
        }
        return subsetBM.asImage()->makeShader(tileMode, tileMode, sampling, lm);
    }

private:
    SkBitmap fBitmap;
};

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset,
                                     const SkBitmap& bm,
                                     const SkSurfaceProps& props) {
    SkASSERT(bm.bounds().contains(subset));

    if (!bm.pixelRef()) {
        return nullptr;
    }

    const SkBitmap* srcBM = &bm;
    SkBitmap tmp;
    // ImageFilters only handle N32 at the moment, so force our src to be that
    if (!valid_for_imagefilters(bm.info())) {
        if (!tmp.tryAllocPixels(bm.info().makeColorType(kN32_SkColorType)) ||
            !bm.readPixels(tmp.info(), tmp.getPixels(), tmp.rowBytes(), 0, 0))
        {
            return nullptr;
        }
        srcBM = &tmp;
    }
    return sk_make_sp<SkSpecialImage_Raster>(subset, *srcBM, props);
}

sk_sp<SkSpecialImage> CopyFromRaster(const SkIRect& subset,
                                     const SkBitmap& bm,
                                     const SkSurfaceProps& props) {
    SkASSERT(bm.bounds().contains(subset));

    if (!bm.pixelRef()) {
        return nullptr;
    }

    SkBitmap tmp;
    SkImageInfo info = bm.info().makeDimensions(subset.size());
    // As in MakeFromRaster, must force src to N32 for ImageFilters
    if (!valid_for_imagefilters(bm.info())) {
        info = info.makeColorType(kN32_SkColorType);
    }
    if (!tmp.tryAllocPixels(info)) {
        return nullptr;
    }
    if (!bm.readPixels(tmp.info(), tmp.getPixels(), tmp.rowBytes(), subset.x(), subset.y())) {
        return nullptr;
    }

    // Since we're making a copy of the raster, the resulting special image is the exact size
    // of the requested subset of the original and no longer needs to be offset by subset's left
    // and top, since those were relative to the original's buffer.
    return sk_make_sp<SkSpecialImage_Raster>(
            SkIRect::MakeWH(subset.width(), subset.height()), tmp, props);
}

sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset,
                                     sk_sp<SkImage> image,
                                     const SkSurfaceProps& props) {
    if (!image || subset.isEmpty()) {
        return nullptr;
    }

    SkASSERT(image->bounds().contains(subset));
    SkASSERT(!image->isTextureBacked());

    // This will not work if the image is uploaded to a GPU render target.
    SkBitmap bm;
    if (as_IB(image)->getROPixels(nullptr, &bm)) {
        return MakeFromRaster(subset, bm, props);
    }
    return nullptr;
}

bool AsBitmap(const SkSpecialImage* img, SkBitmap* result) {
    if (!img || img->isGaneshBacked() || img->isGraphiteBacked()) {
        return false;
    }
    auto rasterImg = static_cast<const SkSpecialImage_Raster*>(img);
    return rasterImg->getROPixels(result);
}

}  // namespace SkSpecialImages

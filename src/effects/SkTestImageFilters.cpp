/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTestImageFilters.h"
#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkDownSampleImageFilter::onFilterImage(SkSpecialImage* source,
                                                             const Context& ctx,
                                                             SkIPoint* offset) const {
    if (fScale > SK_Scalar1 || fScale <= 0) {
        return nullptr;
    }

    int dstW = SkScalarRoundToInt(source->width() * fScale);
    int dstH = SkScalarRoundToInt(source->height() * fScale);
    if (dstW < 1) {
        dstW = 1;
    }
    if (dstH < 1) {
        dstH = 1;
    }

    sk_sp<SkSpecialImage> tmp;

    // downsample
    {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(dstW, dstH);

        sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
        if (!surf) {
            return nullptr;
        }

        SkCanvas* canvas = surf->getCanvas();
        SkASSERT(canvas);

        canvas->clear(0x0);

        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
        paint.setFilterQuality(kLow_SkFilterQuality);

        canvas->scale(fScale, fScale);
        source->draw(canvas, 0, 0, &paint);

        tmp = surf->makeImageSnapshot();
    }

    // upscale
    {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(source->width(), source->height());

        sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
        if (!surf) {
            return nullptr;
        }

        SkCanvas* canvas = surf->getCanvas();
        SkASSERT(canvas);

        canvas->clear(0x0);

        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);

        canvas->scale(SkScalarInvert(fScale), SkScalarInvert(fScale));
        tmp->draw(canvas, 0, 0, &paint);

        return surf->makeImageSnapshot();
    }
}

sk_sp<SkFlattenable> SkDownSampleImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    return Make(buffer.readScalar(), common.getInput(0));
}

void SkDownSampleImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fScale);
}

#ifndef SK_IGNORE_TO_STRING
void SkDownSampleImageFilter::toString(SkString* str) const {
    str->appendf("SkDownSampleImageFilter: (");
    str->append(")");
}
#endif

/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetImageFilter.h"

#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

sk_sp<SkImageFilter> SkOffsetImageFilter::Make(SkScalar dx, SkScalar dy,
                                               sk_sp<SkImageFilter> input,
                                               const CropRect* cropRect) {
    if (!SkScalarIsFinite(dx) || !SkScalarIsFinite(dy)) {
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkOffsetImageFilter(dx, dy, std::move(input), cropRect));
}

sk_sp<SkSpecialImage> SkOffsetImageFilter::onFilterImage(SkSpecialImage* source,
                                                         const Context& ctx,
                                                         SkIPoint* offset) const {
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &srcOffset));
    if (!input) {
        return nullptr;
    }

    SkVector vec;
    ctx.ctm().mapVectors(&vec, &fOffset, 1);

    if (!this->cropRectIsSet()) {
        offset->fX = srcOffset.fX + SkScalarRoundToInt(vec.fX);
        offset->fY = srcOffset.fY + SkScalarRoundToInt(vec.fY);
        return input;
    } else {
        SkIRect bounds;
        SkIRect srcBounds = SkIRect::MakeWH(input->width(), input->height());
        srcBounds.offset(srcOffset);
        if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
            return nullptr;
        }

        SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                                kPremul_SkAlphaType);
        sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
        if (!surf) {
            return nullptr;
        }

        SkCanvas* canvas = surf->getCanvas();
        SkASSERT(canvas);

        // TODO: it seems like this clear shouldn't be necessary (see skbug.com/5075)
        canvas->clear(0x0);

        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas->translate(SkIntToScalar(srcOffset.fX - bounds.fLeft),
                          SkIntToScalar(srcOffset.fY - bounds.fTop));

        input->draw(canvas, vec.x(), vec.y(), &paint);

        offset->fX = bounds.fLeft;
        offset->fY = bounds.fTop;
        return surf->makeImageSnapshot();
    }
}

SkRect SkOffsetImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.offset(fOffset.fX, fOffset.fY);
    return bounds;
}

SkIRect SkOffsetImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                MapDirection direction) const {
    SkVector vec;
    ctm.mapVectors(&vec, &fOffset, 1);
    if (kReverse_MapDirection == direction) {
        vec.negate();
    }

    return src.makeOffset(SkScalarCeilToInt(vec.fX), SkScalarCeilToInt(vec.fY));
}

sk_sp<SkFlattenable> SkOffsetImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkPoint offset;
    buffer.readPoint(&offset);
    return Make(offset.x(), offset.y(), common.getInput(0), &common.cropRect());
}

void SkOffsetImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fOffset);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkScalar dx, SkScalar dy,
                                         sk_sp<SkImageFilter> input,
                                         const CropRect* cropRect)
    : INHERITED(&input, 1, cropRect) {
    fOffset.set(dx, dy);
}

#ifndef SK_IGNORE_TO_STRING
void SkOffsetImageFilter::toString(SkString* str) const {
    str->appendf("SkOffsetImageFilter: (");
    str->appendf("offset: (%f, %f) ", fOffset.fX, fOffset.fY);
    str->append("input: (");
    if (this->getInput(0)) {
        this->getInput(0)->toString(str);
    }
    str->append("))");
}
#endif

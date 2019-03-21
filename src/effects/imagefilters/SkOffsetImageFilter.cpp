/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetImageFilter.h"
#include "SkCanvas.h"
#include "SkImageFilterPriv.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPointPriv.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

static SkIPoint map_offset_vector(const SkMatrix& ctm, const SkVector& offset) {
    SkVector vec = ctm.mapVector(offset.fX, offset.fY);
    return SkIPoint::Make(SkScalarRoundToInt(vec.fX), SkScalarRoundToInt(vec.fY));
}

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

    SkIPoint vec = map_offset_vector(ctx.ctm(), fOffset);

    if (!this->cropRectIsSet()) {
        offset->fX = Sk32_sat_add(srcOffset.fX, vec.fX);
        offset->fY = Sk32_sat_add(srcOffset.fY, vec.fY);
        return input;
    } else {
        SkIRect bounds;
        const SkIRect srcBounds = SkIRect::MakeXYWH(srcOffset.fX, srcOffset.fY,
                                                    input->width(), input->height());
        if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
            return nullptr;
        }

        sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
        if (!surf) {
            return nullptr;
        }

        SkCanvas* canvas = surf->getCanvas();
        SkASSERT(canvas);

        // TODO: it seems like this clear shouldn't be necessary (see skbug.com/5075)
        canvas->clear(0x0);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        canvas->translate(SkIntToScalar(srcOffset.fX - bounds.fLeft),
                          SkIntToScalar(srcOffset.fY - bounds.fTop));

        input->draw(canvas, vec.fX, vec.fY, &paint);

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
                                                MapDirection dir, const SkIRect* inputRect) const {
    SkIPoint vec = map_offset_vector(ctm, fOffset);
    if (kReverse_MapDirection == dir) {
        SkPointPriv::Negate(vec);
    }

    return src.makeOffset(vec.fX, vec.fY);
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

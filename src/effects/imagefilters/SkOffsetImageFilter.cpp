/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkOffsetImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkOffsetImageFilterImpl final : public SkImageFilter_Base {
public:
    SkOffsetImageFilterImpl(SkScalar dx, SkScalar dy, sk_sp<SkImageFilter> input,
                            const CropRect* cropRect)
            : INHERITED(&input, 1, cropRect) {
        fOffset.set(dx, dy);
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void SkOffsetImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkOffsetImageFilterImpl)

    SkVector fOffset;

    typedef SkImageFilter_Base INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkOffsetImageFilter::Make(SkScalar dx, SkScalar dy,
                                               sk_sp<SkImageFilter> input,
                                               const SkImageFilter::CropRect* cropRect) {
    if (!SkScalarIsFinite(dx) || !SkScalarIsFinite(dy)) {
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkOffsetImageFilterImpl(dx, dy, std::move(input), cropRect));
}

void SkOffsetImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkOffsetImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkOffsetImageFilter", SkOffsetImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkOffsetImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkPoint offset;
    buffer.readPoint(&offset);
    return SkOffsetImageFilter::Make(offset.x(), offset.y(), common.getInput(0),
                                     &common.cropRect());
}

void SkOffsetImageFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fOffset);
}

static SkIPoint map_offset_vector(const SkMatrix& ctm, const SkVector& offset) {
    SkVector vec = ctm.mapVector(offset.fX, offset.fY);
    return SkIPoint::Make(SkScalarRoundToInt(vec.fX), SkScalarRoundToInt(vec.fY));
}

sk_sp<SkSpecialImage> SkOffsetImageFilterImpl::onFilterImage(const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &srcOffset));
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

        sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size()));
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

SkRect SkOffsetImageFilterImpl::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.offset(fOffset.fX, fOffset.fY);
    return bounds;
}

SkIRect SkOffsetImageFilterImpl::onFilterNodeBounds(
        const SkIRect& src, const SkMatrix& ctm, MapDirection dir, const SkIRect* inputRect) const {
    SkIPoint vec = map_offset_vector(ctm, fOffset);
    if (kReverse_MapDirection == dir) {
        SkPointPriv::Negate(vec);
    }

    return src.makeOffset(vec.fX, vec.fY);
}

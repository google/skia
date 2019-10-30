/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkPaintImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkPaintImageFilterImpl final : public SkImageFilter_Base {
public:
    SkPaintImageFilterImpl(const SkPaint& paint, const CropRect* rect)
            : INHERITED(nullptr, 0, rect)
            , fPaint(paint) {}

    bool affectsTransparentBlack() const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    friend void SkPaintImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkPaintImageFilterImpl)

    SkPaint fPaint;

    typedef SkImageFilter_Base INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkPaintImageFilter::Make(const SkPaint& paint,
                                              const SkImageFilter::CropRect* cropRect) {
    return sk_sp<SkImageFilter>(new SkPaintImageFilterImpl(paint, cropRect));
}

void SkPaintImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkPaintImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPaintImageFilter", SkPaintImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkPaintImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    SkPaint paint;
    buffer.readPaint(&paint, nullptr);
    return SkPaintImageFilter::Make(paint, &common.cropRect());
}

void SkPaintImageFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePaint(fPaint);
}

sk_sp<SkSpecialImage> SkPaintImageFilterImpl::onFilterImage(const Context& ctx,
                                                            SkIPoint* offset) const {
    SkIRect bounds;
    const SkIRect srcBounds = SkIRect::MakeWH(ctx.sourceImage()->width(),
                                              ctx.sourceImage()->height());
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkRect rect = SkRect::MakeIWH(bounds.width(), bounds.height());
    SkMatrix inverse;
    if (matrix.invert(&inverse)) {
        inverse.mapRect(&rect);
    }
    canvas->setMatrix(matrix);
    if (rect.isFinite()) {
        canvas->drawRect(rect, fPaint);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

bool SkPaintImageFilterImpl::affectsTransparentBlack() const {
    return true;
}

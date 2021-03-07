/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkShaderImageFilter final : public SkImageFilter_Base {
public:
    SkShaderImageFilter(const SkPaint& paint, const SkRect* rect)
            : INHERITED(nullptr, 0, rect)
            , fPaint(paint) {}

    static sk_sp<SkImageFilter> Make(const SkPaint& paint, const SkRect* rect) {
        return sk_sp<SkImageFilter>(new SkShaderImageFilter(paint, rect));
    }

    bool affectsTransparentBlack() const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    friend void ::SkRegisterShaderImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkShaderImageFilter)

    // This filter only applies the shader and dithering policy of the paint.
    SkPaint fPaint;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

// TODO(michaelludwig) - Remove this deprecated factory once modules/svg is updated
sk_sp<SkImageFilter> SkImageFilters::Paint(const SkPaint& paint, const CropRect& cropRect) {
    return SkShaderImageFilter::Make(paint, cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader, Dither dither,
                                            const CropRect& cropRect) {
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setDither((bool) dither);
    return SkShaderImageFilter::Make(paint, cropRect);
}

void SkRegisterShaderImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkShaderImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPaintImageFilter", SkShaderImageFilter::CreateProc);
    SkFlattenable::Register("SkPaintImageFilterImpl", SkShaderImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkShaderImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    SkPaint paint;
    buffer.readPaint(&paint, nullptr);
    return SkShaderImageFilter::Make(paint, common.cropRect());
}

void SkShaderImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePaint(fPaint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkShaderImageFilter::onFilterImage(const Context& ctx,
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

bool SkShaderImageFilter::affectsTransparentBlack() const {
    return true;
}

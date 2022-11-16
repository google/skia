/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#include <utility>

namespace {

class SkShaderImageFilter final : public SkImageFilter_Base {
public:
    SkShaderImageFilter(sk_sp<SkShader> shader, SkImageFilters::Dither dither, const SkRect* rect)
            : INHERITED(nullptr, 0, rect)
            , fShader(std::move(shader))
            , fDither(dither) {}

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    friend void ::SkRegisterShaderImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkShaderImageFilter)

    bool onAffectsTransparentBlack() const override { return true; }

    sk_sp<SkShader> fShader;
    SkImageFilters::Dither fDither;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader,
                                            Dither dither,
                                            const CropRect& cropRect) {
    return sk_sp<SkImageFilter>(new SkShaderImageFilter(std::move(shader), dither, cropRect));
}

void SkRegisterShaderImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkShaderImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPaintImageFilter", SkShaderImageFilter::CreateProc);
    SkFlattenable::Register("SkPaintImageFilterImpl", SkShaderImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkShaderImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    sk_sp<SkShader> shader;
    bool dither;
    if (buffer.isVersionLT(SkPicturePriv::kShaderImageFilterSerializeShader)) {
        // The old implementation stored an entire SkPaint, but we only need the SkShader and dither
        // boolean. We could fail if the paint stores more effects than that, but this is simpler.
        SkPaint paint = buffer.readPaint();
        shader = paint.getShader() ? paint.refShader()
                                   : SkShaders::Color(paint.getColor4f(), nullptr);
        dither = paint.isDither();
    } else {
        shader = buffer.readShader();
        dither = buffer.readBool();
    }
    return SkImageFilters::Shader(std::move(shader),
                                  SkImageFilters::Dither(dither),
                                  common.cropRect());
}

void SkShaderImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fShader.get());
    buffer.writeBool(fDither == SkImageFilters::Dither::kYes);
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
        SkPaint paint;
        paint.setShader(fShader);
        paint.setDither(fDither == SkImageFilters::Dither::kYes);
        canvas->drawRect(rect, paint);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

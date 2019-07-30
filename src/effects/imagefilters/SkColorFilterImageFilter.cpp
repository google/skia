/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkColorFilterImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkColorFilterImageFilterImpl final : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkColorFilter> cf,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr) {
        if (!cf) {
            return nullptr;
        }

        SkColorFilter* inputCF;
        if (input && input->isColorFilterNode(&inputCF)) {
            // This is an optimization, as it collapses the hierarchy by just combining the two
            // colorfilters into a single one, which the new imagefilter will wrap.
            sk_sp<SkColorFilter> newCF = cf->makeComposed(sk_sp<SkColorFilter>(inputCF));
            if (newCF) {
                return sk_sp<SkImageFilter>(new SkColorFilterImageFilterImpl(
                        std::move(newCF), sk_ref_sp(input->getInput(0)), cropRect));
            }
        }

        return sk_sp<SkImageFilter>(new SkColorFilterImageFilterImpl(
                std::move(cf), std::move(input), cropRect));
    }

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    bool onIsColorFilterNode(SkColorFilter**) const override;
    bool onCanHandleComplexCTM() const override { return true; }
    bool affectsTransparentBlack() const override;

private:
    friend void SkColorFilterImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkColorFilterImageFilterImpl)

    SkColorFilterImageFilterImpl(sk_sp<SkColorFilter> cf, sk_sp<SkImageFilter> input,
                                 const CropRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fColorFilter(std::move(cf)) {}

    sk_sp<SkColorFilter> fColorFilter;

    typedef SkImageFilter INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkColorFilterImageFilter::Make(sk_sp<SkColorFilter> cf,
                                                    sk_sp<SkImageFilter> input,
                                                    const SkImageFilter::CropRect* cropRect) {
    return SkColorFilterImageFilterImpl::Make(std::move(cf), std::move(input), cropRect);
}

void SkColorFilterImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkColorFilterImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkColorFilterImageFilter", SkColorFilterImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkColorFilterImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    sk_sp<SkColorFilter> cf(buffer.readColorFilter());
    return Make(std::move(cf), common.getInput(0), &common.cropRect());
}

void SkColorFilterImageFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fColorFilter.get());
}

sk_sp<SkSpecialImage> SkColorFilterImageFilterImpl::onFilterImage(SkSpecialImage* source,
                                                                  const Context& ctx,
                                                                  SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));

    SkIRect inputBounds;
    if (fColorFilter->affectsTransparentBlack()) {
        // If the color filter affects transparent black, the bounds are the entire clip.
        inputBounds = ctx.clipBounds();
    } else if (!input) {
        return nullptr;
    } else {
        inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                        input->width(), input->height());
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    SkPaint paint;

    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColorFilter(fColorFilter);

    // TODO: it may not be necessary to clear or drawPaint inside the input bounds
    // (see skbug.com/5075)
    if (fColorFilter->affectsTransparentBlack()) {
        // The subsequent input->draw() call may not fill the entire canvas. For filters which
        // affect transparent black, ensure that the filter is applied everywhere.
        paint.setColor(SK_ColorTRANSPARENT);
        canvas->drawPaint(paint);
        paint.setColor(SK_ColorBLACK);
    } else {
        canvas->clear(0x0);
    }

    if (input) {
        input->draw(canvas,
                    SkIntToScalar(inputOffset.fX - bounds.fLeft),
                    SkIntToScalar(inputOffset.fY - bounds.fTop),
                    &paint);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

bool SkColorFilterImageFilterImpl::onIsColorFilterNode(SkColorFilter** filter) const {
    SkASSERT(1 == this->countInputs());
    if (!this->cropRectIsSet()) {
        if (filter) {
            *filter = SkRef(fColorFilter.get());
        }
        return true;
    }
    return false;
}

bool SkColorFilterImageFilterImpl::affectsTransparentBlack() const {
    return fColorFilter->affectsTransparentBlack();
}

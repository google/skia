/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkMergeImageFilter.h"

#include "include/core/SkCanvas.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkMergeImageFilterImpl final : public SkImageFilter {
public:
    SkMergeImageFilterImpl(sk_sp<SkImageFilter>* const filters, int count,
                           const CropRect* cropRect)
            : INHERITED(filters, count, cropRect) {
        SkASSERT(count >= 0);
    }

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    friend void SkMergeImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkMergeImageFilterImpl)

    typedef SkImageFilter INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkMergeImageFilter::Make(sk_sp<SkImageFilter>* const filters, int count,
                                               const SkImageFilter::CropRect* cropRect) {
    return sk_sp<SkImageFilter>(new SkMergeImageFilterImpl(filters, count, cropRect));
}

void SkMergeImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMergeImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMergeImageFilter", SkMergeImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkMergeImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    Common common;
    if (!common.unflatten(buffer, -1) || !buffer.isValid()) {
        return nullptr;
    }
    return SkMergeImageFilter::Make(common.inputs(), common.inputCount(), &common.cropRect());
}

sk_sp<SkSpecialImage> SkMergeImageFilterImpl::onFilterImage(
        SkSpecialImage* source, const Context& ctx, SkIPoint* offset) const {
    int inputCount = this->countInputs();
    if (inputCount < 1) {
        return nullptr;
    }

    SkIRect bounds;
    bounds.setEmpty();

    std::unique_ptr<sk_sp<SkSpecialImage>[]> inputs(new sk_sp<SkSpecialImage>[inputCount]);
    std::unique_ptr<SkIPoint[]> offsets(new SkIPoint[inputCount]);

    // Filter all of the inputs.
    for (int i = 0; i < inputCount; ++i) {
        offsets[i] = { 0, 0 };
        inputs[i] = this->filterInput(i, source, ctx, &offsets[i]);
        if (!inputs[i]) {
            continue;
        }
        const SkIRect inputBounds = SkIRect::MakeXYWH(offsets[i].fX, offsets[i].fY,
                                                      inputs[i]->width(), inputs[i]->height());
        bounds.join(inputBounds);
    }
    if (bounds.isEmpty()) {
        return nullptr;
    }

    // Apply the crop rect to the union of the inputs' bounds.
    // Note that the crop rect can only reduce the bounds, since this
    // filter does not affect transparent black.
    bool embiggen = false;
    this->getCropRect().applyTo(bounds, ctx.ctm(), embiggen, &bounds);
    if (!bounds.intersect(ctx.clipBounds())) {
        return nullptr;
    }

    const int x0 = bounds.left();
    const int y0 = bounds.top();

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    // Composite all of the filter inputs.
    for (int i = 0; i < inputCount; ++i) {
        if (!inputs[i]) {
            continue;
        }

        inputs[i]->draw(canvas,
                        SkIntToScalar(offsets[i].x() - x0), SkIntToScalar(offsets[i].y() - y0),
                        nullptr);
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return surf->makeImageSnapshot();
}

/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilterImageFilter.h"

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

SkImageFilter* SkColorFilterImageFilter::Create(SkColorFilter* cf, SkImageFilter* input,
                                                const CropRect* cropRect) {
    if (!cf) {
        return nullptr;
    }

    SkColorFilter* inputCF;
    if (input && input->isColorFilterNode(&inputCF)) {
        // This is an optimization, as it collapses the hierarchy by just combining the two
        // colorfilters into a single one, which the new imagefilter will wrap.
        sk_sp<SkColorFilter> newCF(SkColorFilter::MakeComposeFilter(sk_ref_sp(cf),
                                                                    sk_sp<SkColorFilter>(inputCF)));
        if (newCF) {
            return new SkColorFilterImageFilter(newCF.get(), input->getInput(0), cropRect);
        }
    }

    return new SkColorFilterImageFilter(cf, input, cropRect);
}

SkColorFilterImageFilter::SkColorFilterImageFilter(SkColorFilter* cf,
                                                   SkImageFilter* input,
                                                   const CropRect* cropRect)
    : INHERITED(1, &input, cropRect)
    , fColorFilter(SkRef(cf)) {
}

sk_sp<SkFlattenable> SkColorFilterImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    sk_sp<SkColorFilter> cf(buffer.readColorFilter());
    return sk_sp<SkFlattenable>(Create(cf.get(), common.getInput(0).get(), &common.cropRect()));
}

void SkColorFilterImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fColorFilter.get());
}

sk_sp<SkSpecialImage> SkColorFilterImageFilter::onFilterImage(SkSpecialImage* source,
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

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(), kPremul_SkAlphaType);
    sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    SkPaint paint;

    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColorFilter(fColorFilter);

    // TODO: it may not be necessary to clear or drawPaint inside the input bounds
    // (see skbug.com/5075)
    if (fColorFilter->affectsTransparentBlack()) {
        // The subsequent input->draw() call may not fill the entire canvas. For filters which
        // affect transparent black, ensure that the filter is applied everywhere.
        canvas->drawPaint(paint);
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

bool SkColorFilterImageFilter::onIsColorFilterNode(SkColorFilter** filter) const {
    SkASSERT(1 == this->countInputs());
    if (!this->cropRectIsSet()) {
        if (filter) {
            *filter = SkRef(fColorFilter.get());
        }
        return true;
    }
    return false;
}

bool SkColorFilterImageFilter::affectsTransparentBlack() const {
    return fColorFilter->affectsTransparentBlack();
}

#ifndef SK_IGNORE_TO_STRING
void SkColorFilterImageFilter::toString(SkString* str) const {
    str->appendf("SkColorFilterImageFilter: (");

    str->appendf("input: (");

    if (this->getInput(0)) {
        this->getInput(0)->toString(str);
    }

    str->appendf(") color filter: ");
    fColorFilter->toString(str);

    str->append(")");
}
#endif

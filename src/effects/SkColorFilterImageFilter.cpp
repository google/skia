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

SkFlattenable* SkColorFilterImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    sk_sp<SkColorFilter> cf(buffer.readColorFilter());
    return Create(cf.get(), common.getInput(0).get(), &common.cropRect());
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
    if (!input) {
        return nullptr;
    }

    SkIRect bounds;
    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.fX, inputOffset.fY,
                                                  input->width(), input->height());
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

    // TODO: it seems like this clear shouldn't be necessary (see skbug.com/5075)
    canvas->clear(0x0);

    SkPaint paint;

    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColorFilter(fColorFilter);

    input->draw(canvas,
                SkIntToScalar(inputOffset.fX - bounds.fLeft),
                SkIntToScalar(inputOffset.fY - bounds.fTop),
                &paint);

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

bool SkColorFilterImageFilter::canComputeFastBounds() const {
    if (fColorFilter->affectsTransparentBlack()) {
        return false;
    }
    return INHERITED::canComputeFastBounds();
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

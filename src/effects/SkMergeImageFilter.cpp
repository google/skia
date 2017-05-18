/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMergeImageFilter.h"

#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

sk_sp<SkImageFilter> SkMergeImageFilter::Make(sk_sp<SkImageFilter> first,
                                              sk_sp<SkImageFilter> second,
                                              SkBlendMode mode,
                                              const CropRect* cropRect) {
    sk_sp<SkImageFilter> inputs[2] = { first, second };
    SkBlendMode modes[2] = { mode, mode };
    return sk_sp<SkImageFilter>(new SkMergeImageFilter(inputs, 2, modes, cropRect));
}

sk_sp<SkImageFilter> SkMergeImageFilter::MakeN(sk_sp<SkImageFilter> filters[], int count,
                                               const SkBlendMode modes[],
                                               const CropRect* cropRect) {
    return sk_sp<SkImageFilter>(new SkMergeImageFilter(filters, count, modes, cropRect));
}

///////////////////////////////////////////////////////////////////////////////

void SkMergeImageFilter::initAllocModes() {
    int inputCount = this->countInputs();
    if (inputCount) {
        size_t size = sizeof(uint8_t) * inputCount;
        if (size <= sizeof(fStorage)) {
            fModes = SkTCast<uint8_t*>(fStorage);
        } else {
            fModes = SkTCast<uint8_t*>(sk_malloc_throw(size));
        }
    } else {
        fModes = nullptr;
    }
}

void SkMergeImageFilter::initModes(const SkBlendMode modes[]) {
    if (modes) {
        this->initAllocModes();
        int inputCount = this->countInputs();
        for (int i = 0; i < inputCount; ++i) {
            fModes[i] = SkToU8((unsigned)modes[i]);
        }
    } else {
        fModes = nullptr;
    }
}

SkMergeImageFilter::SkMergeImageFilter(sk_sp<SkImageFilter> filters[], int count,
                                       const SkBlendMode modes[],
                                       const CropRect* cropRect)
    : INHERITED(filters, count, cropRect) {
    SkASSERT(count >= 0);
    this->initModes(modes);
}

SkMergeImageFilter::~SkMergeImageFilter() {

    if (fModes != SkTCast<uint8_t*>(fStorage)) {
        sk_free(fModes);
    }
}

sk_sp<SkSpecialImage> SkMergeImageFilter::onFilterImage(SkSpecialImage* source, const Context& ctx,
                                                        SkIPoint* offset) const {
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
        offsets[i].setZero();
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

        SkPaint paint;
        if (fModes) {
            paint.setBlendMode((SkBlendMode)fModes[i]);
        }

        inputs[i]->draw(canvas,
                        SkIntToScalar(offsets[i].x() - x0), SkIntToScalar(offsets[i].y() - y0),
                        &paint);
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return surf->makeImageSnapshot();
}

sk_sp<SkImageFilter> SkMergeImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkSTArray<5, sk_sp<SkImageFilter>> inputs(this->countInputs());
    SkSTArray<5, SkBlendMode> modes(this->countInputs());
    for (int i = 0; i < this->countInputs(); i++) {
        inputs.push_back(this->getInput(i) ? this->getInput(i)->makeColorSpace(xformer) : nullptr);
        modes.push_back(fModes ? (SkBlendMode) fModes[i] : SkBlendMode::kSrcOver);
    }

    return SkMergeImageFilter::MakeN(inputs.begin(), this->countInputs(), modes.begin(),
                                     this->getCropRectIfSet());
}

sk_sp<SkFlattenable> SkMergeImageFilter::CreateProc(SkReadBuffer& buffer) {
    Common common;
    if (!common.unflatten(buffer, -1)) {
        return nullptr;
    }

    const int count = common.inputCount();
    bool hasModes = buffer.readBool();
    if (hasModes) {
        SkAutoSTArray<4, SkBlendMode> modes(count);
        SkAutoSTArray<4, uint8_t> modes8(count);
        if (!buffer.readByteArray(modes8.get(), count)) {
            return nullptr;
        }
        for (int i = 0; i < count; ++i) {
            modes[i] = (SkBlendMode)modes8[i];
            buffer.validate(SkIsValidMode(modes[i]));
        }
        if (!buffer.isValid()) {
            return nullptr;
        }
        return MakeN(common.inputs(), count, modes.get(), &common.cropRect());
    }
    return MakeN(common.inputs(), count, nullptr, &common.cropRect());
}

void SkMergeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeBool(fModes != nullptr);
    if (fModes) {
        buffer.writeByteArray(fModes, this->countInputs() * sizeof(fModes[0]));
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkMergeImageFilter::toString(SkString* str) const {
    str->appendf("SkMergeImageFilter: (");

    for (int i = 0; i < this->countInputs(); ++i) {
        SkImageFilter* filter = this->getInput(i);
        str->appendf("%d: (", i);
        filter->toString(str);
        str->appendf(")");
    }

    str->append(")");
}
#endif

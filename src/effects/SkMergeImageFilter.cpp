/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMergeImageFilter.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

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

void SkMergeImageFilter::initModes(const SkXfermode::Mode modes[]) {
    if (modes) {
        this->initAllocModes();
        int inputCount = this->countInputs();
        for (int i = 0; i < inputCount; ++i) {
            fModes[i] = SkToU8(modes[i]);
        }
    } else {
        fModes = nullptr;
    }
}

SkMergeImageFilter::SkMergeImageFilter(SkImageFilter* filters[], int count,
                                       const SkXfermode::Mode modes[],
                                       const CropRect* cropRect)
  : INHERITED(count, filters, cropRect) {
    SkASSERT(count >= 0);
    this->initModes(modes);
}

SkMergeImageFilter::~SkMergeImageFilter() {

    if (fModes != SkTCast<uint8_t*>(fStorage)) {
        sk_free(fModes);
    }
}

bool SkMergeImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                       const Context& ctx,
                                       SkBitmap* result, SkIPoint* offset) const {
    int inputCount = this->countInputs();
    if (inputCount < 1) {
        return false;
    }

    SkIRect bounds;

    SkAutoTDeleteArray<SkBitmap> inputs(new SkBitmap[inputCount]);
    SkAutoTDeleteArray<SkIPoint> offsets(new SkIPoint[inputCount]);
    bool didProduceResult = false;

    // Filter all of the inputs.
    for (int i = 0; i < inputCount; ++i) {
        inputs[i] = src;
        offsets[i].setZero();
        if (!this->filterInput(i, proxy, src, ctx, &inputs[i], &offsets[i])) {
            inputs[i].reset();
            continue;
        }
        SkIRect srcBounds;
        inputs[i].getBounds(&srcBounds);
        srcBounds.offset(offsets[i]);
        if (!didProduceResult) {
            bounds = srcBounds;
            didProduceResult = true;
        } else {
            bounds.join(srcBounds);
        }
    }
    if (!didProduceResult) {
        return false;
    }

    // Apply the crop rect to the union of the inputs' bounds.
    if (!this->getCropRect().applyTo(bounds, ctx, &bounds)) {
        return false;
    }

    const int x0 = bounds.left();
    const int y0 = bounds.top();

    // Allocate the destination buffer.
    SkAutoTUnref<SkBaseDevice> dst(proxy->createDevice(bounds.width(), bounds.height()));
    if (nullptr == dst) {
        return false;
    }
    SkCanvas canvas(dst);

    // Composite all of the filter inputs.
    for (int i = 0; i < inputCount; ++i) {
        SkPaint paint;
        if (fModes) {
            paint.setXfermodeMode((SkXfermode::Mode)fModes[i]);
        }
        canvas.drawBitmap(inputs[i], SkIntToScalar(offsets[i].x() - x0),
                                     SkIntToScalar(offsets[i].y() - y0), &paint);
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    *result = dst->accessBitmap(false);
    return true;
}

SkFlattenable* SkMergeImageFilter::CreateProc(SkReadBuffer& buffer) {
    Common common;
    if (!common.unflatten(buffer, -1)) {
        return nullptr;
    }

    const int count = common.inputCount();
    bool hasModes = buffer.readBool();
    if (hasModes) {
        SkAutoSTArray<4, SkXfermode::Mode> modes(count);
        SkAutoSTArray<4, uint8_t> modes8(count);
        if (!buffer.readByteArray(modes8.get(), count)) {
            return nullptr;
        }
        for (int i = 0; i < count; ++i) {
            modes[i] = (SkXfermode::Mode)modes8[i];
            buffer.validate(SkIsValidMode(modes[i]));
        }
        if (!buffer.isValid()) {
            return nullptr;
        }
        return Create(common.inputs(), count, modes.get(), &common.cropRect());
    }
    return Create(common.inputs(), count, nullptr, &common.cropRect());
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

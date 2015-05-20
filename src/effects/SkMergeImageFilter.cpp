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
    int inputCount = countInputs();
    if (inputCount) {
        size_t size = sizeof(uint8_t) * inputCount;
        if (size <= sizeof(fStorage)) {
            fModes = SkTCast<uint8_t*>(fStorage);
        } else {
            fModes = SkTCast<uint8_t*>(sk_malloc_throw(size));
        }
    } else {
        fModes = NULL;
    }
}

void SkMergeImageFilter::initModes(const SkXfermode::Mode modes[]) {
    if (modes) {
        this->initAllocModes();
        int inputCount = countInputs();
        for (int i = 0; i < inputCount; ++i) {
            fModes[i] = SkToU8(modes[i]);
        }
    } else {
        fModes = NULL;
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
    if (countInputs() < 1) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, src, SkIPoint::Make(0, 0), &bounds)) {
        return false;
    }

    const int x0 = bounds.left();
    const int y0 = bounds.top();

    SkAutoTUnref<SkBaseDevice> dst(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == dst) {
        return false;
    }
    SkCanvas canvas(dst);
    SkPaint paint;

    bool didProduceResult = false;
    int inputCount = countInputs();
    for (int i = 0; i < inputCount; ++i) {
        SkBitmap tmp;
        const SkBitmap* srcPtr;
        SkIPoint pos = SkIPoint::Make(0, 0);
        SkImageFilter* filter = getInput(i);
        if (filter) {
            if (!filter->filterImage(proxy, src, ctx, &tmp, &pos)) {
                continue;
            }
            srcPtr = &tmp;
        } else {
            srcPtr = &src;
        }

        if (fModes) {
            paint.setXfermodeMode((SkXfermode::Mode)fModes[i]);
        } else {
            paint.setXfermode(NULL);
        }
        canvas.drawSprite(*srcPtr, pos.x() - x0, pos.y() - y0, &paint);
        didProduceResult = true;
    }

    if (!didProduceResult)
        return false;

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    *result = dst->accessBitmap(false);
    return true;
}

SkFlattenable* SkMergeImageFilter::CreateProc(SkReadBuffer& buffer) {
    Common common;
    if (!common.unflatten(buffer, -1)) {
        return NULL;
    }

    const int count = common.inputCount();
    bool hasModes = buffer.readBool();
    if (hasModes) {
        SkAutoSTArray<4, SkXfermode::Mode> modes(count);
        SkAutoSTArray<4, uint8_t> modes8(count);
        if (!buffer.readByteArray(modes8.get(), count)) {
            return NULL;
        }
        for (int i = 0; i < count; ++i) {
            modes[i] = (SkXfermode::Mode)modes8[i];
            buffer.validate(SkIsValidMode(modes[i]));
        }
        if (!buffer.isValid()) {
            return NULL;
        }
        return Create(common.inputs(), count, modes.get(), &common.cropRect());
    }
    return Create(common.inputs(), count, NULL, &common.cropRect());
}

void SkMergeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeBool(fModes != NULL);
    if (fModes) {
        buffer.writeByteArray(fModes, countInputs() * sizeof(fModes[0]));
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

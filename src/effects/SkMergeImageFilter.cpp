/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMergeImageFilter.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkFlattenableBuffers.h"
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

SkMergeImageFilter::SkMergeImageFilter(SkImageFilter* first, SkImageFilter* second,
                                       SkXfermode::Mode mode,
                                       const CropRect* cropRect) : INHERITED(first, second, cropRect) {
    if (SkXfermode::kSrcOver_Mode != mode) {
        SkXfermode::Mode modes[] = { mode, mode };
        this->initModes(modes);
    } else {
        fModes = NULL;
    }
}

SkMergeImageFilter::SkMergeImageFilter(SkImageFilter* filters[], int count,
                                       const SkXfermode::Mode modes[],
                                       const CropRect* cropRect) : INHERITED(count, filters, cropRect) {
    SkASSERT(count >= 0);
    this->initModes(modes);
}

SkMergeImageFilter::~SkMergeImageFilter() {

    if (fModes != SkTCast<uint8_t*>(fStorage)) {
        sk_free(fModes);
    }
}

bool SkMergeImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                        SkIRect* dst) {
    if (countInputs() < 1) {
        return false;
    }

    SkIRect totalBounds;

    int inputCount = countInputs();
    for (int i = 0; i < inputCount; ++i) {
        SkImageFilter* filter = getInput(i);
        SkIRect r;
        if (filter) {
            if (!filter->filterBounds(src, ctm, &r)) {
                return false;
            }
        } else {
            r = src;
        }
        if (0 == i) {
            totalBounds = r;
        } else {
            totalBounds.join(r);
        }
    }

    // don't modify dst until now, so we don't accidentally change it in the
    // loop, but then return false on the next filter.
    *dst = totalBounds;
    return true;
}

bool SkMergeImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                       const SkMatrix& ctm,
                                       SkBitmap* result, SkIPoint* loc) {
    if (countInputs() < 1) {
        return false;
    }

    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
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

    int inputCount = countInputs();
    for (int i = 0; i < inputCount; ++i) {
        SkBitmap tmp;
        const SkBitmap* srcPtr;
        SkIPoint pos = SkIPoint::Make(0, 0);
        SkImageFilter* filter = getInput(i);
        if (filter) {
            if (!filter->filterImage(proxy, src, ctm, &tmp, &pos)) {
                return false;
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
    }

    loc->fX += bounds.left();
    loc->fY += bounds.top();
    *result = dst->accessBitmap(false);
    return true;
}

void SkMergeImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeBool(fModes != NULL);
    if (fModes) {
        buffer.writeByteArray(fModes, countInputs() * sizeof(fModes[0]));
    }
}

SkMergeImageFilter::SkMergeImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(-1, buffer) {
    bool hasModes = buffer.readBool();
    if (hasModes) {
        this->initAllocModes();
        int nbInputs = countInputs();
        size_t size = nbInputs * sizeof(fModes[0]);
        SkASSERT(buffer.getArrayCount() == size);
        if (buffer.readByteArray(fModes, size)) {
            for (int i = 0; i < nbInputs; ++i) {
                buffer.validate(SkIsValidMode((SkXfermode::Mode)fModes[i]));
            }
        }
    } else {
        fModes = 0;
    }
}

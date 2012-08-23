/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilterImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkColorFilter.h"
#include "SkFlattenableBuffers.h"

SkColorFilterImageFilter::SkColorFilterImageFilter(SkColorFilter* cf, SkImageFilter* input) : INHERITED(input), fColorFilter(cf) {
    SkSafeRef(cf);
}

SkColorFilterImageFilter::SkColorFilterImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fColorFilter = buffer.readFlattenableT<SkColorFilter>();
}

void SkColorFilterImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeFlattenable(fColorFilter);
}

SkColorFilterImageFilter::~SkColorFilterImageFilter() {
    SkSafeUnref(fColorFilter);
}

bool SkColorFilterImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                             const SkMatrix& matrix,
                                             SkBitmap* result,
                                             SkIPoint* loc) {
    SkBitmap src = this->getInputResult(proxy, source, matrix, loc);
    SkColorFilter* cf = fColorFilter;
    if (NULL == cf) {
        *result = src;
        return true;
    }

    SkAutoTUnref<SkDevice> device(proxy->createDevice(src.width(), src.height()));
    SkCanvas canvas(device.get());
    SkPaint paint;

    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColorFilter(fColorFilter);
    canvas.drawSprite(src, 0, 0, &paint);

    *result = device.get()->accessBitmap(false);
    return true;
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkColorFilterImageFilter)


/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRectShaderImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkFlattenableBuffers.h"
#include "SkShader.h"

SkRectShaderImageFilter* SkRectShaderImageFilter::Create(SkShader* s, const SkRect& rect) {
    SkASSERT(s);
#ifdef SK_CROP_RECT_IS_INT
    SkIRect cropRect;
    if (rect.width() == 0 || rect.height() == 0) {
        cropRect = SkIRect::MakeLargest();
    } else {
        rect.roundOut(&cropRect);
    }
#else
    uint32_t flags = CropRect::kHasAll_CropEdge;
    if (rect.width() == 0 || rect.height() == 0) {
        flags = 0x0;
    }
    CropRect cropRect(rect, flags);
#endif
    return SkNEW_ARGS(SkRectShaderImageFilter, (s, &cropRect));
}

SkRectShaderImageFilter* SkRectShaderImageFilter::Create(SkShader* s, const CropRect* cropRect) {
    SkASSERT(s);
    return SkNEW_ARGS(SkRectShaderImageFilter, (s, cropRect));
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkShader* s, const CropRect* cropRect)
  : INHERITED(NULL, cropRect)
  , fShader(s) {
    SkASSERT(s);
    s->ref();
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fShader = buffer.readFlattenableT<SkShader>();
}

void SkRectShaderImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeFlattenable(fShader);
}

SkRectShaderImageFilter::~SkRectShaderImageFilter() {
    SkSafeUnref(fShader);
}

bool SkRectShaderImageFilter::onFilterImage(Proxy* proxy,
                                            const SkBitmap& source,
                                            const SkMatrix& ctm,
                                            SkBitmap* result,
                                            SkIPoint* offset) {
    SkIRect bounds;
    source.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(),
                                                          bounds.height()));
    SkCanvas canvas(device.get());
    SkPaint paint;
    paint.setShader(fShader);
    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    fShader->setLocalMatrix(matrix);
    SkRect rect = SkRect::MakeWH(SkIntToScalar(bounds.width()), SkIntToScalar(bounds.height()));
    canvas.drawRect(rect, paint);
    *result = device.get()->accessBitmap(false);
    offset->fX += bounds.fLeft;
    offset->fY += bounds.fTop;
    return true;
}

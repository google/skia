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
    return SkNEW_ARGS(SkRectShaderImageFilter, (s, rect));
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkShader* s, const SkRect& rect)
  : INHERITED(NULL)
  , fShader(s)
  , fRect(rect) {
    SkASSERT(s);
    s->ref();
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fShader = buffer.readFlattenableT<SkShader>();
    buffer.readRect(&fRect);
}

void SkRectShaderImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeFlattenable(fShader);
    buffer.writeRect(fRect);
}

SkRectShaderImageFilter::~SkRectShaderImageFilter() {
    SkSafeUnref(fShader);
}

bool SkRectShaderImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source,
                                        const SkMatrix& matrix,
                                        SkBitmap* result,
                                        SkIPoint* loc) {
    SkAutoTUnref<SkDevice> device(proxy->createDevice(SkScalarCeilToInt(fRect.width()),
                                                      SkScalarCeilToInt(fRect.height())));
    SkCanvas canvas(device.get());
    SkPaint paint;
    paint.setShader(fShader);
    canvas.drawRect(fRect, paint);
    *result = device.get()->accessBitmap(false);
    return true;
}

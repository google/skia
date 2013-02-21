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

SkRectShaderImageFilter* SkRectShaderImageFilter::Create(SkShader* s, SkRect region) {
    SkASSERT(s);
    return SkNEW_ARGS(SkRectShaderImageFilter, (s, region));
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkShader* s, SkRect region)
  : INHERITED(NULL)
  , fShader(s)
  , fRegion(region) {
    SkASSERT(s);
    SkSafeRef(s);
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fShader = buffer.readFlattenableT<SkShader>();
    buffer.readRect(&fRegion);
}

void SkRectShaderImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeFlattenable(fShader);
    buffer.writeRect(fRegion);
}

SkRectShaderImageFilter::~SkRectShaderImageFilter() {
    SkSafeUnref(fShader);
}

bool SkRectShaderImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source,
                                        const SkMatrix& matrix,
                                        SkBitmap* result,
                                        SkIPoint* loc) {
    SkAutoTUnref<SkDevice> device(proxy->createDevice(SkScalarCeilToInt(fRegion.width()),
                                                      SkScalarCeilToInt(fRegion.height())));
    SkCanvas canvas(device.get());
    SkPaint paint;
    paint.setShader(fShader);
    canvas.drawRect(fRegion, paint);
    *result = device.get()->accessBitmap(false);
    return true;
}


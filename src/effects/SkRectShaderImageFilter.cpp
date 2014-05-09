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
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkShader.h"

SkRectShaderImageFilter* SkRectShaderImageFilter::Create(SkShader* s, const SkRect& rect) {
    SkASSERT(s);
    uint32_t flags = CropRect::kHasAll_CropEdge;
    if (rect.width() == 0 || rect.height() == 0) {
        flags = 0x0;
    }
    CropRect cropRect(rect, flags);
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

SkRectShaderImageFilter::SkRectShaderImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fShader = buffer.readShader();
}

void SkRectShaderImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeFlattenable(fShader);
}

SkRectShaderImageFilter::~SkRectShaderImageFilter() {
    SkSafeUnref(fShader);
}

bool SkRectShaderImageFilter::onFilterImage(Proxy* proxy,
                                            const SkBitmap& source,
                                            const Context& ctx,
                                            SkBitmap* result,
                                            SkIPoint* offset) const {
    SkIRect bounds;
    if (!this->applyCropRect(ctx, source, SkIPoint::Make(0, 0), &bounds)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(),
                                                          bounds.height()));
    if (NULL == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());
    SkPaint paint;
    paint.setShader(fShader);
    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    fShader->setLocalMatrix(matrix);
    SkRect rect = SkRect::MakeWH(SkIntToScalar(bounds.width()), SkIntToScalar(bounds.height()));
    canvas.drawRect(rect, paint);
    *result = device.get()->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

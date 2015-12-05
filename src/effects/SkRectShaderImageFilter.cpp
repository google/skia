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

SkImageFilter* SkRectShaderImageFilter::Create(SkShader* s, const SkRect& rect) {
    SkASSERT(s);
    uint32_t flags = CropRect::kHasAll_CropEdge;
    if (rect.width() == 0 || rect.height() == 0) {
        flags = 0x0;
    }
    CropRect cropRect(rect, flags);
    return s ? new SkRectShaderImageFilter(s, &cropRect) : nullptr;
}

SkImageFilter* SkRectShaderImageFilter::Create(SkShader* s, const CropRect* cropRect) {
    SkASSERT(s);
    return s ? new SkRectShaderImageFilter(s, cropRect) : nullptr;
}

SkRectShaderImageFilter::SkRectShaderImageFilter(SkShader* s, const CropRect* cropRect)
  : INHERITED(0, nullptr, cropRect)
  , fShader(SkRef(s)) {
}

SkFlattenable* SkRectShaderImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    SkAutoTUnref<SkShader> shader(buffer.readShader());
    return Create(shader.get(), &common.cropRect());
}

void SkRectShaderImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fShader);
}

SkRectShaderImageFilter::~SkRectShaderImageFilter() {
    fShader->unref();
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
    if (nullptr == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());

    SkPaint paint;
    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkSafeUnref(paint.setShader(SkShader::CreateLocalMatrixShader(fShader, matrix)));

    SkRect rect = SkRect::MakeWH(SkIntToScalar(bounds.width()), SkIntToScalar(bounds.height()));
    canvas.drawRect(rect, paint);

    *result = device.get()->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

bool SkRectShaderImageFilter::affectsTransparentBlack() const {
    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkRectShaderImageFilter::toString(SkString* str) const {
    str->appendf("SkRectShaderImageFilter: (");
    str->append(")");
}
#endif

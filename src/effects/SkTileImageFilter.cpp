/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTileImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkFlattenableBuffers.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkValidationUtils.h"

bool SkTileImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                      SkBitmap* dst, SkIPoint* offset) {
    SkBitmap source = src;
    SkImageFilter* input = getInput(0);
    SkIPoint localOffset = SkIPoint::Make(0, 0);
    if (input && !input->filterImage(proxy, src, ctm, &source, &localOffset)) {
        return false;
    }

    SkRect dstRect;
    ctm.mapRect(&dstRect, fDstRect);
    int w = SkScalarCeilToInt(dstRect.width());
    int h = SkScalarCeilToInt(dstRect.height());
    if (!fSrcRect.width() || !fSrcRect.height() || !w || !h) {
        return false;
    }

    SkRect srcRect;
    ctm.mapRect(&srcRect, fSrcRect);
    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);
    SkBitmap subset;
    SkIRect bounds;
    source.getBounds(&bounds);
    if (!srcIRect.intersect(bounds)) {
        return true;
    } else if (!source.extractSubset(&subset, srcIRect)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(w, h));
    if (NULL == device.get()) {
        return false;
    }
    SkCanvas canvas(device);
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkAutoTUnref<SkShader> shader(SkShader::CreateBitmapShader(subset,
                                  SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode));
    paint.setShader(shader);
    dstRect.offset(SkIntToScalar(localOffset.fX), SkIntToScalar(localOffset.fY));
    canvas.drawRect(dstRect, paint);
    *dst = device->accessBitmap(false);
    return true;
}

SkTileImageFilter::SkTileImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(1, buffer) {
    buffer.readRect(&fSrcRect);
    buffer.readRect(&fDstRect);
    buffer.validate(buffer.isValid() && SkIsValidRect(fSrcRect) && SkIsValidRect(fDstRect));
}

void SkTileImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
}

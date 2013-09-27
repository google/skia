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

bool SkTileImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                      SkBitmap* dst, SkIPoint* offset) {
    SkBitmap source = src;
    SkImageFilter* input = getInput(0);
    SkIPoint localOffset = SkIPoint::Make(0, 0);
    if (input && !input->filterImage(proxy, src, ctm, &source, &localOffset)) {
        return false;
    }

    int w = SkScalarTruncToInt(fDstRect.width());
    int h = SkScalarTruncToInt(fDstRect.height());
    if (!fSrcRect.width() || !fSrcRect.height() || !w || !h) {
        return false;
    }

    SkIRect srcRect;
    fSrcRect.roundOut(&srcRect);
    SkBitmap subset;
    if (!source.extractSubset(&subset, srcRect)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(w, h));
    SkIRect bounds;
    source.getBounds(&bounds);
    SkCanvas canvas(device);
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkAutoTUnref<SkShader> shader(SkShader::CreateBitmapShader(subset,
                                  SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode));
    paint.setShader(shader);
    SkRect dstRect = fDstRect;
    dstRect.offset(SkIntToScalar(localOffset.fX), SkIntToScalar(localOffset.fY));
    canvas.drawRect(dstRect, paint);
    *dst = device->accessBitmap(false);
    return true;
}

SkTileImageFilter::SkTileImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    buffer.readRect(&fSrcRect);
    buffer.readRect(&fDstRect);
}

void SkTileImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
}

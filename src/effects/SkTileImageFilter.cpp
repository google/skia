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
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkValidationUtils.h"

SkTileImageFilter* SkTileImageFilter::Create(const SkRect& srcRect, const SkRect& dstRect,
                                             SkImageFilter* input) {
    if (!SkIsValidRect(srcRect) || !SkIsValidRect(dstRect)) {
        return NULL;
    }
    return SkNEW_ARGS(SkTileImageFilter, (srcRect, dstRect, input));
}

bool SkTileImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                      const Context& ctx,
                                      SkBitmap* dst, SkIPoint* offset) const {
    SkBitmap source = src;
    SkImageFilter* input = getInput(0);
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (input && !input->filterImage(proxy, src, ctx, &source, &srcOffset)) {
        return false;
    }

    SkRect dstRect;
    ctx.ctm().mapRect(&dstRect, fDstRect);
    const SkIRect dstIRect = dstRect.roundOut();
    int w = dstIRect.width();
    int h = dstIRect.height();
    if (!fSrcRect.width() || !fSrcRect.height() || !w || !h) {
        return false;
    }

    SkRect srcRect;
    ctx.ctm().mapRect(&srcRect, fSrcRect);
    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);
    srcIRect.offset(-srcOffset);
    SkBitmap subset;
    SkIRect bounds;
    source.getBounds(&bounds);

    if (!srcIRect.intersect(bounds)) {
        offset->fX = offset->fY = 0;
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

    SkMatrix shaderMatrix;
    shaderMatrix.setTranslate(SkIntToScalar(srcOffset.fX),
                              SkIntToScalar(srcOffset.fY));
    SkAutoTUnref<SkShader> shader(SkShader::CreateBitmapShader(subset,
                                  SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode,
                                  &shaderMatrix));
    paint.setShader(shader);
    canvas.translate(-dstRect.fLeft, -dstRect.fTop);
    canvas.drawRect(dstRect, paint);
    *dst = device->accessBitmap(false);
    offset->fX = dstIRect.fLeft;
    offset->fY = dstIRect.fTop;
    return true;
}

bool SkTileImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                       SkIRect* dst) const {
    SkRect srcRect;
    ctm.mapRect(&srcRect, fSrcRect);
    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);
    srcIRect.join(src);
    *dst = srcIRect;
    return true;
}

void SkTileImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    // This is a workaround for skia:3194.
    *dst = src;
    dst->join(fDstRect);
}

SkFlattenable* SkTileImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    return Create(src, dst, common.getInput(0));
}

void SkTileImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
}

#ifndef SK_IGNORE_TO_STRING
void SkTileImageFilter::toString(SkString* str) const {
    str->appendf("SkTileImageFilter: (");
    str->appendf("src: %.2f %.2f %.2f %.2f",
                 fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight, fSrcRect.fBottom);
    str->appendf(" dst: %.2f %.2f %.2f %.2f",
                 fDstRect.fLeft, fDstRect.fTop, fDstRect.fRight, fDstRect.fBottom);
    if (this->getInput(0)) {
        str->appendf("input: (");
        this->getInput(0)->toString(str);
        str->appendf(")");
    }
    str->append(")");
}
#endif

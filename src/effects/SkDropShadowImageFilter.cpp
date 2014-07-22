/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDropShadowImageFilter.h"

#include "SkBitmap.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilter.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy,
                                                 SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                                 SkImageFilter* input, const CropRect* cropRect)
    : INHERITED(1, &input, cropRect)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigmaX)
    , fSigmaY(sigmaY)
    , fColor(color)
{
}

SkDropShadowImageFilter::SkDropShadowImageFilter(SkReadBuffer& buffer)
 : INHERITED(1, buffer) {
    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fSigmaX = buffer.readScalar();
    fSigmaY = buffer.readScalar();
    fColor = buffer.readColor();
    buffer.validate(SkScalarIsFinite(fDx) &&
                    SkScalarIsFinite(fDy) &&
                    SkScalarIsFinite(fSigmaX) &&
                    SkScalarIsFinite(fSigmaY));
}

void SkDropShadowImageFilter::flatten(SkWriteBuffer& buffer) const
{
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
}

bool SkDropShadowImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                            const Context& ctx,
                                            SkBitmap* result, SkIPoint* offset) const
{
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset))
        return false;

    SkIRect bounds;
    if (!this->applyCropRect(ctx, src, srcOffset, &bounds)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());

    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkMaxScalar(0, sigma.fX);
    sigma.fY = SkMaxScalar(0, sigma.fY);
    SkAutoTUnref<SkImageFilter> blurFilter(SkBlurImageFilter::Create(sigma.fX, sigma.fY));
    SkAutoTUnref<SkColorFilter> colorFilter(
        SkColorFilter::CreateModeFilter(fColor, SkXfermode::kSrcIn_Mode));
    SkPaint paint;
    paint.setImageFilter(blurFilter.get());
    paint.setColorFilter(colorFilter.get());
    paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
    SkVector offsetVec = SkVector::Make(fDx, fDy);
    ctx.ctm().mapVectors(&offsetVec, 1);
    canvas.translate(SkIntToScalar(srcOffset.fX - bounds.fLeft),
                     SkIntToScalar(srcOffset.fY - bounds.fTop));
    canvas.drawBitmap(src, offsetVec.fX, offsetVec.fY, &paint);
    canvas.drawBitmap(src, 0, 0);
    *result = device->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

void SkDropShadowImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (getInput(0)) {
        getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }

    SkRect shadowBounds = *dst;
    shadowBounds.offset(fDx, fDy);
    shadowBounds.outset(SkScalarMul(fSigmaX, SkIntToScalar(3)),
                        SkScalarMul(fSigmaY, SkIntToScalar(3)));
    dst->join(shadowBounds);
}

bool SkDropShadowImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                             SkIRect* dst) const {
    SkIRect bounds = src;
    if (getInput(0) && !getInput(0)->filterBounds(src, ctm, &bounds)) {
        return false;
    }
    SkVector offsetVec = SkVector::Make(fDx, fDy);
    ctm.mapVectors(&offsetVec, 1);
    bounds.offset(-SkScalarCeilToInt(offsetVec.x()),
                  -SkScalarCeilToInt(offsetVec.y()));
    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctm.mapVectors(&sigma, 1);
    bounds.outset(SkScalarCeilToInt(SkScalarMul(sigma.x(), SkIntToScalar(3))),
                  SkScalarCeilToInt(SkScalarMul(sigma.y(), SkIntToScalar(3))));
    bounds.join(src);
    *dst = bounds;
    return true;
}

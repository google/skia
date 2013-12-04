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
#include "SkFlattenableBuffers.h"

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigma, SkColor color, SkImageFilter* input)
    : INHERITED(input)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigma)
    , fSigmaY(sigma)
    , fColor(color)
{
}

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color, SkImageFilter* input, const CropRect* cropRect)
    : INHERITED(input, cropRect)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigmaX)
    , fSigmaY(sigmaY)
    , fColor(color)
{
}

SkDropShadowImageFilter::SkDropShadowImageFilter(SkFlattenableReadBuffer& buffer)
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

void SkDropShadowImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const
{
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
}

bool SkDropShadowImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source, const SkMatrix& matrix, SkBitmap* result, SkIPoint* loc)
{
    SkBitmap src = source;
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, matrix, &src, loc))
        return false;

    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, matrix)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());

    SkAutoTUnref<SkImageFilter> blurFilter(new SkBlurImageFilter(fSigmaX, fSigmaY));
    SkAutoTUnref<SkColorFilter> colorFilter(SkColorFilter::CreateModeFilter(fColor, SkXfermode::kSrcIn_Mode));
    SkPaint paint;
    paint.setImageFilter(blurFilter.get());
    paint.setColorFilter(colorFilter.get());
    paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
    canvas.translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    canvas.drawBitmap(src, fDx, fDy, &paint);
    canvas.drawBitmap(src, 0, 0);
    *result = device->accessBitmap(false);
    loc->fX += bounds.fLeft;
    loc->fY += bounds.fTop;
    return true;
}

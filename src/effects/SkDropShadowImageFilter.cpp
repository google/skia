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
    : SkImageFilter(input)
    , fDx(dx)
    , fDy(dy)
    , fSigma(sigma)
    , fColor(color)
{
}

SkDropShadowImageFilter::SkDropShadowImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer)
{
    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fSigma = buffer.readScalar();
    fColor = buffer.readColor();
}

void SkDropShadowImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const
{
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigma);
    buffer.writeColor(fColor);
}

bool SkDropShadowImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source, const SkMatrix& matrix, SkBitmap* result, SkIPoint* loc)
{
    SkBitmap src = source;
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, matrix, &src, loc))
        return false;

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(src.width(), src.height()));
    SkCanvas canvas(device.get());

    SkAutoTUnref<SkImageFilter> blurFilter(new SkBlurImageFilter(fSigma, fSigma));
    SkAutoTUnref<SkColorFilter> colorFilter(SkColorFilter::CreateModeFilter(fColor, SkXfermode::kSrcIn_Mode));
    SkPaint paint;
    paint.setImageFilter(blurFilter.get());
    paint.setColorFilter(colorFilter.get());
    paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
    canvas.drawBitmap(src, fDx, fDy, &paint);
    canvas.drawBitmap(src, 0, 0);
    *result = device->accessBitmap(false);
    return true;
}

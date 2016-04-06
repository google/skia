/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDropShadowImageFilter.h"

#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy,
                                                 SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                                 ShadowMode shadowMode, sk_sp<SkImageFilter> input,
                                                 const CropRect* cropRect)
    : INHERITED(&input, 1, cropRect)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigmaX)
    , fSigmaY(sigmaY)
    , fColor(color)
    , fShadowMode(shadowMode) {
}

sk_sp<SkFlattenable> SkDropShadowImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor color = buffer.readColor();
    ShadowMode shadowMode = buffer.isVersionLT(SkReadBuffer::kDropShadowMode_Version) ?
                            kDrawShadowAndForeground_ShadowMode :
                            static_cast<ShadowMode>(buffer.readInt());
    return Make(dx, dy, sigmaX, sigmaY, color, shadowMode, common.getInput(0), &common.cropRect());
}

void SkDropShadowImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
    buffer.writeInt(static_cast<int>(fShadowMode));
}

sk_sp<SkSpecialImage> SkDropShadowImageFilter::onFilterImage(SkSpecialImage* source,
                                                             const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                  input->width(), input->height());
    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                                  kPremul_SkAlphaType);
    sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkMaxScalar(0, sigma.fX);
    sigma.fY = SkMaxScalar(0, sigma.fY);

    SkPaint paint;
    paint.setImageFilter(SkBlurImageFilter::Make(sigma.fX, sigma.fY, nullptr));
    paint.setColorFilter(SkColorFilter::MakeModeFilter(fColor, SkXfermode::kSrcIn_Mode));
    paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);

    SkVector offsetVec = SkVector::Make(fDx, fDy);
    ctx.ctm().mapVectors(&offsetVec, 1);

    canvas->translate(SkIntToScalar(inputOffset.fX - bounds.fLeft),
                      SkIntToScalar(inputOffset.fY - bounds.fTop));
    input->draw(canvas, offsetVec.fX, offsetVec.fY, &paint);

    if (fShadowMode == kDrawShadowAndForeground_ShadowMode) {
        input->draw(canvas, 0, 0, nullptr);
    }
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

SkRect SkDropShadowImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect shadowBounds = bounds;
    shadowBounds.offset(fDx, fDy);
    shadowBounds.outset(SkScalarMul(fSigmaX, SkIntToScalar(3)),
                        SkScalarMul(fSigmaY, SkIntToScalar(3)));
    if (fShadowMode == kDrawShadowAndForeground_ShadowMode) {
        bounds.join(shadowBounds);
    } else {
        bounds = shadowBounds;
    }
    return bounds;
}

SkIRect SkDropShadowImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                    MapDirection direction) const {
    SkVector offsetVec = SkVector::Make(fDx, fDy);
    if (kReverse_MapDirection == direction) {
        offsetVec.negate();
    }
    ctm.mapVectors(&offsetVec, 1);
    SkIRect dst = src.makeOffset(SkScalarCeilToInt(offsetVec.x()),
                                 SkScalarCeilToInt(offsetVec.y()));
    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctm.mapVectors(&sigma, 1);
    dst.outset(SkScalarCeilToInt(SkScalarMul(sigma.x(), SkIntToScalar(3))),
                SkScalarCeilToInt(SkScalarMul(sigma.y(), SkIntToScalar(3))));
    if (fShadowMode == kDrawShadowAndForeground_ShadowMode) {
        dst.join(src);
    }
    return dst;
}

#ifndef SK_IGNORE_TO_STRING
void SkDropShadowImageFilter::toString(SkString* str) const {
    str->appendf("SkDropShadowImageFilter: (");

    str->appendf("dX: %f ", fDx);
    str->appendf("dY: %f ", fDy);
    str->appendf("sigmaX: %f ", fSigmaX);
    str->appendf("sigmaY: %f ", fSigmaY);

    str->append("Color: ");
    str->appendHex(fColor);

    static const char* gModeStrings[] = {
        "kDrawShadowAndForeground", "kDrawShadowOnly"
    };

    static_assert(kShadowModeCount == SK_ARRAY_COUNT(gModeStrings), "enum_mismatch");

    str->appendf(" mode: %s", gModeStrings[fShadowMode]);

    str->append(")");
}
#endif

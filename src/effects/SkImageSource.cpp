/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageSource.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

sk_sp<SkImageFilter> SkImageSource::Make(sk_sp<SkImage> image) {
    if (!image) {
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkImageSource(std::move(image)));
}

sk_sp<SkImageFilter> SkImageSource::Make(sk_sp<SkImage> image,
                                         const SkRect& srcRect,
                                         const SkRect& dstRect,
                                         SkFilterQuality filterQuality) {
    if (!image) {
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkImageSource(std::move(image),
                                                  srcRect, dstRect,
                                                  filterQuality));
}

SkImageSource::SkImageSource(sk_sp<SkImage> image)
    : INHERITED(nullptr, 0, nullptr)
    , fImage(std::move(image))
    , fSrcRect(SkRect::MakeIWH(fImage->width(), fImage->height()))
    , fDstRect(fSrcRect)
    , fFilterQuality(kHigh_SkFilterQuality) {
}

SkImageSource::SkImageSource(sk_sp<SkImage> image,
                             const SkRect& srcRect,
                             const SkRect& dstRect,
                             SkFilterQuality filterQuality)
    : INHERITED(nullptr, 0, nullptr)
    , fImage(std::move(image))
    , fSrcRect(srcRect)
    , fDstRect(dstRect)
    , fFilterQuality(filterQuality) {
}

sk_sp<SkFlattenable> SkImageSource::CreateProc(SkReadBuffer& buffer) {
    SkFilterQuality filterQuality = (SkFilterQuality)buffer.readInt();

    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);

    sk_sp<SkImage> image(buffer.readImage());
    if (!image) {
        return nullptr;
    }

    return SkImageSource::Make(std::move(image), src, dst, filterQuality);
}

void SkImageSource::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fFilterQuality);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
    buffer.writeImage(fImage.get());
}

sk_sp<SkSpecialImage> SkImageSource::onFilterImage(SkSpecialImage* source, const Context& ctx,
                                                   SkIPoint* offset) const {
    SkRect dstRect;
    ctx.ctm().mapRect(&dstRect, fDstRect);

    SkRect bounds = SkRect::MakeIWH(fImage->width(), fImage->height());
    if (fSrcRect == bounds && dstRect == bounds) {
        // No regions cropped out or resized; return entire image.
        offset->fX = offset->fY = 0;
        return SkSpecialImage::MakeFromImage(SkIRect::MakeWH(fImage->width(), fImage->height()),
                                             fImage,
                                             &source->props());
    }

    const SkIRect dstIRect = dstRect.roundOut();

    // SRGBTODO: Propagate SkColorType?
    const SkImageInfo info = SkImageInfo::MakeN32(dstIRect.width(), dstIRect.height(),
                                                  kPremul_SkAlphaType);

    sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    // TODO: it seems like this clear shouldn't be necessary (see skbug.com/5075)
    canvas->clear(0x0);

    SkPaint paint;

    // Subtract off the integer component of the translation (will be applied in offset, below).
    dstRect.offset(-SkIntToScalar(dstIRect.fLeft), -SkIntToScalar(dstIRect.fTop));
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    // FIXME: this probably shouldn't be necessary, but drawImageRect asserts
    // None filtering when it's translate-only
    paint.setFilterQuality(
        fSrcRect.width() == dstRect.width() && fSrcRect.height() == dstRect.height() ?
               kNone_SkFilterQuality : fFilterQuality);
    canvas->drawImageRect(fImage.get(), fSrcRect, dstRect, &paint,
                          SkCanvas::kStrict_SrcRectConstraint);

    offset->fX = dstIRect.fLeft;
    offset->fY = dstIRect.fTop;
    return surf->makeImageSnapshot();
}

SkRect SkImageSource::computeFastBounds(const SkRect& src) const {
    return fDstRect;
}

#ifndef SK_IGNORE_TO_STRING
void SkImageSource::toString(SkString* str) const {
    str->appendf("SkImageSource: (");
    str->appendf("src: (%f,%f,%f,%f) dst: (%f,%f,%f,%f) ",
                 fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight, fSrcRect.fBottom,
                 fDstRect.fLeft, fDstRect.fTop, fDstRect.fRight, fDstRect.fBottom);
    str->appendf("image: (%d,%d)",
                 fImage->width(), fImage->height());
    str->append(")");
}
#endif

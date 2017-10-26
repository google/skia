/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDrawableImageFilter.h"

#include "SkCanvas.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkPictureImageFilter.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

namespace {

class SkPictureDrawable : public SkDrawable {
 public:
  static sk_sp<SkPictureDrawable> Make(sk_sp<SkPicture> picture) {
    if (!picture)
      return nullptr;
    return sk_sp<SkPictureDrawable>(new SkPictureDrawable(std::move(picture)));
  }

  const SkPicture* picture() const { return fPicture.get(); }

  void flatten(SkWriteBuffer& buffer) const override {
    fPicture->flatten(buffer);
  }

 protected:
  SkRect onGetBounds() override { return fPicture->cullRect(); }
  void onDraw(SkCanvas* canvas) override { fPicture->playback(canvas); }

 private:
  SkPictureDrawable(sk_sp<SkPicture> picture)
   : fPicture(std::move(picture)) {}

  sk_sp<SkPicture> fPicture;

  typedef SkDrawable INHERITED;
};

sk_sp<SkDrawable> CreateFromPicture(SkReadBuffer& buffer) {
  if (buffer.isCrossProcess() && SkPicture::PictureIOSecurityPrecautionsEnabled()) {
      buffer.validate(!buffer.readBool());
      return nullptr;
  }

  if (!buffer.readBool())
    return nullptr;

  return SkPictureDrawable::Make(SkPicture::MakeFromBuffer(buffer));
}

sk_sp<SkDrawable> CreateFromDrawable(SkReadBuffer& buffer) {
  if (!buffer.readBool())
    return nullptr;

  // TODO(khushalsagar) : This needs an interface to allow the embedder to
  // create Drawables from the buffer.
  return nullptr;
}

}  // namespace

sk_sp<SkImageFilter> SkPictureImageFilter::Make(sk_sp<SkPicture> picture) {
  return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture)));
}

sk_sp<SkImageFilter> SkPictureImageFilter::Make(sk_sp<SkPicture> picture,
                                                const SkRect& cropRect) {
  return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture),
                                                       cropRect,
                                                       kDeviceSpace_DrawableResolution,
                                                       kLow_SkFilterQuality,
                                                       nullptr));
}

sk_sp<SkImageFilter> SkPictureImageFilter::MakeForLocalSpace(sk_sp<SkPicture> picture,
                                              const SkRect& cropRect,
                                              SkFilterQuality filterQuality) {
  return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(picture),
                                                       cropRect,
                                                       kLocalSpace_DrawableResolution,
                                                       filterQuality,
                                                       nullptr));
}

sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
  // This ensures backwards compatibility with older versions of SkPictureImageFilter, that
  // would not serialize the drawable type.
  return MakeFromBuffer(kPicture_DrawableType, buffer);
}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture)
  : INHERITED(kPicture_DrawableType, SkPictureDrawable::Make(std::move(picture))) {}

SkPictureImageFilter::SkPictureImageFilter(sk_sp<SkPicture> picture,
                                           const SkRect& cropRect,
                                           DrawableResolution resolution,
                                           SkFilterQuality quality,
                                           sk_sp<SkColorSpace> cs)
  : INHERITED(kPicture_DrawableType, SkPictureDrawable::Make(std::move(picture)),
              cropRect, resolution, quality, std::move(cs)) {}

sk_sp<SkImageFilter> SkDrawableImageFilter::Make(sk_sp<SkDrawable> Drawable) {
    return sk_sp<SkImageFilter>(new SkDrawableImageFilter(kDrawable_DrawableType,
                                                          std::move(Drawable)));
}

sk_sp<SkImageFilter> SkDrawableImageFilter::Make(sk_sp<SkDrawable> Drawable,
                                                const SkRect& cropRect) {
    return sk_sp<SkImageFilter>(new SkDrawableImageFilter(kDrawable_DrawableType,
                                                         std::move(Drawable),
                                                         cropRect,
                                                         kDeviceSpace_DrawableResolution,
                                                         kLow_SkFilterQuality,
                                                         nullptr));
}

sk_sp<SkImageFilter> SkDrawableImageFilter::MakeForLocalSpace(sk_sp<SkDrawable> Drawable,
                                                             const SkRect& cropRect,
                                                             SkFilterQuality filterQuality) {
    return sk_sp<SkImageFilter>(new SkDrawableImageFilter(kDrawable_DrawableType,
                                                         std::move(Drawable),
                                                         cropRect,
                                                         kLocalSpace_DrawableResolution,
                                                         filterQuality,
                                                         nullptr));
}

sk_sp<SkImageFilter> SkDrawableImageFilter::MakeFromBuffer(
    DrawableType drawableType, SkReadBuffer& buffer) {
  sk_sp<SkDrawable> drawable;
  SkRect cropRect;

  if (drawableType == kPicture_DrawableType)
    drawable = CreateFromPicture(buffer);
  else
    drawable = CreateFromDrawable(buffer);

  buffer.readRect(&cropRect);
  DrawableResolution drawableResolution = (DrawableResolution)buffer.readInt();

  SkFilterQuality filterQuality = kLow_SkFilterQuality;
  if (kLocalSpace_DrawableResolution == drawableResolution) {
      //filterLevel is only serialized if DrawableResolution is LocalSpace
      filterQuality = (SkFilterQuality)buffer.readInt();
  }

  return sk_sp<SkImageFilter>(new SkDrawableImageFilter(drawableType,
                                                        std::move(drawable),
                                                        cropRect,
                                                        drawableResolution,
                                                        filterQuality,
                                                        nullptr));
}

SkDrawableImageFilter::SkDrawableImageFilter(DrawableType drawableType,
                                             sk_sp<SkDrawable> Drawable)
    : INHERITED(nullptr, 0, nullptr)
    , fDrawableType(drawableType)
    , fDrawable(std::move(Drawable))
    , fCropRect(fDrawable ? fDrawable->getBounds() : SkRect::MakeEmpty())
    , fDrawableResolution(kDeviceSpace_DrawableResolution)
    , fFilterQuality(kLow_SkFilterQuality) {
}

SkDrawableImageFilter::SkDrawableImageFilter(DrawableType drawableType,
                                             sk_sp<SkDrawable> Drawable,
                                             const SkRect& cropRect,
                                             DrawableResolution DrawableResolution,
                                             SkFilterQuality filterQuality,
                                             sk_sp<SkColorSpace> colorSpace)
    : INHERITED(nullptr, 0, nullptr)
    , fDrawableType(drawableType)
    , fDrawable(std::move(Drawable))
    , fCropRect(cropRect)
    , fDrawableResolution(DrawableResolution)
    , fFilterQuality(filterQuality)
    , fColorSpace(std::move(colorSpace)) {
}

sk_sp<SkFlattenable> SkDrawableImageFilter::CreateProc(SkReadBuffer& buffer) {
  DrawableType drawableType = (DrawableType)buffer.readInt();
  return MakeFromBuffer(drawableType, buffer);
}

void SkDrawableImageFilter::flatten(SkWriteBuffer& buffer) const {
  if (fDrawableType == kPicture_DrawableType && buffer.isCrossProcess()
      && SkPicture::PictureIOSecurityPrecautionsEnabled()) {
       buffer.writeBool(false);
   } else {
       bool hasDrawable = (fDrawable != nullptr);
       buffer.writeBool(hasDrawable);
       if (fDrawable) {
         fDrawable->flatten(buffer);
       }
   }
   buffer.writeRect(fCropRect);
   buffer.writeInt(fDrawableResolution);
   if (kLocalSpace_DrawableResolution == fDrawableResolution) {
       buffer.writeInt(fFilterQuality);
   }
}

sk_sp<SkSpecialImage> SkDrawableImageFilter::onFilterImage(SkSpecialImage* source,
                                                          const Context& ctx,
                                                          SkIPoint* offset) const {
    if (!fDrawable) {
        return nullptr;
    }

    SkRect floatBounds;
    ctx.ctm().mapRect(&floatBounds, fCropRect);
    SkIRect bounds = floatBounds.roundOut();
    if (!bounds.intersect(ctx.clipBounds())) {
        return nullptr;
    }

    SkASSERT(!bounds.isEmpty());

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);
    canvas->clear(0x0);

    if (kDeviceSpace_DrawableResolution == fDrawableResolution ||
        0 == (ctx.ctm().getType() & ~SkMatrix::kTranslate_Mask)) {
        this->drawDrawableAtDeviceResolution(canvas, bounds, ctx);
    } else {
        this->drawDrawableAtLocalResolution(source, canvas, bounds, ctx);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

sk_sp<SkImageFilter> SkDrawableImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    sk_sp<SkColorSpace> dstCS = xformer->dst();
    if (SkColorSpace::Equals(dstCS.get(), fColorSpace.get())) {
        return this->refMe();
    }

    return sk_sp<SkImageFilter>(new SkDrawableImageFilter(fDrawableType, fDrawable, fCropRect, fDrawableResolution,
            fFilterQuality, std::move(dstCS)));
}

void SkDrawableImageFilter::drawDrawableAtDeviceResolution(SkCanvas* canvas,
                                                         const SkIRect& deviceBounds,
                                                         const Context& ctx) const {
    std::unique_ptr<SkCanvas> xformCanvas = nullptr;
    if (fColorSpace) {
        // Only non-null in the case where onMakeColorSpace() was called.  This instructs
        // us to do the color space xform on playback.
        xformCanvas = SkCreateColorSpaceXformCanvas(canvas, fColorSpace);
        canvas = xformCanvas.get();
    }
    canvas->translate(-SkIntToScalar(deviceBounds.fLeft), -SkIntToScalar(deviceBounds.fTop));
    canvas->concat(ctx.ctm());
    canvas->drawDrawable(fDrawable.get());
}

void SkDrawableImageFilter::drawDrawableAtLocalResolution(SkSpecialImage* source,
                                                        SkCanvas* canvas,
                                                        const SkIRect& deviceBounds,
                                                        const Context& ctx) const {
    SkMatrix inverseCtm;
    if (!ctx.ctm().invert(&inverseCtm)) {
        return;
    }

    SkRect localBounds = SkRect::Make(ctx.clipBounds());
    inverseCtm.mapRect(&localBounds);
    if (!localBounds.intersect(fCropRect)) {
        return;
    }
    SkIRect localIBounds = localBounds.roundOut();

    sk_sp<SkSpecialImage> localImg;
    {
        sk_sp<SkSpecialSurface> localSurface(source->makeSurface(ctx.outputProperties(),
                                                                 localIBounds.size()));
        if (!localSurface) {
            return;
        }

        SkCanvas* localCanvas = localSurface->getCanvas();
        SkASSERT(localCanvas);
        std::unique_ptr<SkCanvas> xformCanvas = nullptr;
        if (fColorSpace) {
            // Only non-null in the case where onMakeColorSpace() was called.  This instructs
            // us to do the color space xform on playback.
            xformCanvas = SkCreateColorSpaceXformCanvas(localCanvas, fColorSpace);
            localCanvas = xformCanvas.get();
        }

        localCanvas->clear(0x0);

        localCanvas->translate(-SkIntToScalar(localIBounds.fLeft),
                               -SkIntToScalar(localIBounds.fTop));
        localCanvas->drawDrawable(fDrawable.get());

        localImg = localSurface->makeImageSnapshot();
        SkASSERT(localImg);
    }

    {
        canvas->translate(-SkIntToScalar(deviceBounds.fLeft), -SkIntToScalar(deviceBounds.fTop));
        canvas->concat(ctx.ctm());
        SkPaint paint;
        paint.setFilterQuality(fFilterQuality);

        localImg->draw(canvas,
                       SkIntToScalar(localIBounds.fLeft),
                       SkIntToScalar(localIBounds.fTop),
                       &paint);
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkDrawableImageFilter::toString(SkString* str) const {
    str->appendf("SkDrawableImageFilter: (");
    str->appendf("crop: (%f,%f,%f,%f) ",
                 fCropRect.fLeft, fCropRect.fTop, fCropRect.fRight, fCropRect.fBottom);
    if (fDrawable) {
        str->appendf("Drawable: (%f,%f,%f,%f)",
                     fDrawable->getBounds().fLeft, fDrawable->getBounds().fTop,
                     fDrawable->getBounds().fRight, fDrawable->getBounds().fBottom);
    }
    str->append(")");
}
#endif

/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawableImageFilter_DEFINED
#define SkDrawableImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkDrawable.h"

class SK_API SkDrawableImageFilter : public SkImageFilter {
public:
    /**
     *  Refs the passed-in drawable.
     */
    static sk_sp<SkImageFilter> Make(sk_sp<SkDrawable> drawable);

    /**
     *  Refs the passed-in drawable. cropRect can be used to crop or expand the destination rect when
     *  the drawable is drawn. (No scaling is implied by the dest rect; only the CTM is applied.)
     */
    static sk_sp<SkImageFilter> Make(sk_sp<SkDrawable> drawable, const SkRect& cropRect);

    /**
     *  Refs the passed-in drawable. The drawable is rasterized at a resolution that matches the
     *  local coordinate space. If the drawable needs to be resampled for drawing it into the
     *  destination canvas, bilinear filtering will be used. cropRect can be used to crop or
     *  expand the destination rect when the drawable is drawn. (No scaling is implied by the
     *  dest rect; only the CTM is applied.)
     */
    static sk_sp<SkImageFilter> MakeForLocalSpace(sk_sp<SkDrawable> drawable,
                                                  const SkRect& cropRect,
                                                  SkFilterQuality filterQuality);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDrawableImageFilter)

protected:
    enum DrawableResolution {
        kDeviceSpace_DrawableResolution,
        kLocalSpace_DrawableResolution
    };

    enum DrawableType {
      kPicture_DrawableType,
      kDrawable_DrawableType
    };

    static sk_sp<SkImageFilter> MakeFromBuffer(DrawableType drawableType,
                                               SkReadBuffer& buffer);

    SkDrawableImageFilter(DrawableType drawableType, sk_sp<SkDrawable> drawable);
    SkDrawableImageFilter(DrawableType drawableType, sk_sp<SkDrawable> drawable, const SkRect& cropRect,
                         DrawableResolution, SkFilterQuality, sk_sp<SkColorSpace>);

    /*  Constructs an SkDrawableImageFilter object from an SkReadBuffer.
     *  @param SkReadBuffer Serialized drawable data.
     */
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

private:
    void drawDrawableAtDeviceResolution(SkCanvas* canvas,
                                       const SkIRect& deviceBounds,
                                       const Context&) const;
    void drawDrawableAtLocalResolution(SkSpecialImage* source,
                                      SkCanvas*,
                                      const SkIRect& deviceBounds,
                                      const Context&) const;

    DrawableType          fDrawableType;
    sk_sp<SkDrawable>      fDrawable;
    SkRect                fCropRect;
    DrawableResolution     fDrawableResolution;
    SkFilterQuality       fFilterQuality;

    // Should never be set by a public constructor.  This is only used when onMakeColorSpace()
    // forces a deferred color space xform.
    sk_sp<SkColorSpace>   fColorSpace;

    typedef SkImageFilter INHERITED;
};

#endif

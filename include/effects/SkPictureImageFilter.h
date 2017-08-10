/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureImageFilter_DEFINED
#define SkPictureImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkPicture.h"

class SK_API SkPictureImageFilter : public SkImageFilter {
public:
    /**
     *  Refs the passed-in picture.
     */
    static sk_sp<SkImageFilter> Make(sk_sp<SkPicture> picture);

    /**
     *  Refs the passed-in picture. cropRect can be used to crop or expand the destination rect when
     *  the picture is drawn. (No scaling is implied by the dest rect; only the CTM is applied.)
     */
    static sk_sp<SkImageFilter> Make(sk_sp<SkPicture> picture, const SkRect& cropRect);

    /**
     *  Refs the passed-in picture. The picture is rasterized at a resolution that matches the
     *  local coordinate space. If the picture needs to be resampled for drawing it into the
     *  destination canvas, bilinear filtering will be used. cropRect can be used to crop or
     *  expand the destination rect when the picture is drawn. (No scaling is implied by the
     *  dest rect; only the CTM is applied.)
     */
    static sk_sp<SkImageFilter> MakeForLocalSpace(sk_sp<SkPicture> picture,
                                                  const SkRect& cropRect,
                                                  SkFilterQuality filterQuality);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureImageFilter)

protected:
    enum PictureResolution {
        kDeviceSpace_PictureResolution,
        kLocalSpace_PictureResolution
    };

    /*  Constructs an SkPictureImageFilter object from an SkReadBuffer.
     *  Note: If the SkPictureImageFilter object construction requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling this constructor.
     *  @param SkReadBuffer Serialized picture data.
     */
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

private:
    explicit SkPictureImageFilter(sk_sp<SkPicture> picture);
    SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect,
                         PictureResolution, SkFilterQuality, sk_sp<SkColorSpace>);

    void drawPictureAtDeviceResolution(SkCanvas* canvas,
                                       const SkIRect& deviceBounds,
                                       const Context&) const;
    void drawPictureAtLocalResolution(SkSpecialImage* source,
                                      SkCanvas*,
                                      const SkIRect& deviceBounds,
                                      const Context&) const;

    sk_sp<SkPicture>      fPicture;
    SkRect                fCropRect;
    PictureResolution     fPictureResolution;
    SkFilterQuality       fFilterQuality;

    // Should never be set by a public constructor.  This is only used when onMakeColorSpace()
    // forces a deferred color space xform.
    sk_sp<SkColorSpace>   fColorSpace;

    typedef SkImageFilter INHERITED;
};

#endif

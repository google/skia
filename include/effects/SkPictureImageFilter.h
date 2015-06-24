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
    static SkPictureImageFilter* Create(const SkPicture* picture) {
        return SkNEW_ARGS(SkPictureImageFilter, (picture));
    }

    /**
     *  Refs the passed-in picture. cropRect can be used to crop or expand the destination rect when
     *  the picture is drawn. (No scaling is implied by the dest rect; only the CTM is applied.)
     */
    static SkPictureImageFilter* Create(const SkPicture* picture, const SkRect& cropRect) {
        return SkNEW_ARGS(SkPictureImageFilter, (picture, cropRect,
                                                 kDeviceSpace_PictureResolution,
                                                 kLow_SkFilterQuality));
    }

    /**
     *  Refs the passed-in picture. The picture is rasterized at a resolution that matches the
     *  local coordinate space. If the picture needs to be resampled for drawing it into the
     *  destination canvas, bilinear filtering will be used. cropRect can be used to crop or
     *  expand the destination rect when the picture is drawn. (No scaling is implied by the
     *  dest rect; only the CTM is applied.)
     */
    static SkPictureImageFilter* CreateForLocalSpace(const SkPicture* picture,
                                                     const SkRect& cropRect,
                                                     SkFilterQuality filterQuality) {
        return SkNEW_ARGS(SkPictureImageFilter, (picture, cropRect,
                                                 kLocalSpace_PictureResolution, filterQuality));
    }
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureImageFilter)

protected:
    enum PictureResolution {
        kDeviceSpace_PictureResolution,
        kLocalSpace_PictureResolution
    };

    explicit SkPictureImageFilter(const SkPicture* picture);
    SkPictureImageFilter(const SkPicture* picture, const SkRect& cropRect,
                         PictureResolution, SkFilterQuality);
    virtual ~SkPictureImageFilter();
    /*  Constructs an SkPictureImageFilter object from an SkReadBuffer.
     *  Note: If the SkPictureImageFilter object construction requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling this constructor.
     *  @param SkReadBuffer Serialized picture data.
     */
    void flatten(SkWriteBuffer&) const override;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const override;

private:


    void drawPictureAtDeviceResolution(SkBaseDevice*, const SkIRect& deviceBounds,
                                       const Context&) const;
    void drawPictureAtLocalResolution(Proxy*, SkBaseDevice*, const SkIRect& deviceBounds,
                                      const Context&) const;

    const SkPicture*      fPicture;
    SkRect                fCropRect;
    PictureResolution     fPictureResolution;
    SkFilterQuality       fFilterQuality;
    typedef SkImageFilter INHERITED;
};

#endif

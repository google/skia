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
    static SkPictureImageFilter* Create(const SkPicture* picture, int32_t uniqueID = 0) {
        return SkNEW_ARGS(SkPictureImageFilter, (picture, uniqueID));
    }

    /**
     *  Refs the passed-in picture. cropRect can be used to crop or expand the destination rect when
     *  the picture is drawn. (No scaling is implied by the dest rect; only the CTM is applied.)
     */
    static SkPictureImageFilter* Create(const SkPicture* picture, const SkRect& cropRect,
                                        uint32_t uniqueID = 0) {
        return SkNEW_ARGS(SkPictureImageFilter, (picture, cropRect, uniqueID,
                                                 kDeviceSpace_PictureResolution,
                                                 SkPaint::kLow_FilterLevel));
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
                                                     SkPaint::FilterLevel filterLevel,
                                                     uint32_t uniqueID = 0) {
        return SkNEW_ARGS(SkPictureImageFilter, (picture, cropRect, uniqueID,
                                                 kLocalSpace_PictureResolution, filterLevel));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureImageFilter)

protected:
    enum PictureResolution {
        kDeviceSpace_PictureResolution,
        kLocalSpace_PictureResolution
    };

    explicit SkPictureImageFilter(const SkPicture* picture, uint32_t uniqueID);
    SkPictureImageFilter(const SkPicture* picture, const SkRect& cropRect, uint32_t uniqueID,
                         PictureResolution, SkPaint::FilterLevel);
    virtual ~SkPictureImageFilter();
    /*  Constructs an SkPictureImageFilter object from an SkReadBuffer.
     *  Note: If the SkPictureImageFilter object construction requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling this constructor.
     *  @param SkReadBuffer Serialized picture data.
     */
    void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;

private:


    void drawPictureAtDeviceResolution(Proxy*, SkBaseDevice*, const SkIRect& deviceBounds,
                                       const Context&) const;
    void drawPictureAtLocalResolution(Proxy*, SkBaseDevice*, const SkIRect& deviceBounds,
                                      const Context&) const;

    const SkPicture*      fPicture;
    SkRect                fCropRect;
    PictureResolution     fPictureResolution;
    SkPaint::FilterLevel  fFilterLevel;
    typedef SkImageFilter INHERITED;
};

#endif

/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureImageFilter_DEFINED
#define SkPictureImageFilter_DEFINED

#include "SkFlattenable.h"
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


protected:
    /*  Constructs an SkPictureImageFilter object from an SkReadBuffer.
     *  Note: If the SkPictureImageFilter object construction requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling this constructor.
     *  @param SkReadBuffer Serialized picture data.
     */
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SK_FLATTENABLE_HOOKS(SkPictureImageFilter)

    explicit SkPictureImageFilter(sk_sp<SkPicture> picture);
    SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect);

    sk_sp<SkPicture>    fPicture;
    SkRect              fCropRect;

    typedef SkImageFilter INHERITED;
};

#endif

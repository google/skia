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
    explicit SkPictureImageFilter(SkPicture* picture);

    /**
     *  Refs the passed-in picture. rect can be used to crop or expand the destination rect when
     *  the picture is drawn. (No scaling is implied by the dest rect; only the CTM is applied.)
     */
    SkPictureImageFilter(SkPicture* picture, const SkRect& rect);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureImageFilter)

protected:
    virtual ~SkPictureImageFilter();
    /*  Constructs an SkPictureImageFilter object from an SkReadBuffer.
     *  Note: If the SkPictureImageFilter object construction requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling this constructor.
     *  @param SkReadBuffer Serialized picture data.
     */
    explicit SkPictureImageFilter(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;

private:
    SkPicture* fPicture;
    SkRect     fRect;
    typedef SkImageFilter INHERITED;
};

#endif

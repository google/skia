/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePriv_DEFINED
#define SkPicturePriv_DEFINED

#include "include/core/SkPicture.h"

class SkReadBuffer;
class SkWriteBuffer;

class SkPicturePriv {
public:
    /**
     *  Recreate a picture that was serialized into a buffer. If the creation requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling SkPicture::MakeFromBuffer().
     *  @param buffer Serialized picture data.
     *  @return A new SkPicture representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkPicture> MakeFromBuffer(SkReadBuffer& buffer);

    /**
     *  Serialize to a buffer.
     */
    static void Flatten(const sk_sp<const SkPicture> , SkWriteBuffer& buffer);

    // Returns NULL if this is not an SkBigPicture.
    static const SkBigPicture* AsSkBigPicture(const sk_sp<const SkPicture> picture) {
        return picture->asSkBigPicture();
    }
};

#endif

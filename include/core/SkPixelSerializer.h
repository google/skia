/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelSerializer_DEFINED
#define SkPixelSerializer_DEFINED

#include "SkData.h"
#include "SkPixmap.h"
#include "SkRefCnt.h"

struct SkImageInfo;

/**
 *  Interface for serializing pixels, e.g. SkBitmaps in an SkPicture.
 */
class SkPixelSerializer : public SkRefCnt {
public:
    /**
     *  Call to determine if the client wants to serialize the encoded data.
     *
     *  If the encoded data is can be re-encoded (or taken as is), this returns a ref to a data
     *  with the result, which the caller must unref() when they are through. The returned
     *  data may be the same as the input, or it may be different, but either way the caller is
     *  responsible for calling unref() on it.
     *
     *  If the encoded data is not acceptable to this pixel serializer, this returns NULL.
     */
    SkData* reencodeData(SkData* encoded);

    /**
     *  Call to get the client's version of encoding these pixels. If it
     *  returns NULL, serialize the raw pixels.
     */
    SkData* encodePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes);

    /**
     *  Call to get the client's version of encoding these pixels. If it
     *  returns NULL, serialize the raw pixels.
     */
    SkData* encodePixels(const SkPixmap& pixmap);
    
protected:
    // DEPRECATED -- this is no longer called, so remove from your subclasses!
    virtual bool onUseEncodedData(const void*, size_t) { return true; }

    virtual SkData* onReencodeData(SkData* encoded) {
        return SkRef(encoded);
    }

    virtual SkData* onEncodePixels(const SkImageInfo&, const void* pixels, size_t rowBytes) = 0;
};
#endif // SkPixelSerializer_DEFINED

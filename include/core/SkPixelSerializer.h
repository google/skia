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

/**
 *  Interface for serializing pixels, e.g. SkBitmaps in an SkPicture.
 */
class SkPixelSerializer : public SkRefCnt {
public:
    virtual ~SkPixelSerializer() {}

    /**
     *  Call to determine if the client wants to serialize the encoded data. If
     *  false, serialize another version (e.g. the result of encodePixels).
     */
    bool useEncodedData(const void* data, size_t len) {
        return this->onUseEncodedData(data, len);
    }

    /**
     *  Call to get the client's version of encoding these pixels. If it
     *  returns NULL, serialize the raw pixels.
     */
    sk_sp<SkData> encodeToData(const SkPixmap& pixmap) {
        return sk_sp<SkData>(this->onEncode(pixmap));
    }

protected:
    /**
     *  Return true if you want to serialize the encoded data, false if you want
     *  another version serialized (e.g. the result of this->encode()).
     */
    virtual bool onUseEncodedData(const void* data, size_t len) = 0;

    /**
     *  If you want to encode these pixels, return the encoded data as an SkData
     *  Return null if you want to serialize the raw pixels.
     */
    virtual SkData* onEncode(const SkPixmap&) = 0;
};
#endif // SkPixelSerializer_DEFINED

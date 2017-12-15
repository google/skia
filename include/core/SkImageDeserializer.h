/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageDeserializer_DEFINED
#define SkImageDeserializer_DEFINED

#include "SkRefCnt.h"

struct SkIRect;
class SkData;
class SkImage;

class SK_API SkImageDeserializer {
public:
    virtual ~SkImageDeserializer() {}

    /**
     *  Given a data containing serialized content, return an SkImage from it.
     *
     *  @param data The data containing the encoded image. The subclass may ref this for later
     *              decoding, or read it and process it immediately.
     *  @param subset Optional rectangle represent the subset of the encoded data that is being
     *                requested to be turned into an image.
     *  @return The new image, or nullptr on failure.
     *
     *  The default implementation is to call SkImage::MakeFromEncoded(...)
     */
    virtual sk_sp<SkImage> makeFromData(SkData*, const SkIRect* subset);
    virtual sk_sp<SkImage> makeFromMemory(const void* data, size_t length, const SkIRect* subset);
};

#endif

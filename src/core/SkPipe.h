/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipe_DEFINED
#define SkPipe_DEFINED

#include "SkData.h"
#include "SkImage.h"
#include "SkPicture.h"

class SkCanvas;
class SkImageDeserializer;
class SkImageSerializer;
class SkTypeface;
class SkTypefaceDeserializer;
class SkTypefaceSerializer;
class SkWStream;

struct SkRect;

class SkPipeSerializer {
public:
    SkPipeSerializer();
    ~SkPipeSerializer();

    // Ownership is not transferred, so caller must ceep the serializer alive
    void setTypefaceSerializer(SkTypefaceSerializer*);
    void setImageSerializer(SkImageSerializer*);

    void resetCache();

    sk_sp<SkData> writeImage(SkImage*);
    sk_sp<SkData> writePicture(SkPicture*);

    void writeImage(SkImage*, SkWStream*);
    void writePicture(SkPicture*, SkWStream*);

    SkCanvas* beginWrite(const SkRect& cullBounds, SkWStream*);
    void endWrite();

private:
    class Impl;
    std::unique_ptr<Impl> fImpl;
};

class SkPipeDeserializer {
public:
    SkPipeDeserializer();
    ~SkPipeDeserializer();

    // Ownership is not transferred, so caller must ceep the deserializer alive
    void setTypefaceDeserializer(SkTypefaceDeserializer*);
    void setImageDeserializer(SkImageDeserializer*);

    sk_sp<SkImage> readImage(const SkData* data) {
        if (!data) {
            return nullptr;
        }
        return this->readImage(data->data(), data->size());
    }

    sk_sp<SkPicture> readPicture(const SkData* data) {
        if (!data) {
            return nullptr;
        }
        return this->readPicture(data->data(), data->size());
    }

    sk_sp<SkImage> readImage(const void*, size_t);
    sk_sp<SkPicture> readPicture(const void*, size_t);

    bool playback(const void*, size_t, SkCanvas*);

private:
    class Impl;
    std::unique_ptr<Impl> fImpl;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class SkTypefaceSerializer {
public:
    virtual ~SkTypefaceSerializer() {}

    virtual sk_sp<SkData> serialize(SkTypeface*) = 0;
};

class SkImageSerializer {
public:
    virtual ~SkImageSerializer() {}

    virtual sk_sp<SkData> serialize(SkImage*) = 0;
};


class SkTypefaceDeserializer {
public:
    virtual ~SkTypefaceDeserializer() {}

    virtual sk_sp<SkTypeface> deserialize(const void* data, size_t size) = 0;
};

#endif

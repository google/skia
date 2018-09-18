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
#include "SkSerialProcs.h"

class SkCanvas;
class SkTypeface;
class SkWStream;

struct SkRect;

class SkPipeSerializer {
public:
    SkPipeSerializer();
    ~SkPipeSerializer();

    void setSerialProcs(const SkSerialProcs&);

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

    void setDeserialProcs(const SkDeserialProcs&);

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

#endif

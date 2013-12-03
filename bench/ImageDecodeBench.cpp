/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

class SkCanvas;

class ImageDecodeBench : public SkBenchmark {
public:
    ImageDecodeBench(void* p, const char* filename)
    : fName("image_decode_")
    , fFilename(filename)
    , fStream()
    , fValid(false) {
        fName.append(SkOSPath::SkBasename(filename));
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onPreDraw() SK_OVERRIDE {
        SkFILEStream fileStream(fFilename.c_str());
        fValid = fileStream.isValid() && fileStream.getLength() > 0;
        if (fValid) {
            const size_t size = fileStream.getLength();
            void* data = sk_malloc_throw(size);
            if (fileStream.read(data, size) < size) {
                fValid = false;
            } else {
                SkAutoTUnref<SkData> skdata(SkData::NewFromMalloc(data, size));
                fStream.setData(skdata.get());
            }
        }
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
#ifdef SK_DEBUG
        if (!fValid) {
            SkDebugf("stream was invalid: %s\n", fName.c_str());
            return;
        }
#endif
        // Decode a bunch of times
        SkBitmap bm;
        for (int i = 0; i < loops; ++i) {
            SkDEBUGCODE(bool success =) SkImageDecoder::DecodeStream(&fStream, &bm);
#ifdef SK_DEBUG
            if (!success) {
                SkDebugf("failed to decode %s\n", fName.c_str());
                return;
            }
#endif
            SkDEBUGCODE(success =) fStream.rewind();
#ifdef SK_DEBUG
            if (!success) {
                SkDebugf("failed to rewind %s\n", fName.c_str());
                return;
            }
#endif
        }
    }

private:
    SkString        fName;
    const SkString  fFilename;
    SkMemoryStream  fStream;
    bool            fValid;

    typedef SkBenchmark INHERITED;
};

// These are files which call decodePalette
//DEF_BENCH( return SkNEW_ARGS(ImageDecodeBench, ("/usr/local/google/home/scroggo/Downloads/images/hal_163x90.png")); )
//DEF_BENCH( return SkNEW_ARGS(ImageDecodeBench, ("/usr/local/google/home/scroggo/Downloads/images/box_19_top-left.png")); )

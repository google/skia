/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

class SkCanvas;

class SkipZeroesBench : public Benchmark {
public:
    SkipZeroesBench(const char* filename, bool skipZeroes)
    : fName("SkipZeroes_")
    , fDecoder(nullptr)
    , fFilename(filename)
    , fStream()
    , fSkipZeroes(skipZeroes)
    , fValid(false) {
        fName.append(filename);
        if (skipZeroes) {
            fName.append("_skip_zeroes");
        } else {
            fName.append("_write_zeroes");
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        SkString resourcePath = GetResourcePath();
        if (resourcePath.isEmpty()) {
            fValid = false;
            return;
        }

        SkString fullPath = SkOSPath::Join(resourcePath.c_str(), fFilename.c_str());
        SkFILEStream fileStream(fullPath.c_str());
        fValid = fileStream.isValid() && fileStream.getLength() > 0;
        if (fValid) {
            const size_t size = fileStream.getLength();
            void* data = sk_malloc_throw(size);
            if (fileStream.read(data, size) < size) {
                fValid = false;
            } else {
                SkAutoTUnref<SkData> skdata(SkData::NewFromMalloc(data, size));
                fStream.setData(skdata.get());
                fDecoder.reset(SkImageDecoder::Factory(&fStream));
                if (fDecoder.get()) {
                    fDecoder->setSkipWritingZeroes(fSkipZeroes);
                } else {
                    fValid = false;
                }
            }
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        if (!fValid) {
#ifdef SK_DEBUG
            SkDebugf("stream was invalid: %s\n", fFilename.c_str());
#endif
            return;
        }
        // Decode a bunch of times
        SkBitmap bm;
        for (int i = 0; i < loops; ++i) {
            SkDEBUGCODE(SkImageDecoder::Result result =) fDecoder->decode(&fStream, &bm,
                    SkImageDecoder::kDecodePixels_Mode);
#ifdef SK_DEBUG
            if (SkImageDecoder::kFailure == result) {
                SkDebugf("failed to decode %s\n", fFilename.c_str());
                return;
            }
#endif
            SkDEBUGCODE(bool success =) fStream.rewind();
#ifdef SK_DEBUG
            if (!success) {
                SkDebugf("failed to rewind %s\n", fFilename.c_str());
                return;
            }
#endif
        }
    }

private:
    SkString                        fName;
    SkAutoTDelete<SkImageDecoder>   fDecoder;
    const SkString                  fFilename;
    SkMemoryStream                  fStream;
    bool                            fSkipZeroes;
    bool                            fValid;

    typedef Benchmark INHERITED;
};

// Enable the true version once the feature is checked in.
DEF_BENCH(return new SkipZeroesBench("arrow.png", true));
DEF_BENCH(return new SkipZeroesBench("arrow.png", false));

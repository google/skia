
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkImageDecoder.h"
#include "SkString.h"

DEFINE_string(decodeBenchFilename, "resources/CMYK.jpeg", "Path to image for DecodeBench.");

static const char* gConfigName[] = {
    "ERROR", "a1", "a8", "index8", "565", "4444", "8888"
};

class DecodeBench : public SkBenchmark {
    SkBitmap::Config fPrefConfig;
    SkString fName;
public:
    DecodeBench(SkBitmap::Config c) {
        fPrefConfig = c;

        const char* fname = strrchr(FLAGS_decodeBenchFilename[0], '/');
        if (fname) {
            fname++; // skip the slash
        }
        fName.printf("decode_%s_%s", gConfigName[c], fname);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        for (int i = 0; i < loops; i++) {
            SkBitmap bm;
            SkImageDecoder::DecodeFile(FLAGS_decodeBenchFilename[0],
                                       &bm,
                                       fPrefConfig,
                                       SkImageDecoder::kDecodePixels_Mode);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return new DecodeBench(SkBitmap::kARGB_8888_Config); )
DEF_BENCH( return new DecodeBench(SkBitmap::kRGB_565_Config); )
DEF_BENCH( return new DecodeBench(SkBitmap::kARGB_4444_Config); )

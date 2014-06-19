
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkString.h"
#include "sk_tool_utils.h"

DEFINE_string(decodeBenchFilename, "resources/CMYK.jpeg", "Path to image for DecodeBench.");

class DecodeBench : public Benchmark {
    const SkColorType fPrefColorType;
    SkString          fName;
public:
    DecodeBench(SkColorType ct) : fPrefColorType(ct) {
        SkString fname = SkOSPath::SkBasename(FLAGS_decodeBenchFilename[0]);
        fName.printf("decode_%s_%s", sk_tool_utils::colortype_name(ct), fname.c_str());
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
            SkImageDecoder::DecodeFile(FLAGS_decodeBenchFilename[0], &bm, fPrefColorType,
                                       SkImageDecoder::kDecodePixels_Mode);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new DecodeBench(kN32_SkColorType); )
DEF_BENCH( return new DecodeBench(kRGB_565_SkColorType); )
DEF_BENCH( return new DecodeBench(kARGB_4444_SkColorType); )

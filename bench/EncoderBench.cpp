/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkBitmap.h"
#include "SkJpegEncoder.h"
#include "SkPngEncoder.h"
#include "SkWebpEncoder.h"
#include "SkStream.h"

class EncodeBench : public Benchmark {
public:
    using Encoder = bool (*)(SkWStream*, const SkPixmap&);
    EncodeBench(const char* filename, Encoder encoder, const char* encoderName)
        : fSourceFilename(filename)
        , fEncoder(encoder)
        , fName(SkStringPrintf("Encode_%s_%s", filename, encoderName)) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onPreDraw(SkCanvas*) override {
        SkAssertResult(GetResourceAsBitmap(fSourceFilename, &fBitmap));
    }

    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            SkPixmap pixmap;
            SkAssertResult(fBitmap.peekPixels(&pixmap));
            SkNullWStream dst;
            SkAssertResult(fEncoder(&dst, pixmap));
            SkASSERT(dst.bytesWritten() > 0);
        }
    }

private:
    const char* fSourceFilename;
    Encoder     fEncoder;
    SkString    fName;
    SkBitmap    fBitmap;
};

static bool encode_jpeg(SkWStream* dst, const SkPixmap& src) {
    SkJpegEncoder::Options opts;
    opts.fQuality = 90;
    return SkJpegEncoder::Encode(dst, src, opts);
}

static bool encode_webp_lossy(SkWStream* dst, const SkPixmap& src) {
    SkWebpEncoder::Options opts;
    opts.fCompression = SkWebpEncoder::Compression::kLossy;
    opts.fQuality = 90;
    opts.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return SkWebpEncoder::Encode(dst, src, opts);
}

static bool encode_webp_lossless(SkWStream* dst, const SkPixmap& src) {
    SkWebpEncoder::Options opts;
    opts.fCompression = SkWebpEncoder::Compression::kLossless;
    opts.fQuality = 90;
    opts.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return SkWebpEncoder::Encode(dst, src, opts);
}

static bool encode_png(SkWStream* dst,
                       const SkPixmap& src,
                       SkPngEncoder::FilterFlag filters,
                       int zlibLevel) {
    SkPngEncoder::Options opts;
    opts.fFilterFlags = filters;
    opts.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    opts.fZLibLevel = zlibLevel;
    return SkPngEncoder::Encode(dst, src, opts);
}

#define PNG(FLAG, ZLIBLEVEL) [](SkWStream* d, const SkPixmap& s) { \
           return encode_png(d, s, SkPngEncoder::FilterFlag::FLAG, ZLIBLEVEL); }

static const char* srcs[2] = {"mandrill_512.png", "color_wheel.jpg"};

// The Android Photos app uses a quality of 90 on JPEG encodes
DEF_BENCH(return new EncodeBench(srcs[0], &encode_jpeg, "JPEG"));
DEF_BENCH(return new EncodeBench(srcs[1], &encode_jpeg, "JPEG"));

// TODO: What is the appropriate quality to use to benchmark WEBP encodes?
DEF_BENCH(return new EncodeBench(srcs[0], encode_webp_lossy, "WEBP"));
DEF_BENCH(return new EncodeBench(srcs[1], encode_webp_lossy, "WEBP"));

DEF_BENCH(return new EncodeBench(srcs[0], encode_webp_lossless, "WEBP_LL"));
DEF_BENCH(return new EncodeBench(srcs[1], encode_webp_lossless, "WEBP_LL"));

DEF_BENCH(return new EncodeBench(srcs[0], PNG(kAll, 6), "PNG"));
DEF_BENCH(return new EncodeBench(srcs[0], PNG(kAll, 3), "PNG_3"));
DEF_BENCH(return new EncodeBench(srcs[0], PNG(kAll, 1), "PNG_1"));

DEF_BENCH(return new EncodeBench(srcs[0], PNG(kSub, 6), "PNG_6s"));
DEF_BENCH(return new EncodeBench(srcs[0], PNG(kSub, 3), "PNG_3s"));
DEF_BENCH(return new EncodeBench(srcs[0], PNG(kSub, 1), "PNG_1s"));

DEF_BENCH(return new EncodeBench(srcs[0], PNG(kNone, 6), "PNG_6n"));
DEF_BENCH(return new EncodeBench(srcs[0], PNG(kNone, 3), "PNG_3n"));
DEF_BENCH(return new EncodeBench(srcs[0], PNG(kNone, 1), "PNG_1n"));

DEF_BENCH(return new EncodeBench(srcs[1], PNG(kAll, 6), "PNG"));
DEF_BENCH(return new EncodeBench(srcs[1], PNG(kAll, 3), "PNG_3"));
DEF_BENCH(return new EncodeBench(srcs[1], PNG(kAll, 1), "PNG_1"));

DEF_BENCH(return new EncodeBench(srcs[1], PNG(kSub, 6), "PNG_6s"));
DEF_BENCH(return new EncodeBench(srcs[1], PNG(kSub, 3), "PNG_3s"));
DEF_BENCH(return new EncodeBench(srcs[1], PNG(kSub, 1), "PNG_1s"));

DEF_BENCH(return new EncodeBench(srcs[1], PNG(kNone, 6), "PNG_6n"));
DEF_BENCH(return new EncodeBench(srcs[1], PNG(kNone, 3), "PNG_3n"));
DEF_BENCH(return new EncodeBench(srcs[1], PNG(kNone, 1), "PNG_1n"));

#undef PNG

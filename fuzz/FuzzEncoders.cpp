/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "src/base/SkRandom.h"
#include "src/core/SkOSFile.h"

#include <vector>

// These values were picked arbitrarily to hopefully limit the size of the
// serialized SkPixmaps.
constexpr int MAX_WIDTH = 512;
constexpr int MAX_HEIGHT = 512;

static SkBitmap make_fuzzed_bitmap(Fuzz* fuzz) {
    SkBitmap bm;
    uint32_t w, h;
    fuzz->nextRange(&w, 1, MAX_WIDTH);
    fuzz->nextRange(&h, 1, MAX_HEIGHT);
    if (!bm.tryAllocPixels(SkImageInfo::MakeN32Premul(w, h))) {
        return bm;
    }
    uint32_t n = w * h;
    fuzz->nextN((SkPMColor*)bm.getPixels(), n);
    return bm;
}

DEF_FUZZ(PNGEncoder, fuzz) {
    auto bm = make_fuzzed_bitmap(fuzz);

    auto opts = SkPngEncoder::Options{};
    fuzz->nextRange(&opts.fZLibLevel, 0, 9);

    SkDynamicMemoryWStream dest;
    SkPngEncoder::Encode(&dest, bm.pixmap(), opts);
}

DEF_FUZZ(JPEGEncoder, fuzz) {
    auto bm = make_fuzzed_bitmap(fuzz);

    auto opts = SkJpegEncoder::Options{};
    fuzz->nextRange(&opts.fQuality, 0, 100);

    SkDynamicMemoryWStream dest;
    (void)SkJpegEncoder::Encode(&dest, bm.pixmap(), opts);
}

DEF_FUZZ(WEBPEncoder, fuzz) {
    auto bm = make_fuzzed_bitmap(fuzz);

    auto opts = SkWebpEncoder::Options{};
    fuzz->nextRange(&opts.fQuality, 0.0f, 100.0f);
    bool lossy;
    fuzz->next(&lossy);
    if (lossy) {
        opts.fCompression = SkWebpEncoder::Compression::kLossy;
    } else {
        opts.fCompression = SkWebpEncoder::Compression::kLossless;
    }

    SkDynamicMemoryWStream dest;
    (void)SkWebpEncoder::Encode(&dest, bm.pixmap(), opts);
}

// Not a real fuzz endpoint, but a helper to take in real, good images
// and dump out a corpus for this fuzzer.
DEF_FUZZ(_MakeEncoderCorpus, fuzz) {
    sk_sp<SkData> bytes = SkData::MakeWithoutCopy(fuzz->fData, fuzz->fSize);
    SkDebugf("bytes %zu\n", bytes->size());
    auto img = SkImages::DeferredFromEncodedData(bytes);
    if (nullptr == img.get()) {
        SkDebugf("invalid image, could not decode\n");
        return;
    }
    if (img->width() > MAX_WIDTH || img->height() > MAX_HEIGHT) {
        SkDebugf("Too big (%d x %d)\n", img->width(), img->height());
        return;
    }
    std::vector<int32_t> dstPixels;
    int rowBytes = img->width() * 4;
    dstPixels.resize(img->height() * rowBytes);
    SkPixmap pm(SkImageInfo::MakeN32Premul(img->width(), img->height()),
        &dstPixels.front(), rowBytes);
    if (!img->readPixels(nullptr, pm, 0, 0)) {
        SkDebugf("Could not read pixmap\n");
        return;
    }

    SkString s("./encoded_corpus/enc_");
    static SkRandom rand;
    s.appendU32(rand.nextU());
    auto file = sk_fopen(s.c_str(), SkFILE_Flags::kWrite_SkFILE_Flag);
    if (!file) {
        SkDebugf("Can't initialize file\n");
        return;
    }
    auto total = pm.info().bytesPerPixel() * pm.width() * pm.height();
    SkDebugf("Writing %d (%d x %d) bytes\n", total, pm.width(), pm.height());
    // Write out the size in two bytes since that's what the fuzzer will
    // read first.
    uint32_t w = pm.width();
    sk_fwrite(&w, sizeof(uint32_t), file);
    uint32_t h = pm.height();
    sk_fwrite(&h, sizeof(uint32_t), file);
    sk_fwrite(pm.addr(), total, file);
    sk_fclose(file);
}

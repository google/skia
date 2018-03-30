/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkBitmap.h"
#include "SkImage.h"
#include "SkImageInfo.h"
#include "SkJpegEncoder.h"
#include "SkPixmap.h"
#include "SkPngEncoder.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkWebpEncoder.h"
#include "SkOSFile.h"

#include <vector>

const int MAX_WIDTH = 512;
const int MAX_HEIGHT = 512;

SkPixmap makeSkPixmap(Fuzz* fuzz) {
    uint32_t w, h;
    fuzz->nextRange(&w, 1, MAX_WIDTH);
    fuzz->nextRange(&h, 1, MAX_HEIGHT);
    SkAutoTMalloc<SkPMColor> data(w * h);
    if (!data.get()) {
        return SkPixmap();
    }
    SkPixmap src(SkImageInfo::MakeN32Premul(w, h), data.get(), w * sizeof(SkPMColor));
    uint32_t n = w * h;
    fuzz->nextN(data.get(), n);
    (void)data.release();
    return src;
}

DEF_FUZZ(PNGEncoder, fuzz) {
    auto src = makeSkPixmap(fuzz);

    auto opts = SkPngEncoder::Options{};
    fuzz->nextRange(&opts.fZLibLevel, 0, 9);

    SkDynamicMemoryWStream dest;
    (void)SkPngEncoder::Encode(&dest, src, opts);
    // Free the pixels that were allocated, if any.
    sk_free((void*)src.addr());
}

DEF_FUZZ(JPEGEncoder, fuzz) {
    auto src = makeSkPixmap(fuzz);

    auto opts = SkJpegEncoder::Options{};
    fuzz->nextRange(&opts.fQuality, 0, 100);

    SkDynamicMemoryWStream dest;
    (void)SkJpegEncoder::Encode(&dest, src, opts);
    // Free the pixels that were allocated, if any.
    sk_free((void*)src.addr());
}

DEF_FUZZ(WEBPEncoder, fuzz) {
    auto src = makeSkPixmap(fuzz);

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
    (void)SkWebpEncoder::Encode(&dest, src, opts);
    // Free the pixels that were allocated, if any.
    sk_free((void*)src.addr());
}

SkRandom _rand;

// Not a real fuzz endpoint, but a helper to take in real, good images
// and dump out a corpus for this fuzzer.
DEF_FUZZ(_MakeEncoderCorpus, fuzz) {
    auto bytes = fuzz->fBytes;
    SkDebugf("bytes %d\n", bytes->size());
    auto img = SkImage::MakeFromEncoded(bytes);
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
    if (!img->readPixels(pm, 0, 0)) {
        SkDebugf("Could not read pixmap\n");
        return;
    }

    SkString s("./encoded_corpus/enc_");
    s.appendU32(_rand.nextU());
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

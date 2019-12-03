// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkOSFile.h"

static bool update(SkBitmap* maxBitmap, SkBitmap* minBitmap, const SkBitmap& bm) {
    SkASSERT(!bm.drawsNothing());
    SkASSERT(4 == bm.bytesPerPixel());
    if (maxBitmap->drawsNothing()) {
        maxBitmap->allocPixels(bm.info());
        maxBitmap->eraseColor(0x00000000);
        minBitmap->allocPixels(bm.info());
        minBitmap->eraseColor(0xFFFFFFFF);
    }
    if (maxBitmap->dimensions() != bm.dimensions()) {
        return false;
    }
    SkASSERT_RELEASE(maxBitmap->info() == bm.info());
    const SkPixmap& pmin = minBitmap->pixmap();
    const SkPixmap& pmax = maxBitmap->pixmap();
    const SkPixmap& pm = bm.pixmap();
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            uint32_t* minPtr = pmin.writable_addr32(x, y);
            uint32_t* maxPtr = pmax.writable_addr32(x, y);
            uint8_t minColor[4], maxColor[4], color[4];
            memcpy(minColor, minPtr, 4);
            memcpy(maxColor, maxPtr, 4);
            memcpy(color, pm.addr32(x, y), 4);
            for (unsigned i = 0; i < 4; ++i) {
                minColor[i] = std::min(minColor[i], color[i]);
                maxColor[i] = std::max(maxColor[i], color[i]);
            }
            memcpy(minPtr, minColor, 4);
            memcpy(maxPtr, maxColor, 4);
        }
    }
    return true;
}

static SkBitmap decode_to_srgb_8888_unpremul(const char* path) {
    SkBitmap dst;
    if (auto codec = SkCodec::MakeFromData(SkData::MakeFromFileName(path))) {
        SkISize size = codec->getInfo().dimensions();
        SkASSERT(!size.isEmpty());
        dst.allocPixels(SkImageInfo::Make(
                    size.width(), size.height(), kRGBA_8888_SkColorType,
                    kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB()));
        if (SkCodec::kSuccess != codec->getPixels(dst.pixmap())) {
            dst.reset();
        }
    }
    return dst;
}

bool encode_png(const char* path, const SkPixmap& pixmap) {
    if (!pixmap.addr()) {
        return false;
    }
    SkPngEncoder::Options encOpts;
    encOpts.fZLibLevel = 9;  // slow encode;
    SkFILEWStream o(path);
    return o.isValid() && SkPngEncoder::Encode(&o, pixmap, encOpts);
}

int main(int argc, char** argv) {
    SkASSERT_RELEASE(argc > 2);
    const char* src_dir = argv[1];
    const char* dst_dir = argv[2];
    SkBitmap maxBitmap, minBitmap;
    SkOSFile::Iter iter(src_dir);
    SkString name;
    while (iter.next(&name)) {
        name.prependf("%s/", src_dir);
        SkBitmap bm = decode_to_srgb_8888_unpremul(name.c_str());
        if (bm.drawsNothing()) {
            SkDebugf("'%s' failed to decode.\n", name.c_str());
            continue;
        }
        if (!update(&maxBitmap, &minBitmap, bm)) {
            SkDebugf("'%s' has unmatched dimensions.\n", name.c_str());
            continue;
        }
    }
    SkASSERT_RELEASE(sk_mkdir(dst_dir));
    if ((maxBitmap.drawsNothing()) || (maxBitmap.drawsNothing())) {
        SkDebugf("Failure: '%s' '%s'\n", src_dir, dst_dir);
        return 1;
    }
    encode_png(SkStringPrintf("%s/min.png", dst_dir).c_str(), minBitmap.pixmap());
    encode_png(SkStringPrintf("%s/max.png", dst_dir).c_str(), maxBitmap.pixmap());
    return 0;
}

/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkPngChunkReader.h"
#include "include/codec/SkPngRustDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkTemplates.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace {

// Reads every byte of each unknown chunk (e.g. `npTc` when
// `skia_use_rust_png_for_android=true`) so that ASAN catches out-of-bounds
// spans handed across the Rust/C++ boundary.
class FuzzChunkReader final : public SkPngChunkReader {
public:
    bool readChunk(const char tag[], const void* data, size_t length) override {
        volatile uint8_t sink = 0;
        for (size_t i = 0; i < 4; ++i) {
            sink ^= static_cast<uint8_t>(tag[i]);
        }
        const auto* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < length; ++i) {
            sink ^= bytes[i];
        }
        return true;
    }
};

}  // namespace

static void draw_if_decoded(const SkBitmap& bm, SkCodec::Result result) {
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            break;
        default:
            return;
    }
    if (auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(bm.dimensions()))) {
        surface->getCanvas()->drawImage(bm.asImage(), 0, 0);
    }
}

// Exercises one-shot full decode of up to the first 10 frames.
static void fuzz_frames(SkCodec* codec) {
    const SkImageInfo info = codec->getInfo();
    SkBitmap bm;
    if (!bm.tryAllocPixels(info)) {
        return;
    }

    const int frameCount = std::min(codec->getFrameCount(), 10);
    for (int i = 0; i < frameCount; ++i) {
        SkCodec::Options options;
        options.fFrameIndex = i;
        const SkCodec::Result result =
                codec->getPixels(info, bm.getPixels(), bm.rowBytes(), &options);
        draw_if_decoded(bm, result);
    }
}

// Exercises incremental decode. A second call on `kIncompleteInput` without new
// data checks that retrying a recoverable EOF remains safe.
static void fuzz_incremental_decode(SkCodec* codec) {
    const SkImageInfo info = codec->getInfo();
    SkBitmap bm;
    if (!bm.tryAllocPixels(info)) {
        return;
    }
    if (codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes()) != SkCodec::kSuccess) {
        return;
    }

    // Deliberately uninitialized to verify (under MSAN) that incrementalDecode
    // initializes it when it returns kIncompleteInput or kErrorInInput.
    int rowsDecoded;
    SkCodec::Result result = codec->incrementalDecode(&rowsDecoded);
    if (result == SkCodec::kIncompleteInput) {
        result = codec->incrementalDecode(&rowsDecoded);
    }
    if ((result == SkCodec::kIncompleteInput || result == SkCodec::kErrorInInput) &&
        rowsDecoded < bm.height()) {
        // Mirrors what clients do with `rowsDecoded`; ASAN/MSAN catch bogus values.
        void* dst = SkTAddOffset<void>(bm.getPixels(), rowsDecoded * bm.rowBytes());
        sk_bzero(dst, (bm.height() - rowsDecoded) * bm.rowBytes());
    }
    draw_if_decoded(bm, result);
}

// Decodes `codec` at `sampleSize`, restricted to `subset` if non-null.
static void decode_android(SkAndroidCodec* codec, int sampleSize, SkIRect* subset) {
    const SkISize size = subset ? codec->getSampledSubsetDimensions(sampleSize, *subset)
                                : codec->getSampledDimensions(sampleSize);
    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeN32Premul(size))) {
        return;
    }
    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = sampleSize;
    options.fSubset = subset;
    const SkCodec::Result result =
            codec->getAndroidPixels(bm.info(), bm.getPixels(), bm.rowBytes(), &options);
    draw_if_decoded(bm, result);
}

// Exercises SkAndroidCodec (SkSampledCodec) sampled, subset, and gainmap decodes,
// as used by BitmapFactory, ImageDecoder, and BitmapRegionDecoder.
static void fuzz_android_codec(std::unique_ptr<SkAndroidCodec> codec, const uint8_t params[5]) {
    if (!codec) {
        return;
    }

    const int sampleSize = (params[0] % 8) + 1;
    decode_android(codec.get(), sampleSize, /*subset=*/nullptr);

    const SkISize dims = codec->getInfo().dimensions();
    if (!dims.isEmpty()) {
        const int x = params[1] % dims.width();
        const int y = params[2] % dims.height();
        const int w = (params[3] % (dims.width() - x)) + 1;
        const int h = (params[4] % (dims.height() - y)) + 1;
        SkIRect subset = SkIRect::MakeXYWH(x, y, w, h);
        if (codec->getSupportedSubset(&subset)) {
            decode_android(codec.get(), sampleSize, &subset);
        }
    }

    // PNG gainmaps are only exposed through getGainmapAndroidCodec() (PNG does not
    // implement getAndroidGainmap()), which exercises SkPngRustCodec::onDecodeGainmap().
    SkGainmapInfo gainmapInfo;
    std::unique_ptr<SkAndroidCodec> gainmapCodec;
    if (codec->getGainmapAndroidCodec(&gainmapInfo, &gainmapCodec) && gainmapCodec) {
        decode_android(gainmapCodec.get(), sampleSize, /*subset=*/nullptr);
    }
}

bool FuzzPNGRustDecoder(const uint8_t* data, size_t size) {
    // Need enough trailing bytes to derive the sampling/subset parameters above.
    if (size < 5) {
        return false;
    }

    auto chunkReader = sk_make_sp<FuzzChunkReader>();
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngRustDecoder::Decode(
            SkMemoryStream::MakeDirect(data, size), &result, chunkReader.get());
    if (!codec || result != SkCodec::kSuccess) {
        return false;
    }

    fuzz_frames(codec.get());
    fuzz_incremental_decode(codec.get());

    // Derive sampling/subset parameters from the trailing bytes rather than a prefix,
    // so that plain PNG files remain valid seeds.
    std::unique_ptr<SkAndroidCodec> androidCodec =
            SkAndroidCodec::MakeFromCodec(SkPngRustDecoder::Decode(
                    SkMemoryStream::MakeDirect(data, size), &result, chunkReader.get()));
    fuzz_android_codec(std::move(androidCodec), data + size - 5);
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Limit input size to prevent timeouts on heavily-compressed streams.
    if (size > 65536) {
        return 0;
    }

    FuzzPNGRustDecoder(data, size);
    return 0;
}
#endif

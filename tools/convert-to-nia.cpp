/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This program converts an image from stdin (e.g. a JPEG, PNG, etc.) to stdout
// (in the NIA/NIE format, a trivial image file format).
//
// The NIA/NIE file format specification is at:
// https://github.com/google/wuffs/blob/master/doc/spec/nie-spec.md
//
// Pass "-1" or "-first-frame-only" as a command line flag to output NIE (a
// still image) instead of NIA (an animated image). The output format (NIA or
// NIE) depends only on this flag's absence or presence, not on the stdin
// image's format.
//
// There are multiple codec implementations of any given image format. For
// example, as of May 2020, Chromium, Skia and Wuffs each have their own BMP
// decoder implementation. There is no standard "libbmp" that they all share.
// Comparing this program's output (or hashed output) to similar programs in
// other repositories can identify image inputs for which these decoders (or
// different versions of the same decoder) produce different output (pixels).
//
// An equivalent program (using the Chromium image codecs) is at:
// https://crrev.com/c/2210331
//
// An equivalent program (using the Wuffs image codecs) is at:
// https://github.com/google/wuffs/blob/master/example/convert-to-nia/convert-to-nia.c

#include <stdio.h>
#include <string.h>

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "src/core/SkAutoMalloc.h"

static inline void set_u32le(uint8_t* ptr, uint32_t val) {
    ptr[0] = val >> 0;
    ptr[1] = val >> 8;
    ptr[2] = val >> 16;
    ptr[3] = val >> 24;
}

static inline void set_u64le(uint8_t* ptr, uint64_t val) {
    ptr[0] = val >> 0;
    ptr[1] = val >> 8;
    ptr[2] = val >> 16;
    ptr[3] = val >> 24;
    ptr[4] = val >> 32;
    ptr[5] = val >> 40;
    ptr[6] = val >> 48;
    ptr[7] = val >> 56;
}

static void write_nix_header(uint32_t magicU32le, uint32_t width, uint32_t height) {
    uint8_t data[16];
    set_u32le(data + 0, magicU32le);
    set_u32le(data + 4, 0x346E62FF);  // 4 bytes per pixel non-premul BGRA.
    set_u32le(data + 8, width);
    set_u32le(data + 12, height);
    fwrite(data, 1, 16, stdout);
}

static bool write_nia_duration(uint64_t totalDurationMillis) {
    // Flicks are NIA's unit of time. One flick (frame-tick) is 1 / 705_600_000
    // of a second. See https://github.com/OculusVR/Flicks
    static constexpr uint64_t flicksPerMilli = 705600;
    if (totalDurationMillis > (INT64_MAX / flicksPerMilli)) {
        // Converting from millis to flicks would overflow.
        return false;
    }

    uint8_t data[8];
    set_u64le(data + 0, totalDurationMillis * flicksPerMilli);
    fwrite(data, 1, 8, stdout);
    return true;
}

static void write_nie_pixels(uint32_t width, uint32_t height, const SkBitmap& bm) {
    static constexpr size_t kBufferSize = 4096;
    uint8_t                 buf[kBufferSize];
    size_t                  n = 0;
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            SkColor c = bm.getColor(x, y);
            buf[n++] = SkColorGetB(c);
            buf[n++] = SkColorGetG(c);
            buf[n++] = SkColorGetR(c);
            buf[n++] = SkColorGetA(c);
            if (n == kBufferSize) {
                fwrite(buf, 1, n, stdout);
                n = 0;
            }
        }
    }
    if (n > 0) {
        fwrite(buf, 1, n, stdout);
    }
}

static void write_nia_padding(uint32_t width, uint32_t height) {
    // 4 bytes of padding when the width and height are both odd.
    if (width & height & 1) {
        uint8_t data[4];
        set_u32le(data + 0, 0);
        fwrite(data, 1, 4, stdout);
    }
}

static void write_nia_footer(int repetitionCount, bool stillImage) {
    uint8_t data[8];
    if (stillImage || (repetitionCount == SkCodec::kRepetitionCountInfinite)) {
        set_u32le(data + 0, 0);
    } else {
        // NIA's loop count and Skia's repetition count differ by one. See
        // https://github.com/google/wuffs/blob/master/doc/spec/nie-spec.md#nii-footer
        set_u32le(data + 0, 1 + repetitionCount);
    }
    set_u32le(data + 4, 0x80000000);
    fwrite(data, 1, 8, stdout);
}

int main(int argc, char** argv) {
    bool firstFrameOnly = false;
    for (int a = 1; a < argc; a++) {
        if ((strcmp(argv[a], "-1") == 0) || (strcmp(argv[a], "-first-frame-only") == 0)) {
            firstFrameOnly = true;
            break;
        }
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(SkData::MakeFromFILE(stdin)));
    if (!codec) {
        SkDebugf("Decode failed.\n");
        return 1;
    }
    codec->getInfo().makeColorSpace(nullptr);
    SkBitmap bm;
    bm.allocPixels(codec->getInfo());
    size_t bmByteSize = bm.computeByteSize();

    // Cache a frame that future frames may depend on.
    int          cachedFrame = SkCodec::kNoFrame;
    SkAutoMalloc cachedFramePixels;

    uint64_t  totalDurationMillis = 0;
    const int frameCount = codec->getFrameCount();
    if (frameCount == 0) {
        SkDebugf("No frames.\n");
        return 1;
    }
    std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
    bool                            stillImage = frameInfos.size() <= 1;

    for (int i = 0; i < frameCount; i++) {
        SkCodec::Options opts;
        opts.fFrameIndex = i;

        if (!stillImage) {
            int durationMillis = frameInfos[i].fDuration;
            if (durationMillis < 0) {
                SkDebugf("Negative animation duration.\n");
                return 1;
            }
            totalDurationMillis += static_cast<uint64_t>(durationMillis);
            if (totalDurationMillis > INT64_MAX) {
                SkDebugf("Unsupported animation duration.\n");
                return 1;
            }

            if ((cachedFrame != SkCodec::kNoFrame) &&
                (cachedFrame == frameInfos[i].fRequiredFrame) && cachedFramePixels.get()) {
                opts.fPriorFrame = cachedFrame;
                memcpy(bm.getPixels(), cachedFramePixels.get(), bmByteSize);
            }
        }

        if (!firstFrameOnly) {
            if (i == 0) {
                write_nix_header(0x41AFC36E,  // "nïA" magic string as a u32le.
                                 bm.width(), bm.height());
            }

            if (!write_nia_duration(totalDurationMillis)) {
                SkDebugf("Unsupported animation duration.\n");
                return 1;
            }
        }

        const SkCodec::Result result =
            codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes(), &opts);
        if ((result != SkCodec::kSuccess) && (result != SkCodec::kIncompleteInput)) {
            SkDebugf("Decode frame pixels #%d failed.\n", i);
            return 1;
        }

        // If the next frame depends on this one, store it in cachedFrame. It
        // is possible that we may discard a frame that future frames depend
        // on, but the codec will simply redecode the discarded frame.
        if ((static_cast<size_t>(i + 1) < frameInfos.size()) &&
            (frameInfos[i + 1].fRequiredFrame == i)) {
            cachedFrame = i;
            memcpy(cachedFramePixels.reset(bmByteSize), bm.getPixels(), bmByteSize);
        }

        int width = bm.width();
        int height = bm.height();
        write_nix_header(0x45AFC36E,  // "nïE" magic string as a u32le.
                         width, height);
        write_nie_pixels(width, height, bm);
        if (result == SkCodec::kIncompleteInput) {
            SkDebugf("Incomplete input.\n");
            return 1;
        }
        if (firstFrameOnly) {
            return 0;
        }
        write_nia_padding(width, height);
    }
    write_nia_footer(codec->getRepetitionCount(), stillImage);
    return 0;
}

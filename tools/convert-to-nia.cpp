/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This program converts an image from stdin (e.g. in the JPEG, PNG, etc.
// formats) to stdout (in the NIA/NIE format, a trivial image file format).
//
// Pass "-1" or "-first-frame-only" as a command line flag to output NIE (a
// still image) instead of NIA (an animated image).
//
// There are multiple codec implementations of any given image format. For
// example, as of May 2020, Chromium, Skia and Wuffs each have their own BMP
// decoder implementation. There is no standard "libbmp" that they all share.
// Comparing this program's output (or hashed output) to similar programs in
// other repositories can identify image inputs for which these decoders (or
// different versions of the same decoder) produce different output (pixels).
//
// The NIA/NIE file format specification is at:
// https://github.com/google/wuffs/blob/master/doc/spec/nie-spec.md
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

void write_nix_header(uint32_t magic_u32le, uint32_t width, uint32_t height) {
    uint8_t data[16];
    set_u32le(data + 0, magic_u32le);
    set_u32le(data + 4, 0x346E62FF);  // 4 bytes per pixel non-premul BGRA.
    set_u32le(data + 8, width);
    set_u32le(data + 12, height);
    fwrite(data, 1, 16, stdout);
}

bool write_nia_duration(uint64_t total_duration_millis) {
    // Flicks are NIA's unit of time. One flick (frame-tick) is 1 / 705_600_000
    // of a second. See https://github.com/OculusVR/Flicks
    static constexpr uint64_t flicks_per_milli = 705600;
    if (total_duration_millis > (INT64_MAX / flicks_per_milli)) {
        // Converting from millis to flicks would overflow.
        return false;
    }

    uint8_t data[8];
    set_u64le(data + 0, total_duration_millis * flicks_per_milli);
    fwrite(data, 1, 8, stdout);
    return true;
}

void write_nie_pixels(uint32_t width, uint32_t height, SkBitmap& bm) {
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

void write_nia_padding(uint32_t width, uint32_t height) {
    // 4 bytes of padding when the width and height are both odd.
    if (width & height & 1) {
        uint8_t data[4];
        set_u32le(data + 0, 0);
        fwrite(data, 1, 4, stdout);
    }
}

void write_nia_footer(int repetition_count, bool still_image) {
    uint8_t data[8];
    if (still_image || (repetition_count == SkCodec::kRepetitionCountInfinite)) {
        set_u32le(data + 0, 0);
    } else {
        // In the general case, NIA's loop count and Chromium/Skia's repetition
        // count differ by one. See
        // https://github.com/google/wuffs/blob/master/doc/spec/nie-spec.md#nii-footer
        set_u32le(data + 0, 1 + repetition_count);
    }
    set_u32le(data + 4, 0x80000000);
    fwrite(data, 1, 8, stdout);
}

int main(int argc, char** argv) {
    bool first_frame_only = false;
    for (int a = 1; a < argc; a++) {
        if ((strcmp(argv[a], "-1") == 0) || (strcmp(argv[a], "-first-frame-only") == 0)) {
            first_frame_only = true;
            break;
        }
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(SkData::MakeFromFILE(stdin)));
    if (!codec) {
        SkDebugf("Decode failed.\n");
        return 1;
    }
    SkBitmap bm;
    bm.allocPixels(codec->getInfo());

    bool      still_image = false;  // Still means not animated.
    uint64_t  total_duration_millis = 0;
    const int frameCount = codec->getFrameCount();
    for (int i = 0; i < frameCount; i++) {
        int                duration_millis = 0;
        SkCodec::FrameInfo frameInfo;
        if (codec->getFrameInfo(i, &frameInfo)) {
            duration_millis = frameInfo.fDuration;
            if (duration_millis < 0) {
                SkDebugf("Negative animation duration.\n");
                return 1;
            }
        } else if (frameCount == 1) {
            // The SkCodec::getFrameInfo documentation comment says that it is "only
            // supported by multi-frame images". We expect it to fail (return false)
            // for still images.
            still_image = true;
        } else {
            SkDebugf("Decode frame info #%d failed.\n", i);
            return 1;
        }
        total_duration_millis += static_cast<uint64_t>(duration_millis);
        if (total_duration_millis > INT64_MAX) {
            SkDebugf("Unsupported animation duration.\n");
            return 1;
        }

        if (!first_frame_only) {
            if (i == 0) {
                write_nix_header(0x41AFC36E,  // "nïA" magic string as a u32le.
                                 bm.width(), bm.height());
            }

            if (!write_nia_duration(total_duration_millis)) {
                SkDebugf("Unsupported animation duration.\n");
                return 1;
            }
        }

        SkCodec::Options opts;
        opts.fFrameIndex = i;
        const SkCodec::Result result =
            codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes(), &opts);
        if ((result != SkCodec::kSuccess) && (result != SkCodec::kIncompleteInput)) {
            SkDebugf("Decode frame pixels #%d failed.\n", i);
            return 1;
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
        if (first_frame_only) {
            return 0;
        }
        write_nia_padding(width, height);
    }
    write_nia_footer(codec->getRepetitionCount(), still_image);
    return 0;
}

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSwizzle.h"
#include "src/codec/SkSampler.h"
#include "src/core/SkSwizzlePriv.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

static void check_fill(skiatest::Reporter* r,
                       const SkImageInfo& imageInfo,
                       int startRow,
                       int endRow,
                       size_t rowBytes,
                       size_t offset) {
    // Calculate the total size of the image in bytes.  Use the smallest possible size.
    // The offset value tells us to adjust the pointer from the memory we allocate in order
    // to test on different memory alignments.  If offset is nonzero, we need to increase the
    // size of the memory we allocate in order to make sure that we have enough.  We are
    // still allocating the smallest possible size.
    const size_t totalBytes = imageInfo.computeByteSize(rowBytes) + offset;

    // Pre-fill 8-byte-aligned storage with a non-zero sentinel so we verify both that target
    // rows are zeroed and that prefix bytes, surrounding rows, and row padding stay untouched.
    constexpr uint8_t kSentinel = 0xA5;
    auto storage = std::make_unique<uint64_t[]>((totalBytes + 7) / 8 + 1);
    auto* rawStorage = reinterpret_cast<uint8_t*>(storage.get());
    memset(rawStorage, kSentinel, totalBytes);

    // Adjust the pointer in order to test on different memory alignments
    uint8_t* imageData = rawStorage + offset;
    uint8_t* imageStart = imageData + rowBytes * startRow;
    const SkImageInfo fillInfo = imageInfo.makeWH(imageInfo.width(), endRow - startRow + 1);
    SkSampler::Fill(fillInfo, imageStart, rowBytes, SkCodec::kNo_ZeroInitialized);

    for (size_t i = 0; i < offset; ++i) {
        REPORTER_ASSERT(r, rawStorage[i] == kSentinel);
    }
    const size_t rowPixelBytes = imageInfo.minRowBytes();
    const int height = imageInfo.height();
    for (int y = 0; y < height; ++y) {
        const uint8_t* rowPtr = imageData + y * rowBytes;
        const size_t allocatedRowBytes = (y + 1 == height) ? rowPixelBytes : rowBytes;
        const bool inFillRows = (y >= startRow && y <= endRow);
        for (size_t byteIdx = 0; byteIdx < allocatedRowBytes; ++byteIdx) {
            const uint8_t expected = (inFillRows && byteIdx < rowPixelBytes) ? 0x00 : kSentinel;
            REPORTER_ASSERT(r, rowPtr[byteIdx] == expected);
        }
    }
}

static void check_fill_for_info(skiatest::Reporter* r, const SkImageInfo& info) {
    const size_t bpp = info.bytesPerPixel();
    // SkSampler::Fill casts row pointers to uint16_t*/uint32_t*/uint64_t*, so keep rowBytes
    // and offset aligned to at least bpp (notably 8-byte aligned for kRGBA_F16).
    const size_t align = std::max<size_t>(4, bpp);

    for (size_t padding : {size_t(0), align}) {
        const size_t rowBytes = info.minRowBytes() + padding;
        for (size_t offset = 0; offset <= padding; offset += align) {
            for (int startRow = 0; startRow < info.height(); ++startRow) {
                for (int endRow = startRow; endRow < info.height(); ++endRow) {
                    check_fill(r, info, startRow, endRow, rowBytes, offset);
                }
            }
        }
    }
}

// Test Fill() with different combinations of dimensions, alignment, and padding
DEF_TEST(SwizzlerFill, r) {
    constexpr int kWidths[] = {0, 10, 50};
    constexpr int kHeights[] = {1, 5, 10};
    constexpr SkColorType kColorTypes[] = {
            kRGBA_8888_SkColorType,
            kBGRA_8888_SkColorType,
            kRGBA_1010102_SkColorType,
            kRGB_565_SkColorType,
            kGray_8_SkColorType,
            kAlpha_8_SkColorType,
            kRGBA_F16_SkColorType,
    };

    for (int width : kWidths) {
        for (int height : kHeights) {
            for (SkColorType colorType : kColorTypes) {
                check_fill_for_info(
                        r, SkImageInfo::Make(width, height, colorType, kUnknown_SkAlphaType));
            }
        }
    }

#if !defined(SK_DEBUG)
    // In release builds, unsupported color types fall back to a bytewise zero fill.
    check_fill_for_info(r, SkImageInfo::Make(10, 5, kRGB_888x_SkColorType, kUnknown_SkAlphaType));
#endif
}

DEF_TEST(SwizzleOpts, r) {
    uint32_t dst, src;

    // forall c, c*255 == c, c*0 == 0
    for (int c = 0; c <= 255; c++) {
        src = (255<<24) | c;
        SkOpts::RGBA_to_rgbA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == src);
        SkOpts::RGBA_to_bgrA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == (uint32_t)((255<<24) | (c<<16)));

        src = (0<<24) | c;
        SkOpts::RGBA_to_rgbA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == 0);
        SkOpts::RGBA_to_bgrA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == 0);
    }

    // check a totally arbitrary color
    src = 0xFACEB004;
    SkOpts::RGBA_to_rgbA(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFACAAD04);

    // swap red and blue
    SkOpts::RGBA_to_BGRA(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFA04B0CE);

    // all together now
    SkOpts::RGBA_to_bgrA(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFA04ADCA);
}

DEF_TEST(PublicSwizzleOpts, r) {
    uint32_t dst, src;

    // check a totally arbitrary color
    src = 0xFACEB004;
    SkSwapRB(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFA04B0CE);
}

using fn_reciprocal = float (*)(float);
static void test_reciprocal_alpha(
        skiatest::Reporter* reporter,
        fn_reciprocal test255, fn_reciprocal test1) {
    REPORTER_ASSERT(reporter, test255(0) == 0);
    for (uint32_t i = 1; i < 256; ++i) {
        const float r = test255(i);
        const float e = (255.0f / i);
        REPORTER_ASSERT(reporter, r == e);
    }

    REPORTER_ASSERT(reporter, test1(0) == 0);
    for (uint32_t i = 1; i < 256; ++i) {
        const float normalized = i / 255.0f;
        const float r = test1(normalized);
        const float e = (1.0f / normalized);
        REPORTER_ASSERT(reporter, r == e);
    }
}

#define SK_OPTS_NS test
#define SK_OPTS_TARGET SK_OPTS_TARGET_DEFAULT
#include "src/opts/SkOpts_SetTarget.h"
#include "src/opts/SkSwizzler_opts.inc"
DEF_TEST(ReciprocalAlphaOptimized, reporter) {
    test_reciprocal_alpha(reporter,
                          SK_OPTS_NS::reciprocal_alpha_times_255,
                          SK_OPTS_NS::reciprocal_alpha);
}

DEF_TEST(ReciprocalAlphaPortable, reporter) {
    test_reciprocal_alpha(reporter,
                          SK_OPTS_NS::reciprocal_alpha_times_255_portable,
                          SK_OPTS_NS::reciprocal_alpha_portable);
}

// The stages of RasterPipeline unpremul calcExpected needs to simulate.
// SI void from_8888(U32 _8888, F* r, F* g, F* b, F* a) {
//     *r = cast((_8888      ) & 0xff) * (1/255.0f);
//     *g = cast((_8888 >>  8) & 0xff) * (1/255.0f);
//     *b = cast((_8888 >> 16) & 0xff) * (1/255.0f);
//     *a = cast((_8888 >> 24)       ) * (1/255.0f);
// }
// STAGE(unpremul, NoCtx) {
//     float inf = sk_bit_cast<float>(0x7f800000);
//     auto scale = if_then_else(1.0f/a < inf, 1.0f/a, 0.0f);
//     r *= scale;
//     g *= scale;
//     b *= scale;
// }
// STAGE(store_8888, const SkRasterPipelineContexts::MemoryCtx* ctx) {
//     auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);
//
//     U32 px = to_unorm(r, 255)
//            | to_unorm(g, 255) <<  8
//            | to_unorm(b, 255) << 16
//            | to_unorm(a, 255) << 24;
//     store(ptr, px);
// }
uint32_t calcExpected(float alpha, float comp) {
    if (alpha == 0) {
        return 0;
    }
    const float normalized = comp * (1.0f / 255.0f);
    const float normalizedA = alpha * (1.0f / 255.0f);
    const float inverseAlpha = 1.0f / normalizedA;
    const float unpremul = normalized * inverseAlpha;
    const float scaledAndPinned = std::min(255.0f, unpremul * 255.0f);
    return SK_OPTS_NS::pixel_round_as_RP(scaledAndPinned);
}

DEF_TEST(UnpremulSimulatingRP, reporter) {
    for (uint32_t a = 0; a < 256; ++a) {
        for (uint32_t c = 0; c < 256; ++c) {
            const uint32_t expected = calcExpected(a, c);
            const float normalizedA = a * (1.0f / 255.0f);
            const float invA = SK_OPTS_NS::reciprocal_alpha(normalizedA);
            const uint32_t actual = SK_OPTS_NS::unpremul_simulating_RP(invA, c);
            if (actual != expected) {
                SkDebugf("a: %u c: %u expected: %u actual: %u\n", a, c, expected, actual);
            }
            REPORTER_ASSERT(reporter, actual == expected);
        }
    }
}

#include "src/opts/SkOpts_RestoreTarget.h"

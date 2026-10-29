/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/text/gpu/SDFMaskFilter.h"
#include "tests/Test.h"

#include <cstdint>
#include <limits>
#include <vector>

#if !defined(SK_DISABLE_SDF_TEXT)

DEF_TEST(DistanceField_ComputeSize, reporter) {
    // Non-positive dimensions return 0
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(0, 0) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(-1, 10) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(10, -1) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(-10, -10) == 0);

    constexpr int kPad = SK_DistanceFieldPad;
    size_t expected = (16 + 2 * kPad) * (16 + 2 * kPad);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(16, 16) == expected);

    // Overflow dimensions must return 0 instead of wrapping
    constexpr int kMaxInt = std::numeric_limits<int>::max();
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(kMaxInt, kMaxInt) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(kMaxInt, 1) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(1, kMaxInt) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(1 << 30, 1 << 30) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(kMaxInt - 1, 10) == 0);
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(4071, 65527) == 0);

    // Exact DFScratch total buffer boundary (gridBytes + edgeBytes + copyBytes <= INT_MAX):
    // For w=48067, h=2472: total=2,147,483,644 <= INT_MAX (2,147,483,647)
    // For w=48068, h=2472: total=2,147,528,312 > INT_MAX
    REPORTER_ASSERT(reporter,
                    SkComputeDistanceFieldSize(48067, 2472) ==
                            static_cast<size_t>(48067 + 2 * kPad) * (2472 + 2 * kPad));
    REPORTER_ASSERT(reporter, SkComputeDistanceFieldSize(48068, 2472) == 0);
}

DEF_TEST(DistanceField_A8, reporter) {
    std::vector<unsigned char> df(64);
    std::vector<unsigned char> src(64, 0xFF);

    // Non-positive dimensions must fail without reading or writing buffers
    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), 0, 8, 8));
    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), 8, 0, 8));
    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), -1, 8, 8));
    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), 8, -1, 8));

    // Integer overflow in dimensions must fail without allocating
    constexpr int kMaxInt = std::numeric_limits<int>::max();
    REPORTER_ASSERT(reporter,
                    !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), kMaxInt, 1, 8));
    REPORTER_ASSERT(reporter,
                    !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), 1, kMaxInt, 8));
    REPORTER_ASSERT(
            reporter,
            !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), 1 << 30, 1 << 30, 8));
    // Exploit dimensions (4071x65527): required storage overflows int without allocating
    REPORTER_ASSERT(reporter,
                    !SkGenerateDistanceFieldFromA8Image(df.data(), src.data(), 4071, 65527, 4071));

    // Valid small image: 8x8 with a centered 4x4 opaque square
    constexpr int kW = 8;
    constexpr int kH = 8;
    std::vector<unsigned char> a8(kW * kH, 0);
    for (int y = 2; y < 6; ++y) {
        for (int x = 2; x < 6; ++x) {
            a8[y * kW + x] = 0xFF;
        }
    }
    size_t dfSize = SkComputeDistanceFieldSize(kW, kH);
    std::vector<unsigned char> dfOut(dfSize);
    REPORTER_ASSERT(reporter,
                    SkGenerateDistanceFieldFromA8Image(dfOut.data(), a8.data(), kW, kH, kW));
}

DEF_TEST(DistanceField_LCD16, reporter) {
    std::vector<unsigned char> df(64);
    std::vector<uint16_t> src(64, 0xFFFF);

    REPORTER_ASSERT(
            reporter,
            !SkGenerateDistanceFieldFromLCD16Mask(
                    df.data(), reinterpret_cast<const unsigned char*>(src.data()), 0, 8, 16));
    REPORTER_ASSERT(
            reporter,
            !SkGenerateDistanceFieldFromLCD16Mask(
                    df.data(), reinterpret_cast<const unsigned char*>(src.data()), 8, 0, 16));
    REPORTER_ASSERT(
            reporter,
            !SkGenerateDistanceFieldFromLCD16Mask(
                    df.data(), reinterpret_cast<const unsigned char*>(src.data()), -1, 8, 16));

    constexpr int kMaxInt = std::numeric_limits<int>::max();
    REPORTER_ASSERT(
            reporter,
            !SkGenerateDistanceFieldFromLCD16Mask(
                    df.data(), reinterpret_cast<const unsigned char*>(src.data()), kMaxInt, 1, 16));
    REPORTER_ASSERT(reporter,
                    !SkGenerateDistanceFieldFromLCD16Mask(
                            df.data(),
                            reinterpret_cast<const unsigned char*>(src.data()),
                            4071,
                            65527,
                            4071 * 2));

    constexpr int kW = 8;
    constexpr int kH = 8;
    std::vector<uint16_t> lcd(kW * kH, 0);
    for (int y = 2; y < 6; ++y) {
        for (int x = 2; x < 6; ++x) {
            lcd[y * kW + x] = 0xFFFF;
        }
    }
    size_t dfSize = SkComputeDistanceFieldSize(kW, kH);
    std::vector<unsigned char> dfOut(dfSize);
    REPORTER_ASSERT(
            reporter,
            SkGenerateDistanceFieldFromLCD16Mask(dfOut.data(),
                                                 reinterpret_cast<const unsigned char*>(lcd.data()),
                                                 kW,
                                                 kH,
                                                 kW * 2));
}

DEF_TEST(DistanceField_BW, reporter) {
    std::vector<unsigned char> df(64);
    std::vector<unsigned char> src(8, 0xFF);

    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromBWImage(df.data(), src.data(), 0, 8, 1));
    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromBWImage(df.data(), src.data(), 8, 0, 1));
    REPORTER_ASSERT(reporter, !SkGenerateDistanceFieldFromBWImage(df.data(), src.data(), -1, 8, 1));

    constexpr int kMaxInt = std::numeric_limits<int>::max();
    REPORTER_ASSERT(reporter,
                    !SkGenerateDistanceFieldFromBWImage(df.data(), src.data(), kMaxInt, 1, 1));
    REPORTER_ASSERT(reporter,
                    !SkGenerateDistanceFieldFromBWImage(df.data(), src.data(), 4071, 65527, 509));

    constexpr int kW = 8;
    constexpr int kH = 8;
    std::vector<unsigned char> bw(kH, 0);
    for (int y = 2; y < 6; ++y) {
        bw[y] = 0x3C;
    }
    size_t dfSize = SkComputeDistanceFieldSize(kW, kH);
    std::vector<unsigned char> dfOut(dfSize);
    REPORTER_ASSERT(reporter,
                    SkGenerateDistanceFieldFromBWImage(dfOut.data(), bw.data(), kW, kH, 1));
}

DEF_TEST(DistanceField_SDFMaskFilter, reporter) {
    sk_sp<SkMaskFilter> filter = sktext::gpu::SDFMaskFilter::Make();
    REPORTER_ASSERT(reporter, filter != nullptr);

    constexpr int kW = 8;
    constexpr int kH = 8;
    std::vector<unsigned char> a8(kW * kH, 0xFF);

    // Valid mask filter execution
    SkMaskBuilder srcMask;
    srcMask.bounds().setWH(kW, kH);
    srcMask.format() = SkMask::kA8_Format;
    srcMask.rowBytes() = kW;
    srcMask.image() = a8.data();

    SkMaskBuilder dstMask;
    bool success = as_MFB(filter)->filterMask(&dstMask, srcMask, SkMatrix::I(), nullptr);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dstMask.image() != nullptr);
    REPORTER_ASSERT(reporter, dstMask.format() == SkMask::kSDF_Format);
    REPORTER_ASSERT(reporter, dstMask.bounds().width() == kW + 2 * SK_DistanceFieldPad);
    REPORTER_ASSERT(reporter, dstMask.bounds().height() == kH + 2 * SK_DistanceFieldPad);
    SkMaskBuilder::FreeImage(dstMask.image());

    // Empty or negative dimensions must clean up allocated destination image without leaking
    SkMaskBuilder emptyMask;
    emptyMask.bounds().setEmpty();
    emptyMask.format() = SkMask::kA8_Format;
    emptyMask.rowBytes() = 0;
    emptyMask.image() = a8.data();

    SkMaskBuilder emptyDst;
    bool emptyFailed = as_MFB(filter)->filterMask(&emptyDst, emptyMask, SkMatrix::I(), nullptr);
    REPORTER_ASSERT(reporter, !emptyFailed);
    REPORTER_ASSERT(reporter, emptyDst.image() == nullptr);
    REPORTER_ASSERT(reporter, emptyDst.bounds().isEmpty());

    SkMaskBuilder negativeMask;
    negativeMask.bounds().setLTRB(0, 0, -1, 10);
    negativeMask.format() = SkMask::kA8_Format;
    negativeMask.rowBytes() = 0;
    negativeMask.image() = a8.data();

    SkMaskBuilder negativeDst;
    bool negFailed = as_MFB(filter)->filterMask(&negativeDst, negativeMask, SkMatrix::I(), nullptr);
    REPORTER_ASSERT(reporter, !negFailed);
    REPORTER_ASSERT(reporter, negativeDst.image() == nullptr);
    REPORTER_ASSERT(reporter, negativeDst.bounds().isEmpty());

    // Oversized exploit dimensions (4071x65527) must fail both bounds-only and image filtering
    // without allocating the 255 MB destination buffer
    SkMaskBuilder oversizedBoundsOnly;
    oversizedBoundsOnly.bounds().setWH(4071, 65527);
    oversizedBoundsOnly.format() = SkMask::kA8_Format;
    oversizedBoundsOnly.rowBytes() = 4071;
    oversizedBoundsOnly.image() = nullptr;

    SkMaskBuilder oversizedBoundsDst;
    REPORTER_ASSERT(reporter,
                    !as_MFB(filter)->filterMask(
                            &oversizedBoundsDst, oversizedBoundsOnly, SkMatrix::I(), nullptr));
    REPORTER_ASSERT(reporter, oversizedBoundsDst.image() == nullptr);
    REPORTER_ASSERT(reporter, oversizedBoundsDst.bounds().isEmpty());

    SkMaskBuilder oversizedMask;
    oversizedMask.bounds().setWH(4071, 65527);
    oversizedMask.format() = SkMask::kA8_Format;
    oversizedMask.rowBytes() = 4071;
    oversizedMask.image() = a8.data();

    SkMaskBuilder oversizedDst;
    REPORTER_ASSERT(
            reporter,
            !as_MFB(filter)->filterMask(&oversizedDst, oversizedMask, SkMatrix::I(), nullptr));
    REPORTER_ASSERT(reporter, oversizedDst.image() == nullptr);
    REPORTER_ASSERT(reporter, oversizedDst.bounds().isEmpty());
}

#endif  // !defined(SK_DISABLE_SDF_TEXT)

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>
#include <tuple>
#include <vector>
#include "Resources.h"
#include "SkCpu.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkOpts.h"
#include "SkPM4fPriv.h"
#include "SkNx.h"
#include "Test.h"

typedef void (*Blender)(uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);

static inline void srcover_srgb_srgb_1(uint32_t* dst, uint32_t src) {
    auto d = Sk4f_fromS32(*dst),
         s = Sk4f_fromS32( src);
    *dst = Sk4f_toS32(s + d * (1.0f - s[3]));
}

static void brute_force_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    while (ndst > 0) {
        int n = SkTMin(ndst, nsrc);

        for (int i = 0; i < n; i++) {
            srcover_srgb_srgb_1(dst++, src[i]);
        }
        ndst -= n;
    }
}

static SkString mismatch_message(std::string resourceName, int x, int y,
                                  uint32_t src, uint32_t good, uint32_t bad) {
    return SkStringPrintf(
        "%s - missmatch at %d, %d src: %08x good: %08x bad: %08x",
        resourceName.c_str(), x, y, src, good, bad);
}

static void test_blender(std::string resourceName, skiatest::Reporter* reporter) {
    std::string fileName = resourceName + ".png";
    sk_sp<SkImage> image = GetResourceAsImage(fileName.c_str());
    if (image == nullptr) {
        ERRORF(reporter, "image is NULL");
        return;
    }
    SkBitmap bm;
    sk_sp<SkColorSpace> srgbColorSpace = SkColorSpace::MakeSRGB();
    if (!as_IB(image)->getROPixels(&bm, srgbColorSpace.get())) {
        ERRORF(reporter, "Could not read resource");
        return;
    }

    SkPixmap pixmap;
    bm.peekPixels(&pixmap);
    SkASSERTF(pixmap.colorType() == kN32_SkColorType, "colorType: %d", pixmap.colorType());
    SkASSERT(pixmap.alphaType() != kUnpremul_SkAlphaType);
    const uint32_t* src = pixmap.addr32();
    const int width = pixmap.rowBytesAsPixels();
    SkASSERT(width > 0);
    SkASSERT(width < 4000);
    SkAutoTArray<uint32_t> correctDst(width);
    SkAutoTArray<uint32_t> testDst(width);

    for (int y = 0; y < pixmap.height(); y++) {
        // TODO: zero is not the most interesting dst to test srcover...
        sk_bzero(correctDst.get(), width * sizeof(uint32_t));
        sk_bzero(testDst.get(), width * sizeof(uint32_t));
        brute_force_srcover_srgb_srgb(correctDst.get(), src, width, width);
        SkOpts::    srcover_srgb_srgb(   testDst.get(), src, width, width);
        for (int x = 0; x < width; x++) {
            REPORTER_ASSERT_MESSAGE(
                reporter, correctDst[x] == testDst[x],
                mismatch_message(resourceName, x, y, src[x], correctDst[x], testDst[x]));
            if (correctDst[x] != testDst[x]) break;
        }
        src += width;
    }
}

DEF_TEST(SkBlend_optsCheck, reporter) {
    std::vector<std::string> testResources = {
        "yellow_rose", "baby_tux", "plane", "mandrill_512", "iconstrip"
    };

    for (auto& resourceName : testResources) {
        test_blender(resourceName, reporter);
    }
}

DEF_TEST(SkBlend_optsSqrtCheck, reporter) {
    for (int c = 0; c < 256; c++) {
        Sk4f i{(float)c};
        Sk4f ii = i * i;
        Sk4f s = ii.sqrt() + 0.5f;
        Sk4f sf = s.floor();
        REPORTER_ASSERT_MESSAGE(
            reporter, i[0] == sf[0], SkStringPrintf("i: %f, s: %f", i[0], sf[0]));
    }
}

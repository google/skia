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
#include "SkNx.h"
#include "Test.h"
#include "../include/core/SkImageInfo.h"

typedef void (*Blender)(uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);

namespace sk_default {
extern void brute_force_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

namespace sk_default {
extern void trivial_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);

extern void best_non_simd_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);

extern void srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

#if defined(SK_CPU_X86) && !defined(SK_BUILD_NO_OPTS)
namespace sk_sse41 {
extern void srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}
#endif

static SkString missmatch_message(std::string resourceName, std::string name, int x, int y,
                                  uint32_t src, uint32_t good, uint32_t bad) {
    return SkStringPrintf(
        "%s - %s missmatch at %d, %d src: %08x good: %08x bad: %08x",
        resourceName.c_str(), name.c_str(), x, y, src, good, bad);
}

using Spec = std::tuple<Blender, std::string>;

static void test_blender(
    Spec spec,
    std::string resourceName,
    skiatest::Reporter* reporter)
{
    Blender blender;
    std::string name;
    std::tie(blender, name) = spec;

    std::string fileName = resourceName + ".png";
    sk_sp<SkImage> image = GetResourceAsImage(fileName.c_str());
    if (image == nullptr) {
        ERRORF(reporter, "image is NULL");
        return;
    }
    SkBitmap bm;
    if (!as_IB(image)->getROPixels(&bm)) {
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
        memset(correctDst.get(), 0, width * sizeof(uint32_t));
        memset(testDst.get(), 0, width * sizeof(uint32_t));
        sk_default::brute_force_srcover_srgb_srgb(correctDst.get(), src, width, width);
        blender(testDst.get(), src, width, width);
        for (int x = 0; x < width; x++) {
            REPORTER_ASSERT_MESSAGE(
                reporter, correctDst[x] == testDst[x],
                missmatch_message(resourceName, name, x, y, src[x], correctDst[x], testDst[x]));
            if (correctDst[x] != testDst[x]) break;
        }
        src += width;
    }
}

DEF_TEST(SkBlend_optsCheck, reporter) {
    std::vector<Spec> specs = {
        Spec{sk_default::trivial_srcover_srgb_srgb,       "trivial"},
        Spec{sk_default::best_non_simd_srcover_srgb_srgb, "best_non_simd"},
        Spec{sk_default::srcover_srgb_srgb,               "default"},
    };
    #if defined(SK_CPU_X86) && !defined(SK_BUILD_NO_OPTS)
    if (SkCpu::Supports(SkCpu::SSE41)) {
        specs.push_back(Spec{sk_sse41::srcover_srgb_srgb, "sse41", });
    }
    #endif

    std::vector<std::string> testResources = {
        "yellow_rose", "baby_tux", "plane", "mandrill_512", "iconstrip"
    };

    for (auto& spec : specs) {
        for (auto& resourceName : testResources) {
            test_blender(spec, resourceName, reporter);
        }
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

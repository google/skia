/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include "include/gpu/GrContext.h"

static uint32_t pack_unpremul_rgba(SkColor c) {
    uint32_t packed;
    uint8_t* byte = reinterpret_cast<uint8_t*>(&packed);
    byte[0] = SkColorGetR(c);
    byte[1] = SkColorGetG(c);
    byte[2] = SkColorGetB(c);
    byte[3] = SkColorGetA(c);
    return packed;
}

static uint32_t pack_unpremul_bgra(SkColor c) {
    uint32_t packed;
    uint8_t* byte = reinterpret_cast<uint8_t*>(&packed);
    byte[0] = SkColorGetB(c);
    byte[1] = SkColorGetG(c);
    byte[2] = SkColorGetR(c);
    byte[3] = SkColorGetA(c);
    return packed;
}

typedef uint32_t (*PackUnpremulProc)(SkColor);

const struct {
    SkColorType         fColorType;
    PackUnpremulProc    fPackProc;
} gUnpremul[] = {
    { kRGBA_8888_SkColorType, pack_unpremul_rgba },
    { kBGRA_8888_SkColorType, pack_unpremul_bgra },
};

static void fill_surface(SkSurface* surf, SkColorType colorType, PackUnpremulProc proc) {
    // Don't strictly need a bitmap, but its a handy way to allocate the pixels
    SkBitmap bmp;
    bmp.allocN32Pixels(256, 256);

    for (int a = 0; a < 256; ++a) {
        uint32_t* pixels = bmp.getAddr32(0, a);
        for (int r = 0; r < 256; ++r) {
            pixels[r] = proc(SkColorSetARGB(a, r, 0, 0));
        }
    }

    const SkImageInfo info = SkImageInfo::Make(bmp.width(), bmp.height(),
                                               colorType, kUnpremul_SkAlphaType);
    surf->writePixels({info, bmp.getPixels(), bmp.rowBytes()}, 0, 0);
}

static void test_premul_alpha_roundtrip(skiatest::Reporter* reporter, SkSurface* surf) {
    for (size_t upmaIdx = 0; upmaIdx < SK_ARRAY_COUNT(gUnpremul); ++upmaIdx) {
        fill_surface(surf, gUnpremul[upmaIdx].fColorType, gUnpremul[upmaIdx].fPackProc);

        const SkImageInfo info = SkImageInfo::Make(256, 256, gUnpremul[upmaIdx].fColorType,
                                                   kUnpremul_SkAlphaType);
        SkBitmap readBmp1;
        readBmp1.allocPixels(info);
        SkBitmap readBmp2;
        readBmp2.allocPixels(info);

        readBmp1.eraseColor(0);
        readBmp2.eraseColor(0);

        surf->readPixels(readBmp1, 0, 0);
        surf->writePixels(readBmp1, 0, 0);
        surf->readPixels(readBmp2, 0, 0);

        bool success = true;
        for (int y = 0; y < 256 && success; ++y) {
            const uint32_t* pixels1 = readBmp1.getAddr32(0, y);
            const uint32_t* pixels2 = readBmp2.getAddr32(0, y);
            for (int x = 0; x < 256 && success; ++x) {
                // We see sporadic failures here. May help to see where it goes wrong.
                if (pixels1[x] != pixels2[x]) {
                    SkDebugf("%x != %x, x = %d, y = %d\n", pixels1[x], pixels2[x], x, y);
                }
                REPORTER_ASSERT(reporter, success = pixels1[x] == pixels2[x]);
            }
        }
    }
}

DEF_TEST(PremulAlphaRoundTrip, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);

    sk_sp<SkSurface> surf(SkSurface::MakeRaster(info));

    test_premul_alpha_roundtrip(reporter, surf.get());
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PremulAlphaRoundTrip_Gpu, reporter, ctxInfo) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);

    sk_sp<SkSurface> surf(SkSurface::MakeRenderTarget(ctxInfo.grContext(),
                                                      SkBudgeted::kNo,
                                                      info));
    test_premul_alpha_roundtrip(reporter, surf.get());
}

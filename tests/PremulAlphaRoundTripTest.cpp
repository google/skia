/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkConvertPixels.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>

struct GrContextOptions;

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

    const SkImageInfo info = SkImageInfo::Make(bmp.dimensions(), colorType, kUnpremul_SkAlphaType);
    surf->writePixels({info, bmp.getPixels(), bmp.rowBytes()}, 0, 0);
}

static void test_premul_alpha_roundtrip(skiatest::Reporter* reporter, SkSurface* surf) {
    for (size_t upmaIdx = 0; upmaIdx < std::size(gUnpremul); ++upmaIdx) {
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

    sk_sp<SkSurface> surf(SkSurfaces::Raster(info));

    test_premul_alpha_roundtrip(reporter, surf.get());
}
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PremulAlphaRoundTrip_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);

    sk_sp<SkSurface> surf(
            SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo, info));
    test_premul_alpha_roundtrip(reporter, surf.get());
}

DEF_TEST(PremulAlphaRoundTripGrConvertPixels, reporter) {
    // Code that does the same thing as above, but using GrConvertPixels. This simulates what
    // happens if you run the above on a machine with a GPU that doesn't have a valid PM/UPM
    // conversion pair of FPs.
    const SkImageInfo upmInfo =
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    const SkImageInfo pmInfo =
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    GrPixmap src = GrPixmap::Allocate(upmInfo);
    uint32_t* srcPixels = (uint32_t*)src.addr();
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            srcPixels[y * 256 + x] = pack_unpremul_rgba(SkColorSetARGB(y, x, x, x));
        }
    }

    GrPixmap surf = GrPixmap::Allocate(pmInfo);
    GrConvertPixels(surf, src);

    GrPixmap read1 = GrPixmap::Allocate(upmInfo);
    GrConvertPixels(read1, surf);

    GrPixmap surf2 = GrPixmap::Allocate(pmInfo);
    GrConvertPixels(surf2, read1);

    GrPixmap read2 = GrPixmap::Allocate(upmInfo);
    GrConvertPixels(read2, surf2);

    auto get_pixel = [](const GrPixmap& pm, int x, int y) {
        const uint32_t* addr = (const uint32_t*)pm.addr();
        return addr[y * 256 + x];
    };
    auto dump_pixel_history = [&](int x, int y) {
        SkDebugf("Pixel history for (%d, %d):\n", x, y);
        SkDebugf("Src : %08x\n", get_pixel(src, x, y));
        SkDebugf(" -> : %08x\n", get_pixel(surf, x, y));
        SkDebugf(" <- : %08x\n", get_pixel(read1, x, y));
        SkDebugf(" -> : %08x\n", get_pixel(surf2, x, y));
        SkDebugf(" <- : %08x\n", get_pixel(read2, x, y));
    };

    bool success = true;
    for (int y = 0; y < 256 && success; ++y) {
        const uint32_t* pixels1 = (const uint32_t*) read1.addr();
        const uint32_t* pixels2 = (const uint32_t*) read2.addr();
        for (int x = 0; x < 256 && success; ++x) {
            uint32_t c1 = pixels1[y * 256 + x],
                     c2 = pixels2[y * 256 + x];
            // If this ever fails, it's helpful to see where it goes wrong.
            if (c1 != c2) {
                dump_pixel_history(x, y);
            }
            REPORTER_ASSERT(reporter, success = c1 == c2);
        }
    }
}

DEF_TEST(PremulAlphaRoundTripSkConvertPixels, reporter) {
    // ... and now using SkConvertPixels, just for completeness
    const SkImageInfo upmInfo =
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    const SkImageInfo pmInfo =
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap src; src.allocPixels(upmInfo);
    uint32_t* srcPixels = src.getAddr32(0, 0);
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            srcPixels[y * 256 + x] = pack_unpremul_rgba(SkColorSetARGB(y, x, x, x));
        }
    }

    auto convert = [](const SkBitmap& dst, const SkBitmap& src){
        SkAssertResult(SkConvertPixels(dst.info(), dst.getAddr(0, 0), dst.rowBytes(),
                                       src.info(), src.getAddr(0, 0), src.rowBytes()));
    };

    SkBitmap surf; surf.allocPixels(pmInfo);
    convert(surf, src);

    SkBitmap read1; read1.allocPixels(upmInfo);
    convert(read1, surf);

    SkBitmap surf2; surf2.allocPixels(pmInfo);
    convert(surf2, read1);

    SkBitmap read2; read2.allocPixels(upmInfo);
    convert(read2, surf2);

    auto dump_pixel_history = [&](int x, int y) {
        SkDebugf("Pixel history for (%d, %d):\n", x, y);
        SkDebugf("Src : %08x\n", *src.getAddr32(x, y));
        SkDebugf(" -> : %08x\n", *surf.getAddr32(x, y));
        SkDebugf(" <- : %08x\n", *read1.getAddr32(x, y));
        SkDebugf(" -> : %08x\n", *surf2.getAddr32(x, y));
        SkDebugf(" <- : %08x\n", *read2.getAddr32(x, y));
    };

    bool success = true;
    for (int y = 0; y < 256 && success; ++y) {
        const uint32_t* pixels1 = read1.getAddr32(0, 0);
        const uint32_t* pixels2 = read2.getAddr32(0, 0);
        for (int x = 0; x < 256 && success; ++x) {
            uint32_t c1 = pixels1[y * 256 + x],
                     c2 = pixels2[y * 256 + x];
            // If this ever fails, it's helpful to see where it goes wrong.
            if (c1 != c2) {
                dump_pixel_history(x, y);
            }
            REPORTER_ASSERT(reporter, success = c1 == c2);
        }
    }
}


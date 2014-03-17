/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkConfig8888.h"
#include "Test.h"
#include "sk_tool_utils.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"
#endif

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

static void fillCanvas(SkCanvas* canvas, SkColorType colorType, PackUnpremulProc proc) {
    // Don't strictly need a bitmap, but its a handy way to allocate the pixels
    SkBitmap bmp;
    bmp.allocN32Pixels(256, 256);

    for (int a = 0; a < 256; ++a) {
        uint32_t* pixels = bmp.getAddr32(0, a);
        for (int r = 0; r < 256; ++r) {
            pixels[r] = proc(SkColorSetARGB(a, r, 0, 0));
        }
    }

    SkImageInfo info = bmp.info();
    info.fColorType = colorType;
    info.fAlphaType = kUnpremul_SkAlphaType;
    canvas->writePixels(info, bmp.getPixels(), bmp.rowBytes(), 0, 0);
}

DEF_GPUTEST(PremulAlphaRoundTrip, reporter, factory) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);

    SkAutoTUnref<SkBaseDevice> device;
    for (int dtype = 0; dtype < 2; ++dtype) {

        int glCtxTypeCnt = 1;
#if SK_SUPPORT_GPU
        if (0 != dtype)  {
            glCtxTypeCnt = GrContextFactory::kGLContextTypeCnt;
        }
#endif
        for (int glCtxType = 0; glCtxType < glCtxTypeCnt; ++glCtxType) {
            if (0 == dtype) {
                device.reset(SkBitmapDevice::Create(info));
            } else {
#if SK_SUPPORT_GPU
                GrContextFactory::GLContextType type =
                    static_cast<GrContextFactory::GLContextType>(glCtxType);
                if (!GrContextFactory::IsRenderingGLContext(type)) {
                    continue;
                }
                GrContext* context = factory->get(type);
                if (NULL == context) {
                    continue;
                }

                device.reset(SkGpuDevice::Create(context, info, 0));
#else
                continue;
#endif
            }
            SkCanvas canvas(device);

            for (size_t upmaIdx = 0; upmaIdx < SK_ARRAY_COUNT(gUnpremul); ++upmaIdx) {
                fillCanvas(&canvas, gUnpremul[upmaIdx].fColorType, gUnpremul[upmaIdx].fPackProc);

                const SkImageInfo info = SkImageInfo::Make(256, 256, gUnpremul[upmaIdx].fColorType,
                                                           kUnpremul_SkAlphaType);
                SkBitmap readBmp1;
                readBmp1.allocPixels(info);
                SkBitmap readBmp2;
                readBmp2.allocPixels(info);

                readBmp1.eraseColor(0);
                readBmp2.eraseColor(0);

                canvas.readPixels(&readBmp1, 0, 0);
                sk_tool_utils::write_pixels(&canvas, readBmp1, 0, 0, gUnpremul[upmaIdx].fColorType,
                                            kUnpremul_SkAlphaType);
                canvas.readPixels(&readBmp2, 0, 0);

                bool success = true;
                for (int y = 0; y < 256 && success; ++y) {
                    const uint32_t* pixels1 = readBmp1.getAddr32(0, y);
                    const uint32_t* pixels2 = readBmp2.getAddr32(0, y);
                    for (int x = 0; x < 256 && success; ++x) {
                        REPORTER_ASSERT(reporter, success = pixels1[x] == pixels2[x]);
                    }
                }
            }
        }
    }
}

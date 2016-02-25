/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

// This test is specific to the GPU backend.
#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "SkGpuDevice.h"

// This was made indivisible by 4 to ensure we test setting GL_PACK_ALIGNMENT properly.
static const int X_SIZE = 13;
static const int Y_SIZE = 13;

static void validate_alpha_data(skiatest::Reporter* reporter, int w, int h, const uint8_t* actual,
                                size_t actualRowBytes, const uint8_t* expected, SkString extraMsg) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t a = actual[y * actualRowBytes + x];
            uint8_t e = expected[y * w + x];
            if (e != a) {
                ERRORF(reporter,
                       "Failed alpha readback. Expected: 0x%02x, Got: 0x%02x at (%d,%d), %s",
                       e, a, x, y, extraMsg.c_str());
                return;
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadWriteAlpha, reporter, context) {
    unsigned char alphaData[X_SIZE * Y_SIZE];

    bool match;
    static const size_t kRowBytes[] = {0, X_SIZE, X_SIZE + 1, 2 * X_SIZE - 1};
    for (int rt = 0; rt < 2; ++rt) {
        GrSurfaceDesc desc;
        // let Skia know we will be using this texture as a render target
        desc.fFlags     = rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
        // it is a single channel texture
        desc.fConfig    = kAlpha_8_GrPixelConfig;
        desc.fWidth     = X_SIZE;
        desc.fHeight    = Y_SIZE;

        // We are initializing the texture with zeros here
        memset(alphaData, 0, X_SIZE * Y_SIZE);
        SkAutoTUnref<GrTexture> texture(
            context->textureProvider()->createTexture(desc, SkBudgeted::kNo , alphaData, 0));
        if (!texture) {
            if (!rt) {
                ERRORF(reporter, "Could not create alpha texture.");
            }
            continue;
        }

        // create a distinctive texture
        for (int y = 0; y < Y_SIZE; ++y) {
            for (int x = 0; x < X_SIZE; ++x) {
                alphaData[y * X_SIZE + x] = y*X_SIZE+x;
            }
        }

        for (auto rowBytes : kRowBytes) {
            // upload the texture (do per-rowbytes iteration because we may overwrite below).
            texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                 alphaData, 0);

            size_t nonZeroRowBytes = rowBytes ? rowBytes : X_SIZE;
            SkAutoTDeleteArray<uint8_t> readback(new uint8_t[nonZeroRowBytes * Y_SIZE]);
            // clear readback to something non-zero so we can detect readback failures
            memset(readback.get(), 0x1, nonZeroRowBytes * Y_SIZE);

            // read the texture back
            texture->readPixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                readback.get(), rowBytes);

            // make sure the original & read back versions match
            SkString msg;
            msg.printf("rt:%d, rb:%d", rt, SkToU32(rowBytes));
            validate_alpha_data(reporter, X_SIZE, Y_SIZE, readback.get(), nonZeroRowBytes,
                                alphaData, msg);

            // Now try writing on the single channel texture (if we could create as a RT).
            if (texture->asRenderTarget()) {
                SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
                SkAutoTUnref<SkBaseDevice> device(SkGpuDevice::Create(
                    texture->asRenderTarget(), &props, SkGpuDevice::kUninit_InitContents));
                SkCanvas canvas(device);

                SkPaint paint;

                const SkRect rect = SkRect::MakeLTRB(-10, -10, X_SIZE + 10, Y_SIZE + 10);

                paint.setColor(SK_ColorWHITE);

                canvas.drawRect(rect, paint);

                memset(readback.get(), 0x1, nonZeroRowBytes * Y_SIZE);
                texture->readPixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig, readback.get(),
                                    rowBytes);

                match = true;
                for (int y = 0; y < Y_SIZE && match; ++y) {
                    for (int x = 0; x < X_SIZE && match; ++x) {
                        uint8_t rbValue = readback.get()[y * nonZeroRowBytes + x];
                        if (0xFF != rbValue) {
                            ERRORF(reporter,
                                   "Failed alpha readback after clear. Expected: 0xFF, Got: 0x%02x"
                                   " at (%d,%d), rb:%d", rbValue, x, y, SkToU32(rowBytes));
                            match = false;
                        }
                    }
                }
            }
        }
    }

    static const GrPixelConfig kRGBAConfigs[] {
        kRGBA_8888_GrPixelConfig,
        kBGRA_8888_GrPixelConfig,
        kSRGBA_8888_GrPixelConfig
    };

    for (int y = 0; y < Y_SIZE; ++y) {
        for (int x = 0; x < X_SIZE; ++x) {
            alphaData[y * X_SIZE + x] = y*X_SIZE+x;
        }
    }

    // Attempt to read back just alpha from a RGBA/BGRA texture. Once with a texture-only src and
    // once with a render target.
    for (auto cfg : kRGBAConfigs) {
        for (int rt = 0; rt < 2; ++rt) {
            GrSurfaceDesc desc;
            desc.fFlags     = rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
            desc.fConfig    = cfg;
            desc.fWidth     = X_SIZE;
            desc.fHeight    = Y_SIZE;

            uint32_t rgbaData[X_SIZE * Y_SIZE];
            // Make the alpha channel of the rgba texture come from alphaData.
            for (int y = 0; y < Y_SIZE; ++y) {
                for (int x = 0; x < X_SIZE; ++x) {
                    rgbaData[y * X_SIZE + x] = GrColorPackRGBA(6, 7, 8, alphaData[y * X_SIZE + x]);
                }
            }
            SkAutoTUnref<GrTexture> texture(
                context->textureProvider()->createTexture(desc, SkBudgeted::kNo, rgbaData, 0));
            if (!texture) {
                // We always expect to be able to create a RGBA texture
                if (!rt  && kRGBA_8888_GrPixelConfig == desc.fConfig) {
                    ERRORF(reporter, "Failed to create RGBA texture.");
                }
                continue;
            }

            for (auto rowBytes : kRowBytes) {
                size_t nonZeroRowBytes = rowBytes ? rowBytes : X_SIZE;

                SkAutoTDeleteArray<uint8_t> readback(new uint8_t[nonZeroRowBytes * Y_SIZE]);
                // Clear so we don't accidentally see values from previous iteration.
                memset(readback.get(), 0x0, nonZeroRowBytes * Y_SIZE);

                // read the texture back
                texture->readPixels(0, 0, desc.fWidth, desc.fHeight, kAlpha_8_GrPixelConfig,
                                    readback.get(), rowBytes);

                // make sure the original & read back versions match
                SkString msg;
                msg.printf("rt:%d, rb:%d", rt, SkToU32(rowBytes));
                validate_alpha_data(reporter, X_SIZE, Y_SIZE, readback.get(), nonZeroRowBytes,
                                    alphaData, msg);
            }
        }
    }
}

#endif

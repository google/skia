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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadWriteAlpha, reporter, context) {
    unsigned char alphaData[X_SIZE][Y_SIZE];

    memset(alphaData, 0, X_SIZE * Y_SIZE);

    bool match;
    unsigned char readback[X_SIZE][Y_SIZE];

    for (int rt = 0; rt < 2; ++rt) {
        GrSurfaceDesc desc;
        // let Skia know we will be using this texture as a render target
        desc.fFlags     = rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
        // it is a single channel texture
        desc.fConfig    = kAlpha_8_GrPixelConfig;
        desc.fWidth     = X_SIZE;
        desc.fHeight    = Y_SIZE;

        // We are initializing the texture with zeros here
        SkAutoTUnref<GrTexture> texture(
            context->textureProvider()->createTexture(desc, false, alphaData, 0));
        if (!texture) {
            if (!rt) {
                ERRORF(reporter, "Could not create alpha texture.");
            }
            continue;
        }

        // create a distinctive texture
        for (int y = 0; y < Y_SIZE; ++y) {
            for (int x = 0; x < X_SIZE; ++x) {
                alphaData[x][y] = x*Y_SIZE+y;
            }
        }

        // upload the texture
        texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                             alphaData, 0);

        // clear readback to something non-zero so we can detect readback failures
        memset(readback, 0x1, X_SIZE * Y_SIZE);

        // read the texture back
        texture->readPixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                            readback, 0);

        // make sure the original & read back versions match
        match = true;

        for (int y = 0; y < Y_SIZE && match; ++y) {
            for (int x = 0; x < X_SIZE && match; ++x) {
                if (alphaData[x][y] != readback[x][y]) {
                    SkDebugf("Failed alpha readback. Expected: 0x%02x, "
                             "Got: 0x%02x at (%d,%d), rt: %d", alphaData[x][y], readback[x][y], x,
                             y, rt);
                    match = false;
                }
            }
        }

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

            texture->readPixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig, readback, 0);

            match = true;

            for (int y = 0; y < Y_SIZE && match; ++y) {
                for (int x = 0; x < X_SIZE && match; ++x) {
                    if (0xFF != readback[x][y]) {
                        ERRORF(reporter,
                               "Failed alpha readback after clear. Expected: 0xFF, Got: 0x%02x at "
                               "(%d,%d)", readback[x][y], x, y);
                        match = false;
                    }
                }
            }
        }
    }

    // Attempt to read back just alpha from a RGBA/BGRA texture. Once with a texture-only src and
    // once with a render target.
    for (int cfg = 0; cfg < 2; ++cfg) {
        for (int rt = 0; rt < 2; ++rt) {
            GrSurfaceDesc desc;
            desc.fFlags     = rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
            desc.fConfig    = cfg ? kBGRA_8888_GrPixelConfig : kRGBA_8888_GrPixelConfig;
            desc.fWidth     = X_SIZE;
            desc.fHeight    = Y_SIZE;

            uint32_t rgbaData[X_SIZE][Y_SIZE];
            // Make the alpha channel of the rgba texture come from alphaData.
            for (int y = 0; y < Y_SIZE; ++y) {
                for (int x = 0; x < X_SIZE; ++x) {
                    rgbaData[x][y] = GrColorPackRGBA(6, 7, 8, alphaData[x][y]);
                }
            }
            SkAutoTUnref<GrTexture> texture(
                context->textureProvider()->createTexture(desc, false, rgbaData, 0));
            if (!texture) {
                // We always expect to be able to create a RGBA texture
                if (!rt  && kRGBA_8888_GrPixelConfig == desc.fConfig) {
                    ERRORF(reporter, "Failed to create RGBA texture.");
                }
                continue;
            }

            // clear readback to something non-zero so we can detect readback failures
            memset(readback, 0x0, X_SIZE * Y_SIZE);

            // read the texture back
            texture->readPixels(0, 0, desc.fWidth, desc.fHeight, kAlpha_8_GrPixelConfig, readback,
                                0);

            match = true;

            for (int y = 0; y < Y_SIZE && match; ++y) {
                for (int x = 0; x < X_SIZE && match; ++x) {
                    if (alphaData[x][y] != readback[x][y]) {
                        texture->readPixels(0, 0, desc.fWidth, desc.fHeight,
                                            kAlpha_8_GrPixelConfig, readback, 0);
                        ERRORF(reporter,
                               "Failed alpha readback from cfg %d. Expected: 0x%02x, Got: 0x%02x at "
                               "(%d,%d), rt:%d", desc.fConfig, alphaData[x][y], readback[x][y], x,
                               y, rt);
                        match = false;
                    }
                }
            }
        }
    }
}

#endif

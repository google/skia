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
#include "GrContextPriv.h"
#include "GrResourceProvider.h"
#include "GrSurfaceContext.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"
#include "SkCanvas.h"
#include "SkSurface.h"

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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadWriteAlpha, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    unsigned char alphaData[X_SIZE * Y_SIZE];

    static const int kClearValue = 0x2;

    bool match;
    static const size_t kRowBytes[] = {0, X_SIZE, X_SIZE + 1, 2 * X_SIZE - 1};
    {
        GrSurfaceDesc desc;
        desc.fFlags     = kNone_GrSurfaceFlags;
        desc.fConfig    = kAlpha_8_GrPixelConfig;    // it is a single channel texture
        desc.fWidth     = X_SIZE;
        desc.fHeight    = Y_SIZE;

        // We are initializing the texture with zeros here
        memset(alphaData, 0, X_SIZE * Y_SIZE);

        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                 desc,
                                                                 SkBudgeted::kNo,
                                                                 alphaData, 0));
        if (!proxy) {
            ERRORF(reporter, "Could not create alpha texture.");
            return;
        }
        sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeWrappedSurfaceContext(
                                                                  std::move(proxy), nullptr));

        const SkImageInfo ii = SkImageInfo::MakeA8(X_SIZE, Y_SIZE);
        sk_sp<SkSurface> surf(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii));

        // create a distinctive texture
        for (int y = 0; y < Y_SIZE; ++y) {
            for (int x = 0; x < X_SIZE; ++x) {
                alphaData[y * X_SIZE + x] = y*X_SIZE+x;
            }
        }

        for (auto rowBytes : kRowBytes) {

            // upload the texture (do per-rowbytes iteration because we may overwrite below).
            bool result = sContext->writePixels(ii, alphaData, 0, 0, 0);
            REPORTER_ASSERT_MESSAGE(reporter, result, "Initial A8 writePixels failed");

            size_t nonZeroRowBytes = rowBytes ? rowBytes : X_SIZE;
            std::unique_ptr<uint8_t[]> readback(new uint8_t[nonZeroRowBytes * Y_SIZE]);
            // clear readback to something non-zero so we can detect readback failures
            memset(readback.get(), kClearValue, nonZeroRowBytes * Y_SIZE);

            // read the texture back
            result = sContext->readPixels(ii, readback.get(), rowBytes, 0, 0);
            REPORTER_ASSERT_MESSAGE(reporter, result, "Initial A8 readPixels failed");

            // make sure the original & read back versions match
            SkString msg;
            msg.printf("rb:%d A8", SkToU32(rowBytes));
            validate_alpha_data(reporter, X_SIZE, Y_SIZE, readback.get(), nonZeroRowBytes,
                                alphaData, msg);

            // Now try writing to a single channel surface (if we could create one).
            if (surf) {
                SkCanvas* canvas = surf->getCanvas();

                SkPaint paint;

                const SkRect rect = SkRect::MakeLTRB(-10, -10, X_SIZE + 10, Y_SIZE + 10);

                paint.setColor(SK_ColorWHITE);

                canvas->drawRect(rect, paint);

                memset(readback.get(), kClearValue, nonZeroRowBytes * Y_SIZE);
                result = surf->readPixels(ii, readback.get(), nonZeroRowBytes, 0, 0);
                REPORTER_ASSERT_MESSAGE(reporter, result, "A8 readPixels after clear failed");

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

    const SkImageInfo dstInfo = SkImageInfo::Make(X_SIZE, Y_SIZE,
                                                  kAlpha_8_SkColorType,
                                                  kPremul_SkAlphaType);

    // Attempt to read back just alpha from a RGBA/BGRA texture. Once with a texture-only src and
    // once with a render target.
    for (auto config : kRGBAConfigs) {
        for (int rt = 0; rt < 2; ++rt) {
            GrSurfaceDesc desc;
            desc.fFlags     = rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
            desc.fConfig    = config;
            desc.fWidth     = X_SIZE;
            desc.fHeight    = Y_SIZE;

            uint32_t rgbaData[X_SIZE * Y_SIZE];
            // Make the alpha channel of the rgba texture come from alphaData.
            for (int y = 0; y < Y_SIZE; ++y) {
                for (int x = 0; x < X_SIZE; ++x) {
                    rgbaData[y * X_SIZE + x] = GrColorPackRGBA(6, 7, 8, alphaData[y * X_SIZE + x]);
                }
            }
            sk_sp<GrTextureProxy> proxy =
                GrSurfaceProxy::MakeDeferred(context->resourceProvider(), desc, SkBudgeted::kNo,
                                             rgbaData, 0);
            if (!proxy) {
                // We always expect to be able to create a RGBA texture
                if (!rt  && kRGBA_8888_GrPixelConfig == desc.fConfig) {
                    ERRORF(reporter, "Failed to create RGBA texture.");
                }
                continue;
            }

            sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                       std::move(proxy), nullptr);

            for (auto rowBytes : kRowBytes) {
                size_t nonZeroRowBytes = rowBytes ? rowBytes : X_SIZE;

                std::unique_ptr<uint8_t[]> readback(new uint8_t[nonZeroRowBytes * Y_SIZE]);
                // Clear so we don't accidentally see values from previous iteration.
                memset(readback.get(), kClearValue, nonZeroRowBytes * Y_SIZE);

                // read the texture back
                bool result = sContext->readPixels(dstInfo, readback.get(), rowBytes, 0, 0);
                REPORTER_ASSERT_MESSAGE(reporter, result, "8888 readPixels failed");

                // make sure the original & read back versions match
                SkString msg;
                msg.printf("rt:%d, rb:%d 8888", rt, SkToU32(rowBytes));
                validate_alpha_data(reporter, X_SIZE, Y_SIZE, readback.get(), nonZeroRowBytes,
                                    alphaData, msg);
            }
        }
    }
}

#endif

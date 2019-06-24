/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

// This test is specific to the GPU backend.
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkTo.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "tools/gpu/ProxyUtils.h"

// This was made indivisible by 4 to ensure we test setting GL_PACK_ALIGNMENT properly.
static const int X_SIZE = 13;
static const int Y_SIZE = 13;

static void validate_alpha_data(skiatest::Reporter* reporter, int w, int h, const uint8_t* actual,
                                size_t actualRowBytes, const uint8_t* expected, SkString extraMsg,
                                GrColorType colorType) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t a = actual[y * actualRowBytes + x];
            uint8_t e = expected[y * w + x];
            if (GrColorType::kRGBA_1010102 == colorType) {
                // This config only preserves two bits of alpha
                a >>= 6;
                e >>= 6;
            }
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
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

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

        const SkImageInfo ii = SkImageInfo::MakeA8(X_SIZE, Y_SIZE);

        SkPixmap pixmap(ii, alphaData, ii.minRowBytes());
        sk_sp<SkImage> alphaImg = SkImage::MakeRasterCopy(pixmap);
        sk_sp<GrTextureProxy> proxy =
            proxyProvider->createTextureProxy(alphaImg, kNone_GrSurfaceFlags, 1,
                                              SkBudgeted::kNo, SkBackingFit::kExact);
        if (!proxy) {
            ERRORF(reporter, "Could not create alpha texture.");
            return;
        }
        sk_sp<GrSurfaceContext> sContext(
                context->priv().makeWrappedSurfaceContext(std::move(proxy), kPremul_SkAlphaType));

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
            REPORTER_ASSERT(reporter, result, "Initial A8 writePixels failed");

            size_t nonZeroRowBytes = rowBytes ? rowBytes : X_SIZE;
            size_t bufLen = nonZeroRowBytes * Y_SIZE;
            std::unique_ptr<uint8_t[]> readback(new uint8_t[bufLen]);
            // clear readback to something non-zero so we can detect readback failures
            memset(readback.get(), kClearValue, bufLen);

            // read the texture back
            result = sContext->readPixels(ii, readback.get(), rowBytes, 0, 0);
            // We don't require reading from kAlpha_8 to be supported. TODO: At least make this work
            // when kAlpha_8 is renderable.
            if (!result) {
                continue;
            }
            REPORTER_ASSERT(reporter, result, "Initial A8 readPixels failed");

            // make sure the original & read back versions match
            SkString msg;
            msg.printf("rb:%d A8", SkToU32(rowBytes));
            validate_alpha_data(reporter, X_SIZE, Y_SIZE, readback.get(), nonZeroRowBytes,
                                alphaData, msg, GrColorType::kAlpha_8);

            // Now try writing to a single channel surface (if we could create one).
            if (surf) {
                SkCanvas* canvas = surf->getCanvas();

                SkPaint paint;

                const SkRect rect = SkRect::MakeLTRB(-10, -10, X_SIZE + 10, Y_SIZE + 10);

                paint.setColor(SK_ColorWHITE);

                canvas->drawRect(rect, paint);

                // Workaround for a bug in old GCC/glibc used in our Chromecast toolchain:
                // error: call to '__warn_memset_zero_len' declared with attribute warning:
                //        memset used with constant zero length parameter; this could be due
                //        to transposed parameters
                // See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61294
                if (bufLen > 0) {
                    memset(readback.get(), kClearValue, bufLen);
                }
                result = surf->readPixels(ii, readback.get(), nonZeroRowBytes, 0, 0);
                REPORTER_ASSERT(reporter, result, "A8 readPixels after clear failed");

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

    static constexpr struct {
        GrColorType fColorType;
        SkAlphaType fAlphaType;
        GrSRGBEncoded fSRGBEncoded;
    } kInfos[] = {
            {GrColorType::kRGBA_8888,    kPremul_SkAlphaType, GrSRGBEncoded::kNo},
            {GrColorType::kBGRA_8888,    kPremul_SkAlphaType, GrSRGBEncoded::kNo},
            {GrColorType::kRGBA_8888,    kPremul_SkAlphaType, GrSRGBEncoded::kYes},
            {GrColorType::kRGBA_1010102, kPremul_SkAlphaType, GrSRGBEncoded::kNo},
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
    for (auto info : kInfos) {
        for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
            uint32_t rgbaData[X_SIZE * Y_SIZE];
            // Make the alpha channel of the rgba texture come from alphaData.
            for (int y = 0; y < Y_SIZE; ++y) {
                for (int x = 0; x < X_SIZE; ++x) {
                    rgbaData[y * X_SIZE + x] = GrColorPackRGBA(6, 7, 8, alphaData[y * X_SIZE + x]);
                }
            }

            auto origin = GrRenderable::kYes == renderable ? kBottomLeft_GrSurfaceOrigin
                                                           : kTopLeft_GrSurfaceOrigin;
            auto proxy = sk_gpu_test::MakeTextureProxyFromData(
                    context, renderable, X_SIZE, Y_SIZE, info.fColorType, info.fAlphaType,
                    info.fSRGBEncoded, origin, rgbaData, 0);
            if (!proxy) {
                continue;
            }

            sk_sp<GrSurfaceContext> sContext = context->priv().makeWrappedSurfaceContext(
                    std::move(proxy), kPremul_SkAlphaType);

            for (auto rowBytes : kRowBytes) {
                size_t nonZeroRowBytes = rowBytes ? rowBytes : X_SIZE;

                std::unique_ptr<uint8_t[]> readback(new uint8_t[nonZeroRowBytes * Y_SIZE]);
                // Clear so we don't accidentally see values from previous iteration.
                memset(readback.get(), kClearValue, nonZeroRowBytes * Y_SIZE);

                // read the texture back
                bool result = sContext->readPixels(dstInfo, readback.get(), rowBytes, 0, 0);
                REPORTER_ASSERT(reporter, result, "8888 readPixels failed");

                // make sure the original & read back versions match
                SkString msg;
                msg.printf("rt:%d, rb:%d 8888", GrRenderable::kYes == renderable,
                                                SkToU32(rowBytes));
                validate_alpha_data(reporter, X_SIZE, Y_SIZE, readback.get(), nonZeroRowBytes,
                                    alphaData, msg, info.fColorType);
            }
        }
    }
}

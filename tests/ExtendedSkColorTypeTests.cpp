/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrContextPriv.h"

#include "tests/Test.h"
#include "tests/TestUtils.h"

static constexpr int kSize = 32;

static SkColor4f get_trans_black_expected_color(SkColorTypeComponentFlag components) {
    float a = 0;
    if (!(components & kAlpha_SkColorTypeComponentFlag)) {
        a = 1;
    }

    return { 0, 0, 0, a };
}

static SkColor4f get_opaque_white_expected_color(SkColorTypeComponentFlag components) {

    if (components & kGray_SkColorTypeComponentFlag) {
        return { 1, 1, 1, 1 };
    }

    float r = 1, g = 1, b = 1;
    if (!(components & kRed_SkColorTypeComponentFlag)) {
        r = 0;
    }
    if (!(components & kGreen_SkColorTypeComponentFlag)) {
        g = 0;
    }
    if (!(components & kBlue_SkColorTypeComponentFlag)) {
        b = 0;
    }

    return { r, g, b, 1.0f };
}

struct TestCase {
    SkColorType              fColorType;
    SkAlphaType              fAlphaType;
    SkColorTypeComponentFlag fComponents;
    bool                     fCanMakeSurfaces;
};

static const TestCase gTests[] = {
    { kAlpha_8_SkColorType,      kPremul_SkAlphaType, kAlpha_SkColorTypeComponentFlag, true  },
    { kRGB_565_SkColorType,      kOpaque_SkAlphaType, kRGB_SkColorTypeComponentFlags,  true  },
    { kARGB_4444_SkColorType,    kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRGBA_8888_SkColorType,    kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRGB_888x_SkColorType,     kOpaque_SkAlphaType, kRGB_SkColorTypeComponentFlags,  true  },
    { kBGRA_8888_SkColorType,    kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRGBA_1010102_SkColorType, kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRGB_101010x_SkColorType,  kOpaque_SkAlphaType, kRGB_SkColorTypeComponentFlags,  true  },
    { kGray_8_SkColorType,       kOpaque_SkAlphaType, kGray_SkColorTypeComponentFlag,  true  },
    { kRGBA_F16Norm_SkColorType, kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRGBA_F16_SkColorType,     kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRGBA_F32_SkColorType,     kPremul_SkAlphaType, kRGBA_SkColorTypeComponentFlags, true  },
    { kRG_88_SkColorType,        kOpaque_SkAlphaType, kRG_SkColorTypeComponentFlags,   false }
};

static void raster_tests(skiatest::Reporter* reporter, const TestCase& test) {

    const SkImageInfo nativeII = SkImageInfo::Make(kSize, kSize, test.fColorType, test.fAlphaType);
    const SkImageInfo f32Unpremul = SkImageInfo::Make(kSize, kSize, kRGBA_F32_SkColorType,
                                                      kUnpremul_SkAlphaType);

    uint32_t actualComponents = SkColorTypeComponentFlags(test.fColorType);
    REPORTER_ASSERT(reporter, test.fComponents == actualComponents);

    // Not all colorTypes can be drawn to
    {
        auto s = SkSurface::MakeRaster(nativeII);
        REPORTER_ASSERT(reporter, SkToBool(s) == test.fCanMakeSurfaces);
    }

    // opaque formats should make transparent black become opaque
    {
        SkAutoPixmapStorage pm;
        pm.alloc(nativeII);
        pm.erase(SkColors::kTransparent);
        SkColor actual = pm.getColor(0, 0);
        SkColor4f expected = get_trans_black_expected_color(test.fComponents);
        REPORTER_ASSERT(reporter, expected.toSkColor() == actual);
    }

    // unused channels should drop out
    {
        SkAutoPixmapStorage pm;
        pm.alloc(nativeII);
        pm.erase(SkColors::kWhite);
        SkColor actual = pm.getColor(0, 0);
        SkColor4f expected = get_opaque_white_expected_color(test.fComponents);
        REPORTER_ASSERT(reporter, expected.toSkColor() == actual);
    }

    // Reading back from an image to the same colorType should always work
    {
        SkAutoPixmapStorage srcPM;
        srcPM.alloc(nativeII);
        srcPM.erase(SkColors::kWhite);
        auto i = SkImage::MakeFromRaster(srcPM, nullptr, nullptr);
        REPORTER_ASSERT(reporter, SkToBool(i));

        SkAutoPixmapStorage readbackPM;
        readbackPM.alloc(nativeII);
        readbackPM.erase(SkColors::kTransparent);

        REPORTER_ASSERT(reporter, i->readPixels(readbackPM, 0, 0));

        SkColor expected = srcPM.getColor(0, 0);
        SkColor actual = readbackPM.getColor(0, 0);
        REPORTER_ASSERT(reporter, expected == actual);
    }

    // Rendering to an F32 surface should always work
    {
        SkAutoPixmapStorage srcPM;
        srcPM.alloc(nativeII);
        srcPM.erase(SkColors::kWhite);
        auto i = SkImage::MakeFromRaster(srcPM, nullptr, nullptr);
        REPORTER_ASSERT(reporter, SkToBool(i));

        auto s = SkSurface::MakeRaster(f32Unpremul);
        REPORTER_ASSERT(reporter, SkToBool(s));

        {
            auto c = s->getCanvas();
            c->drawImage(i, 0, 0);
        }

        SkAutoPixmapStorage readbackPM;
        readbackPM.alloc(f32Unpremul);
        readbackPM.erase(SkColors::kTransparent);

        REPORTER_ASSERT(reporter, i->readPixels(readbackPM, 0, 0));

        SkColor expected = srcPM.getColor(0, 0);
        SkColor actual = readbackPM.getColor(0, 0);
        REPORTER_ASSERT(reporter, expected == actual);
    }
}

static void compare_pixmaps(skiatest::Reporter* reporter,
                            const SkPixmap& expected, const SkPixmap& actual,
                            SkColorType nativeCT, const char* label) {
    const float tols[4] = {0.0f, 0.0f, 0.0f, 0};

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter, nativeCT, label](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "%d %s - mismatch at %d, %d (%f, %f, %f %f)",
                   nativeCT, label, x, y,
                   diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    compare_pixels(expected, actual, tols, error);
}

static void gpu_tests(GrContext* context, skiatest::Reporter* reporter, const TestCase& test) {

    const SkImageInfo nativeII = SkImageInfo::Make(kSize, kSize, test.fColorType, test.fAlphaType);
    const SkImageInfo f32Unpremul = SkImageInfo::Make(kSize, kSize, kRGBA_F32_SkColorType,
                                                      kUnpremul_SkAlphaType);

    // We had better not be able to render to prohibited colorTypes
    if (!test.fCanMakeSurfaces) {
        auto s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, nativeII);
        REPORTER_ASSERT(reporter, !SkToBool(s));
    }

    if (!context->colorTypeSupportedAsImage(test.fColorType)) {
        return;
    }

    SkAutoPixmapStorage nativeExpected;
    nativeExpected.alloc(nativeII);
    nativeExpected.erase(SkColors::kWhite);

    for (bool fullInit : { false, true }) {
        GrBackendTexture backendTex;

        if (fullInit) {
            backendTex = context->priv().createBackendTexture(&nativeExpected, 1,
                                                              GrRenderable::kNo,
                                                              GrProtected::kNo);
        } else {
            backendTex = context->createBackendTexture(kSize, kSize, test.fColorType,
                                                       SkColors::kWhite, GrMipMapped::kNo,
                                                       GrRenderable::kNo, GrProtected::kNo);
        }
        REPORTER_ASSERT(reporter, backendTex.isValid());

        auto img = SkImage::MakeFromTexture(context, backendTex, kTopLeft_GrSurfaceOrigin,
                                            test.fColorType, test.fAlphaType, nullptr);
        REPORTER_ASSERT(reporter, SkToBool(img));

        {
            SkAutoPixmapStorage nativeActual;
            nativeActual.alloc(nativeII);
            nativeActual.erase(SkColors::kTransparent);

            if (img->readPixels(nativeActual, 0, 0)) {
                compare_pixmaps(reporter, nativeExpected, nativeActual,
                                test.fColorType, "SkImage::readPixels to native CT");
            }

            // SkSurface::readPixels with the same colorType as the source pixels round trips
            // (when allowed)
            if (context->colorTypeSupportedAsSurface(test.fColorType)) {
                auto s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, nativeII);
                REPORTER_ASSERT(reporter, SkToBool(s));

                {
                    SkCanvas* c = s->getCanvas();
                    c->drawImage(img, 0, 0);
                }

                nativeActual.erase(SkColors::kTransparent);
                REPORTER_ASSERT(reporter, s->readPixels(nativeActual, 0, 0));

                compare_pixmaps(reporter, nativeExpected, nativeActual,
                                test.fColorType, "SkSurface::readPixels to native CT");
            }
        }

        {
            SkAutoPixmapStorage f32Expected;
            f32Expected.alloc(f32Unpremul);
            f32Expected.erase(get_opaque_white_expected_color(test.fComponents));

            // read back to F32 if possible
            {
                SkAutoPixmapStorage f32Actual;
                f32Actual.alloc(f32Unpremul);
                f32Actual.erase(SkColors::kTransparent);
                if (img->readPixels(f32Actual, 0, 0)) {
                    compare_pixmaps(reporter, f32Expected, f32Actual,
                                    test.fColorType, "SkImage::readPixels to F32");
                }
            }

            // drawing a native SkImage works appropriately (as assessed by reading back from an
            // RGBA8 surface to an F32 pixmap)
            {
                const SkImageInfo rgba8888Premul = SkImageInfo::Make(kSize, kSize,
                                                                     kRGBA_8888_SkColorType,
                                                                     kPremul_SkAlphaType);

                auto s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, rgba8888Premul);
                REPORTER_ASSERT(reporter, SkToBool(s));

                {
                    SkCanvas* c = s->getCanvas();
                    c->drawImage(img, 0, 0);
                }

                SkAutoPixmapStorage f32Actual;
                f32Actual.alloc(f32Unpremul);
                f32Actual.erase(SkColors::kTransparent);
                REPORTER_ASSERT(reporter, s->readPixels(f32Actual, 0, 0));

                compare_pixmaps(reporter, f32Expected, f32Actual,
                                test.fColorType, "SkSurface::drawn to RGBA8888");
            }
        }

        img.reset();
        context->flush();
        context->deleteBackendTexture(backendTex);
    }
}

DEF_TEST(ExtendedSkColorTypeTests_raster, reporter) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        raster_tests(reporter, gTests[i]);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ExtendedSkColorTypeTests_gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        gpu_tests(context, reporter, gTests[i]);
    }
}

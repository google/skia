/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace {
const SkISize kSize = { 32, 32 };
constexpr int kNumMipLevels = 6;

void check_solid_pixmap(skiatest::Reporter* reporter,
                        const SkColor4f& expected,
                        const SkPixmap& actual,
                        SkColorType ct,
                        const char* label) {
    const float kTols[4] = { 0.01f, 0.01f, 0.01f, 0.01f };

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter, ct, label, expected](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "%s %s - mismatch at %d, %d "
                             "expected: (%.2f, %.2f, %.2f, %.2f) "
                             "- diffs: (%.2f, %.2f, %.2f, %.2f)",
                   ToolUtils::colortype_name(ct), label, x, y,
                   expected.fR, expected.fG, expected.fB, expected.fA,
                   diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    CheckSolidPixels(expected, actual, kTols, error);
}

void update_backend_texture(Recorder* recorder,
                            const BackendTexture& backendTex,
                            SkColorType ct,
                            bool withMips,
                            const SkColor4f colors[6]) {
    SkPixmap pixmaps[6];
    std::unique_ptr<char[]> memForPixmaps;

    int numMipLevels = ToolUtils::make_pixmaps(ct, kPremul_SkAlphaType, withMips, colors, pixmaps,
                                               &memForPixmaps);
    SkASSERT(numMipLevels == 1 || numMipLevels == kNumMipLevels);
    SkASSERT(kSize == pixmaps[0].dimensions());

    recorder->updateBackendTexture(backendTex, pixmaps, numMipLevels);

}

BackendTexture create_backend_texture(skiatest::Reporter* reporter,
                                      const Caps* caps,
                                      Recorder* recorder,
                                      SkColorType ct,
                                      bool withMips,
                                      Renderable renderable,
                                      const SkColor4f colors[6]) {
    Mipmapped mipmapped = withMips ? Mipmapped::kYes : Mipmapped::kNo;
    TextureInfo info = caps->getDefaultSampledTextureInfo(ct,
                                                          mipmapped,
                                                          Protected::kNo,
                                                          renderable);

    BackendTexture backendTex = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    update_backend_texture(recorder, backendTex, ct, withMips, colors);

    return backendTex;
}

sk_sp<SkImage> wrap_backend_texture(skiatest::Reporter* reporter,
                                    Recorder* recorder,
                                    const skgpu::graphite::BackendTexture& backendTex,
                                    SkColorType ct,
                                    bool withMips) {
    sk_sp<SkImage> image = SkImages::WrapTexture(recorder,
                                                 backendTex,
                                                 ct,
                                                 kPremul_SkAlphaType,
                                                 /* colorSpace= */ nullptr);
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->hasMipmaps() == withMips);

    return image;
}

void check_levels(skiatest::Reporter* reporter,
                  Context* context,
                  Recorder* recorder,
                  SkImage* image,
                  bool withMips,
                  const SkColor4f colors[6]) {
    int numLevels = withMips ? kNumMipLevels : 1;

    SkSamplingOptions sampling = withMips
                                 ? SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest)
                                 : SkSamplingOptions(SkFilterMode::kNearest);

    SkImageInfo surfaceII = SkImageInfo::Make(kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder, surfaceII, Mipmapped::kNo);
    SkCanvas* canvas = surf->getCanvas();

    for (int i = 0, drawSize = kSize.width(); i < numLevels; ++i, drawSize /= 2) {
        if (i == 5) {
            // TODO: Metal currently never draws the top-most mip-level (skbug.com/13792)
            continue;
        }

        SkImageInfo readbackII = SkImageInfo::Make({drawSize, drawSize}, kRGBA_8888_SkColorType,
                                                   kUnpremul_SkAlphaType);
        SkAutoPixmapStorage actual;
        SkAssertResult(actual.tryAlloc(readbackII));
        actual.erase(SkColors::kTransparent);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);

        canvas->clear(SkColors::kTransparent);

#if 0
        // This option gives greater control over the tilemodes and texture scaling
        SkMatrix lm;
        lm.setScale(1.0f / (1 << i), 1.0f / (1 << i));

        paint.setShader(image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, sampling, lm));
        canvas->drawRect(SkRect::MakeWH(drawSize, drawSize), paint);
#else
        canvas->drawImageRect(image, SkRect::MakeWH(drawSize, drawSize), sampling, &paint);
#endif

        if (!surf->readPixels(actual, 0, 0)) {
            ERRORF(reporter, "readPixels failed");
            return;
        }

        SkString str;
        str.appendf("mip-level %d", i);

        check_solid_pixmap(reporter, colors[i], actual, image->colorType(), str.c_str());
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(UpdateImageBackendTextureTest, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    // TODO: Remove this check once Vulkan supports creating default TexutreInfo from caps and we
    // implement createBackendTexture.
    if (context->backend() == BackendApi::kVulkan) {
        return;
    }

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr SkColor4f kColors[6] = {
        { 1.0f, 0.0f, 0.0f, 1.0f }, // R
        { 0.0f, 1.0f, 0.0f, 0.9f }, // G
        { 0.0f, 0.0f, 1.0f, 0.7f }, // B
        { 0.0f, 1.0f, 1.0f, 0.5f }, // C
        { 1.0f, 0.0f, 1.0f, 0.3f }, // M
        { 1.0f, 1.0f, 0.0f, 0.2f }, // Y
    };

    constexpr SkColor4f kColorsNew[6] = {
        { 1.0f, 1.0f, 0.0f, 0.2f },  // Y
        { 1.0f, 0.0f, 0.0f, 1.0f },  // R
        { 0.0f, 1.0f, 0.0f, 0.9f },  // G
        { 0.0f, 0.0f, 1.0f, 0.7f },  // B
        { 0.0f, 1.0f, 1.0f, 0.5f },  // C
        { 1.0f, 0.0f, 1.0f, 0.3f },  // M
    };

    // TODO: test more than just RGBA8
    for (SkColorType ct : { kRGBA_8888_SkColorType }) {
        for (bool withMips : { true, false }) {
            for (Renderable renderable : { Renderable::kYes, Renderable::kNo }) {

                BackendTexture backendTex = create_backend_texture(reporter, caps, recorder.get(),
                                                                   ct, withMips, renderable,
                                                                   kColors);

                sk_sp<SkImage> image = wrap_backend_texture(reporter, recorder.get(), backendTex,
                                                            ct, withMips);
                if (!image) {
                    continue;
                }

                check_levels(reporter, context, recorder.get(), image.get(), withMips, kColors);

                image.reset();

                update_backend_texture(recorder.get(), backendTex, ct, withMips, kColorsNew);

                image = wrap_backend_texture(reporter, recorder.get(), backendTex, ct, withMips);
                if (!image) {
                    continue;
                }

                check_levels(reporter, context, recorder.get(), image.get(), withMips, kColorsNew);

                image.reset();

                recorder->deleteBackendTexture(backendTex);
            }
        }
    }
}

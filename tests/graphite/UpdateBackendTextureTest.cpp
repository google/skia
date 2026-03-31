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
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/ComparePixels.h"
#include "tools/ToolUtils.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace {
const SkISize kSize = { 32, 32 };
constexpr int kNumMipLevels = 6;

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

void check_solid_pixmap(skiatest::Reporter* reporter,
                        const SkColor4f& expected,
                        const SkPixmap& actual,
                        SkColorType ct,
                        const char* label) {
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

    // 0.5 / (2^bits) is theoretically the limit but with floating point precision 1/2^N is safer.
    float tol = 1.f / (1 << SkColorTypeMaxBitsPerChannel(ct));
    float tols[4] = {tol, tol, tol, tol};
    CheckSolidPixels(expected, actual,  tols, error);
}

void update_backend_texture(Recorder* recorder,
                            const BackendTexture& backendTex,
                            SkColorType ct,
                            bool withMips,
                            const SkColor4f colors[6],
                            GpuFinishedProc finishedProc = nullptr,
                            GpuFinishedContext finishedCtx = nullptr) {
    SkPixmap pixmaps[6];
    std::unique_ptr<char[]> memForPixmaps;

    int numMipLevels = ToolUtils::make_pixmaps(ct, kPremul_SkAlphaType, withMips, colors, pixmaps,
                                               &memForPixmaps);
    SkASSERT(numMipLevels == 1 || numMipLevels == kNumMipLevels);
    SkASSERT(kSize == pixmaps[0].dimensions());

    recorder->updateBackendTexture(backendTex, pixmaps, numMipLevels, finishedProc, finishedCtx);

}

BackendTexture create_backend_texture(skiatest::Reporter* reporter,
                                      const Caps* caps,
                                      Recorder* recorder,
                                      SkColorType ct,
                                      bool withMips,
                                      Renderable renderable,
                                      skgpu::Protected isProtected,
                                      const SkColor4f colors[6],
                                      GpuFinishedProc finishedProc = nullptr,
                                      GpuFinishedContext finishedCtx = nullptr) {
    Mipmapped mipmapped = withMips ? Mipmapped::kYes : Mipmapped::kNo;
    TextureInfo info = caps->getDefaultSampledTextureInfo(ct,
                                                          mipmapped,
                                                          isProtected,
                                                          renderable);

    BackendTexture backendTex = recorder->createBackendTexture(kSize, info);
    if (backendTex.isValid()) {
        update_backend_texture(recorder, backendTex, ct, withMips, colors,
                               finishedProc, finishedCtx);
    } // else the color type mapped to a texture format not supported by the GPU backend
    return backendTex;
}

sk_sp<SkImage> wrap_backend_texture(skiatest::Reporter* reporter,
                                    Recorder* recorder,
                                    const skgpu::graphite::BackendTexture& backendTex,
                                    bool forceOpaque,
                                    bool withMips) {
    sk_sp<SkImage> image = SkImages::WrapTexture(recorder,
                                                 backendTex,
                                                 forceOpaque ? kUnknown_SkAlphaType
                                                             : kPremul_SkAlphaType,
                                                 /* colorSpace= */ nullptr);
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->hasMipmaps() == withMips);

    return image;
}

void check_levels(skiatest::Reporter* reporter,
                  Context* context,
                  Recorder* recorder,
                  SkImage* image,
                  bool forceOpaque,
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
            // TODO: Metal currently never draws the top-most mip-level (skbug.com/40044877)
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

        // The mipmaps of the backend texture were defined in the test as an unpremul color with
        // an alpha value, and converted to premul RGBA before uploading to the GPU. It is *after*
        // that premultiplication that SkImages::WrapTexture's forceOpaque applies so we expect
        // RGB to match the premul value but have A=1.
        SkColor4f expected = forceOpaque ? colors[i].premul().makeOpaque().unpremul() : colors[i];
        check_solid_pixmap(reporter, expected, actual, image->colorType(), str.c_str());
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(UpdateImageBackendTextureTest, reporter, context,
                                         CtsEnforcement::kApiLevel_202404) {
    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    skgpu::Protected isProtected = skgpu::Protected(caps->protectedSupport());

    // TODO: test more than just RGBA8
    for (SkColorType ct : { kRGBA_8888_SkColorType, kARGB_4444_SkColorType }) {
        for (bool withMips : { true, false }) {
            for (bool forceOpaque : { false, true }) {
                for (Renderable renderable : { Renderable::kYes, Renderable::kNo }) {

                    BackendTexture backendTex = create_backend_texture(reporter,
                                                                       caps,
                                                                       recorder.get(),
                                                                       ct, withMips, renderable,
                                                                       isProtected, kColors);
                    if (!backendTex.isValid()) {
                        continue;
                    }

                    sk_sp<SkImage> image = wrap_backend_texture(reporter, recorder.get(),
                                                                backendTex, forceOpaque, withMips);
                    if (!image) {
                        continue;
                    }

                    if (isProtected == skgpu::Protected::kNo) {
                        check_levels(reporter, context, recorder.get(), image.get(),
                                     forceOpaque, withMips, kColors);
                    }

                    image.reset();

                    update_backend_texture(recorder.get(), backendTex, ct, withMips, kColorsNew);

                    image = wrap_backend_texture(reporter, recorder.get(),
                                                 backendTex, forceOpaque, withMips);
                    if (!image) {
                        continue;
                    }

                    if (isProtected == skgpu::Protected::kNo) {
                        check_levels(reporter, context, recorder.get(), image.get(),
                                     forceOpaque, withMips, kColorsNew);
                    }

                    image.reset();

                    recorder->deleteBackendTexture(backendTex);
                }
            }
        }
    }
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(UpdateBackendTextureFinishedProcTest,
                                               reporter,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kApiLevel_202504) {
    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    skgpu::Protected isProtected = skgpu::Protected(caps->protectedSupport());

    struct FinishContext {
        bool fFinishedUpdate = false;
        skiatest::Reporter* fReporter = nullptr;
    };
    FinishContext finishCtx;
    finishCtx.fReporter = reporter;

    auto finishedProc = [](void* ctx, CallbackResult) {
        FinishContext* finishedCtx = (FinishContext*) ctx;
        REPORTER_ASSERT(finishedCtx->fReporter, !(finishedCtx->fFinishedUpdate));
        finishedCtx->fFinishedUpdate = true;
    };

    BackendTexture backendTex = create_backend_texture(reporter,
                                                       caps,
                                                       recorder.get(),
                                                       kRGBA_8888_SkColorType,
                                                       /*withMips=*/false,
                                                       Renderable::kNo,
                                                       isProtected,
                                                       kColors,
                                                       finishedProc,
                                                       &finishCtx);

    // Vulkan may use VK_EXT_host_image_copy to upload the texture data right away, so finished may
    // already be called.
    if (!caps->supportsHostImageCopy()) {
        REPORTER_ASSERT(reporter, !finishCtx.fFinishedUpdate);
    }

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }
    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);

    if (!caps->supportsHostImageCopy()) {
        REPORTER_ASSERT(reporter, !finishCtx.fFinishedUpdate);
    }

    testContext->syncedSubmit(context);
    REPORTER_ASSERT(reporter, finishCtx.fFinishedUpdate);

    recorder->deleteBackendTexture(backendTex);
}

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU
#include "GrCaps.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkSurface.h"
#include "gl/GrGLGpu.h"

// using anonymous namespace because these functions are used as template params.
namespace {
/** convert 0..1 srgb value to 0..1 linear */
float srgb_to_linear(float srgb) {
    if (srgb <= 0.04045f) {
        return srgb / 12.92f;
    } else {
        return powf((srgb + 0.055f) / 1.055f, 2.4f);
    }
}

/** convert 0..1 linear value to 0..1 srgb */
float linear_to_srgb(float linear) {
    if (linear <= 0.0031308) {
        return linear * 12.92f;
    } else {
        return 1.055f * powf(linear, 1.f / 2.4f) - 0.055f;
    }
}
}

static bool check_value(U8CPU value, U8CPU expected, U8CPU error) {
    if (value >= expected) {
        return (value - expected) <= error;
    } else {
        return (expected - value) <= error;
    }
}

void read_and_check_pixels(skiatest::Reporter* reporter, GrSurfaceContext* context,
                           U8CPU expected, const SkImageInfo& dstInfo,
                           U8CPU error, const char* subtestName) {
    int w = dstInfo.width();
    int h = dstInfo.height();
    SkAutoTMalloc<uint32_t> readData(w * h);
    memset(readData.get(), 0, sizeof(uint32_t) * w * h);

    if (!context->readPixels(dstInfo, readData.get(), 0, 0, 0)) {
        ERRORF(reporter, "Could not read pixels for %s.", subtestName);
        return;
    }

    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            uint32_t read = readData[j * w + i];

            bool success =
                check_value(read & 0xff, expected, error) &&
                check_value((read >> 8) & 0xff, expected, error) &&
                check_value((read >> 16) & 0xff, expected, error);

            if (!success) {
                ERRORF(reporter, "Expected 0xff%02x%02x%02x, read back as 0x%08x in %s at %d, %d.",
                       expected, expected, expected, read, subtestName, i, j);
                return;
            }
        }
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SRGBMipMaps, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->caps()->srgbSupport()) {
        return;
    }

    const int rtS = 16;
    const int texS = rtS * 2;

    // Fill texture with a dither of black and 60% sRGB (~ 32.5% linear) gray. Although there is
    // only one likely failure mode (doing a direct downsample of the sRGB values), this pattern
    // maximizes the minimum error across all three conceivable failure modes:
    // 1) Likely incorrect:
    //      (A + B) / 2
    // 2) No input decode, decode output:
    //      linear_to_srgb((A + B) / 2)
    // 3) Decode input, no output encode:
    //      (srgb_to_linear(A) + srgb_to_linear(B)) / 2

    const U8CPU srgb60 = sk_float_round2int(0.6f * 255.0f);
    static const SkPMColor colors[2] = {
        SkPackARGB32(0xFF, srgb60, srgb60, srgb60),
        SkPackARGB32(0xFF, 0x00, 0x00, 0x00)
    };
    uint32_t texData[texS * texS];
    for (int y = 0; y < texS; ++y) {
        for (int x = 0; x < texS; ++x) {
            texData[y * texS + x] = colors[(x + y) % 2];
        }
    }

    // We can be pretty generous with the error detection, thanks to the choice of input.
    // The closest likely failure mode is off by > 0.1, so anything that encodes within
    // 10/255 of optimal is more than good enough for this test.
    const U8CPU expectedSRGB = sk_float_round2int(
        linear_to_srgb(srgb_to_linear(srgb60 / 255.0f) / 2.0f) * 255.0f);
    const U8CPU expectedLinear = srgb60 / 2;
    const U8CPU error = 10;

    const SkImageInfo iiSRGBA = SkImageInfo::Make(rtS, rtS, kRGBA_8888_SkColorType,
                                                  kPremul_SkAlphaType,
                                                  SkColorSpace::MakeSRGB());
    const SkImageInfo iiRGBA = SkImageInfo::Make(rtS, rtS, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);

    // Create our test texture
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fConfig = kSRGBA_8888_GrPixelConfig;
    desc.fWidth = texS;
    desc.fHeight = texS;

    GrResourceProvider* resourceProvider = context->resourceProvider();
    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(resourceProvider,
                                                               desc, SkBudgeted::kNo,
                                                               texData, 0);

    // Create two render target contexts (L32 and S32)
    sk_sp<SkColorSpace> srgbColorSpace = SkColorSpace::MakeSRGB();
    sk_sp<GrRenderTargetContext> l32RenderTargetContext = context->makeDeferredRenderTargetContext(
        SkBackingFit::kExact, rtS, rtS, kRGBA_8888_GrPixelConfig, nullptr);
    sk_sp<GrRenderTargetContext> s32RenderTargetContext = context->makeDeferredRenderTargetContext(
        SkBackingFit::kExact, rtS, rtS, kSRGBA_8888_GrPixelConfig, std::move(srgbColorSpace));

    SkRect rect = SkRect::MakeWH(SkIntToScalar(rtS), SkIntToScalar(rtS));
    GrNoClip noClip;
    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    GrSamplerParams mipMapParams(SkShader::kRepeat_TileMode, GrSamplerParams::kMipMap_FilterMode);
    paint.addColorTextureProcessor(std::move(proxy),
                                   nullptr, SkMatrix::MakeScale(rtS), mipMapParams);

    // 1) Draw texture to S32 surface (should generate/use sRGB mips)
    paint.setGammaCorrect(true);
    s32RenderTargetContext->drawRect(noClip, GrPaint(paint), GrAA::kNo, SkMatrix::I(), rect);
    read_and_check_pixels(reporter, s32RenderTargetContext.get(), expectedSRGB, iiSRGBA, error,
                          "first render of sRGB");

    // 2) Draw texture to L32 surface (should generate/use linear mips)
    paint.setGammaCorrect(false);
    l32RenderTargetContext->drawRect(noClip, GrPaint(paint), GrAA::kNo, SkMatrix::I(), rect);

    // Right now, this test only runs on GL (because Vulkan doesn't support legacy mip-mapping
    // skbug.com/5048). On GL, we may not have sRGB decode support. In that case, rendering sRGB
    // textures to a legacy surface produces nonsense, so this part of the test is meaningless.
    //
    // We also skip this part of the test on command buffer (via srgbDecodeDisableAffectsMipmaps),
    // because that implementation of the extension doesn't ensure that mips respect the setting.
    //
    // TODO: Once Vulkan supports legacy mip-mapping, we can promote this to GrCaps. Right now,
    // Vulkan has most of the functionality, but not the mip-mapping part that's being tested here.
    GrGLGpu* glGpu = static_cast<GrGLGpu*>(context->getGpu());
    if (glGpu->glCaps().srgbDecodeDisableSupport() &&
        glGpu->glCaps().srgbDecodeDisableAffectsMipmaps()) {
        read_and_check_pixels(reporter, l32RenderTargetContext.get(), expectedLinear, iiRGBA,
                              error, "re-render as linear");
    }

    // 3) Go back to sRGB
    paint.setGammaCorrect(true);
    s32RenderTargetContext->drawRect(noClip, std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    read_and_check_pixels(reporter, s32RenderTargetContext.get(), expectedSRGB, iiSRGBA, error,
                          "re-render as sRGB");
}
#endif

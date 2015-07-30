/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU
#include "SkCanvas.h"

#include "SkSurface.h"
#include "GrContextFactory.h"
#include "GrCaps.h"

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

/** tests a conversion with an error tolerance */
template <float (*CONVERT)(float)> static bool check_conversion(uint32_t input, uint32_t output,
                                                                float error) {
    // alpha should always be exactly preserved.
    if ((input & 0xff000000) != (output & 0xff000000)) {
        return false;
    }

    for (int c = 0; c < 3; ++c) {
        uint8_t inputComponent = (uint8_t) ((input & (0xff << (c*8))) >> (c*8));
        float lower = SkTMax(0.f, (float) inputComponent - error);
        float upper = SkTMin(255.f, (float) inputComponent + error);
        lower = CONVERT(lower / 255.f);
        upper = CONVERT(upper / 255.f);
        SkASSERT(lower >= 0.f && lower <= 255.f);
        SkASSERT(upper >= 0.f && upper <= 255.f);
        uint8_t outputComponent = (output & (0xff << (c*8))) >> (c*8);
        if (outputComponent < SkScalarFloorToInt(lower * 255.f) ||
            outputComponent > SkScalarCeilToInt(upper * 255.f)) {
            return false;
        }
    }
    return true;
}

/** tests a forward and backward conversion with an error tolerance */
template <float (*FORWARD)(float), float (*BACKWARD)(float)>
static bool check_double_conversion(uint32_t input, uint32_t output, float error) {
    // alpha should always be exactly preserved.
    if ((input & 0xff000000) != (output & 0xff000000)) {
        return false;
    }

    for (int c = 0; c < 3; ++c) {
        uint8_t inputComponent = (uint8_t) ((input & (0xff << (c*8))) >> (c*8));
        float lower = SkTMax(0.f, (float) inputComponent - error);
        float upper = SkTMin(255.f, (float) inputComponent + error);
        lower = FORWARD(lower / 255.f);
        upper = FORWARD(upper / 255.f);
        SkASSERT(lower >= 0.f && lower <= 255.f);
        SkASSERT(upper >= 0.f && upper <= 255.f);
        uint8_t upperComponent = SkScalarCeilToInt(upper * 255.f);
        uint8_t lowerComponent = SkScalarFloorToInt(lower * 255.f);
        lower = SkTMax(0.f, (float) lowerComponent - error);
        upper = SkTMin(255.f, (float) upperComponent + error);
        lower = BACKWARD(lowerComponent / 255.f);
        upper = BACKWARD(upperComponent / 255.f);
        SkASSERT(lower >= 0.f && lower <= 255.f);
        SkASSERT(upper >= 0.f && upper <= 255.f);
        upperComponent = SkScalarCeilToInt(upper * 255.f);
        lowerComponent = SkScalarFloorToInt(lower * 255.f);

        uint8_t outputComponent = (output & (0xff << (c*8))) >> (c*8);
        if (outputComponent < lowerComponent || outputComponent > upperComponent) {
            return false;
        }
    }
    return true;
}

static bool check_srgb_to_linear_conversion(uint32_t srgb, uint32_t linear, float error) {
    return check_conversion<srgb_to_linear>(srgb, linear, error);
}

static bool check_linear_to_srgb_conversion(uint32_t linear, uint32_t srgb, float error) {
    return check_conversion<linear_to_srgb>(linear, srgb, error);
}

static bool check_linear_to_srgb_to_linear_conversion(uint32_t input, uint32_t output, float error) {
    return check_double_conversion<linear_to_srgb, srgb_to_linear>(input, output, error);
}

static bool check_srgb_to_linear_to_srgb_conversion(uint32_t input, uint32_t output, float error) {
    return check_double_conversion<srgb_to_linear, linear_to_srgb>(input, output, error);
}

typedef bool (*CheckFn) (uint32_t orig, uint32_t actual, float error);

void read_and_check_pixels(skiatest::Reporter* reporter, GrTexture* texture, uint32_t* origData,
                           GrPixelConfig readConfig, CheckFn checker, float error,
                           const char* subtestName) {
    int w = texture->width();
    int h = texture->height();
    SkAutoTMalloc<uint32_t> readData(w * h);
    memset(readData.get(), 0, sizeof(uint32_t) * w * h);
    if (!texture->readPixels(0, 0, w, h, readConfig, readData.get())) {
        ERRORF(reporter, "Could not read pixels for %s.", subtestName);
        return;
    }
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            uint32_t orig = origData[j * w + i];
            uint32_t read = readData[j * w + i];

            if (!checker(orig, read, error)) {
                ERRORF(reporter, "Expected 0x%08x, read back as 0x%08x in %s at %d, %d).",
                       orig, read, subtestName, i, j);
                return;
            }
        }
    }
}

// TODO: Add tests for copySurface between srgb/linear textures. Add tests for unpremul/premul
// conversion during read/write along with srgb/linear conversions.
DEF_GPUTEST(SRGBReadWritePixels, reporter, factory) {
    static const int kW = 255;
    static const int kH = 255;
    uint32_t origData[kW * kH];
    for (int j = 0; j < kH; ++j) {
        for (int i = 0; i < kW; ++i) {
            origData[j * kW + i] = (j << 24) | (i << 16) | (i << 8) | i;
        }
    }

    for (int t = 0; t < GrContextFactory::kGLContextTypeCnt; ++t) {
        GrContextFactory::GLContextType glType = (GrContextFactory::GLContextType) t;
        GrContext* context;
        // We allow more error on GPUs with lower precision shader variables.
        if (!GrContextFactory::IsRenderingGLContext(glType) || !(context = factory->get(glType))) {
            continue;
        }

        GrSurfaceDesc desc;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fWidth = kW;
        desc.fHeight = kH;
        desc.fConfig = kSRGBA_8888_GrPixelConfig;
        if (context->caps()->isConfigRenderable(desc.fConfig, false) &&
            context->caps()->isConfigTexturable(desc.fConfig)) {
            SkAutoTUnref<GrTexture> tex(context->textureProvider()->createTexture(desc, false));
            if (!tex) {
                ERRORF(reporter, "Could not create SRGBA texture.");
                continue;
            }

            float error = context->caps()->shaderCaps()->floatPrecisionVaries() ? 1.2f  : 0.5f;

            // Write srgba data and read as srgba and then as rgba
            if (tex->writePixels(0, 0, kW, kH, kSRGBA_8888_GrPixelConfig, origData)) {
                // For the all-srgba case, we allow a small error only for devices that have
                // precision variation because the srgba data gets converted to linear and back in
                // the shader.
                float smallError = context->caps()->shaderCaps()->floatPrecisionVaries() ? 1.f :
                                                                                           0.0f;
                read_and_check_pixels(reporter, tex, origData, kSRGBA_8888_GrPixelConfig,
                                      check_srgb_to_linear_to_srgb_conversion, smallError,
                                      "write/read srgba to srgba texture");
                read_and_check_pixels(reporter, tex, origData, kRGBA_8888_GrPixelConfig,
                                      check_srgb_to_linear_conversion, error,
                                      "write srgba/read rgba with srgba texture");
            } else {
                ERRORF(reporter, "Could not write srgba data to srgba texture.");
            }

            // Now verify that we can write linear data
            if (tex->writePixels(0, 0, kW, kH, kRGBA_8888_GrPixelConfig, origData)) {
                // We allow more error on GPUs with lower precision shader variables.
                read_and_check_pixels(reporter, tex, origData, kSRGBA_8888_GrPixelConfig,
                                      check_linear_to_srgb_conversion, error,
                                      "write rgba/read srgba with srgba texture");
                read_and_check_pixels(reporter, tex, origData, kRGBA_8888_GrPixelConfig,
                                      check_linear_to_srgb_to_linear_conversion, error,
                                      "write/read rgba with srgba texture");
            } else {
                ERRORF(reporter, "Could not write rgba data to srgba texture.");
            }

            desc.fConfig = kRGBA_8888_GrPixelConfig;
            tex.reset(context->textureProvider()->createTexture(desc, false));
            if (!tex) {
                ERRORF(reporter, "Could not create RGBA texture.");
                continue;
            }

            // Write srgba data to a rgba texture and read back as srgba and rgba
            if (tex->writePixels(0, 0, kW, kH, kSRGBA_8888_GrPixelConfig, origData)) {
               read_and_check_pixels(reporter, tex, origData, kSRGBA_8888_GrPixelConfig,
                                      check_srgb_to_linear_to_srgb_conversion, error,
                                      "write/read srgba to rgba texture");
               read_and_check_pixels(reporter, tex, origData, kRGBA_8888_GrPixelConfig,
                                     check_srgb_to_linear_conversion, error,
                                     "write srgba/read rgba to rgba texture");
            } else {
                ERRORF(reporter, "Could not write srgba data to rgba texture.");
            }

            // Write rgba data to a rgba texture and read back as srgba
            if (tex->writePixels(0, 0, kW, kH, kRGBA_8888_GrPixelConfig, origData)) {
                read_and_check_pixels(reporter, tex, origData, kSRGBA_8888_GrPixelConfig,
                                      check_linear_to_srgb_conversion, 1.2f,
                                      "write rgba/read srgba to rgba texture");
            } else {
                ERRORF(reporter, "Could not write rgba data to rgba texture.");
            }
}
    }
}
#endif

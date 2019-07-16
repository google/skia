/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"

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

static bool check_no_conversion(uint32_t input, uint32_t output, float error) {
    // This is a bit of a hack to check identity transformations that may lose precision.
    return check_srgb_to_linear_to_srgb_conversion(input, output, error);
}

typedef bool (*CheckFn) (uint32_t orig, uint32_t actual, float error);

void read_and_check_pixels(skiatest::Reporter* reporter, GrSurfaceContext* context,
                           uint32_t* origData,
                           const SkImageInfo& dstInfo, CheckFn checker, float error,
                           const char* subtestName) {
    int w = dstInfo.width();
    int h = dstInfo.height();
    SkAutoTMalloc<uint32_t> readData(w * h);
    memset(readData.get(), 0, sizeof(uint32_t) * w * h);

    if (!context->readPixels(dstInfo, readData.get(), 0, {0, 0})) {
        ERRORF(reporter, "Could not read pixels for %s.", subtestName);
        return;
    }

    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            uint32_t orig = origData[j * w + i];
            uint32_t read = readData[j * w + i];

            if (!checker(orig, read, error)) {
                ERRORF(reporter, "Original 0x%08x, read back as 0x%08x in %s at %d, %d).", orig,
                       read, subtestName, i, j);
                return;
            }
        }
    }
}

namespace {
enum class Encoding {
    kUntagged,
    kLinear,
    kSRGB,
};
}

static sk_sp<SkColorSpace> encoding_as_color_space(Encoding encoding) {
    switch (encoding) {
        case Encoding::kUntagged: return nullptr;
        case Encoding::kLinear:   return SkColorSpace::MakeSRGBLinear();
        case Encoding::kSRGB:     return SkColorSpace::MakeSRGB();
    }
    return nullptr;
}

static const char* encoding_as_str(Encoding encoding) {
    switch (encoding) {
        case Encoding::kUntagged: return "untagged";
        case Encoding::kLinear:   return "linear";
        case Encoding::kSRGB:     return "sRGB";
    }
    return nullptr;
}

static constexpr int kW = 255;
static constexpr int kH = 255;

static std::unique_ptr<uint32_t[]> make_data() {
    std::unique_ptr<uint32_t[]> data(new uint32_t[kW * kH]);
    for (int j = 0; j < kH; ++j) {
        for (int i = 0; i < kW; ++i) {
            data[j * kW + i] = (0xFF << 24) | (i << 16) | (i << 8) | i;
        }
    }
    return data;
}

static sk_sp<GrSurfaceContext> make_surface_context(Encoding contextEncoding, GrContext* context,
                                                    skiatest::Reporter* reporter) {
    auto surfaceContext = context->priv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kW, kH, GrColorType::kRGBA_8888,
            encoding_as_color_space(contextEncoding), 1, GrMipMapped::kNo,
            kBottomLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kNo, GrProtected::kNo);
    if (!surfaceContext) {
        ERRORF(reporter, "Could not create %s surface context.", encoding_as_str(contextEncoding));
    }
    return std::move(surfaceContext);
}

static void test_write_read(Encoding contextEncoding, Encoding writeEncoding, Encoding readEncoding,
                            float error, CheckFn check, GrContext* context,
                            skiatest::Reporter* reporter) {
    auto surfaceContext = make_surface_context(contextEncoding, context, reporter);
    if (!surfaceContext) {
        return;
    }
    auto writeII = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                     encoding_as_color_space(writeEncoding));
    auto data = make_data();
    if (!surfaceContext->writePixels(writeII, data.get(), 0, {0, 0})) {
        ERRORF(reporter, "Could not write %s to %s surface context.",
               encoding_as_str(writeEncoding), encoding_as_str(contextEncoding));
        return;
    }

    auto readII = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                    encoding_as_color_space(readEncoding));
    SkString testName;
    testName.printf("write %s data to a %s context and read as %s.", encoding_as_str(writeEncoding),
                    encoding_as_str(contextEncoding), encoding_as_str(readEncoding));
    read_and_check_pixels(reporter, surfaceContext.get(), data.get(), readII, check, error,
                          testName.c_str());
}

// Test all combinations of writePixels/readPixels where the surface context/write source/read dst
// are sRGB, linear, or untagged RGBA_8888.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SRGBReadWritePixels, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->priv().caps()->isConfigRenderable(kSRGBA_8888_GrPixelConfig) &&
        !context->priv().caps()->isConfigTexturable(kSRGBA_8888_GrPixelConfig)) {
        return;
    }
    // We allow more error on GPUs with lower precision shader variables.
    float error = context->priv().caps()->shaderCaps()->halfIs32Bits() ? 0.5f : 1.2f;
    // For the all-sRGB case, we allow a small error only for devices that have
    // precision variation because the sRGB data gets converted to linear and back in
    // the shader.
    float smallError = context->priv().caps()->shaderCaps()->halfIs32Bits() ? 0.0f : 1.f;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Write sRGB data to a sRGB context - no conversion on the write.

    // back to sRGB - no conversion.
    test_write_read(Encoding::kSRGB, Encoding::kSRGB, Encoding::kSRGB, smallError,
                    check_no_conversion, context, reporter);
    // Reading back to untagged should be a pass through with no conversion.
    test_write_read(Encoding::kSRGB, Encoding::kSRGB, Encoding::kUntagged, error,
                    check_no_conversion, context, reporter);

    // Converts back to linear
    test_write_read(Encoding::kSRGB, Encoding::kSRGB, Encoding::kLinear, error,
                    check_srgb_to_linear_conversion, context, reporter);

    // Untagged source data should be interpreted as sRGB.
    test_write_read(Encoding::kSRGB, Encoding::kUntagged, Encoding::kSRGB, smallError,
                    check_no_conversion, context, reporter);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Write linear data to a sRGB context. It gets converted to sRGB on write. The reads
    // are all the same as the above cases where the original data was untagged.
    test_write_read(Encoding::kSRGB, Encoding::kLinear, Encoding::kSRGB, error,
                    check_linear_to_srgb_conversion, context, reporter);
    // When the dst buffer is untagged there should be no conversion on the read.
    test_write_read(Encoding::kSRGB, Encoding::kLinear, Encoding::kUntagged, error,
                    check_linear_to_srgb_conversion, context, reporter);
    test_write_read(Encoding::kSRGB, Encoding::kLinear, Encoding::kLinear, error,
                    check_linear_to_srgb_to_linear_conversion, context, reporter);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Write data to an untagged context. The write does no conversion no matter what encoding the
    // src data has.
    for (auto writeEncoding : {Encoding::kSRGB, Encoding::kUntagged, Encoding::kLinear}) {
        // The read from untagged to sRGB also does no conversion.
        test_write_read(Encoding::kUntagged, writeEncoding, Encoding::kSRGB, error,
                        check_no_conversion, context, reporter);
        // Reading untagged back as untagged should do no conversion.
        test_write_read(Encoding::kUntagged, writeEncoding, Encoding::kUntagged, error,
                        check_no_conversion, context, reporter);
        // Reading untagged back as linear does convert (context is source, so treated as sRGB),
        // dst is tagged.
        test_write_read(Encoding::kUntagged, writeEncoding, Encoding::kLinear, error,
                        check_srgb_to_linear_conversion, context, reporter);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Write sRGB data to a linear context - converts to sRGB on the write.

    // converts back to sRGB on read.
    test_write_read(Encoding::kLinear, Encoding::kSRGB, Encoding::kSRGB, error,
                    check_srgb_to_linear_to_srgb_conversion, context, reporter);
    // Reading untagged data from linear currently does no conversion.
    test_write_read(Encoding::kLinear, Encoding::kSRGB, Encoding::kUntagged, error,
                    check_srgb_to_linear_conversion, context, reporter);
    // Stays linear when read.
    test_write_read(Encoding::kLinear, Encoding::kSRGB, Encoding::kLinear, error,
                    check_srgb_to_linear_conversion, context, reporter);

    // Untagged source data should be interpreted as sRGB.
    test_write_read(Encoding::kLinear, Encoding::kUntagged, Encoding::kSRGB, error,
                    check_srgb_to_linear_to_srgb_conversion, context, reporter);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Write linear data to a linear context. Does no conversion.

    // Reading to sRGB does a conversion.
    test_write_read(Encoding::kLinear, Encoding::kLinear, Encoding::kSRGB, error,
                    check_linear_to_srgb_conversion, context, reporter);
    // Reading to untagged does no conversion.
    test_write_read(Encoding::kLinear, Encoding::kLinear, Encoding::kUntagged, error,
                    check_no_conversion, context, reporter);
    // Stays linear when read.
    test_write_read(Encoding::kLinear, Encoding::kLinear, Encoding::kLinear, error,
                    check_no_conversion, context, reporter);
}

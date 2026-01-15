/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PaintParamsTestUtils_DEFINED
#define PaintParamsTestUtils_DEFINED

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "src/base/SkRandom.h"

class SkCanvas;

namespace skgpu::graphite {
    class Recorder;
    class PrecompileShader;
    class PrecompileBlender;
    class PrecompileColorFilter;
    class PrecompileImageFilter;
    class PrecompileMaskFilter;
}

namespace skiatest::graphite {

#define SK_ALL_TEST_SHADERS(M) \
    M(Blend)              \
    M(ColorFilter)        \
    M(CoordClamp)         \
    M(ConicalGradient)    \
    M(Image)              \
    M(LinearGradient)     \
    M(LocalMatrix)        \
    M(None)               \
    M(PerlinNoise)        \
    M(Picture)            \
    M(RadialGradient)     \
    M(Runtime)            \
    M(SolidColor)         \
    M(SweepGradient)      \
    M(YUVImage)           \
    M(WorkingColorSpace)

enum class ShaderType {
#define M(type) k##type,
    SK_ALL_TEST_SHADERS(M)
#undef M
    kLast = kWorkingColorSpace
};

const char* ToStr(ShaderType s);

#define SK_ALL_TEST_MASKFILTERS(M) \
    M(None)                        \
    M(Blur)

enum class MaskFilterType {
#define M(type) k##type,
    SK_ALL_TEST_MASKFILTERS(M)
#undef M
    kLast = kBlur
};

const char* ToStr(MaskFilterType mf);

#define SK_ALL_TEST_BLENDERS(M) \
    M(None)        \
    M(PorterDuff)  \
    M(ShaderBased) \
    M(Arithmetic)  \
    M(Runtime)

enum class BlenderType {
#define M(type) k##type,
    SK_ALL_TEST_BLENDERS(M)
#undef M
    kLast = kRuntime
};

const char* ToStr(BlenderType b);

#define SK_ALL_TEST_COLORFILTERS(M) \
    M(None)            \
    M(BlendMode)       \
    M(ColorSpaceXform) \
    M(Compose)         \
    M(Gaussian)        \
    M(HighContrast)    \
    M(HSLAMatrix)      \
    M(Lerp)            \
    M(Lighting)        \
    M(LinearToSRGB)    \
    M(Luma)            \
    M(Matrix)          \
    M(Overdraw)        \
    M(Runtime)         \
    M(SRGBToLinear)    \
    M(Table)           \
    M(WorkingFormat)

enum class ColorFilterType {
#define M(type) k##type,
    SK_ALL_TEST_COLORFILTERS(M)
#undef M
    kLast = kWorkingFormat
};

const char* ToStr(ColorFilterType cf);

#define SK_ALL_TEST_CLIPS(M) \
    M(None)                  \
    M(Shader)                \
    M(Shader_Diff)           \
    M(Analytic)              \
    M(AnalyticAndShader)

enum class ClipType {
#define M(type) k##type,
    SK_ALL_TEST_CLIPS(M)
#undef M
    kLast = kAnalyticAndShader
};

const char* ToStr(ClipType c);

#define SK_ALL_TEST_IMAGE_FILTERS(M) \
    M(None)              \
    M(Arithmetic)        \
    M(BlendMode)         \
    M(RuntimeBlender)    \
    M(Blur)              \
    M(ColorFilter)       \
    M(Displacement)      \
    M(Lighting)          \
    M(MatrixConvolution) \
    M(Morphology)

enum class ImageFilterType {
#define M(type) k##type,
    SK_ALL_TEST_IMAGE_FILTERS(M)
#undef M
    kLast = kMorphology
};

const char* ToStr(ImageFilterType c);

const char* ToStr(skgpu::graphite::DrawTypeFlags dt);

struct DrawData {
    DrawData();

    SkPath fPath;
    sk_sp<SkTextBlob> fBlob;
    sk_sp<SkTextBlob> fLCDBlob;
    sk_sp<SkTextBlob> fEmojiBlob;
    sk_sp<SkTextBlob> fPathBlob;
    sk_sp<SkVertices> fVertsWithColors;
    sk_sp<SkVertices> fVertsWithOutColors;
};

std::pair<sk_sp<SkShader>, sk_sp<skgpu::graphite::PrecompileShader>>
CreateRandomShader(SkRandom* rand, skgpu::graphite::Recorder* recorder, ShaderType type,
                   bool* reqSKPOption = nullptr);

std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>>
CreateRandomBlender(SkRandom* rand, BlenderType type);

std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>>
CreateRandomColorFilter(SkRandom* rand, ColorFilterType type);

std::pair<sk_sp<SkMaskFilter>, sk_sp<skgpu::graphite::PrecompileMaskFilter>>
CreateRandomMaskFilter(SkRandom* rand, MaskFilterType type);

std::pair<sk_sp<SkImageFilter>, sk_sp<skgpu::graphite::PrecompileImageFilter>>
CreateRandomImageFilter(skgpu::graphite::Recorder* recorder, SkRandom* rand, ImageFilterType type);

std::pair<sk_sp<SkShader>, sk_sp<skgpu::graphite::PrecompileShader>>
CreateClipShader(SkRandom* rand, skgpu::graphite::Recorder* recorder);

std::pair<SkPaint, skgpu::graphite::PaintOptions>
CreateRandomPaint(SkRandom* rand,
                  skgpu::graphite::Recorder* recorder,
                  ShaderType shaderType,
                  BlenderType blenderType,
                  ColorFilterType cfType,
                  MaskFilterType mfType,
                  ImageFilterType imageFilterType,
                  bool* reqSKPPaintOption = nullptr);

skgpu::graphite::DrawTypeFlags RandomDrawType(SkRandom* rand);

void ExecuteDraw(SkCanvas* canvas,
                 const SkPaint& paint,
                 const DrawData& drawData,
                 skgpu::graphite::DrawTypeFlags dt);

} // namespace skiatest::graphite

#endif // SK_GRAPHITE

#endif // PaintParamsTestUtils_DEFINED

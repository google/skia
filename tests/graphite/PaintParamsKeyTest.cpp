/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkBlenders.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/base/SkRandom.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PublicPrecompile.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/UniqueKeyUtils.h"

// Set this to 1 for more expansive (aka far slower) local testing
#define EXPANDED_SET 0

// This flag is set to true if during the PaintOption creation an SkPictureShader is created.
// The SkPictureShader will need an additional program in order to draw the contents of its
// SkPicture.
bool gNeedSKPPaintOption = false;

using namespace skgpu::graphite;

namespace {

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom*, Recorder*);
std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom*);
std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(SkRandom*);
[[maybe_unused]] std::pair<sk_sp<SkImageFilter>, SkEnumBitMask<PrecompileImageFilters>>
                                                             create_random_image_filter(SkRandom*);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#define SK_ALL_TEST_SHADERS(M) \
    M(Blend)              \
    M(ColorFilter)        \
    M(CoordClamp)         \
    M(ConicalGradient)    \
    M(Empty)              \
    M(Image)              \
    M(LinearGradient)     \
    M(LocalMatrix)        \
    M(None)               \
    M(PerlinNoise)        \
    M(Picture)            \
    M(RadialGradient)     \
    M(SolidColor)         \
    M(SweepGradient)      \
    M(WorkingColorSpace)

enum class ShaderType {
#define M(type) k##type,
    SK_ALL_TEST_SHADERS(M)
#undef M

    kLast          = kWorkingColorSpace
};

static constexpr int kShaderTypeCount = static_cast<int>(ShaderType::kLast) + 1;

const char* to_str(ShaderType s) {
    switch (s) {
#define M(type) case ShaderType::k##type : return "ShaderType::k" #type;
        SK_ALL_TEST_SHADERS(M)
#undef M
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#define SK_ALL_TEST_BLENDERS(M) \
    M(None)        \
    M(PorterDuff)  \
    M(ShaderBased) \
    M(Arithmetic)  \
    M(Runtime)

// TODO: do we need to add a separable category and/or a category for dstRead requiring blends?
enum class BlenderType {
#define M(type) k##type,
    SK_ALL_TEST_BLENDERS(M)
#undef M

    kLast = kRuntime
};

static constexpr int kBlenderTypeCount = static_cast<int>(BlenderType::kLast) + 1;

const char* to_str(BlenderType b) {
    switch (b) {
#define M(type) case BlenderType::k##type : return "BlenderType::k" #type;
        SK_ALL_TEST_BLENDERS(M)
#undef M
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#define SK_ALL_TEST_COLORFILTERS(M) \
    M(None)            \
    M(BlendMode)       \
    M(ColorSpaceXform) \
    M(Compose)         \
    M(Gaussian)        \
    M(HSLAMatrix)      \
    M(Lerp)            \
    M(Lighting)        \
    M(LinearToSRGB)    \
    M(Luma)            \
    M(Matrix)          \
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

static constexpr int kColorFilterTypeCount = static_cast<int>(ColorFilterType::kLast) + 1;

const char* to_str(ColorFilterType cf) {
    switch (cf) {
#define M(type) case ColorFilterType::k##type : return "ColorFilterType::k" #type;
        SK_ALL_TEST_COLORFILTERS(M)
#undef M
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#define SK_ALL_TEST_CLIPS(M) \
    M(None)            \
    M(Shader)          \
    M(Shader_Diff)

enum class ClipType {
#define M(type) k##type,
    SK_ALL_TEST_CLIPS(M)
#undef M
};

const char* to_str(ClipType c) {
    switch (c) {
#define M(type) case ClipType::k##type : return "ClipType::k" #type;
        SK_ALL_TEST_CLIPS(M)
#undef M
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#define SK_ALL_TEST_IMAGE_FILTERS(M) \
    M(None)            \
    M(Blur)

enum class ImageFilterType {
#define M(type) k##type,
    SK_ALL_TEST_IMAGE_FILTERS(M)
#undef M
    kLast = kBlur
};

static constexpr int kImageFilterTypeCount = static_cast<int>(ImageFilterType::kLast) + 1;

const char* to_str(ImageFilterType c) {
    switch (c) {
#define M(type) case ImageFilterType::k##type : return "ImageFilterType::k" #type;
        SK_ALL_TEST_IMAGE_FILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
static constexpr skcms_TransferFunction gTransferFunctions[] = {
    SkNamedTransferFn::kSRGB,
    SkNamedTransferFn::k2Dot2,
    SkNamedTransferFn::kLinear,
    SkNamedTransferFn::kRec2020,
    SkNamedTransferFn::kPQ,
    SkNamedTransferFn::kHLG,
};

static constexpr int kTransferFunctionCount = std::size(gTransferFunctions);

const skcms_TransferFunction& random_xfer_function(SkRandom* rand) {
    return gTransferFunctions[rand->nextULessThan(kTransferFunctionCount)];
}

static constexpr skcms_Matrix3x3 gGamuts[] = {
    SkNamedGamut::kSRGB,
    SkNamedGamut::kAdobeRGB,
    SkNamedGamut::kDisplayP3,
    SkNamedGamut::kRec2020,
    SkNamedGamut::kXYZ,
};

static constexpr int kGamutCount = std::size(gGamuts);

const skcms_Matrix3x3& random_gamut(SkRandom* rand) {
    return gGamuts[rand->nextULessThan(kGamutCount)];
}

enum class ColorSpaceType {
    kNone,
    kSRGB,
    kSRGBLinear,
    kRGB,

    kLast = kRGB
};

static constexpr int kColorSpaceTypeCount = static_cast<int>(ColorSpaceType::kLast) + 1;

ColorSpaceType random_colorspacetype(SkRandom* rand) {
    return static_cast<ColorSpaceType>(rand->nextULessThan(kColorSpaceTypeCount));
}

sk_sp<SkColorSpace> random_colorspace(SkRandom* rand) {
    ColorSpaceType cs = random_colorspacetype(rand);

    switch (cs) {
        case ColorSpaceType::kNone:
            return nullptr;
        case ColorSpaceType::kSRGB:
            return SkColorSpace::MakeSRGB();
        case ColorSpaceType::kSRGBLinear:
            return SkColorSpace::MakeSRGBLinear();
        case ColorSpaceType::kRGB:
            return SkColorSpace::MakeRGB(random_xfer_function(rand), random_gamut(rand));
    }

    SkUNREACHABLE;
}

enum class ColorConstraint {
    kNone,
    kOpaque,
    kTransparent,
};

SkColor random_color(SkRandom* rand, ColorConstraint constraint) {
    uint32_t color = rand->nextU();

    switch (constraint) {
        case ColorConstraint::kNone:        return color;
        case ColorConstraint::kOpaque:      return 0xff000000 | color;
        case ColorConstraint::kTransparent: return 0x80000000 | color;
    }

    SkUNREACHABLE;
}

SkColor4f random_color4f(SkRandom* rand, ColorConstraint constraint) {
    SkColor4f result = { rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f) };

    switch (constraint) {
        case ColorConstraint::kNone:        return result;
        case ColorConstraint::kOpaque:      result.fA = 1.0f; return result;
        case ColorConstraint::kTransparent: result.fA = 0.5f; return result;
    }

    SkUNREACHABLE;
}

SkTileMode random_tilemode(SkRandom* rand) {
    return static_cast<SkTileMode>(rand->nextULessThan(kSkTileModeCount));
}

ShaderType random_shadertype(SkRandom* rand) {
    return static_cast<ShaderType>(rand->nextULessThan(kShaderTypeCount));
}

SkBlendMode random_porter_duff_bm(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextRangeU((unsigned int) SkBlendMode::kClear,
                                                     (unsigned int) SkBlendMode::kLastCoeffMode));
}

SkBlendMode random_complex_bm(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextRangeU((unsigned int) SkBlendMode::kLastCoeffMode,
                                                     (unsigned int) SkBlendMode::kLastMode));
}

SkBlendMode random_blend_mode(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextULessThan(kSkBlendModeCount));
}

BlenderType random_blendertype(SkRandom* rand) {
    return static_cast<BlenderType>(rand->nextULessThan(kBlenderTypeCount));
}

ColorFilterType random_colorfiltertype(SkRandom* rand) {
    return static_cast<ColorFilterType>(rand->nextULessThan(kColorFilterTypeCount));
}

ImageFilterType random_imagefiltertype(SkRandom* rand) {
    return static_cast<ImageFilterType>(rand->nextULessThan(kImageFilterTypeCount));
}

SkMatrix* random_local_matrix(SkRandom* rand, SkMatrix* storage) {
    switch (rand->nextULessThan(3)) {
        case 0:  return nullptr;
        case 1:  storage->setIdentity(); return storage;
        case 2:  [[fallthrough]];
        default: storage->setTranslate(2.0f, 2.0f); return storage;
    }

    SkUNREACHABLE;
}

sk_sp<SkImage> make_image(SkRandom* rand, Recorder* recorder) {
    SkColorType ct = SkColorType::kRGBA_8888_SkColorType;
    if (rand->nextBool()) {
        ct = SkColorType::kAlpha_8_SkColorType;
    }

    SkImageInfo info = SkImageInfo::Make(32, 32, ct, kPremul_SkAlphaType, random_colorspace(rand));

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorBLACK);

    sk_sp<SkImage> img = bitmap.asImage();

    // TODO: fuzz mipmappedness
    return SkImages::TextureFromImage(recorder, img, {false});
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPicture> make_picture(SkRandom* rand) {
    constexpr SkRect kRect = SkRect::MakeWH(128, 128);

    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(kRect);

    SkPaint paint; // Explicitly using the default SkPaint here

    canvas->drawRect(kRect, paint);

    return recorder.finishRecordingAsPicture();
}

//--------------------------------------------------------------------------------------------------
std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_coord_clamp_shader(SkRandom* rand,
                                                                              Recorder* recorder) {
    auto [s, o] = create_random_shader(rand, recorder);
    SkASSERT(!s == !o);

    if (!s) {
        return { nullptr, nullptr };
    }

    constexpr SkRect kSubset{0, 0, 256, 256}; // this is somewhat arbitrary but we need some subset
    sk_sp<SkShader> ccs = SkShaders::CoordClamp(std::move(s), kSubset);
    sk_sp<PrecompileShader> cco = PrecompileShaders::CoordClamp({ std::move(o) });

    return { ccs, cco };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_empty_shader(SkRandom* /* rand */) {
    sk_sp<SkShader> s = SkShaders::Empty();
    sk_sp<PrecompileShader> o = PrecompileShaders::Empty();

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_perlin_noise_shader(SkRandom* rand) {
    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    if (rand->nextBool()) {
        s = SkShaders::MakeFractalNoise(/* baseFrequencyX= */ 0.3f,
                                        /* baseFrequencyY= */ 0.3f,
                                        /* numOctaves= */ 2,
                                        /* seed= */ 4);
        o = PrecompileShaders::MakeFractalNoise();
    } else {
        s = SkShaders::MakeTurbulence(/* baseFrequencyX= */ 0.3f,
                                      /* baseFrequencyY= */ 0.3f,
                                      /* numOctaves= */ 2,
                                      /* seed= */ 4);
        o = PrecompileShaders::MakeTurbulence();
    }

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_picture_shader(SkRandom* rand) {
    sk_sp<SkPicture> picture = make_picture(rand);

    gNeedSKPPaintOption = true;

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    // TODO: can the clamp, filter mode, or tileRect affect the final program?
    sk_sp<SkShader> s = picture->makeShader(SkTileMode::kClamp,
                                            SkTileMode::kClamp,
                                            SkFilterMode::kLinear,
                                            lmPtr,
                                            /* tileRect= */ nullptr);
    sk_sp<PrecompileShader> o = PrecompileShadersPriv::Picture(SkToBool(lmPtr));

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_solid_shader(
        SkRandom* rand,
        ColorConstraint constraint = ColorConstraint::kNone) {
    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    if (rand->nextBool()) {
        s = SkShaders::Color(random_color(rand, constraint));
        o = PrecompileShaders::Color();
    } else {
        sk_sp<SkColorSpace> cs = random_colorspace(rand);
        s = SkShaders::Color(random_color4f(rand, constraint), cs);
        o = PrecompileShaders::Color(std::move(cs));
    }

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_gradient_shader(
        SkRandom* rand,
        SkShaderBase::GradientType type,
        ColorConstraint constraint = ColorConstraint::kOpaque) {
    // TODO: fuzz the gradient parameters - esp. the number of stops & hard stops
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {random_color(rand, constraint), random_color(rand, constraint)};
    SkScalar offsets[2] = {0.0f, 1.0f};

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    uint32_t flags = rand->nextBool() ? 0x0 : SkGradientShader::kInterpolateColorsInPremul_Flag;

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    SkTileMode tm = random_tilemode(rand);

    switch (type) {
        case SkShaderBase::GradientType::kLinear:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm, flags, lmPtr);
            o = PrecompileShadersPriv::LinearGradient(SkToBool(lmPtr));
            break;
        case SkShaderBase::GradientType::kRadial:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm, flags, lmPtr);
            o = PrecompileShadersPriv::RadialGradient(SkToBool(lmPtr));
            break;
        case SkShaderBase::GradientType::kSweep:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            /* startAngle= */ 0, /* endAngle= */ 359,
                                            flags, lmPtr);
            o = PrecompileShadersPriv::SweepGradient(SkToBool(lmPtr));
            break;
        case SkShaderBase::GradientType::kConical:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2,
                                                      tm, flags, lmPtr);
            o = PrecompileShadersPriv::TwoPointConicalGradient(SkToBool(lmPtr));
            break;
        case SkShaderBase::GradientType::kNone:
            SkDEBUGFAIL("Gradient shader says its type is none");
            break;
    }

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_localmatrix_shader(SkRandom* rand,
                                                                              Recorder* recorder) {
    auto [s, o] = create_random_shader(rand, recorder);
    SkASSERT(!s == !o);

    if (!s) {
        return { nullptr, nullptr };
    }

    SkMatrix lmStorage;
    random_local_matrix(rand, &lmStorage);

    return { s->makeWithLocalMatrix(lmStorage), o->makeWithLocalMatrix() };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_colorfilter_shader(SkRandom* rand,
                                                                              Recorder* recorder) {
    auto [s, o] = create_random_shader(rand, recorder);
    SkASSERT(!s == !o);

    if (!s) {
        return { nullptr, nullptr };
    }

    auto [cf, cfO] = create_random_colorfilter(rand);

    return { s->makeWithColorFilter(std::move(cf)), o->makeWithColorFilter(std::move(cfO)) };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_image_shader(SkRandom* rand,
                                                                        Recorder* recorder) {
    SkTileMode tmX = random_tilemode(rand);
    SkTileMode tmY = random_tilemode(rand);

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    // TODO: the combination system accounts for cubic vs. non-cubic sampling and HW vs. non-HW
    // tiling. We should test those combinations in the fuzzer.
    if (rand->nextBool()) {
        s = SkShaders::Image(make_image(rand, recorder),
                             tmX, tmY,
                             SkSamplingOptions(),
                             lmPtr);
        o = PrecompileShaders::Image();
    } else {
        s = SkShaders::RawImage(make_image(rand, recorder),
                                tmX, tmY,
                                SkSamplingOptions(),
                                lmPtr);
        o = PrecompileShaders::RawImage();
    }

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_blend_shader(SkRandom* rand,
                                                                        Recorder* recorder) {
    // TODO: add explicit testing of the kClear, kDst and kSrc blend modes since they short
    // circuit creation of a true blend shader (i.e., in SkShaders::Blend).
    auto [blender, blenderO] = create_random_blender(rand);

    auto [dstS, dstO] = create_random_shader(rand, recorder);
    SkASSERT(!dstS == !dstO);
    if (!dstS) {
        return { nullptr, nullptr };
    }

    auto [srcS, srcO] = create_random_shader(rand, recorder);
    SkASSERT(!srcS == !srcO);
    if (!srcS) {
        return { nullptr, nullptr };
    }

    auto s = SkShaders::Blend(std::move(blender), std::move(dstS), std::move(srcS));
    auto o = PrecompileShaders::Blend(SkSpan<const sk_sp<PrecompileBlender>>({ blenderO }),
                                      { dstO }, { srcO });

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_workingCS_shader(SkRandom* rand,
                                                                            Recorder* recorder) {
    auto [wrappedS, wrappedO] = create_random_shader(rand, recorder);
    SkASSERT(!wrappedS == !wrappedO);
    if (!wrappedS) {
        return { nullptr, nullptr };
    }

    sk_sp<SkColorSpace> cs = random_colorspace(rand);
    sk_sp<SkShader> s = wrappedS->makeWithWorkingColorSpace(cs);
    sk_sp<PrecompileShader> o = wrappedO->makeWithWorkingColorSpace(std::move(cs));

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>>  create_shader(SkRandom* rand,
                                                                   Recorder* recorder,
                                                                   ShaderType shaderType) {

    switch (shaderType) {
        case ShaderType::kNone:
            return { nullptr, nullptr };
        case ShaderType::kBlend:
            return create_blend_shader(rand, recorder);
        case ShaderType::kColorFilter:
            return create_colorfilter_shader(rand, recorder);
        case ShaderType::kCoordClamp:
            return create_coord_clamp_shader(rand, recorder);
        case ShaderType::kConicalGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kConical);
        case ShaderType::kEmpty:
            return create_empty_shader(rand);
        case ShaderType::kImage:
            return create_image_shader(rand, recorder);
        case ShaderType::kLinearGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kLinear);
        case ShaderType::kLocalMatrix:
            return create_localmatrix_shader(rand, recorder);
        case ShaderType::kPerlinNoise:
            return create_perlin_noise_shader(rand);
        case ShaderType::kPicture:
            return create_picture_shader(rand);
        case ShaderType::kRadialGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kRadial);
        case ShaderType::kSolidColor:
            return create_solid_shader(rand);
        case ShaderType::kSweepGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kSweep);
        case ShaderType::kWorkingColorSpace:
            return create_workingCS_shader(rand, recorder);
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom* rand,
                                                                         Recorder* recorder) {
    return create_shader(rand, recorder, random_shadertype(rand));
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_clip_shader(SkRandom* rand,
                                                                       Recorder* recorder) {
    // The clip shader has to be transparent to be at all interesting.
    // TODO/Note: an opaque clipShader is eliminated from the SkPaint by the normal Skia API
    // but I'm unsure if we should bother capturing that possibility in the precompile system.
    switch (rand->nextULessThan(5)) {
        case 0: return create_gradient_shader(rand, SkShaderBase::GradientType::kConical,
                                              ColorConstraint::kTransparent);
        case 1: return create_gradient_shader(rand, SkShaderBase::GradientType::kLinear,
                                              ColorConstraint::kTransparent);
        case 2: return create_gradient_shader(rand, SkShaderBase::GradientType::kRadial,
                                              ColorConstraint::kTransparent);
        case 3: return create_solid_shader(rand, ColorConstraint::kTransparent);
        case 4: return create_gradient_shader(rand, SkShaderBase::GradientType::kSweep,
                                              ColorConstraint::kTransparent);
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> src_blender() {
    static SkRuntimeEffect* sSrcEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForBlender,
            "half4 main(half4 src, half4 dst) {"
                "return src;"
            "}"
    );

    sk_sp<SkBlender> b = sSrcEffect->makeBlender(/* uniforms= */ nullptr);
    sk_sp<PrecompileBlender> o = MakePrecompileBlender(sk_ref_sp(sSrcEffect));
    return { b , o };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> dest_blender() {
    static SkRuntimeEffect* sDestEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForBlender,
            "half4 main(half4 src, half4 dst) {"
                "return dst;"
            "}"
    );

    sk_sp<SkBlender> b = sDestEffect->makeBlender(/* uniforms= */ nullptr);
    sk_sp<PrecompileBlender> o = MakePrecompileBlender(sk_ref_sp(sDestEffect));
    return { b , o };
}


std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> combo_blender() {
    static SkRuntimeEffect* sComboEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForBlender,
            "uniform float blendFrac;"
            "uniform blender a;"
            "uniform blender b;"
            "half4 main(half4 src, half4 dst) {"
                "return (blendFrac * a.eval(src, dst)) + ((1 - blendFrac) * b.eval(src, dst));"
            "}"
    );

    auto [src, srcO] = src_blender();
    auto [dst, dstO] = dest_blender();

    SkRuntimeEffect::ChildPtr children[] = { src, dst };
    const PrecompileChildPtr childOptions[] = { srcO, dstO };

    const float kUniforms[] = { 1.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
    sk_sp<SkBlender> b = sComboEffect->makeBlender(std::move(uniforms), children);
    sk_sp<PrecompileBlender> o = MakePrecompileBlender(sk_ref_sp(sComboEffect), { childOptions });
    return { b , o };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_bm_blender(SkRandom* rand,
                                                                        SkBlendMode bm) {
    return { SkBlender::Mode(bm), PrecompileBlender::Mode(bm) };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_arithmetic_blender() {
    sk_sp<SkBlender> b = SkBlenders::Arithmetic(/* k1= */ 0.5,
                                                /* k2= */ 0.5,
                                                /* k3= */ 0.5,
                                                /* k4= */ 0.5,
                                                /* enforcePremul= */ true);
    sk_sp<PrecompileBlender> o = PrecompileBlenders::Arithmetic();

    return { std::move(b), std::move(o) };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_rt_blender(SkRandom* rand) {
    int option = rand->nextULessThan(3);

    switch (option) {
        case 0: return src_blender();
        case 1: return dest_blender();
        case 2: return combo_blender();
    }

    return { nullptr, nullptr };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_blender(SkRandom* rand,
                                                                     BlenderType type) {
    switch (type) {
        case BlenderType::kNone:
            return { nullptr, nullptr };
        case BlenderType::kPorterDuff:
            return create_bm_blender(rand, random_porter_duff_bm(rand));
        case BlenderType::kShaderBased:
            return create_bm_blender(rand, random_complex_bm(rand));
        case BlenderType::kArithmetic:
            return create_arithmetic_blender();
        case BlenderType::kRuntime:
            return create_rt_blender(rand);
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom* rand) {
    return create_blender(rand, random_blendertype(rand));
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> double_colorfilter() {
    static SkRuntimeEffect* sSrcEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForColorFilter,
            "half4 main(half4 c) {"
                "return 2*c;"
            "}"
    );

    return { sSrcEffect->makeColorFilter(/* uniforms= */ nullptr),
             MakePrecompileColorFilter(sk_ref_sp(sSrcEffect)) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> half_colorfilter() {
    static SkRuntimeEffect* sDestEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForColorFilter,
            "half4 main(half4 c) {"
                "return 0.5*c;"
            "}"
    );

    return { sDestEffect->makeColorFilter(/* uniforms= */ nullptr),
             MakePrecompileColorFilter(sk_ref_sp(sDestEffect)) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> combo_colorfilter() {
    static SkRuntimeEffect* sComboEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForColorFilter,
            "uniform float blendFrac;"
            "uniform colorFilter a;"
            "uniform colorFilter b;"
            "half4 main(half4 c) {"
                "return (blendFrac * a.eval(c)) + ((1 - blendFrac) * b.eval(c));"
            "}"
    );

    auto [src, srcO] = double_colorfilter();
    auto [dst, dstO] = half_colorfilter();

    SkRuntimeEffect::ChildPtr children[] = { src, dst };
    const PrecompileChildPtr childOptions[] = { srcO, dstO };

    const float kUniforms[] = { 0.5f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
    sk_sp<SkColorFilter> cf = sComboEffect->makeColorFilter(std::move(uniforms), children);
    sk_sp<PrecompileColorFilter> o = MakePrecompileColorFilter(sk_ref_sp(sComboEffect),
                                                               { childOptions });
    return { std::move(cf) , std::move(o) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_rt_colorfilter(
        SkRandom* rand) {
    int option = rand->nextULessThan(3);

    switch (option) {
        case 0: return double_colorfilter();
        case 1: return half_colorfilter();
        case 2: return combo_colorfilter();
    }

    return { nullptr, nullptr };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_lerp_colorfilter(
        SkRandom* rand) {

    auto [dst, dstO] = create_random_colorfilter(rand);
    auto [src, srcO] = create_random_colorfilter(rand);
    // SkColorFilters::Lerp optimizes away the case where src == dst. I don't know if it is worth
    // capturing it in the precompilation API
    while (src == dst) {
        std::tie(src, srcO) = create_random_colorfilter(rand);
    }

    // TODO: SkColorFilters::Lerp will return a different colorFilter depending on the
    // weight value and the child color filters. I don't know if that is worth capturing
    // in the precompile API.
    sk_sp<SkColorFilter> cf = SkColorFilters::Lerp(0.5f, std::move(dst), std::move(src));

    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Lerp({ dstO }, { srcO });

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_lighting_colorfilter() {
    // TODO: the lighting color filter factory special cases when nothing is added and converts it
    // to a blendmode color filter
    return { SkColorFilters::Lighting(SK_ColorGREEN, SK_ColorRED),
             PrecompileColorFilters::Lighting() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_blendmode_colorfilter(
        SkRandom* rand) {

    sk_sp<SkColorFilter> cf;

    // SkColorFilters::Blend is clever and can weed out noop color filters. Loop until we get
    // a valid color filter.
    while (!cf) {
        cf = SkColorFilters::Blend(random_color4f(rand, ColorConstraint::kNone),
                                   random_colorspace(rand),
                                   random_blend_mode(rand));
    }

    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Blend();

    return { std::move(cf), std::move(o) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::Matrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Matrix();

    return { std::move(cf), std::move(o) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_color_space_colorfilter(
        SkRandom* rand) {
    return { SkColorFilterPriv::MakeColorSpaceXform(random_colorspace(rand),
                                                    random_colorspace(rand)),
             PrecompileColorFiltersPriv::ColorSpaceXform() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_linear_to_srgb_colorfilter() {
    return { SkColorFilters::LinearToSRGBGamma(), PrecompileColorFilters::LinearToSRGBGamma() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_srgb_to_linear_colorfilter() {
    return { SkColorFilters::SRGBToLinearGamma(), PrecompileColorFilters::SRGBToLinearGamma() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_luma_colorfilter() {
    return { SkLumaColorFilter::Make(), PrecompileColorFilters::Luma() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_compose_colorfilter(
        SkRandom* rand) {
    auto [outerCF, outerO] = create_random_colorfilter(rand);
    auto [innerCF, innerO] = create_random_colorfilter(rand);

    // TODO: if outerCF is null, innerCF will be returned by Compose. We need a Precompile
    // list object that can encapsulate innerO if there are no combinations in outerO.
    return { SkColorFilters::Compose(std::move(outerCF), std::move(innerCF)),
             PrecompileColorFilters::Compose({ std::move(outerO) }, { std::move(innerO) }) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_gaussian_colorfilter() {
    return { SkColorFilterPriv::MakeGaussian(), PrecompileColorFiltersPriv::Gaussian() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_table_colorfilter() {
    static constexpr uint8_t kTable[256] = { 0 };

    return { SkColorFilters::Table(kTable), PrecompileColorFilters::Table() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_workingformat_colorfilter(
        SkRandom* rand) {
    auto [childCF, childO] = create_random_colorfilter(rand);

    if (!childCF) {
        return { nullptr, nullptr };
    }

    SkASSERT(childCF && childO);

    SkAlphaType unpremul = kUnpremul_SkAlphaType;
    sk_sp<SkColorFilter> cf = SkColorFilterPriv::WithWorkingFormat(std::move(childCF),
                                                                   &random_xfer_function(rand),
                                                                   &random_gamut(rand),
                                                                   &unpremul);

    sk_sp<PrecompileColorFilter> o = PrecompileColorFiltersPriv::WithWorkingFormat(
            { std::move(childO) });

    return { std::move(cf), std::move(o) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_hsla_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::HSLAMatrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::HSLAMatrix();

    return { std::move(cf), std::move(o) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_colorfilter(
        SkRandom* rand,
        ColorFilterType type) {

    switch (type) {
        case ColorFilterType::kNone:
            return { nullptr, nullptr };
        case ColorFilterType::kBlendMode:
            return create_blendmode_colorfilter(rand);
        case ColorFilterType::kColorSpaceXform:
            return create_color_space_colorfilter(rand);
        case ColorFilterType::kCompose:
            return create_compose_colorfilter(rand);
        case ColorFilterType::kGaussian:
            return create_gaussian_colorfilter();
        case ColorFilterType::kHSLAMatrix:
            return create_hsla_matrix_colorfilter();
        case ColorFilterType::kLerp:
            return create_lerp_colorfilter(rand);
        case ColorFilterType::kLighting:
            return create_lighting_colorfilter();
        case ColorFilterType::kLinearToSRGB:
            return create_linear_to_srgb_colorfilter();
        case ColorFilterType::kLuma:
            return create_luma_colorfilter();
        case ColorFilterType::kMatrix:
            return create_matrix_colorfilter();
        case ColorFilterType::kRuntime:
            return create_rt_colorfilter(rand);
        case ColorFilterType::kSRGBToLinear:
            return create_srgb_to_linear_colorfilter();
        case ColorFilterType::kTable:
            return create_table_colorfilter();
        case ColorFilterType::kWorkingFormat:
            return create_workingformat_colorfilter(rand);
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(
        SkRandom* rand) {
    return create_colorfilter(rand, random_colorfiltertype(rand));
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
sk_sp<SkImageFilter> blur_imagefilter(SkRandom* rand,
                                      SkEnumBitMask<PrecompileImageFilters>* imageFilterMask) {

    int option = rand->nextULessThan(3);

    float sigma;
    switch (option) {
        case 0:  sigma = 1.0f; break;  // 1DBlur4
        case 1:  sigma = 2.0f; break;  // 1DBlur8
        case 2:  [[fallthrough]];
        default: sigma = 5.0f; break;  // 1DBlur16
    }

    sk_sp<SkImageFilter> blurIF = SkImageFilters::Blur(sigma, sigma, /* input= */ nullptr);
    *imageFilterMask |= PrecompileImageFilters::kBlur;

    return blurIF;
}

std::pair<sk_sp<SkImageFilter>, SkEnumBitMask<PrecompileImageFilters>> create_image_filter(
        SkRandom* rand,
        ImageFilterType type) {

    sk_sp<SkImageFilter> imgFilter;
    SkEnumBitMask<PrecompileImageFilters> imageFilterMask = PrecompileImageFilters::kNone;

    switch (type) {
        case ImageFilterType::kNone:
            break;
        case ImageFilterType::kBlur:
            imgFilter = blur_imagefilter(rand, &imageFilterMask);
    }

    return { std::move(imgFilter), imageFilterMask };
}

std::pair<sk_sp<SkImageFilter>, SkEnumBitMask<PrecompileImageFilters>>
                                                        create_random_image_filter(SkRandom* rand) {
    return create_image_filter(rand, random_imagefiltertype(rand));
}

//--------------------------------------------------------------------------------------------------
std::pair<SkPaint, PaintOptions> create_paint(SkRandom* rand,
                                              Recorder* recorder,
                                              ShaderType shaderType,
                                              BlenderType blenderType,
                                              ColorFilterType colorFilterType,
                                              ImageFilterType imageFilterType) {
    SkPaint paint;
    paint.setColor(random_color(rand, ColorConstraint::kOpaque));

    PaintOptions paintOptions;

    {
        auto [s, o] = create_shader(rand, recorder, shaderType);
        SkASSERT(!s == !o);

        if (s) {
            paint.setShader(std::move(s));
            paintOptions.setShaders({o});
        }
    }

    {
        auto [cf, o] = create_colorfilter(rand, colorFilterType);
        SkASSERT(!cf == !o);

        if (cf) {
            paint.setColorFilter(std::move(cf));
            paintOptions.setColorFilters({o});
        }
    }

    {
        auto [b, o] = create_blender(rand, blenderType);
        SkASSERT(!b == !o);

        if (b) {
            paint.setBlender(std::move(b));
            paintOptions.setBlenders({o});
        }
    }

    {
        auto [filter, filterO] = create_image_filter(rand, imageFilterType);
        SkASSERT(!filter == !filterO);

        if (filter) {
            paint.setImageFilter(std::move(filter));
            paintOptions.setImageFilters(filterO);
        }
    }

    if (rand->nextBool()) {
        paint.setDither(true);
        paintOptions.setDither(true);
    }

    return { paint, paintOptions };
}

#ifdef SK_DEBUG
void dump(ShaderCodeDictionary* dict, UniquePaintParamsID id) {
    dict->lookup(id).dump(dict);
}
#endif

SkPath make_path() {
    SkPathBuilder path;
    path.moveTo(0, 0);
    path.lineTo(8, 2);
    path.lineTo(16, 0);
    path.lineTo(14, 8);
    path.lineTo(16, 16);
    path.lineTo(8, 14);
    path.lineTo(0, 16);
    path.lineTo(2, 8);
    path.close();
    return path.detach();
}

struct DrawData {
    SkPath fPath;
    sk_sp<SkTextBlob> fBlob;
    sk_sp<SkVertices> fVertsWithColors;
    sk_sp<SkVertices> fVertsWithOutColors;
};

void simple_draws(SkCanvas* canvas, const SkPaint& paint) {
    // TODO: add some drawLine calls
    canvas->drawRect(SkRect::MakeWH(16, 16), paint);
    canvas->drawRRect(SkRRect::MakeOval({0, 0, 16, 16}), paint);
    canvas->drawRRect(SkRRect::MakeRectXY({0, 0, 16, 16}, 4, 4), paint);

    if (!paint.getShader() &&
        !paint.getColorFilter() &&
        !paint.getImageFilter() &&
        paint.asBlendMode().has_value()) {
        // The SkPaint reconstructed inside the drawEdgeAAQuad call needs to match 'paint' for
        // the precompilation checks to work.
        canvas->experimental_DrawEdgeAAQuad(SkRect::MakeWH(16, 16),
                                            /* clip= */ nullptr,
                                            SkCanvas::kAll_QuadAAFlags,
                                            paint.getColor4f(),
                                            paint.asBlendMode().value());
    }
}

void non_simple_draws(SkCanvas* canvas, const SkPaint& paint, const DrawData& drawData) {
    // TODO: add strokeAndFill draws here as well as a stroked non-circular rrect draw
    canvas->drawPath(drawData.fPath, paint);
}

void check_draw(skiatest::Reporter* reporter,
                Context* context,
                skiatest::graphite::GraphiteTestContext* testContext,
                Recorder* recorder,
                const SkPaint& paint,
                DrawTypeFlags dt,
                ClipType clip, sk_sp<SkShader> clipShader,
                const DrawData& drawData) {

    int before = context->priv().globalCache()->numGraphicsPipelines();

#ifdef SK_DEBUG
    std::vector<skgpu::UniqueKey> beforeKeys;

    UniqueKeyUtils::FetchUniqueKeys(context->priv().globalCache(), &beforeKeys);
#endif

    {
        // TODO: vary the colorType of the target surface too
        SkImageInfo ii = SkImageInfo::Make(16, 16,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder, ii);
        SkCanvas* canvas = surf->getCanvas();

        switch (clip) {
            case ClipType::kNone:
                break;
            case ClipType::kShader:
                SkASSERT(clipShader);
                canvas->clipShader(clipShader, SkClipOp::kIntersect);
                break;
            case ClipType::kShader_Diff:
                SkASSERT(clipShader);
                canvas->clipShader(clipShader, SkClipOp::kDifference);
                break;
        }

        switch (dt) {
            case DrawTypeFlags::kSimpleShape:
                simple_draws(canvas, paint);
                break;
            case DrawTypeFlags::kNonSimpleShape:
                non_simple_draws(canvas, paint, drawData);
                break;
            case DrawTypeFlags::kShape:
                simple_draws(canvas, paint);
                non_simple_draws(canvas, paint, drawData);
                break;
            case DrawTypeFlags::kText:
                canvas->drawTextBlob(drawData.fBlob, 0, 16, paint);
                break;
            case DrawTypeFlags::kDrawVertices:
                canvas->drawVertices(drawData.fVertsWithColors, SkBlendMode::kDst, paint);
                canvas->drawVertices(drawData.fVertsWithOutColors, SkBlendMode::kDst, paint);
                break;
            default:
                SkASSERT(false);
                break;
        }

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
        testContext->syncedSubmit(context);
    }

    int after = context->priv().globalCache()->numGraphicsPipelines();

    // Actually using the SkPaint with the specified type of draw shouldn't have caused
    // any additional compilation
    REPORTER_ASSERT(reporter, before == after, "before: %d after: %d", before, after);
#ifdef SK_DEBUG
    if (before != after) {
        const RendererProvider* rendererProvider = context->priv().rendererProvider();
        const ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> afterKeys;

        UniqueKeyUtils::FetchUniqueKeys(context->priv().globalCache(), &afterKeys);

        for (const skgpu::UniqueKey& afterKey : afterKeys) {
            if (std::find(beforeKeys.begin(), beforeKeys.end(), afterKey) == beforeKeys.end()) {
                GraphicsPipelineDesc originalPipelineDesc;
                RenderPassDesc originalRenderPassDesc;
                UniqueKeyUtils::ExtractKeyDescs(context, afterKey,
                                                &originalPipelineDesc,
                                                &originalRenderPassDesc);

                SkDebugf("------- New key from draw:\n");
                afterKey.dump("original key:");
                UniqueKeyUtils::DumpDescs(rendererProvider, dict,
                                          originalPipelineDesc,
                                          originalRenderPassDesc);
            }
        }
    }
#endif // SK_DEBUG
}

} // anonymous namespace

void run_test(skiatest::Reporter*,
              Context*,
              skiatest::graphite::GraphiteTestContext*,
              const KeyContext& precompileKeyContext,
              const DrawData&,
              ShaderType,
              BlenderType,
              ColorFilterType,
              ClipType,
              ImageFilterType);

// This is intended to be a smoke test for the agreement between the two ways of creating a
// PaintParamsKey:
//    via ExtractPaintData (i.e., from an SkPaint)
//    and via the pre-compilation system
//
// TODO: keep this as a smoke test but add a fuzzer that reuses all the helpers
// TODO(b/306174708): enable in SkQP (if it's feasible)
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PaintParamsKeyTest,
                                               reporter,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kNever) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    SkColorInfo destColorInfo = SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                            SkColorSpace::MakeSRGB());

    std::unique_ptr<RuntimeEffectDictionary> rtDict = std::make_unique<RuntimeEffectDictionary>();

    auto dstTexInfo = context->priv().caps()->getDefaultSampledTextureInfo(
            kRGBA_8888_SkColorType,
            skgpu::Mipmapped::kNo,
            skgpu::Protected::kNo,
            skgpu::Renderable::kNo);
    // Use Budgeted::kYes to avoid instantiating the proxy immediately; this test doesn't need
    // a full resource.
    sk_sp<TextureProxy> fakeDstTexture = TextureProxy::Make(context->priv().caps(),
                                                            context->priv().resourceProvider(),
                                                            SkISize::Make(1, 1),
                                                            dstTexInfo,
                                                            "PaintParamsKeyTestFakeDstTexture",
                                                            skgpu::Budgeted::kYes);
    constexpr SkIPoint kFakeDstOffset = SkIPoint::Make(0, 0);

    KeyContext precompileKeyContext(context->priv().caps(),
                                    dict,
                                    rtDict.get(),
                                    destColorInfo,
                                    fakeDstTexture,
                                    kFakeDstOffset);

    SkFont font(ToolUtils::DefaultPortableTypeface(), 16);
    const char text[] = "hambur";

    constexpr int kNumVerts = 4;
    constexpr SkPoint kPositions[kNumVerts] { {0,0}, {0,16}, {16,16}, {16,0} };
    constexpr SkColor kColors[kNumVerts] = { SK_ColorBLUE, SK_ColorGREEN,
                                             SK_ColorCYAN, SK_ColorYELLOW };

    DrawData drawData = {
            make_path(),
            SkTextBlob::MakeFromText(text, strlen(text), font),
            SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, kNumVerts,
                                 kPositions, kPositions, kColors),
            SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, kNumVerts,
                                 kPositions, kPositions, /* colors= */ nullptr),
    };

    ShaderType shaders[] = {
            ShaderType::kBlend,
            ShaderType::kImage,
            ShaderType::kRadialGradient,
            ShaderType::kSolidColor,
#if EXPANDED_SET
            ShaderType::kNone,
            ShaderType::kColorFilter,
            ShaderType::kCoordClamp,
            ShaderType::kConicalGradient,
            ShaderType::kEmpty,
            ShaderType::kLinearGradient,
            ShaderType::kLocalMatrix,
            ShaderType::kPerlinNoise,
            ShaderType::kPicture,
            ShaderType::kSweepGradient,
            ShaderType::kWorkingColorSpace,
#endif
    };

    BlenderType blenders[] = {
            BlenderType::kPorterDuff,
            BlenderType::kShaderBased,
            BlenderType::kRuntime,
#if EXPANDED_SET
            BlenderType::kNone,
            BlenderType::kArithmetic,
#endif
    };

    ColorFilterType colorFilters[] = {
            ColorFilterType::kNone,
            ColorFilterType::kBlendMode,
            ColorFilterType::kMatrix,
#if EXPANDED_SET
            ColorFilterType::kColorSpaceXform,
            ColorFilterType::kCompose,
            ColorFilterType::kGaussian,
            ColorFilterType::kHSLAMatrix,
            ColorFilterType::kLerp,
            ColorFilterType::kLighting,
            ColorFilterType::kLinearToSRGB,
            ColorFilterType::kLuma,
            ColorFilterType::kRuntime,
            ColorFilterType::kSRGBToLinear,
            ColorFilterType::kTable,
            ColorFilterType::kWorkingFormat,
#endif
    };

    ClipType clips[] = {
            ClipType::kNone,
#if EXPANDED_SET
            ClipType::kShader,        // w/ a SkClipOp::kIntersect
            ClipType::kShader_Diff,   // w/ a SkClipOp::kDifference
#endif
    };

    ImageFilterType imageFilters[] = {
            ImageFilterType::kNone,
#if EXPANDED_SET
            ImageFilterType::kBlur,
#endif
    };

#if EXPANDED_SET
    size_t kExpected = std::size(shaders) * std::size(blenders) * std::size(colorFilters) *
                       std::size(clips) * std::size(imageFilters);
    int current = 0;
#endif

    for (auto shader : shaders) {
        for (auto blender : blenders) {
            for (auto cf : colorFilters) {
                for (auto clip : clips) {
                    for (auto imageFilter : imageFilters) {
#if EXPANDED_SET
                        SkDebugf("%d/%zu\n", current, kExpected);
                        ++current;
#endif
                        run_test(reporter, context, testContext, precompileKeyContext, drawData,
                                 shader, blender, cf, clip, imageFilter);
                    }
                }
            }
        }
    }

#if EXPANDED_SET
    SkASSERT(current == (int) kExpected);
#endif
}

void run_test(skiatest::Reporter* reporter,
              Context* context,
              skiatest::graphite::GraphiteTestContext* testContext,
              const KeyContext& precompileKeyContext,
              const DrawData& drawData,
              ShaderType s,
              BlenderType bm,
              ColorFilterType cf,
              ClipType clip,
              ImageFilterType imageFilter) {
    SkRandom rand;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    sk_sp<SkShader> clipShader;
    sk_sp<PrecompileShader> clipShaderOption;

    if (clip == ClipType::kShader || clip == ClipType::kShader_Diff) {
        std::tie(clipShader, clipShaderOption) = create_clip_shader(&rand, recorder.get());
        SkASSERT(!clipShader == !clipShaderOption);
    }

    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer paramsGatherer(recorder->priv().caps(), Layout::kMetal);
    PipelineDataGatherer precompileGatherer(recorder->priv().caps(), Layout::kMetal);

    gNeedSKPPaintOption = false;
    auto [paint, paintOptions] = create_paint(&rand, recorder.get(), s, bm, cf, imageFilter);

    for (DrawTypeFlags dt : { DrawTypeFlags::kSimpleShape,
                              DrawTypeFlags::kNonSimpleShape,
                              DrawTypeFlags::kShape,
                              DrawTypeFlags::kText,
                              DrawTypeFlags::kDrawVertices }) {

        // Note: 'withPrimitiveBlender' and 'primitiveBlender' are only used in ExtractPaintData
        // and PaintOptions::buildCombinations. Thus, as long as those two uses agree, it doesn't
        // matter if the actual draw uses a primitive blender (i.e., those variables are only used
        // in a local unit test independent of the follow-on Precompile/check_draw test)
        for (bool withPrimitiveBlender : { false, true }) {

            sk_sp<SkBlender> primitiveBlender;
            if (withPrimitiveBlender) {
                if (dt != DrawTypeFlags::kDrawVertices) {
                    // Only drawVertices calls need a primitive blender
                    continue;
                }

                primitiveBlender = SkBlender::Mode(SkBlendMode::kSrcOver);
            }

            constexpr Coverage coverageOptions[3] = {
                    Coverage::kNone, Coverage::kSingleChannel, Coverage::kLCD};
            Coverage coverage = coverageOptions[rand.nextULessThan(3)];

            DstReadRequirement dstReadReq = DstReadRequirement::kNone;
            const SkBlenderBase* blender = as_BB(paint.getBlender());
            if (blender) {
                dstReadReq = GetDstReadRequirement(recorder->priv().caps(),
                                                   blender->asBlendMode(),
                                                   coverage);
            }
            bool needsDstSample = dstReadReq == DstReadRequirement::kTextureCopy ||
                                  dstReadReq == DstReadRequirement::kTextureSample;
            sk_sp<TextureProxy> curDst = needsDstSample ? precompileKeyContext.dstTexture()
                                                        : nullptr;

            // In the normal API this modification happens in SkDevice::clipShader()
            // All clipShaders get wrapped in a CTMShader
            sk_sp<SkShader> modifiedClipShader = clipShader
                                                   ? as_SB(clipShader)->makeWithCTM(SkMatrix::I())
                                                   : nullptr;
            if (clip == ClipType::kShader_Diff && modifiedClipShader) {
                // The CTMShader gets further wrapped in a ColorFilterShader for kDifference clips
                modifiedClipShader = modifiedClipShader->makeWithColorFilter(
                        SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrcOut));
            }

            auto [paintID, uData, tData] =
                    ExtractPaintData(recorder.get(),
                                     &paramsGatherer,
                                     &builder,
                                     Layout::kMetal,
                                     {},
                                     PaintParams(paint,
                                                 primitiveBlender,
                                                 std::move(modifiedClipShader),
                                                 dstReadReq,
                                                 /* skipColorXform= */ false),
                                     {},
                                     curDst,
                                     precompileKeyContext.dstOffset(),
                                     precompileKeyContext.dstColorInfo());

            paintOptions.setClipShaders({ clipShaderOption });

            std::vector<UniquePaintParamsID> precompileIDs;
            paintOptions.priv().buildCombinations(precompileKeyContext,
                                                  &precompileGatherer,
                                                  DrawTypeFlags::kNone,
                                                  withPrimitiveBlender,
                                                  coverage,
                                                  [&precompileIDs](UniquePaintParamsID id,
                                                                   DrawTypeFlags,
                                                                   bool /* withPrimitiveBlender */,
                                                                   Coverage) {
                                                      precompileIDs.push_back(id);
                                                  });

            // Although we've gathered both sets of uniforms (i.e., from the paint
            // params and the precompilation paths) we can't compare the two since the
            // precompilation path may have generated multiple sets
            // and the last one created may not be the one that matches the paint
            // params' set. Additionally, for runtime effects we just skip gathering
            // the uniforms in the precompilation path.

            // The specific key generated by ExtractPaintData should be one of the
            // combinations generated by the combination system.
            auto result = std::find(precompileIDs.begin(), precompileIDs.end(), paintID);

            if (result == precompileIDs.end()) {
                SkDebugf("Failure on case: %s %s %s %s %s\n",
                         to_str(s), to_str(bm), to_str(cf), to_str(clip), to_str(imageFilter));
            }

#ifdef SK_DEBUG
            if (result == precompileIDs.end()) {
                SkDebugf("From paint: ");
                dump(dict, paintID);

                SkDebugf("From combination builder [%d]:", static_cast<int>(precompileIDs.size()));
                for (auto iter : precompileIDs) {
                    dump(dict, iter);
                }
            }
#endif

            REPORTER_ASSERT(reporter, result != precompileIDs.end());

            {
                context->priv().globalCache()->resetGraphicsPipelines();

                int before = context->priv().globalCache()->numGraphicsPipelines();
                Precompile(context, paintOptions, dt);
                if (gNeedSKPPaintOption) {
                    // The skp draws a rect w/ a default SkPaint
                    PaintOptions skpPaintOptions;
                    Precompile(context, skpPaintOptions, DrawTypeFlags::kSimpleShape);
                }
                int after = context->priv().globalCache()->numGraphicsPipelines();

                REPORTER_ASSERT(reporter, before == 0);
                REPORTER_ASSERT(reporter, after > before);

                check_draw(reporter,
                           context,
                           testContext,
                           recorder.get(),
                           paint,
                           dt,
                           clip, clipShader,
                           drawData);
            }
        }
    }
}

#endif // SK_GRAPHITE

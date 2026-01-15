/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/precompile/PaintParamsTestUtils.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/effects/SkBlenders.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradient.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"
#include "include/gpu/graphite/precompile/PrecompileMaskFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/precompile/PrecompileColorFiltersPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/graphite/precompile/PrecompileEffectFactories.h"

using namespace skgpu::graphite;
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

namespace skiatest::graphite {

//--------------------------------------------------------------------------------------------------
// String Converters
//--------------------------------------------------------------------------------------------------

const char* ToStr(ShaderType s) {
    switch (s) {
#define M(type)               \
    case ShaderType::k##type: \
        return "ShaderType::k" #type;
        SK_ALL_TEST_SHADERS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* ToStr(MaskFilterType mf) {
    switch (mf) {
#define M(type)                   \
    case MaskFilterType::k##type: \
        return "MaskFilterType::k" #type;
        SK_ALL_TEST_MASKFILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* ToStr(BlenderType b) {
    switch (b) {
#define M(type)                \
    case BlenderType::k##type: \
        return "BlenderType::k" #type;
        SK_ALL_TEST_BLENDERS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* ToStr(ColorFilterType cf) {
    switch (cf) {
#define M(type)                    \
    case ColorFilterType::k##type: \
        return "ColorFilterType::k" #type;
        SK_ALL_TEST_COLORFILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* ToStr(ClipType c) {
    switch (c) {
#define M(type)             \
    case ClipType::k##type: \
        return "ClipType::k" #type;
        SK_ALL_TEST_CLIPS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* ToStr(ImageFilterType c) {
    switch (c) {
#define M(type)                    \
    case ImageFilterType::k##type: \
        return "ImageFilterType::k" #type;
        SK_ALL_TEST_IMAGE_FILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* ToStr(DrawTypeFlags dt) {
    // Note: This logic assumes single-bit flags for simple string conversion,
    // but the flags can be combined.
    // We mask out modifiers (like kAnalyticClip) for the base check.
    DrawTypeFlags base = static_cast<DrawTypeFlags>(dt & ~DrawTypeFlags::kAnalyticClip);

    switch (base) {
        case DrawTypeFlags::kBitmapText_Mask:
            return "DrawTypeFlags::kBitmapText_Mask";
        case DrawTypeFlags::kBitmapText_LCD:
            return "DrawTypeFlags::kBitmapText_LCD";
        case DrawTypeFlags::kBitmapText_Color:
            return "DrawTypeFlags::kBitmapText_Color";
        case DrawTypeFlags::kSDFText:
            return "DrawTypeFlags::kSDFText";
        case DrawTypeFlags::kSDFText_LCD:
            return "DrawTypeFlags::kSDFText_LCD";
        case DrawTypeFlags::kDrawVertices:
            return "DrawTypeFlags::kDrawVertices";
        case DrawTypeFlags::kCircularArc:
            return "DrawTypeFlags::kCircularArc";
        case DrawTypeFlags::kAnalyticRRect:
            return "DrawTypeFlags::kAnalyticRRect";
        case DrawTypeFlags::kPerEdgeAAQuad:
            return "DrawTypeFlags::kPerEdgeAAQuad";
        case DrawTypeFlags::kNonAAFillRect:
            return "DrawTypeFlags::kNonAAFillRect";
        case DrawTypeFlags::kNonSimpleShape:
            return "DrawTypeFlags::kNonSimpleShape";
        default:
            break;
    }

    if (dt == DrawTypeFlags::kNone) return "DrawTypeFlags::kNone";

    return "DrawTypeFlags::kMultiple/Combo";
}

//--------------------------------------------------------------------------------------------------
// Internal Helpers (Static)
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

static const skcms_TransferFunction& random_xfer_function(SkRandom* rand) {
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

static const skcms_Matrix3x3& random_gamut(SkRandom* rand) {
    return gGamuts[rand->nextULessThan(kGamutCount)];
}

enum class ColorSpaceType { kNone, kSRGB, kSRGBLinear, kSRGBSpin, kRGB, kLast = kRGB };

static constexpr int kColorSpaceTypeCount = static_cast<int>(ColorSpaceType::kLast) + 1;

static ColorSpaceType random_colorspacetype(SkRandom* rand) {
    return static_cast<ColorSpaceType>(rand->nextULessThan(kColorSpaceTypeCount));
}

static sk_sp<SkColorSpace> random_colorspace(SkRandom* rand) {
    ColorSpaceType cs = random_colorspacetype(rand);

    switch (cs) {
        case ColorSpaceType::kNone:
            return nullptr;
        case ColorSpaceType::kSRGB:
            return SkColorSpace::MakeSRGB();
        case ColorSpaceType::kSRGBLinear:
            return SkColorSpace::MakeSRGBLinear();
        case ColorSpaceType::kSRGBSpin:
            return SkColorSpace::MakeSRGB()->makeColorSpin();
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

static SkColor random_color(SkRandom* rand, ColorConstraint constraint) {
    uint32_t color = rand->nextU();
    switch (constraint) {
        case ColorConstraint::kNone:
            return color;
        case ColorConstraint::kOpaque:
            return 0xff000000 | color;
        case ColorConstraint::kTransparent:
            return 0x80000000 | color;
    }
    SkUNREACHABLE;
}

static SkColor4f random_color4f(SkRandom* rand, ColorConstraint constraint) {
    SkColor4f result = {rand->nextRangeF(0.0f, 1.0f),
                        rand->nextRangeF(0.0f, 1.0f),
                        rand->nextRangeF(0.0f, 1.0f),
                        rand->nextRangeF(0.0f, 1.0f)};
    switch (constraint) {
        case ColorConstraint::kNone:
            return result;
        case ColorConstraint::kOpaque:
            result.fA = 1.0f;
            return result;
        case ColorConstraint::kTransparent:
            result.fA = 0.5f;
            return result;
    }
    SkUNREACHABLE;
}

static SkTileMode random_tilemode(SkRandom* rand) {
    return static_cast<SkTileMode>(rand->nextULessThan(kSkTileModeCount));
}

static ShaderType random_shadertype(SkRandom* rand) {
    // defined in PaintParamsTestUtils.h, assumes contiguous enum ending at kLast
    return static_cast<ShaderType>(rand->nextULessThan(static_cast<int>(ShaderType::kLast) + 1));
}

static SkBlendMode random_porter_duff_bm(SkRandom* rand) {
    return static_cast<SkBlendMode>(
            rand->nextRangeU((unsigned int)SkBlendMode::kClear, (unsigned int)SkBlendMode::kPlus));
}

static SkBlendMode random_complex_bm(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextRangeU((unsigned int)SkBlendMode::kPlus,
                                                     (unsigned int)SkBlendMode::kLastMode));
}

static SkBlendMode random_blend_mode(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextULessThan(kSkBlendModeCount));
}

static BlenderType random_blendertype(SkRandom* rand) {
    return static_cast<BlenderType>(rand->nextULessThan(static_cast<int>(BlenderType::kLast) + 1));
}

static ColorFilterType random_colorfiltertype(SkRandom* rand) {
    return static_cast<ColorFilterType>(
            rand->nextULessThan(static_cast<int>(ColorFilterType::kLast) + 1));
}

[[maybe_unused]] static ImageFilterType random_imagefiltertype(SkRandom* rand) {
    return static_cast<ImageFilterType>(
            rand->nextULessThan(static_cast<int>(ImageFilterType::kLast) + 1));
}

DrawTypeFlags RandomDrawType(SkRandom* rand) {
    uint32_t index = rand->nextULessThan(11);

    switch (index) {
        case 0:
            return DrawTypeFlags::kBitmapText_Mask;
        case 1:
            return DrawTypeFlags::kBitmapText_LCD;
        case 2:
            return DrawTypeFlags::kBitmapText_Color;
        case 3:
            return DrawTypeFlags::kSDFText;
        case 4:
            return DrawTypeFlags::kSDFText_LCD;
        case 5:
            return DrawTypeFlags::kDrawVertices;
        case 6:
            return DrawTypeFlags::kCircularArc;
        case 7:
            return DrawTypeFlags::kAnalyticRRect;
        case 8:
            return DrawTypeFlags::kPerEdgeAAQuad;
        case 9:
            return DrawTypeFlags::kNonAAFillRect;
        case 10:
            return DrawTypeFlags::kNonSimpleShape;
    }

    SkASSERT(0);
    return DrawTypeFlags::kNone;
}

enum LocalMatrixConstraint {
    kNone,
    kWithPerspective,
};

static SkMatrix* random_local_matrix(
        SkRandom* rand,
        SkMatrix* storage,
        LocalMatrixConstraint constraint = LocalMatrixConstraint::kNone) {
    uint32_t matrix = rand->nextULessThan(constraint == LocalMatrixConstraint::kNone ? 4 : 3);
    switch (matrix) {
        case 0:
            storage->setTranslate(2.0f, 2.0f);
            break;
        case 1:
            storage->setIdentity();
            break;
        case 2:
            storage->setScale(0.25f, 0.25f, 0.0f, 0.0f);
            break;
        case 3:
            return nullptr;
    }
    if (constraint == LocalMatrixConstraint::kWithPerspective) {
        storage->setPerspX(0.5f);
    }
    return storage;
}

static sk_sp<SkImage> make_image(SkRandom* rand, Recorder* recorder) {
    SkColorType ct = SkColorType::kRGBA_8888_SkColorType;
    if (rand->nextBool()) {
        ct = SkColorType::kAlpha_8_SkColorType;
    }
    SkImageInfo info = SkImageInfo::Make(32, 32, ct, kPremul_SkAlphaType, random_colorspace(rand));
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorBLACK);
    sk_sp<SkImage> img = bitmap.asImage();
    return SkImages::TextureFromImage(recorder, img, {false});
}

static sk_sp<SkImage> make_yuv_image(SkRandom* rand, Recorder* recorder) {
    SkYUVAInfo::PlaneConfig planeConfig = SkYUVAInfo::PlaneConfig::kY_UV;
    if (rand->nextBool()) {
        planeConfig = SkYUVAInfo::PlaneConfig::kY_U_V_A;
    }
    SkYUVAInfo yuvaInfo(
            {
                    32,
                    32,
            },
            planeConfig,
            SkYUVAInfo::Subsampling::k420,
            kJPEG_Full_SkYUVColorSpace);
    SkYUVAPixmapInfo pmInfo(yuvaInfo, SkYUVAPixmapInfo::DataType::kUnorm8, nullptr);
    SkYUVAPixmaps pixmaps = SkYUVAPixmaps::Allocate(pmInfo);
    for (int i = 0; i < pixmaps.numPlanes(); ++i) {
        pixmaps.plane(i).erase(SK_ColorBLACK);
    }
    sk_sp<SkColorSpace> cs;
    if (rand->nextBool()) {
        cs = SkColorSpace::MakeSRGBLinear();
    }
    return SkImages::TextureFromYUVAPixmaps(recorder,
                                            pixmaps,
                                            {/* fMipmapped= */ false},
                                            /* limitToMaxTextureSize= */ false,
                                            std::move(cs));
}

static sk_sp<SkPicture> make_picture(SkRandom* rand) {
    constexpr SkRect kRect = SkRect::MakeWH(128, 128);
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(kRect);
    SkPaint paint;
    canvas->drawRect(kRect, paint);
    return recorder.finishRecordingAsPicture();
}

//--------------------------------------------------------------------------------------------------
// Shader Generators
//--------------------------------------------------------------------------------------------------

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_coord_clamp_shader(SkRandom* rand,
                                                                              Recorder* recorder,
                                                                              bool* reqSKPOption) {
    auto [s, o] = CreateRandomShader(rand, recorder, random_shadertype(rand), reqSKPOption);
    SkASSERT(!s == !o);
    if (!s) {
        return {nullptr, nullptr};
    }
    constexpr SkRect kSubset{0, 0, 256, 256};
    sk_sp<SkShader> ccs = SkShaders::CoordClamp(std::move(s), kSubset);
    sk_sp<PrecompileShader> cco = PrecompileShaders::CoordClamp({std::move(o)});
    return {ccs, cco};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_perlin_noise_shader(SkRandom* rand) {
    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;
    if (rand->nextBool()) {
        s = SkShaders::MakeFractalNoise(0.3f, 0.3f, 2, 4);
        o = PrecompileShaders::MakeFractalNoise();
    } else {
        s = SkShaders::MakeTurbulence(0.3f, 0.3f, 2, 4);
        o = PrecompileShaders::MakeTurbulence();
    }
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_picture_shader(SkRandom* rand,
                                                                          bool* reqSKPOption) {
    sk_sp<SkPicture> picture = make_picture(rand);

    if (reqSKPOption) {
        *reqSKPOption = true;
    }

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    sk_sp<SkShader> s = picture->makeShader(
            SkTileMode::kClamp, SkTileMode::kClamp, SkFilterMode::kLinear, lmPtr, nullptr);
    sk_sp<PrecompileShader> o = PrecompileShadersPriv::Picture(SkToBool(lmPtr));
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_solid_shader(
        SkRandom* rand, ColorConstraint constraint = ColorConstraint::kNone) {
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
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_gradient_shader(
        SkRandom* rand,
        SkShaderBase::GradientType type,
        ColorConstraint constraint = ColorConstraint::kOpaque) {
    static constexpr int kMaxNumStops = 9;
    SkColor4f colors[kMaxNumStops];
    for (int i = 0; i < kMaxNumStops; ++i) colors[i] = random_color4f(rand, constraint);

    static const SkPoint kPts[kMaxNumStops] = {{-100.0f, -100.0f},
                                               {-50.0f, -50.0f},
                                               {-25.0f, -25.0f},
                                               {-12.5f, -12.5f},
                                               {0.0f, 0.0f},
                                               {12.5f, 12.5f},
                                               {25.0f, 25.0f},
                                               {50.0f, 50.0f},
                                               {100.0f, 100.0f}};
    static const float kOffsets[kMaxNumStops] = {
            0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f};

    size_t numStops;
    switch (rand->nextULessThan(3)) {
        case 0:
            numStops = 2;
            break;
        case 1:
            numStops = 7;
            break;
        case 2:
            [[fallthrough]];
        default:
            numStops = kMaxNumStops;
            break;
    }

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);
    const SkGradient::Interpolation::InPremul inPremul =
            rand->nextBool() ? SkGradient::Interpolation::InPremul::kYes
                             : SkGradient::Interpolation::InPremul::kNo;
    const SkGradient::Interpolation::ColorSpace colorSpace =
            static_cast<SkGradient::Interpolation::ColorSpace>(
                    rand->nextULessThan(SkGradient::Interpolation::kColorSpaceCount));
    SkGradient::Interpolation interpolation = {inPremul, colorSpace};

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;
    SkTileMode tm = random_tilemode(rand);
    const SkGradient grad = {{{colors, numStops}, {kOffsets, numStops}, tm}, interpolation};

    switch (type) {
        case SkShaderBase::GradientType::kLinear:
            s = SkShaders::LinearGradient(kPts, grad, lmPtr);
            o = PrecompileShaders::LinearGradient(GradientShaderFlags::kAll, interpolation);
            break;
        case SkShaderBase::GradientType::kRadial:
            s = SkShaders::RadialGradient(/* center= */ {0, 0}, /* radius= */ 100, grad, lmPtr);
            o = PrecompileShaders::RadialGradient(GradientShaderFlags::kAll, interpolation);
            break;
        case SkShaderBase::GradientType::kSweep:
            s = SkShaders::SweepGradient(/* center= */ {0, 0}, grad, lmPtr);
            o = PrecompileShaders::SweepGradient(GradientShaderFlags::kAll, interpolation);
            break;
        case SkShaderBase::GradientType::kConical:
            s = SkShaders::TwoPointConicalGradient(/* start= */ {100, 100},
                                                   /* startRadius= */ 100,
                                                   /* end= */ {-100, -100},
                                                   /* endRadius= */ 100,
                                                   grad, lmPtr);
            o = PrecompileShaders::TwoPointConicalGradient(GradientShaderFlags::kAll,
                                                           interpolation);
            break;
        case SkShaderBase::GradientType::kNone:
            SkDEBUGFAIL("Gradient shader says its type is none");
            break;
    }
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_localmatrix_shader(SkRandom* rand,
                                                                              Recorder* recorder,
                                                                              bool* reqSKPOption) {
    auto [s, o] = CreateRandomShader(rand, recorder, random_shadertype(rand), reqSKPOption);
    SkASSERT(!s == !o);
    if (!s) {
        return {nullptr, nullptr};
    }
    bool hasPerspective = rand->nextBool();
    SkMatrix lmStorage;
    random_local_matrix(rand,
                        &lmStorage,
                        hasPerspective ? LocalMatrixConstraint::kWithPerspective
                                       : LocalMatrixConstraint::kNone);
    return {s->makeWithLocalMatrix(lmStorage), o->makeWithLocalMatrix(hasPerspective)};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_colorfilter_shader(SkRandom* rand,
                                                                              Recorder* recorder,
                                                                              bool* reqSKPOption) {
    auto [s, o] = CreateRandomShader(rand, recorder, random_shadertype(rand), reqSKPOption);
    SkASSERT(!s == !o);
    if (!s) {
        return {nullptr, nullptr};
    }
    auto [cf, cfO] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    return {s->makeWithColorFilter(std::move(cf)), o->makeWithColorFilter(std::move(cfO))};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_image_shader(SkRandom* rand,
                                                                        Recorder* recorder) {
    SkTileMode tmX = random_tilemode(rand);
    SkTileMode tmY = random_tilemode(rand);
    std::vector<SkTileMode> precompileTileModes =
            (tmX == tmY) ? std::vector<SkTileMode>{tmX}
                         : std::vector<SkTileMode>{SkTileMode::kClamp, SkTileMode::kRepeat};

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;
    sk_sp<SkImage> image = make_image(rand, recorder);
    SkColorInfo colorInfo = image->imageInfo().colorInfo();

    switch (rand->nextULessThan(4)) {
        case 0: {  // Non-subset image
            s = SkShaders::Image(std::move(image), tmX, tmY, SkSamplingOptions(), lmPtr);
            o = PrecompileShaders::Image(ImageShaderFlags::kAll, {colorInfo}, precompileTileModes);
        } break;
        case 1: {  // Subset image
            const SkRect subset = SkRect::MakeWH(image->width() / 2, image->height() / 2);
            s = SkImageShader::MakeSubset(
                    std::move(image), subset, tmX, tmY, SkSamplingOptions(), lmPtr);
            o = PrecompileShaders::Image(ImageShaderFlags::kAll, {colorInfo}, precompileTileModes);
        } break;
        case 2: {  // Cubic-sampled image
            s = SkShaders::Image(std::move(image), tmX, tmY, SkCubicResampler::Mitchell(), lmPtr);
            o = PrecompileShaders::Image(ImageShaderFlags::kAll, {colorInfo}, precompileTileModes);
        } break;
        default: {  // Raw image draw
            s = SkShaders::RawImage(std::move(image), tmX, tmY, SkSamplingOptions(), lmPtr);
            o = PrecompileShaders::RawImage(
                    ImageShaderFlags::kExcludeCubic, {colorInfo}, precompileTileModes);
        } break;
    }
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_yuv_image_shader(SkRandom* rand,
                                                                            Recorder* recorder) {
    SkTileMode tmX = random_tilemode(rand);
    SkTileMode tmY = random_tilemode(rand);
    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);
    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;
    SkSamplingOptions samplingOptions(SkFilterMode::kLinear);
    bool useCubic = rand->nextBool();
    if (useCubic) {
        samplingOptions = SkCubicResampler::Mitchell();
    }
    sk_sp<SkImage> yuvImage = make_yuv_image(rand, recorder);
    SkColorInfo colorInfo = yuvImage->imageInfo().colorInfo();

    if (rand->nextBool()) {
        s = SkImageShader::MakeSubset(std::move(yuvImage),
                                      SkRect::MakeXYWH(8, 8, 16, 16),
                                      tmX,
                                      tmY,
                                      samplingOptions,
                                      lmPtr);
    } else {
        s = SkShaders::Image(std::move(yuvImage), tmX, tmY, samplingOptions, lmPtr);
    }
    o = PrecompileShaders::YUVImage(
            useCubic ? YUVImageShaderFlags::kCubicSampling : YUVImageShaderFlags::kExcludeCubic,
            {colorInfo});
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_blend_shader(SkRandom* rand,
                                                                        Recorder* recorder,
                                                                        bool* reqSKPOption) {
    auto [blender, blenderO] = CreateRandomBlender(rand, random_blendertype(rand));
    auto [dstS, dstO] = CreateRandomShader(rand, recorder, random_shadertype(rand), reqSKPOption);
    if (!dstS) return {nullptr, nullptr};
    auto [srcS, srcO] = CreateRandomShader(rand, recorder, random_shadertype(rand), reqSKPOption);
    if (!srcS) return {nullptr, nullptr};

    auto s = SkShaders::Blend(std::move(blender), std::move(dstS), std::move(srcS));
    auto o = PrecompileShaders::Blend(
            SkSpan<const sk_sp<PrecompileBlender>>({blenderO}), {dstO}, {srcO});
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_workingCS_shader(SkRandom* rand,
                                                                            Recorder* recorder,
                                                                            bool* reqSKPOption) {
    auto [wrappedS, wrappedO] =
            CreateRandomShader(rand, recorder, random_shadertype(rand), reqSKPOption);
    if (!wrappedS) return {nullptr, nullptr};
    sk_sp<SkColorSpace> cs = random_colorspace(rand);
    sk_sp<SkShader> s = wrappedS->makeWithWorkingColorSpace(cs);
    sk_sp<PrecompileShader> o = wrappedO->makeWithWorkingColorSpace(std::move(cs));
    return {s, o};
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> CreateRandomShader(SkRandom* rand,
                                                                       Recorder* recorder,
                                                                       ShaderType shaderType,
                                                                       bool* reqSKPOption) {
    switch (shaderType) {
        case ShaderType::kNone:
            return {nullptr, nullptr};
        case ShaderType::kBlend:
            return create_blend_shader(rand, recorder, reqSKPOption);
        case ShaderType::kColorFilter:
            return create_colorfilter_shader(rand, recorder, reqSKPOption);
        case ShaderType::kCoordClamp:
            return create_coord_clamp_shader(rand, recorder, reqSKPOption);
        case ShaderType::kConicalGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kConical);
        case ShaderType::kImage:
            return create_image_shader(rand, recorder);
        case ShaderType::kLinearGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kLinear);
        case ShaderType::kLocalMatrix:
            return create_localmatrix_shader(rand, recorder, reqSKPOption);
        case ShaderType::kPerlinNoise:
            return create_perlin_noise_shader(rand);
        case ShaderType::kPicture:
            return create_picture_shader(rand, reqSKPOption);
        case ShaderType::kRadialGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kRadial);
        case ShaderType::kRuntime:
            return PrecompileFactories::CreateAnnulusRuntimeShader();
        case ShaderType::kSolidColor:
            return create_solid_shader(rand);
        case ShaderType::kSweepGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kSweep);
        case ShaderType::kYUVImage:
            return create_yuv_image_shader(rand, recorder);
        case ShaderType::kWorkingColorSpace:
            return create_workingCS_shader(rand, recorder, reqSKPOption);
    }
    SkUNREACHABLE;
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> CreateClipShader(SkRandom* rand,
                                                                     Recorder* recorder) {
    switch (rand->nextULessThan(5)) {
        case 0:
            return create_gradient_shader(
                    rand, SkShaderBase::GradientType::kConical, ColorConstraint::kTransparent);
        case 1:
            return create_gradient_shader(
                    rand, SkShaderBase::GradientType::kLinear, ColorConstraint::kTransparent);
        case 2:
            return create_gradient_shader(
                    rand, SkShaderBase::GradientType::kRadial, ColorConstraint::kTransparent);
        case 3:
            return create_solid_shader(rand, ColorConstraint::kTransparent);
        case 4:
            return create_gradient_shader(
                    rand, SkShaderBase::GradientType::kSweep, ColorConstraint::kTransparent);
    }
    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
// Blender Generators
//--------------------------------------------------------------------------------------------------

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_bm_blender(SkRandom* rand,
                                                                        SkBlendMode bm) {
    return {SkBlender::Mode(bm), PrecompileBlenders::Mode(bm)};
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_arithmetic_blender() {
    sk_sp<SkBlender> b = SkBlenders::Arithmetic(0.5f, 0.5f, 0.5f, 0.5f, true);
    sk_sp<PrecompileBlender> o = PrecompileBlenders::Arithmetic();
    return {std::move(b), std::move(o)};
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_rt_blender(SkRandom* rand) {
    int option = rand->nextULessThan(3);
    switch (option) {
        case 0:
            return PrecompileFactories::CreateSrcRuntimeBlender();
        case 1:
            return PrecompileFactories::CreateDstRuntimeBlender();
        case 2:
            return PrecompileFactories::CreateComboRuntimeBlender();
    }
    return {nullptr, nullptr};
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateRandomBlender(SkRandom* rand,
                                                                          BlenderType type) {
    switch (type) {
        case BlenderType::kNone:
            return {nullptr, nullptr};
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

//--------------------------------------------------------------------------------------------------
// ColorFilter Generators
//--------------------------------------------------------------------------------------------------

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_rt_colorfilter(
        SkRandom* rand) {
    int option = rand->nextULessThan(3);
    switch (option) {
        case 0:
            return PrecompileFactories::CreateDoubleRuntimeColorFilter();
        case 1:
            return PrecompileFactories::CreateHalfRuntimeColorFilter();
        case 2:
            return PrecompileFactories::CreateComboRuntimeColorFilter();
    }
    return {nullptr, nullptr};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_lerp_colorfilter(
        SkRandom* rand) {
    auto [dst, dstO] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    auto [src, srcO] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    while (src == dst) {  // Avoid optimization where src == dst
        std::tie(src, srcO) = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    }
    sk_sp<SkColorFilter> cf = SkColorFilters::Lerp(0.5f, std::move(dst), std::move(src));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Lerp({dstO}, {srcO});
    return {cf, o};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_lighting_colorfilter() {
    return {SkColorFilters::Lighting(SK_ColorGREEN, SK_ColorRED),
            PrecompileColorFilters::Lighting()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_blendmode_colorfilter(
        SkRandom* rand) {
    sk_sp<SkColorFilter> cf;
    SkBlendMode blend;
    while (!cf) {
        blend = random_blend_mode(rand);
        cf = SkColorFilters::Blend(
                random_color4f(rand, ColorConstraint::kNone), random_colorspace(rand), blend);
    }
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Blend({&blend, 1});
    return {std::move(cf), std::move(o)};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::Matrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Matrix();
    return {std::move(cf), std::move(o)};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_color_space_colorfilter(
        SkRandom* rand) {
    sk_sp<SkColorSpace> src = random_colorspace(rand);
    sk_sp<SkColorSpace> dst = random_colorspace(rand);
    return {SkColorFilterPriv::MakeColorSpaceXform(src, dst),
            PrecompileColorFiltersPriv::ColorSpaceXform({src}, {dst})};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_compose_colorfilter(
        SkRandom* rand) {
    auto [outerCF, outerO] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    auto [innerCF, innerO] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    return {SkColorFilters::Compose(std::move(outerCF), std::move(innerCF)),
            PrecompileColorFilters::Compose({std::move(outerO)}, {std::move(innerO)})};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_gaussian_colorfilter() {
    return {SkColorFilterPriv::MakeGaussian(), PrecompileColorFiltersPriv::Gaussian()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_table_colorfilter() {
    static constexpr uint8_t kTable[256] = {0};
    return {SkColorFilters::Table(kTable), PrecompileColorFilters::Table()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_workingformat_colorfilter(
        SkRandom* rand) {
    auto [childCF, childO] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    if (!childCF) return {nullptr, nullptr};
    const skcms_TransferFunction* tf = rand->nextBool() ? &random_xfer_function(rand) : nullptr;
    const skcms_Matrix3x3* gamut = rand->nextBool() ? &random_gamut(rand) : nullptr;
    const SkAlphaType unpremul = kUnpremul_SkAlphaType;
    sk_sp<SkColorFilter> cf =
            SkColorFilterPriv::WithWorkingFormat(std::move(childCF), tf, gamut, &unpremul);
    sk_sp<PrecompileColorFilter> o = PrecompileColorFiltersPriv::WithWorkingFormat(
            {std::move(childO)}, tf, gamut, &unpremul);
    return {std::move(cf), std::move(o)};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_overdraw_colorfilter() {
    static const SkColor kColors[SkOverdrawColorFilter::kNumColors] = {
            SK_ColorBLACK, SK_ColorBLUE, SK_ColorCYAN, SK_ColorGREEN, SK_ColorYELLOW, SK_ColorRED};
    return {SkOverdrawColorFilter::MakeWithSkColors(kColors), PrecompileColorFilters::Overdraw()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_high_contrast_colorfilter() {
    SkHighContrastConfig config(false, SkHighContrastConfig::InvertStyle::kInvertBrightness, 0.5f);
    return {SkHighContrastFilter::Make(config), PrecompileColorFilters::HighContrast()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_luma_colorfilter() {
    return {SkLumaColorFilter::Make(), PrecompileColorFilters::Luma()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_linear_to_srgb_colorfilter() {
    return {SkColorFilters::LinearToSRGBGamma(), PrecompileColorFilters::LinearToSRGBGamma()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_srgb_to_linear_colorfilter() {
    return {SkColorFilters::SRGBToLinearGamma(), PrecompileColorFilters::SRGBToLinearGamma()};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_hsla_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::HSLAMatrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::HSLAMatrix();
    return {std::move(cf), std::move(o)};
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateRandomColorFilter(
        SkRandom* rand, ColorFilterType type) {
    switch (type) {
        case ColorFilterType::kNone:
            return {nullptr, nullptr};
        case ColorFilterType::kBlendMode:
            return create_blendmode_colorfilter(rand);
        case ColorFilterType::kColorSpaceXform:
            return create_color_space_colorfilter(rand);
        case ColorFilterType::kCompose:
            return create_compose_colorfilter(rand);
        case ColorFilterType::kGaussian:
            return create_gaussian_colorfilter();
        case ColorFilterType::kHighContrast:
            return create_high_contrast_colorfilter();
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
        case ColorFilterType::kOverdraw:
            return create_overdraw_colorfilter();
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

//--------------------------------------------------------------------------------------------------
// ImageFilter Generators
//--------------------------------------------------------------------------------------------------

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> arithmetic_imagefilter(SkRandom*) {
    sk_sp<SkImageFilter> arithmeticIF =
            SkImageFilters::Arithmetic(0.5f, 0.5f, 0.5f, 0.5f, false, nullptr, nullptr);
    sk_sp<PrecompileImageFilter> option = PrecompileImageFilters::Arithmetic(nullptr, nullptr);
    return {std::move(arithmeticIF), std::move(option)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> blendmode_imagefilter(
        SkRandom* rand) {
    SkBlendMode bm = random_blend_mode(rand);
    sk_sp<SkImageFilter> blendIF = SkImageFilters::Blend(bm, nullptr, nullptr);
    sk_sp<PrecompileImageFilter> blendO = PrecompileImageFilters::Blend(bm, nullptr, nullptr);
    return {std::move(blendIF), std::move(blendO)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> runtime_blender_imagefilter(
        SkRandom* rand) {
    auto [blender, blenderO] = CreateRandomBlender(rand, BlenderType::kRuntime);
    sk_sp<SkImageFilter> blenderIF = SkImageFilters::Blend(std::move(blender), nullptr, nullptr);
    sk_sp<PrecompileImageFilter> option =
            PrecompileImageFilters::Blend(std::move(blenderO), nullptr, nullptr);
    return {std::move(blenderIF), std::move(option)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> blur_imagefilter(SkRandom* rand) {
    float sigma = 5.0f;
    int option = rand->nextULessThan(3);
    if (option == 0)
        sigma = 1.0f;
    else if (option == 1)
        sigma = 2.0f;
    sk_sp<SkImageFilter> blurIF = SkImageFilters::Blur(sigma, sigma, nullptr);
    sk_sp<PrecompileImageFilter> blurO = PrecompileImageFilters::Blur(nullptr);
    return {std::move(blurIF), std::move(blurO)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> displacement_imagefilter(
        Recorder* recorder, SkRandom* rand) {
    sk_sp<SkImage> checkerboard =
            ToolUtils::create_checkerboard_image(16, 16, SK_ColorWHITE, SK_ColorBLACK, 4);
    checkerboard = SkImages::TextureFromImage(recorder, std::move(checkerboard), {false});
    sk_sp<SkImageFilter> imageIF(
            SkImageFilters::Image(std::move(checkerboard), SkFilterMode::kLinear));
    sk_sp<SkImageFilter> displacementIF = SkImageFilters::DisplacementMap(
            SkColorChannel::kR, SkColorChannel::kB, 2.0f, std::move(imageIF), nullptr);
    sk_sp<PrecompileImageFilter> option = PrecompileImageFilters::DisplacementMap(nullptr);
    return {std::move(displacementIF), std::move(option)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> colorfilter_imagefilter(
        SkRandom* rand) {
    auto [cf, o] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
    sk_sp<SkImageFilter> inputIF;
    sk_sp<PrecompileImageFilter> inputO;
    if (rand->nextBool()) {
        auto [cf2, o2] = CreateRandomColorFilter(rand, random_colorfiltertype(rand));
        inputIF = SkImageFilters::ColorFilter(std::move(cf2), nullptr);
        inputO = PrecompileImageFilters::ColorFilter(std::move(o2), nullptr);
    }
    sk_sp<SkImageFilter> cfIF = SkImageFilters::ColorFilter(std::move(cf), std::move(inputIF));
    sk_sp<PrecompileImageFilter> cfIFO =
            PrecompileImageFilters::ColorFilter(std::move(o), std::move(inputO));
    return {std::move(cfIF), std::move(cfIFO)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> lighting_imagefilter(SkRandom* rand) {
    static constexpr SkPoint3 kLocation{10.0f, 2.0f, 30.0f};
    static constexpr SkPoint3 kTarget{0, 0, 0};
    static constexpr SkPoint3 kDirection{0, 1, 0};
    sk_sp<SkImageFilter> lightingIF;
    int option = rand->nextULessThan(6);
    switch (option) {
        case 0:
            lightingIF =
                    SkImageFilters::DistantLitDiffuse(kDirection, SK_ColorRED, 1.0f, 0.5f, nullptr);
            break;
        case 1:
            lightingIF =
                    SkImageFilters::PointLitDiffuse(kLocation, SK_ColorGREEN, 1.0f, 0.5f, nullptr);
            break;
        case 2:
            lightingIF = SkImageFilters::SpotLitDiffuse(
                    kLocation, kTarget, 2.0f, 30.0f, SK_ColorBLUE, 1.0f, 0.5f, nullptr);
            break;
        case 3:
            lightingIF = SkImageFilters::DistantLitSpecular(
                    kDirection, SK_ColorCYAN, 1.0f, 0.5f, 2.0f, nullptr);
            break;
        case 4:
            lightingIF = SkImageFilters::PointLitSpecular(
                    kLocation, SK_ColorMAGENTA, 1.0f, 0.5f, 2.0f, nullptr);
            break;
        case 5:
            lightingIF = SkImageFilters::SpotLitSpecular(
                    kLocation, kTarget, 2.0f, 30.0f, SK_ColorYELLOW, 1.0f, 4.0f, 0.5f, nullptr);
            break;
    }
    sk_sp<PrecompileImageFilter> lightingO = PrecompileImageFilters::Lighting(nullptr);
    return {std::move(lightingIF), std::move(lightingO)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> matrix_convolution_imagefilter(
        SkRandom* rand) {
    int kernelSize = 1;
    int option = rand->nextULessThan(3);
    if (option == 0)
        kernelSize = 3;
    else if (option == 1)
        kernelSize = 7;
    else if (option == 2)
        kernelSize = 11;

    int center = (kernelSize * kernelSize - 1) / 2;
    std::vector<float> kernel(kernelSize * kernelSize, SkIntToScalar(1));
    kernel[center] = 2.0f - kernelSize * kernelSize;

    sk_sp<SkImageFilter> matrixConvIF = SkImageFilters::MatrixConvolution({kernelSize, kernelSize},
                                                                          kernel.data(),
                                                                          0.3f,
                                                                          100.0f,
                                                                          {1, 1},
                                                                          SkTileMode::kMirror,
                                                                          false,
                                                                          nullptr);
    sk_sp<PrecompileImageFilter> convOption = PrecompileImageFilters::MatrixConvolution(nullptr);
    return {std::move(matrixConvIF), std::move(convOption)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> morphology_imagefilter(
        SkRandom* rand) {
    static constexpr float kRadX = 2.0f, kRadY = 4.0f;
    sk_sp<SkImageFilter> morphologyIF;
    if (rand->nextBool()) {
        morphologyIF = SkImageFilters::Erode(kRadX, kRadY, nullptr);
    } else {
        morphologyIF = SkImageFilters::Dilate(kRadX, kRadY, nullptr);
    }
    sk_sp<PrecompileImageFilter> option = PrecompileImageFilters::Morphology(nullptr);
    return {std::move(morphologyIF), std::move(option)};
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> CreateRandomImageFilter(
        Recorder* recorder, SkRandom* rand, ImageFilterType type) {
    switch (type) {
        case ImageFilterType::kNone:
            return {};
        case ImageFilterType::kArithmetic:
            return arithmetic_imagefilter(rand);
        case ImageFilterType::kBlendMode:
            return blendmode_imagefilter(rand);
        case ImageFilterType::kRuntimeBlender:
            return runtime_blender_imagefilter(rand);
        case ImageFilterType::kBlur:
            return blur_imagefilter(rand);
        case ImageFilterType::kColorFilter:
            return colorfilter_imagefilter(rand);
        case ImageFilterType::kDisplacement:
            return displacement_imagefilter(recorder, rand);
        case ImageFilterType::kLighting:
            return lighting_imagefilter(rand);
        case ImageFilterType::kMatrixConvolution:
            return matrix_convolution_imagefilter(rand);
        case ImageFilterType::kMorphology:
            return morphology_imagefilter(rand);
    }
    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
// MaskFilter Generators
//--------------------------------------------------------------------------------------------------

std::pair<sk_sp<SkMaskFilter>, sk_sp<PrecompileMaskFilter>> create_blur_maskfilter(SkRandom* rand) {
    SkBlurStyle style;
    switch (rand->nextULessThan(4)) {
        case 0:
            style = kNormal_SkBlurStyle;
            break;
        case 1:
            style = kSolid_SkBlurStyle;
            break;
        case 2:
            style = kOuter_SkBlurStyle;
            break;
        case 3:
            [[fallthrough]];
        default:
            style = kInner_SkBlurStyle;
            break;
    }
    float sigma = 1.0f;
    switch (rand->nextULessThan(2)) {
        case 0:
            sigma = 1.0f;
            break;
        case 1:
            sigma = 2.0f;
            break;
        case 2:
            [[fallthrough]];
        default:
            sigma = 5.0f;
            break;
    }
    bool respectCTM = rand->nextBool();
    return {SkMaskFilter::MakeBlur(style, sigma, respectCTM), PrecompileMaskFilters::Blur()};
}

std::pair<sk_sp<SkMaskFilter>, sk_sp<PrecompileMaskFilter>> CreateRandomMaskFilter(
        SkRandom* rand, MaskFilterType type) {
    switch (type) {
        case MaskFilterType::kNone:
            return {nullptr, nullptr};
        case MaskFilterType::kBlur:
            return create_blur_maskfilter(rand);
    }
    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
// Main Entry Points
//--------------------------------------------------------------------------------------------------

std::pair<SkPaint, PaintOptions> CreateRandomPaint(SkRandom* rand,
                                                   Recorder* recorder,
                                                   ShaderType shaderType,
                                                   BlenderType blenderType,
                                                   ColorFilterType cfType,
                                                   MaskFilterType mfType,
                                                   ImageFilterType imageFilterType,
                                                   bool* reqSKPPaintOption) {
    SkColor paintColor = random_color(rand, ColorConstraint::kNone);
    SkPaint paint;
    paint.setColor(paintColor);
    PaintOptions paintOptions;
    paintOptions.setPaintColorIsOpaque(SkColorGetA(paintColor) == 0xFF);

    {
        std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> sAndO;
        if (shaderType == ShaderType::kPicture) {
            sAndO = create_picture_shader(rand, reqSKPPaintOption);
        } else {
            sAndO = CreateRandomShader(rand, recorder, shaderType, reqSKPPaintOption);
        }

        auto [s, o] = sAndO;
        SkASSERT(!s == !o);

        if (s) {
            paint.setShader(std::move(s));
            paintOptions.setShaders({o});
        }
    }

    {
        auto [cf, o] = CreateRandomColorFilter(rand, cfType);
        SkASSERT(!cf == !o);
        if (cf) {
            paint.setColorFilter(std::move(cf));
            paintOptions.setColorFilters({o});
        }
    }

    {
        auto [mf, o] = CreateRandomMaskFilter(rand, mfType);
        SkASSERT(!mf == !o);
        if (mf) {
            paint.setMaskFilter(std::move(mf));
            paintOptions.setMaskFilters({o});
        }
    }

    {
        auto [b, o] = CreateRandomBlender(rand, blenderType);
        SkASSERT(!b == !o);
        if (b) {
            paint.setBlender(std::move(b));
            paintOptions.setBlenders({o});
        }
    }

    {
        auto [filter, o] = CreateRandomImageFilter(recorder, rand, imageFilterType);
        SkASSERT(!filter == !o);
        if (filter) {
            paint.setImageFilter(std::move(filter));
            paintOptions.setImageFilters({o});
        }
    }

    if (rand->nextBool()) {
        paint.setDither(true);
        paintOptions.setDither(true);
    }

    return {paint, paintOptions};
}

static SkPath make_path() {
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

DrawData::DrawData() {
    static constexpr int kMaskTextFontSize = 16;
    static constexpr int kPathTextFontSize = 300;

    SkFont font(ToolUtils::DefaultPortableTypeface(), kMaskTextFontSize);
    SkFont lcdFont(ToolUtils::DefaultPortableTypeface(), kMaskTextFontSize);
    lcdFont.setSubpixel(true);
    lcdFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    ToolUtils::EmojiTestSample emojiTestSample =
            ToolUtils::EmojiSample(ToolUtils::EmojiFontFormat::ColrV0);
    SkFont emojiFont(emojiTestSample.typeface);
    SkFont pathFont(ToolUtils::DefaultPortableTypeface(), kPathTextFontSize);

    const char text[] = "hambur1";
    constexpr int kNumVerts = 4;
    constexpr SkPoint kPositions[kNumVerts]{{0, 0}, {0, 16}, {16, 16}, {16, 0}};
    constexpr SkColor kColors[kNumVerts] = {
            SK_ColorBLUE, SK_ColorGREEN, SK_ColorCYAN, SK_ColorYELLOW};

    fPath = make_path();
    fBlob = SkTextBlob::MakeFromText(text, strlen(text), font);
    fLCDBlob = SkTextBlob::MakeFromText(text, strlen(text), lcdFont);
    fEmojiBlob = SkTextBlob::MakeFromText(
            emojiTestSample.sampleText, strlen(emojiTestSample.sampleText), emojiFont);
    fPathBlob = SkTextBlob::MakeFromText(text, strlen(text), pathFont);
    fVertsWithColors = SkVertices::MakeCopy(
            SkVertices::kTriangleFan_VertexMode, kNumVerts, kPositions, kPositions, kColors);
    fVertsWithOutColors = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
                                               kNumVerts,
                                               kPositions,
                                               kPositions,
                                               /* colors= */ nullptr);
}

void non_simple_draws(SkCanvas* canvas, const SkPaint& paint, const DrawData& drawData) {
    canvas->drawPath(drawData.fPath, paint);
    canvas->drawTextBlob(drawData.fPathBlob, 0, 16, paint);
    if (paint.getStyle() == SkPaint::kStroke_Style) {
        canvas->drawArc({0, 0, 16, 16}, 0, 90, /* useCenter= */ true, paint);
    }
}

void ExecuteDraw(SkCanvas* canvas,
                 const SkPaint& paint,
                 const DrawData& drawData,
                 DrawTypeFlags dt) {
    // Mask out the modifiers that don't affect which API is called on the canvas
    DrawTypeFlags drawType = static_cast<DrawTypeFlags>(dt & ~DrawTypeFlags::kAnalyticClip);

    switch (drawType) {
        case DrawTypeFlags::kBitmapText_Mask:
            canvas->drawTextBlob(drawData.fBlob, 0, 16, paint);
            break;
        case DrawTypeFlags::kBitmapText_LCD:
            canvas->drawTextBlob(drawData.fLCDBlob, 0, 16, paint);
            break;
        case DrawTypeFlags::kBitmapText_Color:
            canvas->drawTextBlob(drawData.fEmojiBlob, 0, 16, paint);
            break;
        case DrawTypeFlags::kSDFText: {
            SkMatrix perspective;
            perspective.setPerspX(0.01f);
            perspective.setPerspY(0.001f);
            canvas->save();
            canvas->concat(perspective);
            canvas->drawTextBlob(drawData.fBlob, 0, 16, paint);
            canvas->restore();
        } break;
        case DrawTypeFlags::kSDFText_LCD: {
            SkMatrix perspective;
            perspective.setPerspX(0.01f);
            perspective.setPerspY(0.001f);
            canvas->save();
            canvas->concat(perspective);
            canvas->drawTextBlob(drawData.fLCDBlob, 0, 16, paint);
            canvas->restore();
        } break;
        case DrawTypeFlags::kDrawVertices:
            canvas->drawVertices(drawData.fVertsWithColors, SkBlendMode::kDst, paint);
            canvas->drawVertices(drawData.fVertsWithOutColors, SkBlendMode::kDst, paint);
            break;
        case DrawTypeFlags::kCircularArc:
            canvas->drawArc({0, 0, 16, 16}, 0, 90, /* useCenter= */ false, paint);
            if (paint.getStyle() == SkPaint::kFill_Style) {
                canvas->drawArc({0, 0, 16, 16}, 0, 90, /* useCenter= */ true, paint);
            }
            break;
        case DrawTypeFlags::kAnalyticRRect:
            canvas->drawRRect(SkRRect::MakeOval({0, 0, 15, 15}), paint);
            canvas->drawRRect(SkRRect::MakeRectXY({0, 0, 15, 15}, 4, 4), paint);
            break;
        case DrawTypeFlags::kPerEdgeAAQuad:
            if (!paint.getShader() && !paint.getColorFilter() && !paint.getImageFilter() &&
                paint.asBlendMode().has_value()) {
                canvas->experimental_DrawEdgeAAQuad(SkRect::MakeWH(15, 15),
                                                    nullptr,
                                                    SkCanvas::kAll_QuadAAFlags,
                                                    paint.getColor4f(),
                                                    paint.asBlendMode().value());
            }
            break;
        case DrawTypeFlags::kNonAAFillRect:
            canvas->drawRect(SkRect::MakeWH(15, 15), paint);
            break;
        case DrawTypeFlags::kNonSimpleShape:
            non_simple_draws(canvas, paint, drawData);
            break;
        default:
            SkASSERT(false);
            break;
    }
}

}  // namespace skiatest::graphite

#endif  // SK_GRAPHITE

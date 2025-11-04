/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/effects/SkBlenders.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"
#include "include/gpu/graphite/precompile/PrecompileMaskFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/base/SkRandom.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"
#include "src/gpu/graphite/precompile/PrecompileColorFiltersPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/UniqueKeyUtils.h"
#include "tools/graphite/precompile/PrecompileEffectFactories.h"

// Set this to 1 for more expansive (aka far slower) local testing
#define EXPANDED_SET 0

// This flag is set to true if during the PaintOption creation an SkPictureShader is created.
// The SkPictureShader will need an additional program in order to draw the contents of its
// SkPicture.
bool gNeedSKPPaintOption = false;

constexpr uint32_t kDefaultSeed = 0;

using namespace skgpu::graphite;
using namespace skiatest::graphite;
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

namespace {

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom*, Recorder*);
std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom*);
std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(SkRandom*);

[[maybe_unused]] std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>>
create_random_image_filter(Recorder*, SkRandom*);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
sk_sp<DrawContext> get_precompile_draw_context(
            const skgpu::graphite::Caps* caps, Context* context) {
    std::unique_ptr<Recorder> drawRecorder = context->makeRecorder();
    ResourceProvider* resourceProvider = drawRecorder->priv().resourceProvider();
    constexpr SkISize drawSize = {128, 128};
    const SkColorInfo colorInfo = SkColorInfo(kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType,
                                              SkColorSpace::MakeSRGB());
    TextureInfo texInfo = caps->getDefaultSampledTextureInfo(colorInfo.colorType(),
                                                             skgpu::Mipmapped::kNo,
                                                             skgpu::Protected::kNo,
                                                             skgpu::Renderable::kYes);
    sk_sp<TextureProxy> target = TextureProxy::Make(caps,
                                                    resourceProvider,
                                                    drawSize,
                                                    texInfo,
                                                    "PrecompileTarget",
                                                    skgpu::Budgeted::kYes);
    sk_sp<DrawContext> precompileDrawContext = DrawContext::Make(caps,
                                                                 std::move(target),
                                                                 drawSize,
                                                                 colorInfo,
                                                                 {});
    return precompileDrawContext;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//    M(Empty)
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
#define SK_ALL_TEST_MASKFILTERS(M) \
    M(None)                        \
    M(Blur)

enum class MaskFilterType {
#define M(type) k##type,
    SK_ALL_TEST_MASKFILTERS(M)
#undef M

    kLast = kBlur
};

static constexpr int kMaskFilterTypeCount = static_cast<int>(MaskFilterType::kLast) + 1;

const char* to_str(MaskFilterType mf) {
    switch (mf) {
#define M(type) case MaskFilterType::k##type : return "MaskFilterType::k" #type;
        SK_ALL_TEST_MASKFILTERS(M)
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

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_blender(SkRandom*, BlenderType);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
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

static constexpr int kClipTypeCount = static_cast<int>(ClipType::kLast) + 1;

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

static constexpr int kImageFilterTypeCount = static_cast<int>(ImageFilterType::kLast) + 1;

const char* to_str(ImageFilterType c) {
    switch (c) {
#define M(type) case ImageFilterType::k##type : return "ImageFilterType::k" #type;
        SK_ALL_TEST_IMAGE_FILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

const char* to_str(DrawTypeFlags dt) {
    SkASSERT(SkPopCount(static_cast<uint32_t>(dt)) == 1);

    switch (dt) {
        case DrawTypeFlags::kBitmapText_Mask:  return "DrawTypeFlags::kBitmapText_Mask";
        case DrawTypeFlags::kBitmapText_LCD:   return "DrawTypeFlags::kBitmapText_LCD";
        case DrawTypeFlags::kBitmapText_Color: return "DrawTypeFlags::kBitmapText_Color";
        case DrawTypeFlags::kSDFText:          return "DrawTypeFlags::kSDFText";
        case DrawTypeFlags::kSDFText_LCD:      return "DrawTypeFlags::kSDFText_LCD";
        case DrawTypeFlags::kDrawVertices:     return "DrawTypeFlags::kDrawVertices";
        case DrawTypeFlags::kCircularArc:      return "DrawTypeFlags::kCircularArc";
        case DrawTypeFlags::kAnalyticRRect:    return "DrawTypeFlags::kAnalyticRRect";
        case DrawTypeFlags::kPerEdgeAAQuad:    return "DrawTypeFlags::kPerEdgeAAQuad";
        case DrawTypeFlags::kNonAAFillRect:    return "DrawTypeFlags::kNonAAFillRect";
        case DrawTypeFlags::kNonSimpleShape:   return "DrawTypeFlags::kNonSimpleShape";
        default:                               SkASSERT(0); return "DrawTypeFlags::kNone";
    }

    SkUNREACHABLE;
}

void log_run(const char* label,
             uint32_t seed,
             ShaderType s,
             BlenderType bm,
             ColorFilterType cf,
             MaskFilterType mf,
             ImageFilterType imageFilter,
             ClipType clipType,
             DrawTypeFlags drawTypeFlags) {
    SkDebugf("%s:\n"
             "//------------------------\n"
             "uint32_t seed = %u;\n"
             "ShaderType shaderType = %s;\n"
             "BlenderType blenderType = %s;\n"
             "ColorFilterType colorFilterType = %s;\n"
             "MaskFilterType maskFilterType = %s;\n"
             "ImageFilterType imageFilterType = %s;\n"
             "ClipType clipType = %s;\n"
             "DrawTypeFlags drawTypeFlags = %s;\n"
             "//-----------------------\n",
             label,
             seed,
             to_str(s),
             to_str(bm),
             to_str(cf),
             to_str(mf),
             to_str(imageFilter),
             to_str(clipType),
             to_str(drawTypeFlags));
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
    kSRGBSpin,
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
    // NOTE: The porter duff modes refer to being able to be consolidated into a single
    // call to sk_porter_duff_blend(), which has slightly fewer compatible blend modes than the
    // "coefficient" blend mode that supports HW blending.
    return static_cast<SkBlendMode>(rand->nextRangeU((unsigned int) SkBlendMode::kClear,
                                                     (unsigned int) SkBlendMode::kPlus));
}

SkBlendMode random_complex_bm(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextRangeU((unsigned int) SkBlendMode::kPlus,
                                                     (unsigned int) SkBlendMode::kLastMode));
}

SkBlendMode random_blend_mode(SkRandom* rand) {
    return static_cast<SkBlendMode>(rand->nextULessThan(kSkBlendModeCount));
}

[[maybe_unused]] MaskFilterType random_maskfiltertype(SkRandom* rand) {
    if (rand->nextBool()) {
        return MaskFilterType::kNone; // bias this towards no mask filter
    }

    return static_cast<MaskFilterType>(rand->nextULessThan(kMaskFilterTypeCount));
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

[[maybe_unused]] ClipType random_cliptype(SkRandom* rand) {
    if (rand->nextBool()) {
        return ClipType::kNone;  // bias this towards no clip
    }

    return static_cast<ClipType>(rand->nextULessThan(kClipTypeCount));
}

[[maybe_unused]] DrawTypeFlags random_drawtype(SkRandom* rand) {
    uint32_t index = rand->nextULessThan(11);

    switch (index) {
        case 0:  return DrawTypeFlags::kBitmapText_Mask;
        case 1:  return DrawTypeFlags::kBitmapText_LCD;
        case 2:  return DrawTypeFlags::kBitmapText_Color;
        case 3:  return DrawTypeFlags::kSDFText;
        case 4:  return DrawTypeFlags::kSDFText_LCD;
        case 5:  return DrawTypeFlags::kDrawVertices;
        case 6:  return DrawTypeFlags::kCircularArc;
        case 7:  return DrawTypeFlags::kAnalyticRRect;
        case 8:  return DrawTypeFlags::kPerEdgeAAQuad;
        case 9:  return DrawTypeFlags::kNonAAFillRect;
        case 10: return DrawTypeFlags::kNonSimpleShape;
    }

    SkASSERT(0);
    return DrawTypeFlags::kNone;
}

enum LocalMatrixConstraint {
    kNone,
    kWithPerspective,
};

SkMatrix* random_local_matrix(SkRandom* rand,
                              SkMatrix* storage,
                              LocalMatrixConstraint constaint = LocalMatrixConstraint::kNone) {
    // Only return nullptr if constraint == kNone.
    uint32_t matrix = rand->nextULessThan(constaint == LocalMatrixConstraint::kNone ? 4 : 3);

    switch (matrix) {
        case 0:  storage->setTranslate(2.0f, 2.0f); break;
        case 1:  storage->setIdentity(); break;
        case 2:  storage->setScale(0.25f, 0.25f, 0.0f, 0.0f); break;
        case 3:  return nullptr;
    }

    if (constaint == LocalMatrixConstraint::kWithPerspective) {
        storage->setPerspX(0.5f);
    }

    return storage;
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

sk_sp<SkImage> make_yuv_image(SkRandom* rand, Recorder* recorder) {

    SkYUVAInfo::PlaneConfig planeConfig = SkYUVAInfo::PlaneConfig::kY_UV;
    if (rand->nextBool()) {
        planeConfig = SkYUVAInfo::PlaneConfig::kY_U_V_A;
    }

    SkYUVAInfo yuvaInfo({ 32, 32, },
                        planeConfig,
                        SkYUVAInfo::Subsampling::k420,
                        kJPEG_Full_SkYUVColorSpace);

    SkASSERT(yuvaInfo.isValid());

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

#if 0
std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_empty_shader(SkRandom* /* rand */) {
    sk_sp<SkShader> s = SkShaders::Empty();
    sk_sp<PrecompileShader> o = PrecompileShaders::Empty();

    return { s, o };
}
#endif

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
    // TODO: fuzz more of the gradient parameters

    static constexpr int kMaxNumStops = 9;
    SkColor4f colors[kMaxNumStops] = {
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint),
            random_color4f(rand, constraint)
    };
    static const SkPoint kPts[kMaxNumStops] = {
            { -100.0f, -100.0f },
            { -50.0f, -50.0f },
            { -25.0f, -25.0f },
            { -12.5f, -12.5f },
            { 0.0f, 0.0f },
            { 12.5f, 12.5f },
            { 25.0f, 25.0f },
            { 50.0f, 50.0f },
            { 100.0f, 100.0f }
    };
    static const float kOffsets[kMaxNumStops] =
            { 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };

    int numStops;

    switch (rand->nextULessThan(3)) {
        case 0:  numStops = 2; break;
        case 1:  numStops = 7; break;
        case 2:  [[fallthrough]];
        default: numStops = kMaxNumStops; break;
    }

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    const SkGradientShader::Interpolation::InPremul inPremul =
            rand->nextBool() ? SkGradientShader::Interpolation::InPremul::kYes
                             : SkGradientShader::Interpolation::InPremul::kNo;
    const SkGradientShader::Interpolation::ColorSpace colorSpace =
            static_cast<SkGradientShader::Interpolation::ColorSpace>(
                    rand->nextULessThan(SkGradientShader::Interpolation::kColorSpaceCount));
    SkGradientShader::Interpolation interpolation = {inPremul, colorSpace};

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    SkTileMode tm = random_tilemode(rand);

    switch (type) {
        case SkShaderBase::GradientType::kLinear:
            s = SkGradientShader::MakeLinear(kPts,
                                             colors, /* colorSpace= */ nullptr, kOffsets, numStops,
                                             tm, interpolation, lmPtr);
            o = PrecompileShaders::LinearGradient(GradientShaderFlags::kAll, interpolation);
            break;
        case SkShaderBase::GradientType::kRadial:
            s = SkGradientShader::MakeRadial(/* center= */ {0, 0}, /* radius= */ 100,
                                             colors, /* colorSpace= */ nullptr, kOffsets, numStops,
                                             tm, interpolation, lmPtr);
            o = PrecompileShaders::RadialGradient(GradientShaderFlags::kAll, interpolation);
            break;
        case SkShaderBase::GradientType::kSweep:
            s = SkGradientShader::MakeSweep(/* cx= */ 0, /* cy= */ 0,
                                            colors, /* colorSpace= */ nullptr, kOffsets, numStops,
                                            tm, /* startAngle= */ 0, /* endAngle= */ 359,
                                            interpolation, lmPtr);
            o = PrecompileShaders::SweepGradient(GradientShaderFlags::kAll, interpolation);
            break;
        case SkShaderBase::GradientType::kConical:
            s = SkGradientShader::MakeTwoPointConical(/* start= */ {100, 100},
                                                      /* startRadius= */ 100,
                                                      /* end= */ {-100, -100},
                                                      /* endRadius= */ 100,
                                                      colors,
                                                      /* colorSpace= */ nullptr,
                                                      kOffsets, numStops, tm, interpolation, lmPtr);
            o = PrecompileShaders::TwoPointConicalGradient(GradientShaderFlags::kAll,
                                                           interpolation);
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

    bool hasPerspective = rand->nextBool();

    SkMatrix lmStorage;
    random_local_matrix(rand, &lmStorage, hasPerspective ? LocalMatrixConstraint::kWithPerspective
                                                         : LocalMatrixConstraint::kNone);

    return { s->makeWithLocalMatrix(lmStorage), o->makeWithLocalMatrix(hasPerspective) };
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

// TODO: With the new explicit PrecompileImageFilter API we need to test out complete DAGS of IFs
std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_image_shader(SkRandom* rand,
                                                                        Recorder* recorder) {
    SkTileMode tmX = random_tilemode(rand);
    SkTileMode tmY = random_tilemode(rand);

    std::vector<SkTileMode> precompileTileModes =
            (tmX == tmY) ? std::vector<SkTileMode>{ tmX }
                         : std::vector<SkTileMode>{ SkTileMode::kClamp, SkTileMode::kRepeat };

    SkMatrix lmStorage;
    SkMatrix* lmPtr = random_local_matrix(rand, &lmStorage);

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    sk_sp<SkImage> image = make_image(rand, recorder);
    SkColorInfo colorInfo = image->imageInfo().colorInfo();

    switch (rand->nextULessThan(4)) {
        case 0: {
            // Non-subset image.
            s = SkShaders::Image(std::move(image), tmX, tmY, SkSamplingOptions(), lmPtr);
            o = PrecompileShaders::Image(ImageShaderFlags::kAll,
                                         { colorInfo },
                                         precompileTileModes);
        } break;
        case 1: {
            // Subset image.
            const SkRect subset = SkRect::MakeWH(image->width() / 2, image->height() / 2);
            s = SkImageShader::MakeSubset(
                    std::move(image), subset, tmX, tmY, SkSamplingOptions(), lmPtr);
            o = PrecompileShaders::Image(ImageShaderFlags::kAll,
                                         { colorInfo },
                                         precompileTileModes);
        } break;
        case 2: {
            // Cubic-sampled image.
            s = SkShaders::Image(std::move(image), tmX, tmY, SkCubicResampler::Mitchell(), lmPtr);
            o = PrecompileShaders::Image(ImageShaderFlags::kAll,
                                         { colorInfo },
                                         precompileTileModes);
        } break;
        default: {
            // Raw image draw.
            s = SkShaders::RawImage(std::move(image), tmX, tmY, SkSamplingOptions(), lmPtr);
            o = PrecompileShaders::RawImage(ImageShaderFlags::kExcludeCubic,
                                            { colorInfo },
                                            precompileTileModes);
        } break;
    }

    return { s, o };
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
        s = SkImageShader::MakeSubset(std::move(yuvImage), SkRect::MakeXYWH(8, 8, 16, 16),
                                      tmX, tmY, samplingOptions, lmPtr);
    } else {
        s = SkShaders::Image(std::move(yuvImage), tmX, tmY, samplingOptions, lmPtr);
    }

    o = PrecompileShaders::YUVImage(useCubic ? YUVImageShaderFlags::kCubicSampling
                                             : YUVImageShaderFlags::kExcludeCubic,
                                    { colorInfo });

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
//        case ShaderType::kEmpty:
//            return create_empty_shader(rand);
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
        case ShaderType::kRuntime:
            return PrecompileFactories::CreateAnnulusRuntimeShader();
        case ShaderType::kSolidColor:
            return create_solid_shader(rand);
        case ShaderType::kSweepGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kSweep);
        case ShaderType::kYUVImage:
            return create_yuv_image_shader(rand, recorder);
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

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_bm_blender(SkRandom* rand,
                                                                        SkBlendMode bm) {
    return { SkBlender::Mode(bm), PrecompileBlenders::Mode(bm) };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_arithmetic_blender() {
    sk_sp<SkBlender> b = SkBlenders::Arithmetic(/* k1= */ 0.5f,
                                                /* k2= */ 0.5f,
                                                /* k3= */ 0.5f,
                                                /* k4= */ 0.5f,
                                                /* enforcePremul= */ true);
    sk_sp<PrecompileBlender> o = PrecompileBlenders::Arithmetic();

    return { std::move(b), std::move(o) };
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_rt_blender(SkRandom* rand) {
    int option = rand->nextULessThan(3);

    switch (option) {
        case 0: return PrecompileFactories::CreateSrcRuntimeBlender();
        case 1: return PrecompileFactories::CreateDstRuntimeBlender();
        case 2: return PrecompileFactories::CreateComboRuntimeBlender();
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


std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_rt_colorfilter(
        SkRandom* rand) {
    int option = rand->nextULessThan(3);

    switch (option) {
        case 0: return PrecompileFactories::CreateDoubleRuntimeColorFilter();
        case 1: return PrecompileFactories::CreateHalfRuntimeColorFilter();
        case 2: return PrecompileFactories::CreateComboRuntimeColorFilter();
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
    SkBlendMode blend;
    while (!cf) {
        blend = random_blend_mode(rand);
        cf = SkColorFilters::Blend(random_color4f(rand, ColorConstraint::kNone),
                                   random_colorspace(rand),
                                   blend);
    }

    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Blend({&blend, 1});

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
    sk_sp<SkColorSpace> src = random_colorspace(rand);
    sk_sp<SkColorSpace> dst = random_colorspace(rand);
    return { SkColorFilterPriv::MakeColorSpaceXform(src, dst),
             PrecompileColorFiltersPriv::ColorSpaceXform({ src }, { dst }) };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_linear_to_srgb_colorfilter() {
    return { SkColorFilters::LinearToSRGBGamma(), PrecompileColorFilters::LinearToSRGBGamma() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_srgb_to_linear_colorfilter() {
    return { SkColorFilters::SRGBToLinearGamma(), PrecompileColorFilters::SRGBToLinearGamma() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_high_contrast_colorfilter() {
    SkHighContrastConfig config(/* grayscale= */ false,
                                SkHighContrastConfig::InvertStyle::kInvertBrightness,
                                /* contrast= */ 0.5f);
    return { SkHighContrastFilter::Make(config), PrecompileColorFilters::HighContrast() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_luma_colorfilter() {
    return { SkLumaColorFilter::Make(), PrecompileColorFilters::Luma() };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_overdraw_colorfilter() {
    // Black to red heat map gradation
    static const SkColor kColors[SkOverdrawColorFilter::kNumColors] = {
        SK_ColorBLACK,
        SK_ColorBLUE,
        SK_ColorCYAN,
        SK_ColorGREEN,
        SK_ColorYELLOW,
        SK_ColorRED
    };

    return { SkOverdrawColorFilter::MakeWithSkColors(kColors), PrecompileColorFilters::Overdraw() };
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

    const skcms_TransferFunction* tf = rand->nextBool() ? &random_xfer_function(rand) : nullptr;
    const skcms_Matrix3x3* gamut = rand->nextBool() ? &random_gamut(rand) : nullptr;
    const SkAlphaType unpremul = kUnpremul_SkAlphaType;

    sk_sp<SkColorFilter> cf =
            SkColorFilterPriv::WithWorkingFormat(std::move(childCF), tf, gamut, &unpremul);

    sk_sp<PrecompileColorFilter> o = PrecompileColorFiltersPriv::WithWorkingFormat(
            { std::move(childO) }, tf, gamut, &unpremul);

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

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(
        SkRandom* rand) {
    return create_colorfilter(rand, random_colorfiltertype(rand));
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> arithmetic_imagefilter(
        SkRandom* /* rand */) {

    sk_sp<SkImageFilter> arithmeticIF = SkImageFilters::Arithmetic(/* k1= */ 0.5f,
                                                                   /* k2= */ 0.5f,
                                                                   /* k3= */ 0.5f,
                                                                   /* k4= */ 0.5f,
                                                                   /* enforcePMColor= */ false,
                                                                   /* background= */ nullptr,
                                                                   /* foreground= */ nullptr);
    sk_sp<PrecompileImageFilter> option = PrecompileImageFilters::Arithmetic(
            /* background= */ nullptr,
            /* foreground= */ nullptr);

    return { std::move(arithmeticIF), std::move(option) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> blendmode_imagefilter(
        SkRandom* rand) {

    SkBlendMode bm = random_blend_mode(rand);
    sk_sp<SkImageFilter> blendIF = SkImageFilters::Blend(bm,
                                                         /* background= */ nullptr,
                                                         /* foreground= */ nullptr);
    sk_sp<PrecompileImageFilter> blendO = PrecompileImageFilters::Blend(
            bm,
            /* background= */ nullptr,
            /* foreground= */ nullptr);

    return { std::move(blendIF), std::move(blendO) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> runtime_blender_imagefilter(
        SkRandom* rand) {

    auto [blender, blenderO] = create_blender(rand, BlenderType::kRuntime);
    sk_sp<SkImageFilter> blenderIF = SkImageFilters::Blend(std::move(blender),
                                                           /* background= */ nullptr,
                                                           /* foreground= */ nullptr);
    sk_sp<PrecompileImageFilter> option = PrecompileImageFilters::Blend(std::move(blenderO),
                                                                        /* background= */ nullptr,
                                                                        /* foreground= */ nullptr);

    return { std::move(blenderIF), std::move(option) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> blur_imagefilter(
        SkRandom* rand) {

    int option = rand->nextULessThan(3);

    float sigma;
    switch (option) {
        case 0:  sigma = 1.0f; break;  // 1DBlur4
        case 1:  sigma = 2.0f; break;  // 1DBlur8
        case 2:  [[fallthrough]];
        default: sigma = 5.0f; break;  // 1DBlur16
    }

    sk_sp<SkImageFilter> blurIF = SkImageFilters::Blur(sigma, sigma, /* input= */ nullptr);
    sk_sp<PrecompileImageFilter> blurO = PrecompileImageFilters::Blur(/* input= */ nullptr);

    return { std::move(blurIF), std::move(blurO) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> displacement_imagefilter(
        Recorder* recorder,
        SkRandom* rand) {

    sk_sp<SkImage> checkerboard = ToolUtils::create_checkerboard_image(16, 16,
                                                                       SK_ColorWHITE,
                                                                       SK_ColorBLACK,
                                                                       /* checkSize= */ 4);
    checkerboard = SkImages::TextureFromImage(recorder, std::move(checkerboard), {false});
    SkASSERT(checkerboard);

    sk_sp<SkImageFilter> imageIF(SkImageFilters::Image(std::move(checkerboard),
                                                       SkFilterMode::kLinear));

    sk_sp<SkImageFilter> displacementIF;

    displacementIF = SkImageFilters::DisplacementMap(SkColorChannel::kR,
                                                     SkColorChannel::kB,
                                                     /* scale= */ 2.0f,
                                                     /* displacement= */ std::move(imageIF),
                                                     /* color= */ nullptr);
    sk_sp<PrecompileImageFilter> option =
            PrecompileImageFilters::DisplacementMap(/* input= */ nullptr);

    return { std::move(displacementIF), std::move(option) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> colorfilter_imagefilter(
        SkRandom* rand) {

    auto [cf, o] = create_random_colorfilter(rand);

    sk_sp<SkImageFilter> inputIF;
    sk_sp<PrecompileImageFilter> inputO;
    if (rand->nextBool()) {
        // Exercise color filter collapsing in the factories
        auto [cf2, o2] = create_random_colorfilter(rand);
        inputIF = SkImageFilters::ColorFilter(std::move(cf2), /* input= */ nullptr);
        inputO = PrecompileImageFilters::ColorFilter(std::move(o2), /* input= */ nullptr);
    }

    sk_sp<SkImageFilter> cfIF = SkImageFilters::ColorFilter(std::move(cf), std::move(inputIF));
    sk_sp<PrecompileImageFilter> cfIFO = PrecompileImageFilters::ColorFilter(std::move(o),
                                                                             std::move(inputO));

    return { std::move(cfIF), std::move(cfIFO) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> lighting_imagefilter(
        SkRandom* rand) {
    static constexpr SkPoint3 kLocation{10.0f, 2.0f, 30.0f};
    static constexpr SkPoint3 kTarget{0, 0, 0};
    static constexpr SkPoint3 kDirection{0, 1, 0};

    sk_sp<SkImageFilter> lightingIF;

    int option = rand->nextULessThan(6);
    switch (option) {
        case 0:
            lightingIF = SkImageFilters::DistantLitDiffuse(kDirection, SK_ColorRED,
                                                           /* surfaceScale= */ 1.0f,
                                                           /* kd= */ 0.5f,
                                                           /* input= */ nullptr);
            break;
        case 1:
            lightingIF = SkImageFilters::PointLitDiffuse(kLocation, SK_ColorGREEN,
                                                         /* surfaceScale= */ 1.0f,
                                                         /* kd= */ 0.5f,
                                                         /* input= */ nullptr);
            break;
        case 2:
            lightingIF = SkImageFilters::SpotLitDiffuse(kLocation, kTarget,
                                                        /* falloffExponent= */ 2.0f,
                                                        /* cutoffAngle= */ 30.0f,
                                                        SK_ColorBLUE,
                                                        /* surfaceScale= */ 1.0f,
                                                        /* kd= */ 0.5f,
                                                        /* input= */ nullptr);
            break;
        case 3:
            lightingIF = SkImageFilters::DistantLitSpecular(kDirection, SK_ColorCYAN,
                                                            /* surfaceScale= */ 1.0f,
                                                            /* ks= */ 0.5f,
                                                            /* shininess= */ 2.0f,
                                                            /* input= */ nullptr);
            break;
        case 4:
            lightingIF = SkImageFilters::PointLitSpecular(kLocation, SK_ColorMAGENTA,
                                                          /* surfaceScale= */ 1.0f,
                                                          /* ks= */ 0.5f,
                                                          /* shininess= */ 2.0f,
                                                          /* input= */ nullptr);
            break;
        case 5:
            lightingIF = SkImageFilters::SpotLitSpecular(kLocation, kTarget,
                                                         /* falloffExponent= */ 2.0f,
                                                         /* cutoffAngle= */ 30.0f,
                                                         SK_ColorYELLOW,
                                                         /* surfaceScale= */ 1.0f,
                                                         /* ks= */ 4.0f,
                                                         /* shininess= */ 0.5f,
                                                         /* input= */ nullptr);
            break;
    }

    sk_sp<PrecompileImageFilter> lightingO = PrecompileImageFilters::Lighting(/* input= */ nullptr);
    return { std::move(lightingIF), std::move(lightingO) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>>  matrix_convolution_imagefilter(
        SkRandom* rand) {

    int kernelSize = 1;

    int option = rand->nextULessThan(3);
    switch (option) {
        case 0: kernelSize = 3;  break;
        case 1: kernelSize = 7;  break;
        case 2: kernelSize = 11; break;
    }

    int center = (kernelSize * kernelSize - 1) / 2;
    std::vector<float> kernel(kernelSize * kernelSize, SkIntToScalar(1));
    kernel[center] = 2.0f - kernelSize * kernelSize;

    sk_sp<SkImageFilter> matrixConvIF;
    matrixConvIF = SkImageFilters::MatrixConvolution({ kernelSize, kernelSize },
                                                     /* kernel= */ kernel.data(),
                                                     /* gain= */ 0.3f,
                                                     /* bias= */ 100.0f,
                                                     /* kernelOffset= */ { 1, 1 },
                                                     SkTileMode::kMirror,
                                                     /* convolveAlpha= */ false,
                                                     /* input= */ nullptr);
    SkASSERT(matrixConvIF);
    sk_sp<PrecompileImageFilter> convOption = PrecompileImageFilters::MatrixConvolution(
            /* input= */ nullptr);

    return { std::move(matrixConvIF), std::move(convOption) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> morphology_imagefilter(
        SkRandom* rand) {
    static constexpr float kRadX = 2.0f, kRadY = 4.0f;

    sk_sp<SkImageFilter> morphologyIF;

    if (rand->nextBool()) {
        morphologyIF = SkImageFilters::Erode(kRadX, kRadY, /* input= */ nullptr);
    } else {
        morphologyIF = SkImageFilters::Dilate(kRadX, kRadY, /* input= */ nullptr);
    }
    SkASSERT(morphologyIF);
    sk_sp<PrecompileImageFilter> option = PrecompileImageFilters::Morphology(/* input= */ nullptr);

    return { std::move(morphologyIF), std::move(option) };
}

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> create_image_filter(
        Recorder* recorder,
        SkRandom* rand,
        ImageFilterType type) {

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

std::pair<sk_sp<SkImageFilter>, sk_sp<PrecompileImageFilter>> create_random_image_filter(
        Recorder* recorder,
        SkRandom* rand) {
    return create_image_filter(recorder, rand, random_imagefiltertype(rand));
}

std::pair<sk_sp<SkMaskFilter>, sk_sp<PrecompileMaskFilter>> create_blur_maskfilter(SkRandom* rand) {
    SkBlurStyle style;
    switch (rand->nextULessThan(4)) {
        case 0:  style = kNormal_SkBlurStyle; break;
        case 1:  style = kSolid_SkBlurStyle;  break;
        case 2:  style = kOuter_SkBlurStyle;  break;
        case 3:  [[fallthrough]];
        default: style = kInner_SkBlurStyle;  break;
    }

    float sigma = 1.0f;
    switch (rand->nextULessThan(2)) {
        case 0:  sigma = 1.0f; break;
        case 1:  sigma = 2.0f; break;
        case 2:  [[fallthrough]];
        default: sigma = 5.0f; break;
    }

    bool respectCTM = rand->nextBool();

    return { SkMaskFilter::MakeBlur(style, sigma, respectCTM), PrecompileMaskFilters::Blur() };
}

std::pair<sk_sp<SkMaskFilter>, sk_sp<PrecompileMaskFilter>> create_maskfilter(SkRandom* rand,
                                                                              MaskFilterType type) {
    switch (type) {
        case MaskFilterType::kNone: return {nullptr, nullptr};
        case MaskFilterType::kBlur: return create_blur_maskfilter(rand);
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
std::pair<SkPaint, PaintOptions> create_paint(SkRandom* rand,
                                              Recorder* recorder,
                                              ShaderType shaderType,
                                              BlenderType blenderType,
                                              ColorFilterType colorFilterType,
                                              MaskFilterType maskFilterType,
                                              ImageFilterType imageFilterType) {
    SkColor paintColor = random_color(rand, ColorConstraint::kNone);

    SkPaint paint;
    paint.setColor(paintColor);

    PaintOptions paintOptions;
    paintOptions.setPaintColorIsOpaque(SkColorGetA(paintColor) == 0xFF);

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
        auto [mf, o] = create_maskfilter(rand, maskFilterType);
        SkASSERT(!mf == !o);

        if (mf) {
            paint.setMaskFilter(std::move(mf));
            paintOptions.setMaskFilters({o});
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
        auto [filter, o] = create_image_filter(recorder, rand, imageFilterType);
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

    return { paint, paintOptions };
}

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
    DrawData() {
        static constexpr int kMaskTextFontSize = 16;
        // A large font size can bump text to be drawn as a path.
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
        constexpr SkPoint kPositions[kNumVerts] { { 0, 0 }, { 0, 16 }, { 16, 16 }, { 16, 0 } };
        constexpr SkColor kColors[kNumVerts] = { SK_ColorBLUE, SK_ColorGREEN,
                                                 SK_ColorCYAN, SK_ColorYELLOW };

        fPath = make_path();
        fBlob = SkTextBlob::MakeFromText(text, strlen(text), font);
        fLCDBlob = SkTextBlob::MakeFromText(text, strlen(text), lcdFont);
        fEmojiBlob = SkTextBlob::MakeFromText(emojiTestSample.sampleText,
                                              strlen(emojiTestSample.sampleText),
                                              emojiFont);
        fPathBlob = SkTextBlob::MakeFromText(text, strlen(text), pathFont);

        fVertsWithColors = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, kNumVerts,
                                                kPositions, kPositions, kColors);
        fVertsWithOutColors = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, kNumVerts,
                                                   kPositions, kPositions, /* colors= */ nullptr);
    }

    SkPath fPath;
    sk_sp<SkTextBlob> fBlob;
    sk_sp<SkTextBlob> fLCDBlob;
    sk_sp<SkTextBlob> fEmojiBlob;
    sk_sp<SkTextBlob> fPathBlob;
    sk_sp<SkVertices> fVertsWithColors;
    sk_sp<SkVertices> fVertsWithOutColors;
};

void non_simple_draws(SkCanvas* canvas, const SkPaint& paint, const DrawData& drawData) {
    // TODO: add strokeAndFill draws here as well as a stroked non-circular rrect draw
    canvas->drawPath(drawData.fPath, paint);
    canvas->drawTextBlob(drawData.fPathBlob, 0, 16, paint);
    if (paint.getStyle() == SkPaint::kStroke_Style) {
        canvas->drawArc({0, 0, 16, 16}, 0, 90, /* useCenter= */ true, paint);
    }
}

#ifdef SK_DEBUG
void dump_keys(PrecompileContext* precompileContext,
               const std::vector<skgpu::UniqueKey>& needleKeys,
               const std::vector<skgpu::UniqueKey>& hayStackKeys,
               const char* needleName,
               const char* haystackName) {

    SkDebugf("-------------------------- %zu %s pipelines\n", needleKeys.size(), needleName);

    int count = 0;
    for (const skgpu::UniqueKey& k : needleKeys) {
        bool found = std::find(hayStackKeys.begin(), hayStackKeys.end(), k) != hayStackKeys.end();

        GraphicsPipelineDesc originalPipelineDesc;
        RenderPassDesc originalRenderPassDesc;
        UniqueKeyUtils::ExtractKeyDescs(precompileContext, k,
                                        &originalPipelineDesc,
                                        &originalRenderPassDesc);

        SkString label;
        label.appendf("--- %s key %d (%s in %s):\n",
                      needleName, count++, found ? "found" : "not-found", haystackName);
        k.dump(label.c_str());
        UniqueKeyUtils::DumpDescs(precompileContext,
                                  originalPipelineDesc,
                                  originalRenderPassDesc);
    }
}
#endif

void check_draw(skiatest::Reporter* reporter,
                Context* context,
                PrecompileContext* precompileContext,
                skiatest::graphite::GraphiteTestContext* testContext,
                Recorder* recorder,
                const SkPaint& paint,
                DrawTypeFlags dt,
                ClipType clipType,
                sk_sp<SkShader> clipShader) {
    static const DrawData kDrawData;

    std::vector<skgpu::UniqueKey> precompileKeys, drawKeys;

    UniqueKeyUtils::FetchUniqueKeys(precompileContext, &precompileKeys);

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    {
        // TODO: vary the colorType of the target surface too
        SkImageInfo ii = SkImageInfo::Make(16, 16,
                                           kBGRA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        SkSurfaceProps props;

        if (dt == DrawTypeFlags::kBitmapText_LCD || dt == DrawTypeFlags::kSDFText_LCD) {
            props = SkSurfaceProps(/* flags= */ 0x0, SkPixelGeometry::kRGB_H_SkPixelGeometry);
        }

        sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder, ii,
                                                         skgpu::Mipmapped::kNo,
                                                         &props);
        SkCanvas* canvas = surf->getCanvas();

        // NOTE: The specific coordinates for the clip[R]Rect and draw[R]Rect calls are chosen to
        // avoid geometrically combining the clip into the geometry, and to avoid covering the
        // render target entirely, both of which would simplify the pipeline required.

        switch (clipType) {
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
            case ClipType::kAnalytic:
                canvas->clipRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(1, 1, 15, 15), 5, 5));
                break;
            case ClipType::kAnalyticAndShader:
                SkASSERT(clipShader);
                canvas->clipRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(1, 1, 15, 15), 5, 5));
                canvas->clipShader(clipShader, SkClipOp::kIntersect);
                break;
        }

        switch (dt) {
            case DrawTypeFlags::kBitmapText_Mask:
                canvas->drawTextBlob(kDrawData.fBlob, 0, 16, paint);
                break;
            case DrawTypeFlags::kBitmapText_LCD:
                canvas->drawTextBlob(kDrawData.fLCDBlob, 0, 16, paint);
                break;
            case DrawTypeFlags::kBitmapText_Color:
                canvas->drawTextBlob(kDrawData.fEmojiBlob, 0, 16, paint);
                break;
            case DrawTypeFlags::kSDFText: {
                SkMatrix perspective;
                perspective.setPerspX(0.01f);
                perspective.setPerspY(0.001f);
                canvas->save();
                canvas->concat(perspective);
                canvas->drawTextBlob(kDrawData.fBlob, 0, 16, paint);
                canvas->restore();
            } break;
            case DrawTypeFlags::kSDFText_LCD: {
                SkMatrix perspective;
                perspective.setPerspX(0.01f);
                perspective.setPerspY(0.001f);
                canvas->save();
                canvas->concat(perspective);
                canvas->drawTextBlob(kDrawData.fLCDBlob, 0, 16, paint);
                canvas->restore();
            } break;
            case DrawTypeFlags::kDrawVertices:
                canvas->drawVertices(kDrawData.fVertsWithColors, SkBlendMode::kDst, paint);
                canvas->drawVertices(kDrawData.fVertsWithOutColors, SkBlendMode::kDst, paint);
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
                // TODO: add a case that uses the SkCanvas::experimental_DrawEdgeAAImageSet
                //  entry point
                if (!paint.getShader() &&
                    !paint.getColorFilter() &&
                    !paint.getImageFilter() &&
                    paint.asBlendMode().has_value()) {
                    // The SkPaint reconstructed inside the drawEdgeAAQuad call needs to match
                    // 'paint' for the precompilation checks to work.
                    canvas->experimental_DrawEdgeAAQuad(SkRect::MakeWH(15, 15),
                                                        /* clip= */ nullptr,
                                                        SkCanvas::kAll_QuadAAFlags,
                                                        paint.getColor4f(),
                                                        paint.asBlendMode().value());
                }
                break;
            case DrawTypeFlags::kNonAAFillRect:
                canvas->drawRect(SkRect::MakeWH(15, 15), paint);
                break;
            case DrawTypeFlags::kNonSimpleShape:
                non_simple_draws(canvas, paint, kDrawData);
                break;
            default:
                SkASSERT(false);
                break;
        }

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
        testContext->syncedSubmit(context);
    }

    UniqueKeyUtils::FetchUniqueKeys(precompileContext, &drawKeys);

    // Actually using the SkPaint with the specified type of draw shouldn't have added
    // any additional pipelines
    int missingPipelines = 0;
    for (const skgpu::UniqueKey& k : drawKeys) {
        bool found =
                std::find(precompileKeys.begin(), precompileKeys.end(), k) != precompileKeys.end();
        if (!found) {
            ++missingPipelines;
        }
    }

    REPORTER_ASSERT(reporter, missingPipelines == 0,
                    "precompile pipelines: %zu draw pipelines: %zu - %d missing from precompile",
                    precompileKeys.size(), drawKeys.size(), missingPipelines);
#ifdef SK_DEBUG
    if (missingPipelines) {
        dump_keys(precompileContext, drawKeys, precompileKeys, "draw", "precompile");
        dump_keys(precompileContext, precompileKeys, drawKeys, "precompile", "draw");
    }
#endif // SK_DEBUG

}

// This subtest compares the output of paintParams.toKey() (applied to an SkPaint) and
// PaintOptions::buildCombinations (applied to a matching PaintOptions). The actual check
// performed is that the UniquePaintParamsID created by paintParams.toKey() is contained in the
// set of IDs generated by buildCombinations.
[[maybe_unused]]
void extract_vs_build_subtest(skiatest::Reporter* reporter,
                              Context* context,
                              DrawContext* drawContext,
                              skiatest::graphite::GraphiteTestContext* /* testContext */,
                              const KeyContext& precompileKeyContext,
                              Recorder* recorder,
                              const SkPaint& paint,
                              const PaintOptions& paintOptions,
                              ShaderType s,
                              BlenderType bm,
                              ColorFilterType cf,
                              MaskFilterType mf,
                              ImageFilterType imageFilter,
                              ClipType clipType,
                              sk_sp<SkShader> clipShader,
                              DrawTypeFlags dt,
                              uint32_t seed,
                              SkRandom* rand,
                              bool verbose) {
    PipelineDataGatherer paramsGatherer(Layout::kMetal);

    for (bool withPrimitiveBlender: {false, true}) {

        sk_sp<SkBlender> primitiveBlender;
        if (withPrimitiveBlender) {
            if (dt != DrawTypeFlags::kDrawVertices) {
                // Only drawVertices calls need a primitive blender
                continue;
            }

            primitiveBlender = SkBlender::Mode(SkBlendMode::kSrcOver);
        }

        constexpr Coverage coverageOptions[3] = {
                Coverage::kNone, Coverage::kSingleChannel, Coverage::kLCD
        };
        Coverage coverage = coverageOptions[rand->nextULessThan(3)];

        // In the normal API this modification happens in SkDevice::clipShader()
        // All clipShaders get wrapped in a CTMShader
        sk_sp<SkShader> modifiedClipShader = clipShader
                                             ? as_SB(clipShader)->makeWithCTM(SkMatrix::I())
                                             : nullptr;
        if (clipType == ClipType::kShader_Diff && modifiedClipShader) {
            // The CTMShader gets further wrapped in a ColorFilterShader for kDifference clips
            modifiedClipShader = modifiedClipShader->makeWithColorFilter(
                    SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrcOut));
        }

        bool hasAnalyticClip = clipType == ClipType::kAnalytic ||
                               clipType == ClipType::kAnalyticAndShader;
        NonMSAAClip clipData;
        if (hasAnalyticClip) {
            clipData.fAnalyticClip.fBounds = SkRect::MakeWH(15, 15);
            clipData.fAnalyticClip.fRadius = 5;
        }

        PaintParams paintParams{paint, primitiveBlender.get()};
        ShadingParams shadingParams{recorder->priv().caps(),
                                    paintParams,
                                    clipData,
                                    modifiedClipShader.get(),
                                    coverage,
                                    TextureFormat::kRGBA8};
        paramsGatherer.resetForDraw();
        KeyContext keyContext(recorder,
                              drawContext,
                              precompileKeyContext.floatStorageManager(),
                              precompileKeyContext.paintParamsKeyBuilder(),
                              &paramsGatherer,
                              {},
                              precompileKeyContext.dstColorInfo(),
                              KeyGenFlags::kDisableSamplingOptimization,
                              paintParams.color());
        auto keyResult = shadingParams.toKey(keyContext);
        UniquePaintParamsID paintID = keyResult.has_value() ? std::get<0>(*keyResult)
                                                            : UniquePaintParamsID::Invalid();

        RenderPassDesc unusedRenderPassDesc;
        std::vector<UniquePaintParamsID> precompileIDs;
        paintOptions.priv().buildCombinations(precompileKeyContext,
                                              hasAnalyticClip ? DrawTypeFlags::kAnalyticClip
                                                              : DrawTypeFlags::kNone,
                                              withPrimitiveBlender,
                                              coverage,
                                              unusedRenderPassDesc,
                                              [&precompileIDs](UniquePaintParamsID id,
                                                               DrawTypeFlags,
                                                               bool /* withPrimitiveBlender */,
                                                               Coverage,
                                                               const RenderPassDesc&) {
                                                  precompileIDs.push_back(id);
                                              });

        if (verbose) {
            SkDebugf("Precompilation generated %zu unique keys\n", precompileIDs.size());
        }

        // Although we've gathered both sets of uniforms (i.e., from the paint
        // params and the precompilation paths) we can't compare the two since the
        // precompilation path may have generated multiple sets
        // and the last one created may not be the one that matches the paint
        // params' set. Additionally, for runtime effects we just skip gathering
        // the uniforms in the precompilation path.

        // The specific key generated by paintParams.toKey() should be one of the
        // combinations generated by the combination system.
        auto result = std::find(precompileIDs.begin(), precompileIDs.end(), paintID);

        if (result == precompileIDs.end()) {
            log_run("Failure on case", seed, s, bm, cf, mf, imageFilter, clipType, dt);
        }

#ifdef SK_DEBUG
        if (result == precompileIDs.end()) {
            SkDebugf("From paint: ");
            precompileKeyContext.dict()->dump(precompileKeyContext.caps(), paintID);

            SkDebugf("From combination builder [%d]:", static_cast<int>(precompileIDs.size()));
            for (auto iter: precompileIDs) {
                precompileKeyContext.dict()->dump(precompileKeyContext.caps(), iter);
            }
        }
#endif

        REPORTER_ASSERT(reporter, result != precompileIDs.end());
    }
}

// This subtest verifies that, given an equivalent SkPaint and PaintOptions, the
// Precompile system will, at least, generate all the pipelines a real draw would generate.
void precompile_vs_real_draws_subtest(skiatest::Reporter* reporter,
                                      Context* context,
                                      PrecompileContext* precompileContext,
                                      skiatest::graphite::GraphiteTestContext* testContext,
                                      Recorder* recorder,
                                      const SkPaint& paint,
                                      const PaintOptions& paintOptions,
                                      ClipType clipType,
                                      sk_sp<SkShader> clipShader,
                                      DrawTypeFlags dt,
                                      bool /* verbose */) {
    GlobalCache* globalCache = precompileContext->priv().globalCache();

    globalCache->resetGraphicsPipelines();

    const skgpu::graphite::Caps* caps = context->priv().caps();

    const SkColorType kColorType = kBGRA_8888_SkColorType;

    static const RenderPassProperties kDepth_Stencil_4 { DepthStencilFlags::kDepthStencil,
                                                         kColorType,
                                                         /* dstColorSpace= */ nullptr,
                                                         /* requiresMSAA= */ true };
    static const RenderPassProperties kDepth_1 { DepthStencilFlags::kDepth,
                                                 kColorType,
                                                 /* dstColorSpace= */ nullptr,
                                                 /* requiresMSAA= */ false };

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 skgpu::Protected::kNo,
                                                                 skgpu::Renderable::kYes);

    const bool msaaSupported =
            caps->msaaRenderToSingleSampledSupport() ||
            caps->isSampleCountSupported(TextureInfoPriv::ViewFormat(textureInfo),
                                         caps->defaultMSAASamplesCount());

    bool vello = false;
#ifdef SK_ENABLE_VELLO_SHADERS
    vello = caps->computeSupport();
#endif

    // Using Vello skips using MSAA for complex paths. Additionally, Intel Macs avoid MSAA
    // in favor of path rendering.
    const RenderPassProperties* pathProperties = (msaaSupported && !vello) ? &kDepth_Stencil_4
                                                                           : &kDepth_1;

    DrawTypeFlags combinedDrawType = dt;
    if (clipType == ClipType::kAnalytic || clipType == ClipType::kAnalyticAndShader) {
        combinedDrawType = static_cast<DrawTypeFlags>(dt | DrawTypeFlags::kAnalyticClip);
    }

    int before = globalCache->numGraphicsPipelines();
    Precompile(precompileContext, paintOptions,
               combinedDrawType,
               dt == kNonSimpleShape ? SkSpan(pathProperties, 1) : SkSpan(&kDepth_1, 1));
    if (gNeedSKPPaintOption) {
        // The skp draws a rect w/ a default SkPaint and RGBA dst color type
        PaintOptions skpPaintOptions;
        Precompile(precompileContext, skpPaintOptions, DrawTypeFlags::kNonAAFillRect,
                   { { kDepth_1.fDSFlags, kRGBA_8888_SkColorType, kDepth_1.fDstCS,
                       kDepth_1.fRequiresMSAA } });
    }
    int after = globalCache->numGraphicsPipelines();

    REPORTER_ASSERT(reporter, before == 0);
    REPORTER_ASSERT(reporter, after > before);

    check_draw(reporter,
               context,
               precompileContext,
               testContext,
               recorder,
               paint,
               dt,
               clipType,
               clipShader);
}

void run_test(skiatest::Reporter* reporter,
              Context* context,
              DrawContext* drawContext,
              PrecompileContext* precompileContext,
              skiatest::graphite::GraphiteTestContext* testContext,
              const KeyContext& precompileKeyContext,
              ShaderType s,
              BlenderType bm,
              ColorFilterType cf,
              MaskFilterType mf,
              ImageFilterType imageFilter,
              ClipType clipType,
              DrawTypeFlags dt,
              uint32_t seed,
              bool verbose) {
    SkRandom rand(seed);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    sk_sp<SkShader> clipShader;
    sk_sp<PrecompileShader> clipShaderOption;

    if (clipType == ClipType::kShader ||
        clipType == ClipType::kShader_Diff ||
        clipType == ClipType::kAnalyticAndShader) {
        std::tie(clipShader, clipShaderOption) = create_clip_shader(&rand, recorder.get());
        SkASSERT(!clipShader == !clipShaderOption);
    }

    gNeedSKPPaintOption = false;
    auto [paint, paintOptions] = create_paint(&rand, recorder.get(), s, bm, cf, mf, imageFilter);

    // The PaintOptions' clipShader can be handled here while the SkPaint's clipShader handling
    // must be performed later (in paintParams.toKey() or when an SkCanvas is accessible for
    // a SkCanvas::clipShader call).
    paintOptions.priv().setClipShaders({clipShaderOption});

    extract_vs_build_subtest(reporter, context, drawContext, testContext, precompileKeyContext,
                             recorder.get(), paint, paintOptions, s, bm, cf, mf, imageFilter,
                             clipType, clipShader, dt, seed, &rand, verbose);
    precompile_vs_real_draws_subtest(reporter, context, precompileContext,
                                     testContext, recorder.get(),
                                     paint, paintOptions, clipType, clipShader, dt, verbose);
}

} // anonymous namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PaintParamsKeyTestReduced,
                                               reporter,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kNever) {
    const skgpu::graphite::Caps* caps = context->priv().caps();
    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    // Currently, we just use this as a valid parameter for keyContext (will hit asserts otherwise)
    sk_sp<DrawContext> precompileDrawContext = get_precompile_draw_context(caps, context);

    FloatStorageManager floatStorageManager;
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer gatherer(Layout::kMetal);
    sk_sp<RuntimeEffectDictionary> rtDict = sk_make_sp<RuntimeEffectDictionary>();
    KeyContext keyContext(caps,
                          &floatStorageManager,
                          &builder,
                          &gatherer,
                          dict,
                          rtDict,
                          SkColorInfo(kRGBA_8888_SkColorType,
                                      kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB()));

#if 1
    //----------------------
    uint32_t seed = std::time(nullptr) % std::numeric_limits<uint32_t>::max();
    SkRandom rand(seed);
    ShaderType shaderType = random_shadertype(&rand);
    BlenderType blenderType = random_blendertype(&rand);
    ColorFilterType colorFilterType = random_colorfiltertype(&rand);
    MaskFilterType maskFilterType = random_maskfiltertype(&rand);
    ImageFilterType imageFilterType = ImageFilterType::kNone; // random_imagefiltertype(&rand);
    ClipType clipType = random_cliptype(&rand);
    DrawTypeFlags drawTypeFlags = random_drawtype(&rand);
    //----------------------
#else
    //------------------------
    uint32_t seed = 0;
    ShaderType shaderType = ShaderType::kYUVImage;
    BlenderType blenderType = BlenderType::kPorterDuff;
    ColorFilterType colorFilterType = ColorFilterType::kNone;
    MaskFilterType maskFilterType = MaskFilterType::kNone;
    ImageFilterType imageFilterType = ImageFilterType::kNone;
    ClipType clipType = ClipType::kNone;
    DrawTypeFlags drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
    //-----------------------
#endif

    SkString logMsg("Running ");
    logMsg += BackendApiToStr(context->backend());

    log_run(logMsg.c_str(), seed, shaderType, blenderType, colorFilterType, maskFilterType,
            imageFilterType, clipType, drawTypeFlags);

    run_test(reporter,
             context,
             precompileDrawContext.get(),
             precompileContext.get(),
             testContext,
             keyContext,
             shaderType,
             blenderType,
             colorFilterType,
             maskFilterType,
             imageFilterType,
             clipType,
             drawTypeFlags,
             seed,
             /* verbose= */ true);
}

// This is intended to be a smoke test for the agreement between the two ways of creating a
// PaintParamsKey:
//    via paintParams.toKey() (i.e., from an SkPaint)
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
    const skgpu::graphite::Caps* caps = context->priv().caps();
    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    // Currently, we just use this as a valid parameter for keyContext (will hit asserts otherwise)
    sk_sp<DrawContext> precompileDrawContext = get_precompile_draw_context(caps, context);

    FloatStorageManager floatStorageManager;
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer gatherer(Layout::kMetal);
    sk_sp<RuntimeEffectDictionary> rtDict = sk_make_sp<RuntimeEffectDictionary>();
    KeyContext precompileKeyContext(caps,
                                    &floatStorageManager,
                                    &builder,
                                    &gatherer,
                                    dict,
                                    rtDict,
                                    SkColorInfo(kRGBA_8888_SkColorType,
                                                kPremul_SkAlphaType,
                                                SkColorSpace::MakeSRGB()));

    ShaderType shaders[] = {
            ShaderType::kImage,
            ShaderType::kRadialGradient,
            ShaderType::kSolidColor,
            ShaderType::kYUVImage,
#if EXPANDED_SET
            ShaderType::kNone,
            ShaderType::kBlend,
            ShaderType::kColorFilter,
            ShaderType::kCoordClamp,
            ShaderType::kConicalGradient,
            ShaderType::kLinearGradient,
            ShaderType::kLocalMatrix,
            ShaderType::kPerlinNoise,
            ShaderType::kPicture,
            ShaderType::kRuntime,
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
            ColorFilterType::kBlendMode,
            ColorFilterType::kMatrix,
#if EXPANDED_SET
            ColorFilterType::kNone,
            ColorFilterType::kColorSpaceXform,
            ColorFilterType::kCompose,
            ColorFilterType::kGaussian,
            ColorFilterType::kHighContrast,
            ColorFilterType::kHSLAMatrix,
            ColorFilterType::kLerp,
            ColorFilterType::kLighting,
            ColorFilterType::kLinearToSRGB,
            ColorFilterType::kLuma,
            ColorFilterType::kOverdraw,
            ColorFilterType::kRuntime,
            ColorFilterType::kSRGBToLinear,
            ColorFilterType::kTable,
            ColorFilterType::kWorkingFormat,
#endif
    };

    MaskFilterType maskFilters[] = {
            MaskFilterType::kNone,
#if EXPANDED_SET
            MaskFilterType::kBlur,
#endif
    };

    ImageFilterType imageFilters[] = {
            ImageFilterType::kNone,
#if EXPANDED_SET
            ImageFilterType::kArithmetic,
            ImageFilterType::kBlendMode,
            ImageFilterType::kRuntimeBlender,
            ImageFilterType::kBlur,
            ImageFilterType::kColorFilter,
            ImageFilterType::kDisplacement,
            ImageFilterType::kLighting,
            ImageFilterType::kMatrixConvolution,
            ImageFilterType::kMorphology,
#endif
    };

    ClipType clips[] = {
            ClipType::kNone,
            ClipType::kAnalytic,
#if EXPANDED_SET
            ClipType::kShader,        // w/ a SkClipOp::kIntersect
            ClipType::kShader_Diff,   // w/ a SkClipOp::kDifference
            ClipType::kAnalyticAndShader, // w/ a SkClipOp::kIntersect
#endif
    };

    static const DrawTypeFlags kDrawTypeFlags[] = {
            DrawTypeFlags::kBitmapText_Mask,
            DrawTypeFlags::kBitmapText_LCD,
            DrawTypeFlags::kBitmapText_Color,
            DrawTypeFlags::kSDFText,
            DrawTypeFlags::kSDFText_LCD,
            DrawTypeFlags::kDrawVertices,
            DrawTypeFlags::kCircularArc,
            DrawTypeFlags::kAnalyticRRect,
            DrawTypeFlags::kPerEdgeAAQuad,
            DrawTypeFlags::kNonAAFillRect,
            DrawTypeFlags::kNonSimpleShape,
    };

#if EXPANDED_SET
    size_t kExpected = std::size(shaders) * std::size(blenders) * std::size(colorFilters) *
                       std::size(maskFilters) * std::size(imageFilters) * std::size(clips) *
                       std::size(kDrawTypeFlags);
    int current = 0;
#endif

    for (auto shader : shaders) {
        for (auto blender : blenders) {
            for (auto cf : colorFilters) {
                for (auto mf : maskFilters) {
                    for (auto imageFilter : imageFilters) {
                        for (auto clip : clips) {
                            for (DrawTypeFlags dt : kDrawTypeFlags) {
#if EXPANDED_SET
                                SkDebugf("%d/%zu\n", current, kExpected);
                                ++current;
#endif

                                run_test(reporter, context, precompileDrawContext.get(),
                                         precompileContext.get(),
                                         testContext, precompileKeyContext,
                                         shader, blender, cf, mf, imageFilter, clip, dt,
                                         kDefaultSeed, /* verbose= */ false);
                            }
                        }
                    }
                }
            }
        }
    }

#if EXPANDED_SET
    SkASSERT(current == (int) kExpected);
#endif
}

#endif // SK_GRAPHITE

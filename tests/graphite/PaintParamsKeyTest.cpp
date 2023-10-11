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
#include "include/core/SkShader.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/base/SkRandom.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PublicPrecompile.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom*, Recorder*);
std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom*);
std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(SkRandom*);

enum class ShaderType {
    kNone,
    kSolidColor,
    kLinearGradient,
    kRadialGradient,
    kSweepGradient,
    kConicalGradient,
    kLocalMatrix,
    kColorFilter,
    kImage,
    kBlend,

    kLast          = kBlend
};

static constexpr int kShaderTypeCount = static_cast<int>(ShaderType::kLast) + 1;

// TODO: do we need to add a separable category and/or a category for dstRead requiring blends?
enum class BlenderType {
    kNone,
    kPorterDuff,
    kShaderBased,
    kRuntime,

    kLast = kRuntime
};

static constexpr int kBlenderTypeCount = static_cast<int>(BlenderType::kLast) + 1;

enum class ColorFilterType {
    kNone,
    kBlend,
    kMatrix,
    kHSLAMatrix,
    // TODO: add more color filters

    kLast = kHSLAMatrix
};

static constexpr int kColorFilterTypeCount = static_cast<int>(ColorFilterType::kLast) + 1;

static constexpr skcms_TransferFunction gTransferFunctions[] = {
    SkNamedTransferFn::kSRGB,
    SkNamedTransferFn::k2Dot2,
    SkNamedTransferFn::kLinear,
    SkNamedTransferFn::kRec2020,
    SkNamedTransferFn::kPQ,
    SkNamedTransferFn::kHLG,
};

static constexpr int kTransferFunctionCount = std::size(gTransferFunctions);

static constexpr skcms_Matrix3x3 gGamuts[] = {
    SkNamedGamut::kSRGB,
    SkNamedGamut::kAdobeRGB,
    SkNamedGamut::kDisplayP3,
    SkNamedGamut::kRec2020,
    SkNamedGamut::kXYZ,
};

static constexpr int kGamutCount = std::size(gGamuts);

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
            return SkColorSpace::MakeRGB(
                    gTransferFunctions[rand->nextULessThan(kTransferFunctionCount)],
                    gGamuts[rand->nextULessThan(kGamutCount)]);
    }

    SkUNREACHABLE;
}


SkColor random_opaque_color(SkRandom* rand) {
    return 0xff000000 | rand->nextU();
}

SkColor4f random_color(SkRandom* rand) {
    SkColor4f result = { rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f),
                         rand->nextRangeF(0.0f, 1.0f) };

    if (rand->nextBool()) {
        result.fA = 1.0f;
    }

    return result;
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

sk_sp<SkImage> make_image(SkRandom* rand, Recorder* recorder) {
    // TODO: add alpha-only images too
    SkImageInfo info = SkImageInfo::Make(32, 32,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         random_colorspace(rand));

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorBLACK);

    sk_sp<SkImage> img = bitmap.asImage();

    // TODO: fuzz mipmappedness
    return SkImages::TextureFromImage(recorder, img, {false});
}

//--------------------------------------------------------------------------------------------------
std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_solid_shader(SkRandom* rand) {
    sk_sp<SkShader> s = SkShaders::Color(random_opaque_color(rand));
    sk_sp<PrecompileShader> o = PrecompileShaders::Color();

    return { s, o };
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_gradient_shader(
        SkRandom* rand,
        SkShaderBase::GradientType type) {
    // TODO: fuzz the gradient parameters - esp. the number of stops & hard stops
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    sk_sp<PrecompileShader> o;

    SkTileMode tm = random_tilemode(rand);

    switch (type) {
        case SkShaderBase::GradientType::kLinear:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm);
            o = PrecompileShaders::LinearGradient();
            break;
        case SkShaderBase::GradientType::kRadial:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm);
            o = PrecompileShaders::RadialGradient();
            break;
        case SkShaderBase::GradientType::kSweep:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            0, 359, 0, nullptr);
            o = PrecompileShaders::SweepGradient();
            break;
        case SkShaderBase::GradientType::kConical:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, tm);
            o = PrecompileShaders::TwoPointConicalGradient();
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

    SkMatrix tmp = SkMatrix::Scale(1.5f, 2.0f); // TODO: fuzz

    return { s->makeWithLocalMatrix(tmp), o->makeWithLocalMatrix() };
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

    sk_sp<SkShader> s = SkImageShader::Make(make_image(rand, recorder), tmX, tmY,
                                            SkSamplingOptions(), nullptr);
    sk_sp<PrecompileShader> o = PrecompileShaders::Image();

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

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>>  create_shader(SkRandom* rand,
                                                                   Recorder* recorder,
                                                                   ShaderType shaderType) {
    switch (shaderType) {
        case ShaderType::kNone:
            return { nullptr, nullptr };
        case ShaderType::kSolidColor:
            return create_solid_shader(rand);
        case ShaderType::kLinearGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kLinear);
        case ShaderType::kRadialGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kRadial);
        case ShaderType::kSweepGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kSweep);
        case ShaderType::kConicalGradient:
            return create_gradient_shader(rand, SkShaderBase::GradientType::kConical);
        case ShaderType::kLocalMatrix:
            return create_localmatrix_shader(rand, recorder);
        case ShaderType::kColorFilter:
            return create_colorfilter_shader(rand, recorder);
        case ShaderType::kImage:
            return create_image_shader(rand, recorder);
        case ShaderType::kBlend:
            return create_blend_shader(rand, recorder);
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> create_random_shader(SkRandom* rand,
                                                                         Recorder* recorder) {
    return create_shader(rand, recorder, random_shadertype(rand));
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
        case BlenderType::kRuntime:
            return create_rt_blender(rand);
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> create_random_blender(SkRandom* rand) {
    return create_blender(rand, random_blendertype(rand));
}

//--------------------------------------------------------------------------------------------------
std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_blend_colorfilter(
        SkRandom* rand) {

    sk_sp<SkColorFilter> cf;

    // SkColorFilters::Blend is clever and can weed out noop color filters. Loop until we get
    // a valid color filter.
    while (!cf) {
        cf = SkColorFilters::Blend(random_color(rand),
                                   random_colorspace(rand),
                                   random_blend_mode(rand));
    }

    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Blend();

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::Matrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::Matrix();

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_hsla_matrix_colorfilter() {
    sk_sp<SkColorFilter> cf = SkColorFilters::HSLAMatrix(
            SkColorMatrix::RGBtoYUV(SkYUVColorSpace::kJPEG_Full_SkYUVColorSpace));
    sk_sp<PrecompileColorFilter> o = PrecompileColorFilters::HSLAMatrix();

    return { cf, o };
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_colorfilter(
        SkRandom* rand,
        ColorFilterType type) {

    switch (type) {
        case ColorFilterType::kNone:
            return { nullptr, nullptr };
        case ColorFilterType::kBlend:
            return create_blend_colorfilter(rand);
        case ColorFilterType::kMatrix:
            return create_matrix_colorfilter();
        case ColorFilterType::kHSLAMatrix:
            return create_hsla_matrix_colorfilter();
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(
        SkRandom* rand) {
    return create_colorfilter(rand, random_colorfiltertype(rand));
}

//--------------------------------------------------------------------------------------------------
std::pair<SkPaint, PaintOptions> create_paint(SkRandom* rand,
                                              Recorder* recorder,
                                              ShaderType shaderType,
                                              BlenderType blenderType,
                                              ColorFilterType colorFilterType) {
    SkPaint paint;
    paint.setColor(random_opaque_color(rand));

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
    sk_sp<SkVertices> fVerts;
};

void check_draw(skiatest::Reporter* reporter,
                Context* context,
                Recorder* recorder,
                const SkPaint& paint,
                DrawTypeFlags dt,
                const DrawData& drawData) {
    int before = context->priv().globalCache()->numGraphicsPipelines();

    {
        // TODO: vary the colorType of the target surface too
        SkImageInfo ii = SkImageInfo::Make(16, 16,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder, ii);
        SkCanvas* canvas = surf->getCanvas();

        switch (dt) {
            case DrawTypeFlags::kShape:
                canvas->drawRect(SkRect::MakeWH(16, 16), paint);
                canvas->drawPath(drawData.fPath, paint);
                break;
            case DrawTypeFlags::kText:
                canvas->drawTextBlob(drawData.fBlob, 0, 16, paint);
                break;
            case DrawTypeFlags::kDrawVertices:
                canvas->drawVertices(drawData.fVerts, SkBlendMode::kDst, paint);
                break;
            default:
                SkASSERT(false);
                break;
        }

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
        context->submit(SyncToCpu::kYes);
    }

    int after = context->priv().globalCache()->numGraphicsPipelines();

    // Actually using the SkPaint with the specified type of draw shouldn't have caused
    // any additional compilation
    REPORTER_ASSERT(reporter, before == after);
}

} // anonymous namespace

// This is intended to be a smoke test for the agreement between the two ways of creating a
// PaintParamsKey:
//    via ExtractPaintData (i.e., from an SkPaint)
//    and via the pre-compilation system
//
// TODO: keep this as a smoke test but add a fuzzer that reuses all the helpers
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PaintParamsKeyTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    SkColorInfo ci = SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                 SkColorSpace::MakeSRGB());

    std::unique_ptr<RuntimeEffectDictionary> rtDict = std::make_unique<RuntimeEffectDictionary>();
    KeyContext precompileKeyContext(recorder->priv().caps(),
                                    dict,
                                    rtDict.get(),
                                    ci,
                                    /* dstTexture= */ nullptr,
                                    /* dstOffset= */ {0, 0});

    sk_sp<TextureProxy> fakeDstTexture = TextureProxy::Make(recorder->priv().caps(),
                                                            SkISize::Make(1, 1),
                                                            kRGBA_8888_SkColorType,
                                                            skgpu::Mipmapped::kNo,
                                                            skgpu::Protected::kNo,
                                                            skgpu::Renderable::kYes,
                                                            skgpu::Budgeted::kNo);
    constexpr SkIPoint fakeDstOffset = SkIPoint::Make(0, 0);

    SkFont font(ToolUtils::create_portable_typeface(), 16);
    const char text[] = "hambur";

    // TODO: add a drawVertices call w/o colors. That impacts whether the RenderSteps emit
    // a primitive color blender
    constexpr int kNumVerts = 4;
    constexpr SkPoint kPositions[kNumVerts] { {0,0}, {0,16}, {16,16}, {16,0} };
    constexpr SkColor kColors[kNumVerts] = { SK_ColorBLUE, SK_ColorGREEN,
                                             SK_ColorCYAN, SK_ColorYELLOW };

    DrawData drawData = {
            make_path(),
            SkTextBlob::MakeFromText(text, strlen(text), font),
            SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, kNumVerts,
                                 kPositions, kPositions, kColors),
    };

    SkRandom rand;

    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer paramsGatherer(Layout::kMetal);
    PipelineDataGatherer precompileGatherer(Layout::kMetal);

    for (auto s : { ShaderType::kSolidColor,
                    ShaderType::kRadialGradient,
                    ShaderType::kImage,
                    ShaderType::kBlend  }) {
        for (auto bm : { BlenderType::kPorterDuff,
                         BlenderType::kShaderBased,
                         BlenderType::kRuntime }) {
            for (auto cf : { ColorFilterType::kNone,
                             ColorFilterType::kBlend,
                             ColorFilterType::kMatrix }) {

                auto [paint, paintOptions] = create_paint(&rand, recorder.get(), s, bm, cf);

                for (auto dt : { DrawTypeFlags::kShape,
                                 DrawTypeFlags::kText,
                                 DrawTypeFlags::kDrawVertices }) {

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
                        sk_sp<TextureProxy> curDst = needsDstSample ? fakeDstTexture : nullptr;

                        auto [paintID, uData, tData] = ExtractPaintData(
                                recorder.get(), &paramsGatherer, &builder, Layout::kMetal, {},
                                PaintParams(paint,
                                            std::move(primitiveBlender),
                                            dstReadReq,
                                            /* skipColorXform= */ false),
                                curDst, fakeDstOffset, ci);

                        std::vector<UniquePaintParamsID> precompileIDs;
                        paintOptions.priv().buildCombinations(precompileKeyContext,
                                                              &precompileGatherer,
                                                              withPrimitiveBlender,
                                                              coverage,
                                                              [&](UniquePaintParamsID id) {
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
                        auto result = std::find(precompileIDs.begin(), precompileIDs.end(),
                                                paintID);

#ifdef SK_DEBUG
                        if (result == precompileIDs.end()) {
                            SkDebugf("From paint: ");
                            dump(dict, paintID);

                            SkDebugf("From combination builder:");
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
                            int after = context->priv().globalCache()->numGraphicsPipelines();

                            REPORTER_ASSERT(reporter, before == 0);
                            REPORTER_ASSERT(reporter, after > before);

                            check_draw(reporter, context, recorder.get(), paint, dt, drawData);
                        }
                    }
                }
            }
        }
    }
}

#endif // SK_GRAPHITE

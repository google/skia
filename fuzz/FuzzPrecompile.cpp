/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkColorMatrix.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkBlenderBase.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PublicPrecompile.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/graphite/ContextFactory.h"

using namespace skgpu::graphite;

namespace {

SkBlendMode random_blend_mode(Fuzz* fuzz) {
    uint32_t temp;
    fuzz->next(&temp);
    return (SkBlendMode) (temp % kSkBlendModeCount);
}

SkColor random_opaque_skcolor(Fuzz* fuzz) {
    SkColor color;
    fuzz->next(&color);
    return 0xff000000 | color;
}

SkColor4f random_color4f(Fuzz* fuzz) {
    bool makeOpaque;
    fuzz->next(&makeOpaque);

    SkColor4f color;
    fuzz->nextRange(&color.fR, 0, 1);
    fuzz->nextRange(&color.fG, 0, 1);
    fuzz->nextRange(&color.fB, 0, 1);
    if (makeOpaque) {
        color.fA = 1.0;
    } else {
        fuzz->nextRange(&color.fA, 0, 1);
    }

    return color;
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

#ifdef SK_DEBUG
void dump(ShaderCodeDictionary* dict, UniquePaintParamsID id) {
    dict->lookup(id).dump(dict);
}
#endif

//--------------------------------------------------------------------------------------------------
// color spaces

const skcms_TransferFunction& random_transfer_function(Fuzz* fuzz) {
    static constexpr skcms_TransferFunction gTransferFunctions[] = {
            SkNamedTransferFn::kSRGB,
            SkNamedTransferFn::k2Dot2,
            SkNamedTransferFn::kLinear,
            SkNamedTransferFn::kRec2020,
            SkNamedTransferFn::kPQ,
            SkNamedTransferFn::kHLG,
    };

    uint32_t xferFunction;
    fuzz->next(&xferFunction);
    xferFunction %= std::size(gTransferFunctions);
    return gTransferFunctions[xferFunction];
}

const skcms_Matrix3x3& random_gamut(Fuzz* fuzz) {
    static constexpr skcms_Matrix3x3 gGamuts[] = {
            SkNamedGamut::kSRGB,
            SkNamedGamut::kAdobeRGB,
            SkNamedGamut::kDisplayP3,
            SkNamedGamut::kRec2020,
            SkNamedGamut::kXYZ,
    };

    uint32_t gamut;
    fuzz->next(&gamut);
    gamut %= std::size(gGamuts);
    return gGamuts[gamut];
}

enum class ColorSpaceType {
    kNone,
    kSRGB,
    kSRGBLinear,
    kRGB,

    kLast = kRGB
};

static constexpr int kColorSpaceTypeCount = static_cast<int>(ColorSpaceType::kLast) + 1;

sk_sp<SkColorSpace> create_colorspace(Fuzz* fuzz, ColorSpaceType csType) {
    switch (csType) {
        case ColorSpaceType::kNone:
            return nullptr;
        case ColorSpaceType::kSRGB:
            return SkColorSpace::MakeSRGB();
        case ColorSpaceType::kSRGBLinear:
            return SkColorSpace::MakeSRGBLinear();
        case ColorSpaceType::kRGB:
            return SkColorSpace::MakeRGB(random_transfer_function(fuzz), random_gamut(fuzz));
    }

    SkUNREACHABLE;
}

sk_sp<SkColorSpace> create_random_colorspace(Fuzz* fuzz) {
    uint32_t temp;
    fuzz->next(&temp);
    ColorSpaceType csType = (ColorSpaceType) (temp % kColorSpaceTypeCount);

    return create_colorspace(fuzz, csType);
}

//--------------------------------------------------------------------------------------------------
// color filters

enum class ColorFilterType {
    kNone,
    kBlend,
    kMatrix,
    kHSLAMatrix,
    // TODO: add more color filters

    kLast = kHSLAMatrix
};

static constexpr int kColorFilterTypeCount = static_cast<int>(ColorFilterType::kLast) + 1;

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_blend_colorfilter(
        Fuzz* fuzz) {

    sk_sp<SkColorFilter> cf;

    // SkColorFilters::Blend is clever and can weed out noop color filters. Loop until we get
    // a valid color filter.
    while (!cf && !fuzz->exhausted()) {
        cf = SkColorFilters::Blend(random_color4f(fuzz),
                                   create_random_colorspace(fuzz),
                                   random_blend_mode(fuzz));
    }

    sk_sp<PrecompileColorFilter> o = cf ? PrecompileColorFilters::Blend() : nullptr;

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
        Fuzz* fuzz,
        ColorFilterType type,
        int depth) {
    if (depth <= 0) {
        return {};
    }

    switch (type) {
        case ColorFilterType::kNone:
            return { nullptr, nullptr };
        case ColorFilterType::kBlend:
            return create_blend_colorfilter(fuzz);
        case ColorFilterType::kMatrix:
            return create_matrix_colorfilter();
        case ColorFilterType::kHSLAMatrix:
            return create_hsla_matrix_colorfilter();
    }

    SkUNREACHABLE;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> create_random_colorfilter(
        Fuzz* fuzz,
        int depth) {

    uint32_t temp;
    fuzz->next(&temp);
    ColorFilterType cf = (ColorFilterType) (temp % kColorFilterTypeCount);

    return create_colorfilter(fuzz, cf, depth);
}

//--------------------------------------------------------------------------------------------------
std::pair<SkPaint, PaintOptions> create_random_paint(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return {};
    }

    SkPaint paint;
    paint.setColor(random_opaque_skcolor(fuzz));

    PaintOptions paintOptions;

    {
        auto [cf, o] = create_random_colorfilter(fuzz, depth - 1);
        SkASSERT_RELEASE(!cf == !o);

        if (cf) {
            paint.setColorFilter(std::move(cf));
            paintOptions.setColorFilters({o});
        }
    }

    return { paint, paintOptions };
}

//--------------------------------------------------------------------------------------------------
void check_draw(Context* context,
                Recorder* recorder,
                const SkPaint& paint,
                DrawTypeFlags dt,
                const SkPath& path) {
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
                canvas->drawPath(path, paint);
                break;
            default:
                SkASSERT_RELEASE(false);
                break;
        }

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
        context->submit(SyncToCpu::kYes);
    }

    int after = context->priv().globalCache()->numGraphicsPipelines();

    // Actually using the SkPaint with the specified type of draw shouldn't have caused
    // any additional compilation
    SkASSERT_RELEASE(before == after);
}

void fuzz_graphite(Fuzz* fuzz, Context* context, int depth = 9) {
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

    DrawTypeFlags kDrawType = DrawTypeFlags::kShape;
    SkPath path = make_path();

    Layout layout = context->backend() == skgpu::BackendApi::kMetal ? Layout::kMetal
                                                                    : Layout::kStd140;

    PaintParamsKeyBuilder builder(dict);
    PipelineDataGatherer gatherer(layout);


    auto [paint, paintOptions] = create_random_paint(fuzz, depth);

    constexpr Coverage coverageOptions[3] = {
            Coverage::kNone, Coverage::kSingleChannel, Coverage::kLCD};
    uint32_t temp;
    fuzz->next(&temp);
    Coverage coverage = coverageOptions[temp % 3];

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
            recorder.get(), &gatherer, &builder, layout, {},
            PaintParams(paint,
                        /* primitiveBlender= */ nullptr,
                        dstReadReq,
                        /* skipColorXform= */ false),
            curDst, fakeDstOffset, ci);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(precompileKeyContext,
                                          /* addPrimitiveBlender= */ false,
                                          coverage,
                                          [&](UniquePaintParamsID id) {
                                              precompileIDs.push_back(id);
                                          });

    // The specific key generated by ExtractPaintData should be one of the
    // combinations generated by the combination system.
    auto result = std::find(precompileIDs.begin(), precompileIDs.end(), paintID);

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

    SkASSERT_RELEASE(result != precompileIDs.end());

    {
        context->priv().globalCache()->resetGraphicsPipelines();

        int before = context->priv().globalCache()->numGraphicsPipelines();
        Precompile(context, paintOptions, kDrawType);
        int after = context->priv().globalCache()->numGraphicsPipelines();

        SkASSERT_RELEASE(before == 0);
        SkASSERT_RELEASE(after > before);

        check_draw(context, recorder.get(), paint, kDrawType, path);
    }
}

} // anonymous namespace

DEF_FUZZ(Precompile, fuzz) {
    skiatest::graphite::ContextFactory factory;

    skgpu::ContextType contextType;
#if defined(SK_METAL)
    contextType = skgpu::ContextType::kMetal;
#elif defined(SK_VULKAN)
    contextType = skgpu::ContextType::kVulkan;
#else
    contextType = skgpu::ContextType::kMock;
#endif

    skiatest::graphite::ContextInfo ctxInfo = factory.getContextInfo(contextType);
    skgpu::graphite::Context* context = ctxInfo.fContext;
    if (!context) {
        return;
    }

    fuzz_graphite(fuzz, context);
}

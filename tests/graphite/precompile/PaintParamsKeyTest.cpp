/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/base/SkRandom.h"
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
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include "tests/graphite/precompile/PaintParamsTestUtils.h"

// Set this to 1 for more expansive (aka far slower) local testing
#define EXPANDED_SET 0

constexpr uint32_t kDefaultSeed = 0;

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

sk_sp<DrawContext> get_precompile_draw_context(const skgpu::graphite::Caps* caps,
                                               Context* context) {
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
    return DrawContext::Make(caps,
                             std::move(target),
                             drawSize,
                             colorInfo,
                             {});
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
             ToStr(s),
             ToStr(bm),
             ToStr(cf),
             ToStr(mf),
             ToStr(imageFilter),
             ToStr(clipType),
             ToStr(drawTypeFlags));
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

        ExecuteDraw(canvas, paint, kDrawData, dt);
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
                                      bool useSKPShader,
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

    const bool msaaSupported = caps->getCompatibleMSAASampleCount(textureInfo) > SampleCount::k1;

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
        combinedDrawType = static_cast<DrawTypeFlags>(
            static_cast<int>(dt) | static_cast<int>(DrawTypeFlags::kAnalyticClip));
    }

    int before = globalCache->numGraphicsPipelines();
    Precompile(precompileContext,
               paintOptions,
               combinedDrawType,
               dt == DrawTypeFlags::kNonSimpleShape ? SkSpan(pathProperties, 1)
                                                    : SkSpan(&kDepth_1, 1));
    if (useSKPShader) {
        // The skp draws a rect w/ a default SkPaint and RGBA dst color type
        PaintOptions skpPaintOptions;
        Precompile(precompileContext, skpPaintOptions, DrawTypeFlags::kNonAAFillRect,
                   {{ { kDepth_1.fDSFlags, kRGBA_8888_SkColorType, kDepth_1.fDstCS,
                       kDepth_1.fRequiresMSAA } }});
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
        std::tie(clipShader, clipShaderOption) = CreateClipShader(&rand, recorder.get());
        SkASSERT(!clipShader == !clipShaderOption);
    }

    bool reqSKP = false;
    auto [paint, paintOptions] =
            CreateRandomPaint(&rand, recorder.get(), s, bm, cf, mf, imageFilter, &reqSKP);

    // The PaintOptions' clipShader can be handled here while the SkPaint's clipShader handling
    // must be performed later (in paintParams.toKey() or when an SkCanvas is accessible for
    // a SkCanvas::clipShader call).
    paintOptions.priv().setClipShaders({{clipShaderOption}});

    extract_vs_build_subtest(reporter, context, drawContext, testContext, precompileKeyContext,
                             recorder.get(), paint, paintOptions, s, bm, cf, mf, imageFilter,
                             clipType, clipShader, dt, seed, &rand, verbose);
    precompile_vs_real_draws_subtest(reporter, context, precompileContext, testContext,
                                     recorder.get(), paint, paintOptions, clipType, clipShader, dt,
                                     reqSKP, verbose);
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
    ShaderType shaderType =
            static_cast<ShaderType>(rand.nextULessThan(static_cast<int>(ShaderType::kLast) + 1));
    BlenderType blenderType =
            static_cast<BlenderType>(rand.nextULessThan(static_cast<int>(BlenderType::kLast) + 1));
    ColorFilterType colorFilterType = static_cast<ColorFilterType>(
            rand.nextULessThan(static_cast<int>(ColorFilterType::kLast) + 1));
    MaskFilterType maskFilterType = MaskFilterType::kNone;
    if (rand.nextBool()) {
        maskFilterType = static_cast<MaskFilterType>(
                rand.nextULessThan(static_cast<int>(MaskFilterType::kLast) + 1));
    }
    ImageFilterType imageFilterType = ImageFilterType::kNone;  // random_imagefiltertype(&rand);
    ClipType clipType = ClipType::kNone;
    if (rand.nextBool()) {
        clipType = static_cast<ClipType>(rand.nextULessThan(static_cast<int>(ClipType::kLast) + 1));
    }
    DrawTypeFlags drawTypeFlags = RandomDrawType(&rand);
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

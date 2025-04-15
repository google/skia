/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace {

using namespace skgpu::graphite;

void compile(const RendererProvider* rendererProvider,
             ResourceProvider* resourceProvider,
             const KeyContext& keyContext,
             UniquePaintParamsID uniqueID,
             DrawTypeFlags drawTypes,
             const RenderPassDesc& renderPassDesc,
             bool withPrimitiveBlender,
             Coverage coverage) {
    for (const Renderer* r : rendererProvider->renderers()) {
        if (!(r->drawTypes() & drawTypes)) {
            continue;
        }

        if (r->emitsPrimitiveColor() != withPrimitiveBlender) {
            // UniqueIDs are explicitly built either w/ or w/o primitiveBlending so must
            // match what the Renderer requires
            continue;
        }

        if (r->coverage() != coverage) {
            // For now, UniqueIDs are explicitly built with a specific type of coverage so must
            // match what the Renderer requires
            continue;
        }

        for (auto&& s : r->steps()) {
            SkASSERT(!s->performsShading() || s->emitsPrimitiveColor() == withPrimitiveBlender);

            UniquePaintParamsID paintID = s->performsShading() ? uniqueID
                                                               : UniquePaintParamsID::Invalid();
            GraphicsPipelineDesc pipelineDesc(s->renderStepID(), paintID);

            sk_sp<GraphicsPipeline> pipeline = resourceProvider->findOrCreateGraphicsPipeline(
                    keyContext.rtEffectDict(),
                    pipelineDesc,
                    renderPassDesc,
                    PipelineCreationFlags::kForPrecompilation);
            if (!pipeline) {
                SKGPU_LOG_W("Failed to create GraphicsPipeline in precompile!");
                return;
            }
        }
    }
}

} // anonymous namespace

namespace skgpu::graphite {

void Precompile(PrecompileContext* precompileContext,
                const PaintOptions& options,
                DrawTypeFlags drawTypes,
                SkSpan<const RenderPassProperties> renderPassProperties) {

    ShaderCodeDictionary* dict = precompileContext->priv().shaderCodeDictionary();
    const Caps* caps = precompileContext->priv().caps();

    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>();

    for (const RenderPassProperties& rpp : renderPassProperties) {
        // TODO: Allow the client to pass in mipmapping and protection too?
        TextureInfo info = caps->getDefaultSampledTextureInfo(rpp.fDstCT,
                                                              Mipmapped::kNo,
                                                              Protected::kNo,
                                                              Renderable::kYes);

        Swizzle writeSwizzle = caps->getWriteSwizzle(rpp.fDstCT, info);

        // TODO(robertphillips): address mismatches between the MSAA requirements of the Renderers
        // associated w/ the requested drawTypes and the specified MSAA setting

        // On Native Metal, the LoadOp, StoreOp and clearColor fields don't influence
        // the actual RenderPassDescKey.
        // For Dawn, the LoadOp will sometimes matter. We add an extra LoadOp::kLoad combination
        // when necessary.
        const LoadOp kLoadOps[2] = { LoadOp::kClear, LoadOp::kLoad };

        int numLoadOps = 1;
        if (rpp.fRequiresMSAA &&
            !caps->msaaRenderToSingleSampledSupport() &&
            caps->loadOpAffectsMSAAPipelines()) {
            numLoadOps = 2;
        }

        for (int loadOpIndex = 0; loadOpIndex < numLoadOps; ++loadOpIndex) {
            const RenderPassDesc renderPassDesc =
                    RenderPassDesc::Make(caps,
                                         info,
                                         kLoadOps[loadOpIndex],
                                         StoreOp::kStore,
                                         rpp.fDSFlags,
                                         /* clearColor= */ { .0f, .0f, .0f, .0f },
                                         rpp.fRequiresMSAA,
                                         writeSwizzle,
                                         caps->getDstReadStrategy());

            SkColorInfo ci(rpp.fDstCT, kPremul_SkAlphaType, rpp.fDstCS);
            KeyContext keyContext(caps, dict, rtEffectDict.get(), ci);

            for (Coverage coverage : { Coverage::kNone, Coverage::kSingleChannel }) {
                PrecompileCombinations(
                        precompileContext->priv().rendererProvider(),
                        precompileContext->priv().resourceProvider(),
                        options, keyContext,
                        static_cast<DrawTypeFlags>(drawTypes & ~(DrawTypeFlags::kBitmapText_Color |
                                                                 DrawTypeFlags::kBitmapText_LCD |
                                                                 DrawTypeFlags::kSDFText_LCD |
                                                                 DrawTypeFlags::kDrawVertices)),
                        /* withPrimitiveBlender= */ false,
                        coverage,
                        renderPassDesc);
            }

            if (drawTypes & DrawTypeFlags::kBitmapText_Color) {
                // For color emoji text, shaders don't affect the final color
                PaintOptions tmp = options;
                tmp.setShaders({});

                // ARGB text doesn't emit coverage and always has a primitive blender
                PrecompileCombinations(precompileContext->priv().rendererProvider(),
                                       precompileContext->priv().resourceProvider(),
                                       tmp,
                                       keyContext,
                                       DrawTypeFlags::kBitmapText_Color,
                                       /* withPrimitiveBlender= */ true,
                                       Coverage::kNone,
                                       renderPassDesc);
            }

            if (drawTypes & (DrawTypeFlags::kBitmapText_LCD | DrawTypeFlags::kSDFText_LCD)) {
                // LCD-based text always emits LCD coverage but never has primitiveBlenders
                PrecompileCombinations(
                        precompileContext->priv().rendererProvider(),
                        precompileContext->priv().resourceProvider(),
                        options, keyContext,
                        static_cast<DrawTypeFlags>(drawTypes & (DrawTypeFlags::kBitmapText_LCD |
                                                                DrawTypeFlags::kSDFText_LCD)),
                        /* withPrimitiveBlender= */ false,
                        Coverage::kLCD,
                        renderPassDesc);
            }

            if (drawTypes & DrawTypeFlags::kDrawVertices) {
                // drawVertices w/ colors use a primitiveBlender while those w/o don't. It never
                // emits coverage.
                for (bool withPrimitiveBlender : { true, false }) {
                    PrecompileCombinations(precompileContext->priv().rendererProvider(),
                                           precompileContext->priv().resourceProvider(),
                                           options, keyContext,
                                           DrawTypeFlags::kDrawVertices,
                                           withPrimitiveBlender,
                                           Coverage::kNone,
                                           renderPassDesc);
                }
            }
        }
    }
}

void PrecompileCombinations(const RendererProvider* rendererProvider,
                            ResourceProvider* resourceProvider,
                            const PaintOptions& options,
                            const KeyContext& keyContext,
                            DrawTypeFlags drawTypes,
                            bool withPrimitiveBlender,
                            Coverage coverage,
                            const RenderPassDesc& renderPassDescIn) {
    if (drawTypes == DrawTypeFlags::kNone) {
        return;
    }

    // Since the precompilation path's uniforms aren't used and don't change the key,
    // the exact layout doesn't matter
    PipelineDataGatherer gatherer(Layout::kMetal);

    options.priv().buildCombinations(
        keyContext,
        &gatherer,
        drawTypes,
        withPrimitiveBlender,
        coverage,
        renderPassDescIn,
        [rendererProvider, resourceProvider, &keyContext](UniquePaintParamsID uniqueID,
                                                          DrawTypeFlags drawTypes,
                                                          bool withPrimitiveBlender,
                                                          Coverage coverage,
                                                          const RenderPassDesc& renderPassDesc) {
               compile(rendererProvider,
                       resourceProvider,
                       keyContext,
                       uniqueID,
                       drawTypes,
                       renderPassDesc,
                       withPrimitiveBlender,
                       coverage);
        });
}

} // namespace skgpu::graphite

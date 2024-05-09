/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PublicPrecompile.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace {

using namespace skgpu::graphite;

void compile(const RendererProvider* rendererProvider,
             ResourceProvider* resourceProvider,
             const KeyContext& keyContext,
             UniquePaintParamsID uniqueID,
             DrawTypeFlags drawTypes,
             SkSpan<const RenderPassDesc> renderPassDescs,
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
                                                               : UniquePaintParamsID::InvalidID();
            GraphicsPipelineDesc pipelineDesc(s, paintID);

            for (const RenderPassDesc& renderPassDesc : renderPassDescs) {
                sk_sp<GraphicsPipeline> pipeline = resourceProvider->findOrCreateGraphicsPipeline(
                        keyContext.rtEffectDict(),
                        pipelineDesc,
                        renderPassDesc);
                if (!pipeline) {
                    SKGPU_LOG_W("Failed to create GraphicsPipeline in precompile!");
                    return;
                }
            }
        }
    }
}

} // anonymous namespace

namespace skgpu::graphite {

bool Precompile(Context* context,
                RuntimeEffectDictionary* rteDict,
                const GraphicsPipelineDesc& pipelineDesc,
                const RenderPassDesc& renderPassDesc) {
    ResourceProvider* resourceProvider = context->priv().resourceProvider();

    sk_sp<GraphicsPipeline> pipeline = resourceProvider->findOrCreateGraphicsPipeline(
            rteDict,
            pipelineDesc,
            renderPassDesc);
    if (!pipeline) {
        SKGPU_LOG_W("Failed to create GraphicsPipeline in precompile!");
        return false;
    }

    return true;
}

void Precompile(Context* context, const PaintOptions& options, DrawTypeFlags drawTypes) {

    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();
    const Caps* caps = context->priv().caps();

    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>();

    SkColorInfo ci(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    KeyContext keyContext(
            caps, dict, rtEffectDict.get(), ci, /* dstTexture= */ nullptr, /* dstOffset= */ {0, 0});

    for (Coverage coverage : { Coverage::kNone, Coverage::kSingleChannel, Coverage::kLCD }) {
        PrecompileCombinations(
                context, options, keyContext,
                static_cast<DrawTypeFlags>(drawTypes & ~DrawTypeFlags::kDrawVertices),
                /* withPrimitiveBlender= */ false,
                coverage);
    }

    if (drawTypes & DrawTypeFlags::kDrawVertices) {
        for (Coverage coverage: { Coverage::kNone, Coverage::kSingleChannel, Coverage::kLCD }) {
            // drawVertices w/ colors use a primitiveBlender while those w/o don't
            for (bool withPrimitiveBlender : { true, false }) {
                PrecompileCombinations(context, options, keyContext,
                                       DrawTypeFlags::kDrawVertices,
                                       withPrimitiveBlender,
                                       coverage);
            }
        }
    }
}

void PrecompileCombinations(Context* context,
                            const PaintOptions& options,
                            const KeyContext& keyContext,
                            DrawTypeFlags drawTypes,
                            bool withPrimitiveBlender,
                            Coverage coverage) {
    const Caps* caps = keyContext.caps();
    // Since the precompilation path's uniforms aren't used and don't change the key,
    // the exact layout doesn't matter
    PipelineDataGatherer gatherer(caps, Layout::kMetal);

    SkColorType destCT = keyContext.dstColorInfo().colorType();
    // TODO: we need iterate over a broader set of TextureInfos here. Perhaps, allow the client
    // to pass in colorType, mipmapping and protection.
    TextureInfo info = caps->getDefaultSampledTextureInfo(destCT,
                                                          Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kYes);

    Swizzle writeSwizzle = caps->getWriteSwizzle(destCT, info);
    // Note: at least on Metal, the LoadOp, StoreOp and clearColor fields don't influence the
    // actual RenderPassDescKey.
    // TODO: if all of the Renderers associated w/ the requested drawTypes require MSAA we
    // do not need to generate the combinations w/ the non-MSAA RenderPassDescs.
    const RenderPassDesc renderPassDescs[] = {
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepth,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ true,
                             writeSwizzle),
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepthStencil,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ true,
                             writeSwizzle),
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepth,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ false,
                             writeSwizzle),
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepthStencil,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ false,
                             writeSwizzle),
    };

    options.priv().buildCombinations(
        keyContext,
        &gatherer,
        drawTypes,
        withPrimitiveBlender,
        coverage,
        [context, &keyContext, &renderPassDescs](UniquePaintParamsID uniqueID,
                                                 DrawTypeFlags drawTypes,
                                                 bool withPrimitiveBlender,
                                                 Coverage coverage) {
               compile(context->priv().rendererProvider(),
                       context->priv().resourceProvider(),
                       keyContext,
                       uniqueID,
                       drawTypes,
                       renderPassDescs,
                       withPrimitiveBlender,
                       coverage);
        });
}

} // namespace skgpu::graphite

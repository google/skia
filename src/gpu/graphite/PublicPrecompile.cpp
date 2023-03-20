/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PublicPrecompile.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
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
             SkSpan<RenderPassDesc> renderPassDescs,
             bool withPrimitiveBlender) {
    for (const Renderer* r : rendererProvider->renderers()) {
        if (!(r->drawTypes() & drawTypes)) {
            continue;
        }

        if (r->emitsPrimitiveColor() != withPrimitiveBlender) {
            // UniqueIDs are explicitly built either w/ or w/o primitiveBlending so must
            // match what the Renderer requires
            continue;
        }

        for (auto&& s : r->steps()) {
            SkASSERT(!s->performsShading() || s->emitsPrimitiveColor() == withPrimitiveBlender);

            UniquePaintParamsID paintID = s->performsShading() ? uniqueID
                                                               : UniquePaintParamsID::InvalidID();
            GraphicsPipelineDesc pipelineDesc(s, paintID);

            for (RenderPassDesc renderPassDesc : renderPassDescs) {
                auto pipeline = resourceProvider->findOrCreateGraphicsPipeline(
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

void Precompile(Context* context, const PaintOptions& options, DrawTypeFlags drawTypes) {

    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();
    const Caps* caps = context->priv().caps();

    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>();

    SkColorInfo ci(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    KeyContext keyContext(dict, rtEffectDict.get(), ci);

    // TODO: we need iterate over a broader set of TextureInfos here. Perhaps, allow the client
    // to pass in colorType, mipmapping and protection.
    TextureInfo info = caps->getDefaultSampledTextureInfo(ci.colorType(),
                                                          Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kYes);

    // Note: at least on Metal, the LoadOp, StoreOp and clearColor fields don't influence the
    // actual RenderPassDescKey.
    // TODO: if all of the Renderers associated w/ the requested drawTypes require MSAA we
    // do not need to generate the combinations w/ the non-MSAA RenderPassDescs.
    RenderPassDesc renderPassDescs[] = {
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepth,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ true),
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepthStencil,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ true),
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepth,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ false),
        RenderPassDesc::Make(caps,
                             info,
                             LoadOp::kClear,
                             StoreOp::kStore,
                             DepthStencilFlags::kDepthStencil,
                             /* clearColor= */ { .0f, .0f, .0f, .0f },
                             /* requiresMSAA= */ false),
    };

    options.priv().buildCombinations(
        keyContext,
        /* addPrimitiveBlender= */ false,
         [&](UniquePaintParamsID uniqueID) {
             compile(context->priv().rendererProvider(),
                     context->priv().resourceProvider(),
                     keyContext, uniqueID,
                     static_cast<DrawTypeFlags>(drawTypes & ~DrawTypeFlags::kDrawVertices),
                     renderPassDescs, /* withPrimitiveBlender= */ false);
         });

    if (drawTypes & DrawTypeFlags::kDrawVertices) {
        options.priv().buildCombinations(
            keyContext,
            /* addPrimitiveBlender= */ true,
            [&](UniquePaintParamsID uniqueID) {
                compile(context->priv().rendererProvider(),
                        context->priv().resourceProvider(),
                        keyContext, uniqueID,
                        DrawTypeFlags::kDrawVertices,
                        renderPassDescs, /* withPrimitiveBlender= */ true);
            });
    }
}

} // namespace skgpu::graphite

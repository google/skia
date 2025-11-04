/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/UniqueKeyUtils.h"

#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"

using namespace skgpu::graphite;
using namespace skgpu;


namespace UniqueKeyUtils {

void FetchUniqueKeys(PrecompileContext* precompileContext,
                     std::vector<UniqueKey>* keys) {
    GlobalCache* globalCache = precompileContext->priv().globalCache();

    keys->reserve(globalCache->numGraphicsPipelines());
    globalCache->forEachGraphicsPipeline([keys](const UniqueKey& key,
                                                const GraphicsPipeline* pipeline) {
                                                    keys->push_back(key);
                                         });
}

#ifdef SK_DEBUG
void DumpDescs(PrecompileContext* precompileContext,
               const GraphicsPipelineDesc& pipelineDesc,
               const RenderPassDesc& rpd) {
    const RendererProvider* rendererProvider = precompileContext->priv().rendererProvider();
    const ShaderCodeDictionary* dict = precompileContext->priv().shaderCodeDictionary();

    const RenderStep* rs = rendererProvider->lookup(pipelineDesc.renderStepID());
    SkDebugf("GraphicsPipelineDesc: %u %s\n", pipelineDesc.paintParamsID().asUInt(), rs->name());

    dict->dump(precompileContext->priv().caps(), pipelineDesc.paintParamsID());

    SkDebugf("RenderPassDesc:\n");
    SkDebugf("   colorAttach: %s\n", rpd.fColorAttachment.toString().c_str());
    SkDebugf("   colorResolveAttach: %s\n", rpd.fColorResolveAttachment.toString().c_str());
    SkDebugf("   depthStencilAttach: %s\n", rpd.fDepthStencilAttachment.toString().c_str());
    SkDebugf("   clearColor: %.2f %.2f %.2f %.2f\n"
             "   clearDepth: %.2f\n"
             "   stencilClear: %u\n"
             "   writeSwizzle: %s\n"
             "   sampleCount: %u\n",
             rpd.fClearColor[0], rpd.fClearColor[1], rpd.fClearColor[2], rpd.fClearColor[3],
             rpd.fClearDepth,
             rpd.fClearStencil,
             rpd.fWriteSwizzle.asString().c_str(),
             rpd.fSampleCount);

}
#endif // SK_DEBUG

bool ExtractKeyDescs(PrecompileContext* precompileContext,
                     const UniqueKey& origKey,
                     GraphicsPipelineDesc* pipelineDesc,
                     RenderPassDesc* renderPassDesc) {
    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();
    const RendererProvider* rendererProvider = precompileContext->priv().rendererProvider();

    bool extracted = caps->extractGraphicsDescs(origKey, pipelineDesc, renderPassDesc,
                                                rendererProvider);
    if (!extracted) {
        SkASSERT(0);
        return false;
    }

#ifdef SK_DEBUG
    UniqueKey newKey = caps->makeGraphicsPipelineKey(*pipelineDesc, *renderPassDesc);
    if (origKey != newKey) {
        SkDebugf("------- The UniqueKey didn't round trip!\n");
        origKey.dump("original key:");
        newKey.dump("reassembled key:");
        DumpDescs(precompileContext, *pipelineDesc, *renderPassDesc);
        SkDebugf("------------------------\n");
    }
    SkASSERT(origKey == newKey);
#endif

    return true;
}

}  // namespace UniqueKeyUtils

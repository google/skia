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
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"

using namespace skgpu::graphite;
using namespace skgpu;


namespace UniqueKeyUtils {

void FetchUniqueKeys(GlobalCache* globalCache,
                     std::vector<UniqueKey>* keys) {
    keys->reserve(globalCache->numGraphicsPipelines());
    globalCache->forEachGraphicsPipeline([keys](const UniqueKey& key,
                                                const GraphicsPipeline* pipeline) {
                                                    keys->push_back(key);
                                         });
}

#ifdef SK_DEBUG
void DumpDescs(const RendererProvider* rendererProvider,
               const ShaderCodeDictionary* dict,
               const GraphicsPipelineDesc& pipelineDesc,
               const RenderPassDesc& rpd) {
    const RenderStep* rs = rendererProvider->lookup(pipelineDesc.renderStepID());
    SkDebugf("GraphicsPipelineDesc: %u %s\n", pipelineDesc.paintParamsID().asUInt(), rs->name());

    dict->dump(pipelineDesc.paintParamsID());

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

bool ExtractKeyDescs(Context* context,
                     const UniqueKey& origKey,
                     GraphicsPipelineDesc* pipelineDesc,
                     RenderPassDesc* renderPassDesc) {
    const Caps* caps = context->priv().caps();
    const RendererProvider* rendererProvider = context->priv().rendererProvider();

    bool extracted = caps->extractGraphicsDescs(origKey, pipelineDesc, renderPassDesc,
                                                rendererProvider);
    if (!extracted) {
        SkASSERT(0);
        return false;
    }

#ifdef SK_DEBUG
    const ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    UniqueKey newKey = caps->makeGraphicsPipelineKey(*pipelineDesc, *renderPassDesc);
    if (origKey != newKey) {
        SkDebugf("------- The UniqueKey didn't round trip!\n");
        origKey.dump("original key:");
        newKey.dump("reassembled key:");
        DumpDescs(rendererProvider, dict, *pipelineDesc, *renderPassDesc);
        SkDebugf("------------------------\n");
    }
    SkASSERT(origKey == newKey);
#endif

    return true;
}

}  // namespace UniqueKeyUtils

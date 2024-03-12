/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/UniqueKeyUtils.h"

#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RendererProvider.h"

using namespace skgpu::graphite;
using namespace skgpu;

#ifdef SK_DEBUG

namespace {

const char* to_str(LoadOp op) {
    switch (op) {
        case LoadOp::kLoad:    return "kLoad";
        case LoadOp::kClear:   return "kClear";
        case LoadOp::kDiscard: return "kDiscard";
    }

    SkUNREACHABLE;
}

const char* to_str(StoreOp op) {
    switch (op) {
        case StoreOp::kStore:   return "kStore";
        case StoreOp::kDiscard: return "kDiscard";
    }

    SkUNREACHABLE;
}

void dump_attachment(const char* label, const AttachmentDesc& attachment) {
    if (attachment.fTextureInfo.isValid()) {
        SkDebugf("%s %s loadOp: %s storeOp: %s\n",
                 label,
                 attachment.fTextureInfo.toString().c_str(),
                 to_str(attachment.fLoadOp),
                 to_str(attachment.fStoreOp));
    } else {
        SkDebugf("%s invalid attachment\n", label);
    }
}

} // anonymous namespace

#endif // SK_DEBUG


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
               const GraphicsPipelineDesc& pipelineDesc,
               const RenderPassDesc& rpd) {
    const RenderStep* rs = rendererProvider->lookup(pipelineDesc.renderStepID());

    SkDebugf("GraphicsPipelineDesc: %d %s\n", pipelineDesc.paintParamsID().asUInt(), rs->name());

    SkDebugf("RenderPassDesc:\n");
    dump_attachment("   colorAttach:", rpd.fColorAttachment);
    SkDebugf("   clearColor: %.2f %.2f %.2f %.2f\n",
             rpd.fClearColor[0], rpd.fClearColor[1], rpd.fClearColor[2], rpd.fClearColor[3]);
    dump_attachment("   colorResolveAttach:", rpd.fColorResolveAttachment);
    dump_attachment("   depthStencilAttach:", rpd.fDepthStencilAttachment);
    SkDebugf("   clearDepth: %.2f\n"
             "   stencilClear: %d\n"
             "   writeStencil: %s\n"
             "   sampleCount: %d\n",
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
    UniqueKey newKey = caps->makeGraphicsPipelineKey(*pipelineDesc, *renderPassDesc);
    if (origKey != newKey) {
        SkDebugf("------- The UniqueKey didn't round trip!\n");
        origKey.dump("original key:");
        newKey.dump("reassembled key:");
        DumpDescs(rendererProvider, *pipelineDesc, *renderPassDesc);
        SkDebugf("------------------------\n");
    }
    SkASSERT(origKey == newKey);
#endif

    return true;
}

}  // namespace UniqueKeyUtils

/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RenderPassDesc.h"

#include "src/gpu/graphite/Caps.h"

namespace skgpu::graphite {

namespace {

const char* to_str(LoadOp op) {
    switch (op) {
        case LoadOp::kLoad:    return "load";
        case LoadOp::kClear:   return "clear";
        case LoadOp::kDiscard: return "discard";
    }

    SkUNREACHABLE;
}

const char* to_str(StoreOp op) {
    switch (op) {
        case StoreOp::kStore:   return "store";
        case StoreOp::kDiscard: return "discard";
    }

    SkUNREACHABLE;
}

} // anonymous namespace

RenderPassDesc RenderPassDesc::Make(const Caps* caps,
                                    const TextureInfo& targetInfo,
                                    LoadOp loadOp,
                                    StoreOp storeOp,
                                    SkEnumBitMask<DepthStencilFlags> depthStencilFlags,
                                    const std::array<float, 4>& clearColor,
                                    bool requiresMSAA,
                                    Swizzle writeSwizzle) {
    RenderPassDesc desc;
    desc.fWriteSwizzle = writeSwizzle;
    desc.fSampleCount = 1;
    // It doesn't make sense to have a storeOp for our main target not be store. Why are we doing
    // this DrawPass then
    SkASSERT(storeOp == StoreOp::kStore);
    if (requiresMSAA) {
        if (caps->msaaRenderToSingleSampledSupport()) {
            desc.fColorAttachment.fTextureInfo = targetInfo;
            desc.fColorAttachment.fLoadOp = loadOp;
            desc.fColorAttachment.fStoreOp = storeOp;
            desc.fSampleCount = caps->defaultMSAASamplesCount();
        } else {
            // TODO: If the resolve texture isn't readable, the MSAA color attachment will need to
            // be persistently associated with the framebuffer, in which case it's not discardable.
            auto msaaTextureInfo = caps->getDefaultMSAATextureInfo(targetInfo, Discardable::kYes);
            if (msaaTextureInfo.isValid()) {
                desc.fColorAttachment.fTextureInfo = msaaTextureInfo;
                if (loadOp != LoadOp::kClear) {
                    desc.fColorAttachment.fLoadOp = LoadOp::kDiscard;
                } else {
                    desc.fColorAttachment.fLoadOp = LoadOp::kClear;
                }
                desc.fColorAttachment.fStoreOp = StoreOp::kDiscard;

                desc.fColorResolveAttachment.fTextureInfo = targetInfo;
                if (loadOp != LoadOp::kLoad) {
                    desc.fColorResolveAttachment.fLoadOp = LoadOp::kDiscard;
                } else {
                    desc.fColorResolveAttachment.fLoadOp = LoadOp::kLoad;
                }
                desc.fColorResolveAttachment.fStoreOp = storeOp;

                desc.fSampleCount = msaaTextureInfo.numSamples();
            } else {
                // fall back to single sampled
                desc.fColorAttachment.fTextureInfo = targetInfo;
                desc.fColorAttachment.fLoadOp = loadOp;
                desc.fColorAttachment.fStoreOp = storeOp;
            }
        }
    } else {
        desc.fColorAttachment.fTextureInfo = targetInfo;
        desc.fColorAttachment.fLoadOp = loadOp;
        desc.fColorAttachment.fStoreOp = storeOp;
    }
    desc.fClearColor = clearColor;

    if (depthStencilFlags != DepthStencilFlags::kNone) {
        desc.fDepthStencilAttachment.fTextureInfo = caps->getDefaultDepthStencilTextureInfo(
                depthStencilFlags, desc.fSampleCount, targetInfo.isProtected());
        // Always clear the depth and stencil to 0 at the start of a DrawPass, but discard at the
        // end since their contents do not affect the next frame.
        desc.fDepthStencilAttachment.fLoadOp = LoadOp::kClear;
        desc.fClearDepth = 0.f;
        desc.fClearStencil = 0;
        desc.fDepthStencilAttachment.fStoreOp = StoreOp::kDiscard;
    }

    return desc;
}

SkString RenderPassDesc::toString() const {
    return SkStringPrintf("RP(color: %s, resolve: %s, ds: %s, samples: %u, swizzle: %s, "
                          "clear: c(%f,%f,%f,%f), d(%f), s(0x%02x))",
                          fColorAttachment.toString().c_str(),
                          fColorResolveAttachment.toString().c_str(),
                          fDepthStencilAttachment.toString().c_str(),
                          fSampleCount,
                          fWriteSwizzle.asString().c_str(),
                          fClearColor[0], fClearColor[1], fClearColor[2], fClearColor[3],
                          fClearDepth,
                          fClearStencil);
}

SkString RenderPassDesc::toPipelineLabel() const {
    // This intentionally only includes the fixed state that impacts pipeline compilation.
    // We include the load op of the color attachment when there is a resolve attachment because
    // the load may trigger a different renderpass description.
    const char* colorLoadStr = "";
    if (fColorAttachment.fLoadOp == LoadOp::kLoad &&
        (fColorResolveAttachment.fTextureInfo.isValid() || fSampleCount > 1)) {
        colorLoadStr = " w/ msaa load";
    }
    // TODO: Remove `fSampleCount` in label when the Dawn backend manages its MSAA color attachments
    // directly instead of relying on msaaRenderToSingleSampledSupport().
    return SkStringPrintf("RP(color: %s%s, resolve: %s, ds: %s, samples: %u, swizzle: %s)",
                          fColorAttachment.fTextureInfo.toRPAttachmentString().c_str(),
                          colorLoadStr,
                          fColorResolveAttachment.fTextureInfo.toRPAttachmentString().c_str(),
                          fDepthStencilAttachment.fTextureInfo.toRPAttachmentString().c_str(),
                          fSampleCount,
                          fWriteSwizzle.asString().c_str());
}

SkString AttachmentDesc::toString() const {
    if (fTextureInfo.isValid()) {
        return SkStringPrintf("info: %s loadOp: %s storeOp: %s",
                              fTextureInfo.toString().c_str(),
                              to_str(fLoadOp),
                              to_str(fStoreOp));
    } else {
        return SkString("invalid attachment");
    }
}

} // namespace skgpu::graphite

/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RenderPassDesc.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

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
                                    Swizzle writeSwizzle,
                                    const DstReadStrategy dstReadStrategy) {
    // It doesn't make sense to have a storeOp for our main target not be store. Why are we doing
    // this DrawPass then
    SkASSERT(storeOp == StoreOp::kStore);

    RenderPassDesc desc;
    desc.fClearColor = clearColor;
    desc.fClearDepth = 0.f; // Depth and stencil is currently always cleared to 0 if it's used
    desc.fClearStencil = 0;
    desc.fWriteSwizzle = writeSwizzle;
    desc.fDstReadStrategy = dstReadStrategy;

    TextureFormat colorFormat = TextureInfoPriv::ViewFormat(targetInfo);
    // The render pass's overall sample count will either be the target's sample count
    // (when single-sampling or already multisampled), or the default sample count (which will then
    // be either the implicit sample count for msaa-render-to-single-sample or the explicit sample
    // count of a separate color attachment).
    //
    // Higher-level logic should ensure the default MSAA sample count is supported if using either
    // msaa-render-to-single-sample or with separate attachments, and select non-MSAA techniques if
    // they weren't supported. Downgrade to single-sampled if we get here somehow anyways.
    const uint8_t defaultSamples = caps->defaultMSAASamplesCount();
    const bool canUseDefaultMSAA = caps->msaaRenderToSingleSampledSupport() ||
                                   caps->isSampleCountSupported(colorFormat, defaultSamples);
    desc.fSampleCount = requiresMSAA && targetInfo.numSamples() == 1
            ? (canUseDefaultMSAA ? defaultSamples : 1)
            : targetInfo.numSamples();

    // We need to handle MSAA with an extra color attachment if:
    const bool needsMSAAColorAttachment =
            desc.fSampleCount > 1 &&                    // using MSAA for the render pass,
            targetInfo.numSamples() == 1 &&             // the target isn't already MSAA'ed,
            !caps->msaaRenderToSingleSampledSupport();  // can't use an MSAA->single extension.
    if (needsMSAAColorAttachment) {
        // We set the color and resolve attachments up the same regardless of if the backend ends up
        // using msaaRenderToSingleSampledSupport() to skip explicitly creating the MSAA attachment.
        // The color attachment (and any depth/stencil attachment) will use `sampleCount` and the
        // resolve attachment will be single-sampled.
        desc.fColorAttachment = {colorFormat,
                                 loadOp != LoadOp::kClear ? LoadOp::kDiscard : LoadOp::kClear,
                                 StoreOp::kDiscard,
                                 desc.fSampleCount};
        desc.fColorResolveAttachment = {colorFormat,
                                        loadOp != LoadOp::kLoad ? LoadOp::kDiscard : LoadOp::kLoad,
                                        storeOp,
                                        /*sampleCount=*/1};
    } else {
        // The target will be the color attachment and skip configuring the resolve attachment.
        SkASSERT(desc.fColorResolveAttachment.fFormat == TextureFormat::kUnsupported);
        desc.fColorAttachment = {colorFormat,
                                 loadOp,
                                 storeOp,
                                 SkTo<uint8_t>(targetInfo.numSamples())};
    }

    if (depthStencilFlags != DepthStencilFlags::kNone) {
        TextureFormat dsFormat = caps->getDepthStencilFormat(depthStencilFlags);
        SkASSERT(dsFormat != TextureFormat::kUnsupported);
        // Depth and stencil values are currently always cleared and don't need to persist.
        // The sample count should always match the color attachment.
        desc.fDepthStencilAttachment = {dsFormat,
                                        LoadOp::kClear,
                                        StoreOp::kDiscard,
                                        desc.fColorAttachment.fSampleCount};
    } else {
        SkASSERT(desc.fDepthStencilAttachment.fFormat == TextureFormat::kUnsupported);
    }

    return desc;
}

SkString RenderPassDesc::toString() const {
    return SkStringPrintf("RP(color: %s, resolve: %s, ds: %s, samples: %u, swizzle: %s, "
                          "clear: c(%f,%f,%f,%f), d(%f), s(0x%02x), dst read: %u)",
                          fColorAttachment.toString().c_str(),
                          fColorResolveAttachment.toString().c_str(),
                          fDepthStencilAttachment.toString().c_str(),
                          fSampleCount,
                          fWriteSwizzle.asString().c_str(),
                          fClearColor[0], fClearColor[1], fClearColor[2], fClearColor[3],
                          fClearDepth,
                          fClearStencil,
                          (unsigned)fDstReadStrategy);
}

SkString RenderPassDesc::toPipelineLabel() const {
    // Given current policies, these assumptions should hold and mean the conciseness in the label
    // is still unambiguous.
    SkASSERT(fColorAttachment.fFormat != TextureFormat::kUnsupported);
    SkASSERT(fColorResolveAttachment.fFormat == TextureFormat::kUnsupported ||
             fColorResolveAttachment.fFormat == fColorAttachment.fFormat);
    SkASSERT(fDepthStencilAttachment.fFormat == TextureFormat::kUnsupported ||
             fDepthStencilAttachment.fSampleCount == fColorAttachment.fSampleCount);
    SkASSERT(fColorResolveAttachment.fFormat == TextureFormat::kUnsupported ||
             fColorResolveAttachment.fSampleCount == 1);
    SkASSERT(fColorAttachment.fSampleCount == fSampleCount ||
             (fColorAttachment.fSampleCount == 1 && fSampleCount > 1));

    const char* colorFormatStr = TextureFormatName(fColorAttachment.fFormat);
    const char* dsFormatStr = "{}";
    if (fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
        dsFormatStr = TextureFormatName(fDepthStencilAttachment.fFormat);
    }

    // This intentionally only includes the fixed state that impacts pipeline compilation.
    // We include the load op of the color attachment when there is a resolve attachment because
    // the load may trigger a different renderpass description.
    const char* colorLoadStr = "";
    const bool loadMsaaFromResolve =
            fColorResolveAttachment.fFormat != TextureFormat::kUnsupported &&
            fColorResolveAttachment.fLoadOp == LoadOp::kLoad;

    // This should, technically, check Caps::loadOpAffectsMSAAPipelines before adding the extra
    // string. Only the Metal backend doesn't set that flag, however, so we just assume it is set
    // to reduce plumbing. Since the Metal backend doesn't differentiate its UniqueKeys wrt
    // resolve-loads, this can lead to instances where two Metal Pipeline labels will map to the
    // same UniqueKey (i.e., one with "w/ msaa load" and one without it).
    if (loadMsaaFromResolve /* && Caps::loadOpAffectsMSAAPipelines() */) {
        colorLoadStr = " w/ msaa load";
    }

    // There are three supported ways of achieving MSAA rendering that we distinguish compactly.
    // 1. Direct sampling w/ N samples (includes single sample)
    // 2. MSAA render to single-sampled extensions
    // 3. Explicit MSAA color attachment w/ resolve
    // Since we don't expect to be mixing case 2 and 3 on the same device, treating them the same
    // in the pipeline labels makes it more convenient when writing test expectations.
    SkString sampleCountStr;
    if (fColorResolveAttachment.fFormat == TextureFormat::kUnsupported &&
        fSampleCount == fColorAttachment.fSampleCount) {
        // Case 1: "xN"
        sampleCountStr = SkStringPrintf("x%u", fSampleCount);
    } else {
        // Case 2 and 3: "xN->1"
        sampleCountStr = SkStringPrintf("x%u->1", fSampleCount);
    }
    // NOTE: This label does not differentiate between explicitly resolved MSAA color attachments
    // and MSAA-render-to-single-sample renderpasses. For a given set of Caps, we currently only
    // expect to generate one or the other variety.
    return SkStringPrintf("RP((%s+%s %s).%s%s)",
                          colorFormatStr,
                          dsFormatStr,
                          sampleCountStr.c_str(),
                          fWriteSwizzle.asString().c_str(),
                          colorLoadStr);
}

SkString AttachmentDesc::toString() const {
    if (fFormat == TextureFormat::kUnsupported) {
        return SkString("{}");
    } else {
        return SkStringPrintf("{f: %s x%u, ops: %s->%s}",
                              TextureFormatName(fFormat),
                              fSampleCount,
                              to_str(fLoadOp),
                              to_str(fStoreOp));
    }
}

bool AttachmentDesc::isCompatible(const TextureInfo& texInfo) const {
    return fFormat == TextureInfoPriv::ViewFormat(texInfo) &&
           fSampleCount == texInfo.numSamples();
}

} // namespace skgpu::graphite

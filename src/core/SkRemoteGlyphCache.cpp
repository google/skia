/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

struct WireTypeface {
    // std::thread::id thread_id;  // TODO:need to figure a good solution
    SkFontID        typeface_id;
    SkFontStyle     style;
    bool            is_fixed;
};

void SkRemoteGlyphCacheRenderer::prepareSerializeProcs(SkSerialProcs* procs) {
    auto encode = [](SkTypeface* tf, void* ctx) {
        return reinterpret_cast<SkRemoteGlyphCacheRenderer*>(ctx)->encodeTypeface(tf);
    };
    procs->fTypefaceProc = encode;
    procs->fTypefaceCtx = this;
}

SkScalerContext* SkRemoteGlyphCacheRenderer::generateScalerContext(
    const SkScalerContextRecDescriptor& desc, SkFontID typefaceId)
{
    auto scaler = fScalerContextMap.find(desc);
    if (scaler == nullptr) {
        auto typefaceIter = fTypefaceMap.find(typefaceId);
        if (typefaceIter == nullptr) {
            // TODO: handle this with some future fallback strategy.
            SK_ABORT("unknown type face");
            // Should never happen
            return nullptr;
        }
        auto tf = typefaceIter->get();
        SkScalerContextEffects effects;
        auto mapSc = tf->createScalerContext(effects, &desc.desc(), false);
        scaler = fScalerContextMap.set(desc, std::move(mapSc));
    }
    return scaler->get();
}

sk_sp<SkData> SkRemoteGlyphCacheRenderer::encodeTypeface(SkTypeface* tf) {
    WireTypeface wire = {
        SkTypeface::UniqueID(tf),
        tf->fontStyle(),
        tf->isFixedPitch()
    };
    auto typeFace = fTypefaceMap.find(SkTypeface::UniqueID(tf));
    if (typeFace == nullptr) {
        fTypefaceMap.set(SkTypeface::UniqueID(tf), sk_ref_sp(tf));
    }
    // Can this be done with no copy?
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

SkRemoteGlyphCacheGPU::SkRemoteGlyphCacheGPU(
    std::unique_ptr<SkRemoteScalerContext> remoteScalerContext)
    : fRemoteScalerContext{std::move(remoteScalerContext)} { }

void SkRemoteGlyphCacheGPU::prepareDeserializeProcs(SkDeserialProcs* procs) {
    auto decode = [](const void* buf, size_t len, void* ctx) {
        return reinterpret_cast<SkRemoteGlyphCacheGPU*>(ctx)->decodeTypeface(buf, len);
    };
    procs->fTypefaceProc = decode;
    procs->fTypefaceCtx = this;
}


sk_sp<SkTypeface> SkRemoteGlyphCacheGPU::decodeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len < sizeof(wire)) {
        SK_ABORT("Incomplete transfer");
        return nullptr;
    }
    memcpy(&wire, buf, sizeof(wire));

    auto typeFace = fMapIdToTypeface.find(wire.typeface_id);
    if (typeFace == nullptr) {

        auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.typeface_id,
            wire.style,
            wire.is_fixed,
            fRemoteScalerContext.get());

        typeFace = fMapIdToTypeface.set(wire.typeface_id, newTypeface);
    }
    return *typeFace;
}



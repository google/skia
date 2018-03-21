/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

struct WireTypeface {
    // std::thread::id thread_id;  // TODO:need to figure a good solution
    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
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
        // TODO: make effects really work.
        SkScalerContextEffects effects;
        auto mapSc = tf->createScalerContext(effects, &desc.desc(), false);
        scaler = fScalerContextMap.set(desc, std::move(mapSc));
    }
    return scaler->get();
}

sk_sp<SkData> SkRemoteGlyphCacheRenderer::encodeTypeface(SkTypeface* tf) {
    WireTypeface wire = {
        SkTypeface::UniqueID(tf),
        tf->countGlyphs(),
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

SkTypeface* SkRemoteGlyphCacheGPU::lookupTypeface(SkFontID id) {
    auto typeface = fMapIdToTypeface.find(id);
    SkASSERT(typeface != nullptr);
    return typeface->get();
}

sk_sp<SkTypeface> SkRemoteGlyphCacheGPU::decodeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len < sizeof(wire)) {
        SK_ABORT("Incomplete transfer");
        return nullptr;
    }
    memcpy(&wire, buf, sizeof(wire));

    auto typeFace = fMapIdToTypeface.find(wire.typefaceID);
    if (typeFace == nullptr) {

        auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.typefaceID,
            wire.glyphCount,
            wire.style,
            wire.isFixed,
            fRemoteScalerContext.get());

        typeFace = fMapIdToTypeface.set(wire.typefaceID, newTypeface);
    }
    return *typeFace;
}



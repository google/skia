/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

#include <iostream>

struct WireTypeface {
    std::thread::id thread_id;
    SkFontID        typeface_id;
    SkFontStyle     style;
    bool            is_fixed;
};

void SkRemoteGlyphCacheRenderer::prepareSerializeProcs(SkSerialProcs* procs) {
    auto encode = [](SkTypeface* tf, void* ctx) {
        return reinterpret_cast<SkRemoteGlyphCacheRenderer*>(ctx)->encodeTypeface(tf);
    };
    procs->fTypefaceProc = +encode;
    procs->fTypefaceCtx = this;
}

SkScalerContext* SkRemoteGlyphCacheRenderer::generateScalerContext(
    const SkScalerContextRecDescriptor& desc, SkFontID typefaceId)
{
    auto scalerIter = fScalerContextMap.find(desc);
    if (scalerIter == fScalerContextMap.end()) {
        auto typefaceIter = fTypefaceMap.find(typefaceId);
        if (typefaceIter == fTypefaceMap.end()) {
            // TODO: change to get the default typeface from FontMgr.
            std::cerr << "bad typeface id: " << typefaceId << std::endl;
            SK_ABORT("unknown type face");
        }
        auto tf = typefaceIter->second;
        SkScalerContextEffects effects;
        auto mapSc = tf->createScalerContext(effects, &desc.desc(), false);
        scalerIter = fScalerContextMap.emplace_hint(scalerIter, desc, std::move(mapSc));
    }
    return scalerIter->second.get();
}

sk_sp<SkData> SkRemoteGlyphCacheRenderer::encodeTypeface(SkTypeface* tf) {
    WireTypeface wire = {
        std::this_thread::get_id(),
        SkTypeface::UniqueID(tf),
        tf->fontStyle(),
        tf->isFixedPitch()
    };
    auto iter = fTypefaceMap.find(SkTypeface::UniqueID(tf));
    if (iter == fTypefaceMap.end()) {
        fTypefaceMap.emplace_hint(iter, SkTypeface::UniqueID(tf), sk_ref_sp(tf));
    }
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

SkRemoteGlyphCacheGPU::SkRemoteGlyphCacheGPU(
    std::unique_ptr<SkRemoteScalerContext> remoteScalerContext)
    : fRemoteScalerContext{std::move(remoteScalerContext)} { }

void SkRemoteGlyphCacheGPU::prepareDeserializeProcs(SkDeserialProcs* procs) {
    auto decode = [](const void* buf, size_t len, void* ctx) {
        return reinterpret_cast<SkRemoteGlyphCacheGPU*>(ctx)->decodeTypeface(buf, len);
    };
    procs->fTypefaceProc = +decode;
    procs->fTypefaceCtx = this;
}

sk_sp<SkTypeface> SkRemoteGlyphCacheGPU::decodeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len < sizeof(wire)) {
        SK_ABORT("Incomplete transfer");
        return nullptr;
    }
    memcpy(&wire, buf, sizeof(wire));
    auto iter = fMapIdToTypeface.find(wire.typeface_id);
    if (iter == fMapIdToTypeface.end()) {

        auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.typeface_id,
            wire.thread_id,
            wire.style,
            wire.is_fixed,
            fRemoteScalerContext.get());

        iter = fMapIdToTypeface.emplace_hint(iter, wire.typeface_id, newTypeface);
    }
    return iter->second;
}



/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrContext_Base.h"

#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/gpu/effects/GrSkSLFP.h"

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidGenID);
    return id;
}

GrContext_Base::GrContext_Base(GrBackendApi backend,
                               const GrContextOptions& options,
                               uint32_t contextID)
        : fBackend(backend)
        , fOptions(options)
        , fContextID(SK_InvalidGenID == contextID ? next_id() : contextID) {
}

GrContext_Base::~GrContext_Base() { }

bool GrContext_Base::init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) {
    SkASSERT(caps && FPFactoryCache);

    fCaps = caps;
    fFPFactoryCache = FPFactoryCache;
    return true;
}

const GrCaps* GrContext_Base::caps() const { return fCaps.get(); }
sk_sp<const GrCaps> GrContext_Base::refCaps() const { return fCaps; }

sk_sp<GrSkSLFPFactoryCache> GrContext_Base::fpFactoryCache() { return fFPFactoryCache; }

GrBackendFormat GrContext_Base::defaultBackendFormat(SkColorType skColorType,
                                                     GrRenderable renderable) const {
    const GrCaps* caps = this->caps();

    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);

    GrBackendFormat format = caps->getDefaultBackendFormat(grColorType, renderable);
    if (!format.isValid()) {
        return GrBackendFormat();
    }

    SkASSERT(caps->isFormatTexturableAndUploadable(grColorType, format));
    SkASSERT(renderable == GrRenderable::kNo ||
             caps->isFormatAsColorTypeRenderable(grColorType, format));

    return format;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrBaseContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrBaseContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

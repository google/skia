/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrContext_Base.h"

#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrShaderUtils.h"
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

bool GrContext_Base::init(sk_sp<const GrCaps> caps) {
    SkASSERT(caps);

    fCaps = caps;
    return true;
}

const GrCaps* GrContext_Base::caps() const { return fCaps.get(); }
sk_sp<const GrCaps> GrContext_Base::refCaps() const { return fCaps; }

GrBackendFormat GrContext_Base::defaultBackendFormat(SkColorType skColorType,
                                                     GrRenderable renderable) const {
    const GrCaps* caps = this->caps();

    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);

    GrBackendFormat format = caps->getDefaultBackendFormat(grColorType, renderable);
    if (!format.isValid()) {
        return GrBackendFormat();
    }

    SkASSERT(renderable == GrRenderable::kNo ||
             caps->isFormatAsColorTypeRenderable(grColorType, format));

    return format;
}

GrBackendFormat GrContext_Base::compressedBackendFormat(SkImage::CompressionType c) const {
    const GrCaps* caps = this->caps();

    GrBackendFormat format = caps->getBackendFormatFromCompressionType(c);

    SkASSERT(!format.isValid() || caps->isFormatTexturable(format));
    return format;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrBaseContextPriv::refCaps() const {
    return fContext->refCaps();
}

GrContextOptions::ShaderErrorHandler* GrBaseContextPriv::getShaderErrorHandler() const {
    const GrContextOptions& options(this->options());
    return options.fShaderErrorHandler ? options.fShaderErrorHandler
                                       : GrShaderUtils::DefaultShaderErrorHandler();
}

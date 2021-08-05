/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrContext_Base.h"

#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/effects/GrSkSLFP.h"

GrContext_Base::GrContext_Base(sk_sp<GrContextThreadSafeProxy> proxy)
        : fThreadSafeProxy(std::move(proxy)) {
}

GrContext_Base::~GrContext_Base() { }

bool GrContext_Base::init() {
    SkASSERT(fThreadSafeProxy->isValid());

    return true;
}

uint32_t GrContext_Base::contextID() const { return fThreadSafeProxy->priv().contextID(); }
GrBackendApi GrContext_Base::backend() const { return fThreadSafeProxy->priv().backend(); }

const GrContextOptions& GrContext_Base::options() const {
    return fThreadSafeProxy->priv().options();
}

const GrCaps* GrContext_Base::caps() const { return fThreadSafeProxy->priv().caps(); }
sk_sp<const GrCaps> GrContext_Base::refCaps() const { return fThreadSafeProxy->priv().refCaps(); }

GrBackendFormat GrContext_Base::defaultBackendFormat(SkColorType skColorType,
                                                     GrRenderable renderable) const {
    return fThreadSafeProxy->defaultBackendFormat(skColorType, renderable);
}

GrBackendFormat GrContext_Base::compressedBackendFormat(SkImage::CompressionType c) const {
    return fThreadSafeProxy->compressedBackendFormat(c);
}

sk_sp<GrContextThreadSafeProxy> GrContext_Base::threadSafeProxy() { return fThreadSafeProxy; }

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrBaseContextPriv::refCaps() const {
    return this->context()->refCaps();
}

GrContextOptions::ShaderErrorHandler* GrBaseContextPriv::getShaderErrorHandler() const {
    const GrContextOptions& options(this->options());
    return options.fShaderErrorHandler ? options.fShaderErrorHandler
                                       : GrShaderUtils::DefaultShaderErrorHandler();
}

/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/ganesh/GrContext_Base.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrBaseContextPriv.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"

#include <utility>

enum SkColorType : int;

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

GrBackendFormat GrContext_Base::compressedBackendFormat(SkTextureCompressionType c) const {
    return fThreadSafeProxy->compressedBackendFormat(c);
}

int GrContext_Base::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    return fThreadSafeProxy->maxSurfaceSampleCountForColorType(colorType);
}

sk_sp<GrContextThreadSafeProxy> GrContext_Base::threadSafeProxy() { return fThreadSafeProxy; }

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrBaseContextPriv::refCaps() const {
    return this->context()->refCaps();
}

GrContextOptions::ShaderErrorHandler* GrBaseContextPriv::getShaderErrorHandler() const {
    const GrContextOptions& options(this->options());
    return options.fShaderErrorHandler ? options.fShaderErrorHandler
                                       : skgpu::DefaultShaderErrorHandler();
}

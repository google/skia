/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendObject.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"

GrBackendObject::GrBackendObject(GrContext* context, GrBackendTexture backendTex)
        : fContext(context)
        , fBackendTex(backendTex) {
}

GrBackendObject::~GrBackendObject() {
    if (fContext->abandoned() || !fBackendTex.isValid()) {
        return;
    }

    GrGpu* gpu = fContext->priv().getGpu();

    gpu->deleteTestingOnlyBackendTexture(fBackendTex);
}

sk_sp<GrBackendObject> GrBackendObject::Make(GrContext* context,
                                             int width, int height,
                                             SkColorType colorType,
                                             GrMipMapped mipMapped, GrRenderable renderable) {

    GrBackendFormat format = context->priv().caps()->getBackendFormatFromColorType(colorType);
    if (!format.isValid()) {
        return nullptr;
    }

    return Make(context, width, height, format, mipMapped, renderable);
}

sk_sp<GrBackendObject> GrBackendObject::Make(GrContext* context,
                                             int width, int height,
                                             GrBackendFormat backendFormat,
                                             GrMipMapped mipMapped, GrRenderable renderable) {
    if (!context->priv().asDirectContext()) {
        return nullptr;
    }

    if (context->abandoned()) {
        return nullptr;
    }

    GrGpu* gpu = context->priv().getGpu();

    if (!backendFormat.isValid()) {
        return nullptr;
    }

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(width, height, backendFormat,
                                                                       mipMapped, renderable,
                                                                       nullptr, 0);
    if (!backendTex.isValid()) {
        return nullptr;
    }

    return sk_sp<GrBackendObject>(new GrBackendObject(context, backendTex));
}


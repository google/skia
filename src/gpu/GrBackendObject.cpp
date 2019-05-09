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

sk_sp<GrBackendObject> GrBackendObject::Make(GrContext* context,
                                             int width, int height,
                                             SkColorType ct, GrMipMapped mipMapped) {
    if (context->priv().asDirectContext()) {
        return nullptr;
    }

    if (context->abandoned()) {
        return nullptr;
    }

    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(nullptr, width, height, ct,
                                                                       false, mipMapped, 0);
    if (!backendTex.isValid()) {
        return nullptr;
    }

    return sk_make_sp<GrBackendObject>(context, backendTex);
}

GrBackendObject::~GrBackendObject() {
    if (fContext->abandoned() || !fBackendTex.isValid()) {
        return;
    }

    GrGpu* gpu = fContext->priv().getGpu();

    gpu->deleteTestingOnlyBackendTexture(fBackendTex);
}

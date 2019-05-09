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

sk_sp<GrBackendObject> GrBackendObject::Gimme(GrContext* context,
                                              int width, int height, SkColorType ct) {
    if (context->priv().asDirectContext()) {
        return nullptr;
    }

    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(nullptr, width, height, ct,
                                                                       false, GrMipMapped::kNo, 0);

    return sk_make_sp<GrBackendObject>(backendTex);
}

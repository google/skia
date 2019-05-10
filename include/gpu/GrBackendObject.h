/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendObject_DEFINED
#define GrBackendObject_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"

// questions:
//    I force the client to go through the GrContext to create these - reasonable?
//    We could just give them access to the factories here (and ensure we have a direct context)

class SK_API GrBackendObject : public SkRefCnt {
public:
    ~GrBackendObject() override;

    GrBackendTexture backendTexture() { return fBackendTex; }

    // Create an uninitialized backend texture
    static sk_sp<GrBackendObject> Make(GrContext* context,
                                       int width, int height,
                                       SkColorType,
                                       GrMipMapped, GrRenderable);

    // Create an uninitialized backend texture
    static sk_sp<GrBackendObject> Make(GrContext* context,
                                       int width, int height,
                                       GrBackendFormat,
                                       GrMipMapped, GrRenderable);

private:
    GrBackendObject(GrContext* context, GrBackendTexture backendTex);

    // TODO: we need some restrictions on which thread we delete on!
    GrContext*       fContext; // guaranteed to be a direct context
    GrBackendTexture fBackendTex;

    typedef SkRefCnt INHERITED;
};

#endif


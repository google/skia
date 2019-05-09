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

class SK_API GrBackendObject : public SkRefCnt {
public:
    ~GrBackendObject() override;

protected:
    friend class GrContext; // for factories

    // Create an uninitialized non-renderable backend texture
    static sk_sp<GrBackendObject> Make(GrContext* context, int width, int height,
                                       SkColorType ct, GrMipMapped mipMapped);

private:
    GrBackendObject(GrContext* context, GrBackendTexture backendTex)
            : fContext(context)
            , fBackendTex(backendTex) {
    }

    // TODO: we need some restrictions on which thread we delete on!
    GrContext*       fContext; // guaranteed to be a direct context
    GrBackendTexture fBackendTex;

    typedef SkRefCnt INHERITED;
};

#endif


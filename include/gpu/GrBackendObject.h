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

/*
 * This allows clients to use Skia to create backend objects outside of Skia proper (i.e.,
 * Skia's caching system will not know about them.)
 *
 * It is the client's responsibility to delete all these object before deleting the GrContext
 * to which they are attached. Additionally, clients should only delete these objects on the
 * thread for which the GrContext is active.
 *
 * Additionally, the client is responsible for ensuring synchronization between different uses
 * of the backend object.
 */
class SK_API GrBackendObject : public SkRefCnt {
public:
    ~GrBackendObject() override;

    GrBackendTexture backendTexture() { return fBackendTex; }

    // Create an uninitialized backend texture
    static sk_sp<GrBackendObject> Make(GrContext* context,
                                       int width, int height,
                                       GrBackendFormat,
                                       GrMipMapped, GrRenderable);

#if GR_TEST_UTILS
    // Create an uninitialized backend texture
    // This is a helper that is strictly for testing. It is expected that clients will
    // know the true backend format of their desired resources.
    static sk_sp<GrBackendObject> Make(GrContext* context,
                                       int width, int height,
                                       SkColorType,
                                       GrMipMapped, GrRenderable);
#endif

private:
    GrBackendObject(GrContext* context, GrBackendTexture backendTex);

    GrContext*       fContext; // guaranteed to be a direct context
    GrBackendTexture fBackendTex;

    typedef SkRefCnt INHERITED;
};

#endif


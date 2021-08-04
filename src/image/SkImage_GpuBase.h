/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuBase_DEFINED
#define SkImage_GpuBase_DEFINED

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/image/SkImage_Base.h"

class GrColorSpaceXform;
class GrDirectContext;
class GrImageContext;
class SkColorSpace;

class SkImage_GpuBase : public SkImage_Base {
public:
    GrImageContext* context() const final { return fContext.get(); }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset, GrDirectContext*) const final;

    bool onReadPixels(GrDirectContext *dContext,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRB,
                      int srcX,
                      int srcY,
                      CachingHint) const override;

    bool onIsValid(GrRecordingContext*) const final;

    static bool ValidateBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                       GrColorType grCT, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
    static bool ValidateCompressedBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                                 SkAlphaType);

    // Helper for making a lazy proxy for a promise image.
    // PromiseImageTextureFulfillProc must not be null.
    static sk_sp<GrTextureProxy> MakePromiseImageLazyProxy(GrContextThreadSafeProxy*,
                                                           SkISize dimensions,
                                                           GrBackendFormat,
                                                           GrMipmapped,
                                                           PromiseImageTextureFulfillProc,
                                                           sk_sp<GrRefCntedCallback> releaseHelper);

protected:
    SkImage_GpuBase(sk_sp<GrImageContext>, SkImageInfo, uint32_t uniqueID);

    sk_sp<GrImageContext> fContext;

private:
    using INHERITED = SkImage_Base;
};

#endif

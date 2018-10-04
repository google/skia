/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuBase_DEFINED
#define SkImage_GpuBase_DEFINED

#include "SkImage_Base.h"

class GrContext;
class SkColorSpace;

class SkImage_GpuBase : public SkImage_Base {
public:
    SkImage_GpuBase(sk_sp<GrContext>, int width, int height, uint32_t uniqueID,
                    SkAlphaType, SkBudgeted, sk_sp<SkColorSpace>);
    ~SkImage_GpuBase() override;

    GrContext* context() const final { return fContext.get(); }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset) const final;

    sk_sp<GrTextureProxy> asTextureProxyRef() const override {
        // we shouldn't end up calling this
        SkASSERT(false);
        return this->INHERITED::asTextureProxyRef();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerState&, SkColorSpace*,
                                            sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const final;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const final {
        *uniqueID = this->uniqueID();
        return this->asTextureProxyRef();
    }

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    GrTexture* onGetTexture() const final;

    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>) const final;

    bool onIsValid(GrContext*) const final;

    static bool ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                                       GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
protected:
    sk_sp<GrContext>      fContext;
    const SkAlphaType     fAlphaType;  // alpha type for final image
    const SkBudgeted      fBudgeted;
    sk_sp<SkColorSpace>   fColorSpace; // color space for final image

private:
    typedef SkImage_Base INHERITED;
};

#endif

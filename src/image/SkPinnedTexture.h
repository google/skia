/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"

#if SK_SUPPORT_GPU

#include "GrTextureAdjuster.h"
#include "SkGr.h"

class SkPinnedTexture {
public:
    ~SkPinnedTexture() {
        // want the caller to have manually unpinned
        SkASSERT(nullptr == fPinnedProxy.get());
    }

    // If the texture is currently, pinned, this returns it (or an adjusted version, for use with
    // the passed-in params). Otherwise, returns nullptr.
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const SkBitmap&, const GrSamplerParams&,
                                            SkScalar scaleAdjust[2]) const;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const;
    bool pinAsTexture(GrContext*, const SkBitmap&) const;
    void unpinAsTexture(GrContext*) const;

private:
    mutable sk_sp<GrTextureProxy> fPinnedProxy;
    mutable int32_t fPinnedCount = 0;
    mutable uint32_t fPinnedUniqueID = 0;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> SkPinnedTexture::asTextureProxyRef(GrContext* context,
                                                         const SkBitmap& bitmap,
                                                         const GrSamplerParams& params,
                                                         SkScalar scaleAdjust[2]) const {
    if (!context) {
        return nullptr;
    }

    uint32_t uniqueID;
    sk_sp<GrTextureProxy> tex = this->refPinnedTextureProxy(&uniqueID);
    if (tex) {
        GrTextureAdjuster adjuster(context, fPinnedProxy,
                                   bitmap.alphaType(), bitmap.bounds(),
                                   fPinnedUniqueID, bitmap.colorSpace());
        return adjuster.refTextureProxySafeForParams(params, nullptr, scaleAdjust);
    }
    return nullptr;
}

sk_sp<GrTextureProxy> SkPinnedTexture::refPinnedTextureProxy(uint32_t* uniqueID) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
        *uniqueID = fPinnedUniqueID;
        return fPinnedProxy;
    }
    return nullptr;
}

bool SkPinnedTexture::pinAsTexture(GrContext* ctx, const SkBitmap& bitmap) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
    } else {
        SkASSERT(fPinnedCount == 0);
        SkASSERT(fPinnedUniqueID == 0);
        fPinnedProxy = GrRefCachedBitmapTextureProxy(ctx, bitmap,
                                                     GrSamplerParams::ClampNoFilter(), nullptr);
        if (!fPinnedProxy) {
            return false;
        }
        fPinnedUniqueID = bitmap.getGenerationID();
    }
    // Note: we only increment if the texture was successfully pinned
    ++fPinnedCount;
    return true;
}

void SkPinnedTexture::unpinAsTexture(GrContext* ctx) const {
    // Note: we always decrement, even if fPinnedTexture is null
    SkASSERT(fPinnedCount > 0);
    SkASSERT(fPinnedUniqueID != 0);

    if (0 == --fPinnedCount) {
        fPinnedProxy.reset(nullptr);
        fPinnedUniqueID = 0;
    }
}

#endif

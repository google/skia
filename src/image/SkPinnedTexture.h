/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrTextureAdjuster.h"

class SkPinnedTexture {
public:
    ~SkPinnedTexture() {
        // want the caller to have manually unpinned
        SkASSERT(nullptr == fPinnedProxy.get());
    }

    // If the texture is currently, pinned, this returns it (or an adjusted version, for use with
    // the passed-in params). Otherwise, returns nullptr.
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const SkImageInfo&, const GrSamplerParams&,
                                            SkScalar scaleAdjust[2]) const;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const;

    typedef std::function<std::pair<sk_sp<GrTextureProxy>, uint32_t>()> PinFunc;

    bool pinAsTexture(PinFunc pinFunc) const;
    void unpinAsTexture() const;

private:
    mutable sk_sp<GrTextureProxy> fPinnedProxy;
    mutable int32_t fPinnedCount = 0;
    mutable uint32_t fPinnedUniqueID = 0;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> SkPinnedTexture::asTextureProxyRef(GrContext* context,
                                                         const SkImageInfo& info,
                                                         const GrSamplerParams& params,
                                                         SkScalar scaleAdjust[2]) const {
    if (!context) {
        return nullptr;
    }

    uint32_t uniqueID;
    sk_sp<GrTextureProxy> tex = this->refPinnedTextureProxy(&uniqueID);
    if (tex) {
        GrTextureAdjuster adjuster(context, fPinnedProxy,
                                   info.alphaType(), info.bounds(),
                                   fPinnedUniqueID, info.colorSpace());
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

bool SkPinnedTexture::pinAsTexture(PinFunc pinFunc) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
    } else {
        SkASSERT(fPinnedCount == 0);
        SkASSERT(fPinnedUniqueID == 0);

        std::tie(fPinnedProxy, fPinnedUniqueID) = pinFunc();
        if (!fPinnedProxy) {
            fPinnedUniqueID = 0;
            return false;
        }
    }
    // Note: we only increment if the texture was successfully pinned
    ++fPinnedCount;
    return true;
}

void SkPinnedTexture::unpinAsTexture() const {
    // Note: we always decrement, even if fPinnedTexture is null
    SkASSERT(fPinnedCount > 0);
    SkASSERT(fPinnedUniqueID != 0);

    if (0 == --fPinnedCount) {
        fPinnedProxy.reset(nullptr);
        fPinnedUniqueID = 0;
    }
}

#endif

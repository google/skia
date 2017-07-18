/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkCanvas.h"
#include "SkColorSpaceXform_Base.h"
#include "SkColorSpaceXformImageGenerator.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkImageInfoPriv.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkSurface.h"
#include "SkTLazy.h"
#include "SkUnPreMultiplyPriv.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTextureAdjuster.h"
#include "SkGr.h"
#endif

// fixes https://bug.skia.org/5096
static bool is_not_subset(const SkBitmap& bm) {
    SkASSERT(bm.pixelRef());
    SkISize dim = SkISize::Make(bm.pixelRef()->width(), bm.pixelRef()->height());
    SkASSERT(dim != bm.dimensions() || bm.pixelRefOrigin().isZero());
    return dim == bm.dimensions();
}

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes, size_t* minSize) {
        const int maxDimension = SK_MaxS32 >> 2;

        if (info.width() <= 0 || info.height() <= 0) {
            return false;
        }
        if (info.width() > maxDimension || info.height() > maxDimension) {
            return false;
        }
        if ((unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType) {
            return false;
        }
        if ((unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType) {
            return false;
        }

        if (kUnknown_SkColorType == info.colorType()) {
            return false;
        }
        if (rowBytes < info.minRowBytes()) {
            return false;
        }

        size_t size = info.getSafeSize(rowBytes);
        if (0 == size) {
            return false;
        }

        if (minSize) {
            *minSize = size;
        }
        return true;
    }

    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb,
                   uint32_t id = kNeedNewImageUniqueID);
    ~SkImage_Raster() override;

    SkImageInfo onImageInfo() const override {
        return fBitmap.info();
    }
    SkAlphaType onAlphaType() const override {
        return fBitmap.alphaType();
    }

    bool onReadPixels(const SkImageInfo&, void*, size_t, int srcX, int srcY, CachingHint) const override;
    bool onPeekPixels(SkPixmap*) const override;
    const SkBitmap* onPeekBitmap() const override { return &fBitmap; }

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerParams&,
                                            SkColorSpace*, sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const override;
#endif

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const override;

    SkImage_Raster(const SkBitmap& bm, bool bitmapMayBeMutable = false)
        : INHERITED(bm.width(), bm.height(),
                    is_not_subset(bm) ? bm.getGenerationID()
                                      : (uint32_t)kNeedNewImageUniqueID)
        , fBitmap(bm)
    {
        SkASSERT(bitmapMayBeMutable || fBitmap.isImmutable());
    }

    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>, SkColorType,
                                    SkTransferFunctionBehavior) const override;

    bool onIsValid(GrContext* context) const override { return true; }

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const override;
    bool onPinAsTexture(GrContext*) const override;
    void onUnpinAsTexture(GrContext*) const override;
#endif

private:
    SkBitmap fBitmap;

#if SK_SUPPORT_GPU
    mutable sk_sp<GrTextureProxy> fPinnedProxy;
    mutable int32_t fPinnedCount = 0;
    mutable uint32_t fPinnedUniqueID = 0;
#endif

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const Info& info, sk_sp<SkData> data, size_t rowBytes, uint32_t id)
    : INHERITED(info.width(), info.height(), id)
{
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, release_data, data.release());
    fBitmap.setImmutable();
}

SkImage_Raster::~SkImage_Raster() {
#if SK_SUPPORT_GPU
    SkASSERT(nullptr == fPinnedProxy.get());  // want the caller to have manually unpinned
#endif
}

bool SkImage_Raster::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                  int srcX, int srcY, CachingHint) const {
    SkBitmap shallowCopy(fBitmap);
    return shallowCopy.readPixels(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
}

bool SkImage_Raster::onPeekPixels(SkPixmap* pm) const {
    return fBitmap.peekPixels(pm);
}

bool SkImage_Raster::getROPixels(SkBitmap* dst, SkColorSpace* dstColorSpace, CachingHint) const {
    *dst = fBitmap;
    return true;
}

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkImage_Raster::asTextureProxyRef(GrContext* context,
                                                        const GrSamplerParams& params,
                                                        SkColorSpace* dstColorSpace,
                                                        sk_sp<SkColorSpace>* texColorSpace,
                                                        SkScalar scaleAdjust[2]) const {
    if (!context) {
        return nullptr;
    }

    if (texColorSpace) {
        *texColorSpace = sk_ref_sp(fBitmap.colorSpace());
    }

    uint32_t uniqueID;
    sk_sp<GrTextureProxy> tex = this->refPinnedTextureProxy(&uniqueID);
    if (tex) {
        GrTextureAdjuster adjuster(context, fPinnedProxy,
                                   fBitmap.alphaType(), fBitmap.bounds(),
                                   fPinnedUniqueID, fBitmap.colorSpace());
        return adjuster.refTextureProxySafeForParams(params, nullptr, scaleAdjust);
    }

    return GrRefCachedBitmapTextureProxy(context, fBitmap, params, scaleAdjust);
}
#endif

#if SK_SUPPORT_GPU

sk_sp<GrTextureProxy> SkImage_Raster::refPinnedTextureProxy(uint32_t* uniqueID) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
        *uniqueID = fPinnedUniqueID;
        return fPinnedProxy;
    }
    return nullptr;
}

bool SkImage_Raster::onPinAsTexture(GrContext* ctx) const {
    if (fPinnedProxy) {
        SkASSERT(fPinnedCount > 0);
        SkASSERT(fPinnedUniqueID != 0);
    } else {
        SkASSERT(fPinnedCount == 0);
        SkASSERT(fPinnedUniqueID == 0);
        fPinnedProxy = GrRefCachedBitmapTextureProxy(ctx, fBitmap,
                                                     GrSamplerParams::ClampNoFilter(), nullptr);
        if (!fPinnedProxy) {
            return false;
        }
        fPinnedUniqueID = fBitmap.getGenerationID();
    }
    // Note: we only increment if the texture was successfully pinned
    ++fPinnedCount;
    return true;
}

void SkImage_Raster::onUnpinAsTexture(GrContext* ctx) const {
    // Note: we always decrement, even if fPinnedTexture is null
    SkASSERT(fPinnedCount > 0);
    SkASSERT(fPinnedUniqueID != 0);

    if (0 == --fPinnedCount) {
        fPinnedProxy.reset(nullptr);
        fPinnedUniqueID = 0;
    }
}
#endif

sk_sp<SkImage> SkImage_Raster::onMakeSubset(const SkIRect& subset) const {
    // TODO : could consider heurist of sharing pixels, if subset is pretty close to complete

    SkImageInfo info = SkImageInfo::MakeN32(subset.width(), subset.height(), fBitmap.alphaType());
    auto surface(SkSurface::MakeRaster(info));
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawImage(this, SkIntToScalar(-subset.x()), SkIntToScalar(-subset.y()),
                                    nullptr);
    return surface->makeImageSnapshot();
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> MakeRasterCopyPriv(const SkPixmap& pmap, uint32_t id) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    sk_sp<SkData> data(SkData::MakeWithCopy(pmap.addr(), size));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), id);
}

sk_sp<SkImage> SkImage::MakeRasterCopy(const SkPixmap& pmap) {
    return MakeRasterCopyPriv(pmap, kNeedNewImageUniqueID);
}

sk_sp<SkImage> SkImage::MakeRasterData(const SkImageInfo& info, sk_sp<SkData> data,
                                       size_t rowBytes) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Raster>(info, std::move(data), rowBytes);
}

sk_sp<SkImage> SkImage::MakeFromRaster(const SkPixmap& pmap, RasterReleaseProc proc,
                                       ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes());
}

sk_sp<SkImage> SkMakeImageFromRasterBitmapPriv(const SkBitmap& bm, SkCopyPixelsMode cpm,
                                               uint32_t idForCopy) {
    if (kAlways_SkCopyPixelsMode == cpm || (!bm.isImmutable() && kNever_SkCopyPixelsMode != cpm)) {
        SkPixmap pmap;
        if (bm.peekPixels(&pmap)) {
            return MakeRasterCopyPriv(pmap, idForCopy);
        } else {
            return sk_sp<SkImage>();
        }
    }

    return sk_make_sp<SkImage_Raster>(bm, kNever_SkCopyPixelsMode == cpm);
}

sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap& bm, SkCopyPixelsMode cpm) {
    if (!SkImageInfoIsValidAllowNumericalCS(bm.info()) || bm.rowBytes() < bm.info().minRowBytes()) {
        return nullptr;
    }

    return SkMakeImageFromRasterBitmapPriv(bm, cpm, kNeedNewImageUniqueID);
}

sk_sp<SkImage> SkMakeImageInColorSpace(const SkBitmap& bm, sk_sp<SkColorSpace> dstCS, uint32_t id,
                                       SkCopyPixelsMode cpm) {
    if (!SkImageInfoIsValidAllowNumericalCS(bm.info()) || !bm.getPixels() ||
            bm.rowBytes() < bm.info().minRowBytes() || !dstCS) {
        return nullptr;
    }

    sk_sp<SkColorSpace> srcCS = bm.info().refColorSpace();
    if (!srcCS) {
        // Treat nullptr as sRGB.
        srcCS = SkColorSpace::MakeSRGB();
    }

    sk_sp<SkImage> image = nullptr;

    // For the Android use case, this is very likely to be true.
    if (SkColorSpace::Equals(srcCS.get(), dstCS.get())) {
        SkASSERT(kNeedNewImageUniqueID == id || bm.getGenerationID() == id);
        image = SkMakeImageFromRasterBitmapPriv(bm, cpm, id);
    } else {
        image = SkImage::MakeFromGenerator(SkColorSpaceXformImageGenerator::Make(bm, dstCS, cpm,
                                                                                 id));
    }

    // If the caller suplied an id, we must propagate that to the image we return
    SkASSERT(kNeedNewImageUniqueID == id || image->uniqueID() == id);
    return image;
}

const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* image) {
    return ((const SkImage_Raster*)image)->getPixelRef();
}

bool SkImage_Raster::onAsLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    if (kRO_LegacyBitmapMode == mode) {
        // When we're a snapshot from a surface, our bitmap may not be marked immutable
        // even though logically always we are, but in that case we can't physically share our
        // pixelref since the caller might call setImmutable() themselves
        // (thus changing our state).
        if (fBitmap.isImmutable()) {
            bitmap->setInfo(fBitmap.info(), fBitmap.rowBytes());
            bitmap->setPixelRef(sk_ref_sp(fBitmap.pixelRef()),
                                fBitmap.pixelRefOrigin().x(),
                                fBitmap.pixelRefOrigin().y());
            return true;
        }
    }
    return this->INHERITED::onAsLegacyBitmap(bitmap, mode);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Raster::onMakeColorSpace(sk_sp<SkColorSpace> target,
                                                SkColorType targetColorType,
                                                SkTransferFunctionBehavior premulBehavior) const {
    SkPixmap src;
    SkAssertResult(fBitmap.peekPixels(&src));

    // Treat nullptr srcs as sRGB.
    if (!src.colorSpace()) {
        if (target->isSRGB()) {
            return sk_ref_sp(const_cast<SkImage*>((SkImage*)this));
        }

        src.setColorSpace(SkColorSpace::MakeSRGB());
    }

    SkImageInfo dstInfo = fBitmap.info().makeColorType(targetColorType).makeColorSpace(target);
    SkBitmap dst;
    dst.allocPixels(dstInfo);

    SkAssertResult(dst.writePixels(src, 0, 0, premulBehavior));
    dst.setImmutable();
    return SkImage::MakeFromBitmap(dst);
}

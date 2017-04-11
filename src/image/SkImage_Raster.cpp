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
#include "SkColorSpaceXformPriv.h"
#include "SkColorTable.h"
#include "SkData.h"
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
    SkISize dim = bm.pixelRef()->info().dimensions();
    SkASSERT(dim != bm.dimensions() || bm.pixelRefOrigin().isZero());
    return dim == bm.dimensions();
}

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes, bool hasColorTable,
                          size_t* minSize) {
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

        const bool needsCT = kIndex_8_SkColorType == info.colorType();
        if (needsCT != hasColorTable) {
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

    SkImage_Raster(const SkImageInfo&, sk_sp<SkData>, size_t rb, SkColorTable*);
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
        if (bm.pixelRef()->isPreLocked()) {
            // we only preemptively lock if there is no chance of triggering something expensive
            // like a lazy decode or imagegenerator. PreLocked means it is flat pixels already.
            fBitmap.lockPixels();
        }
        SkASSERT(bitmapMayBeMutable || fBitmap.isImmutable());
    }

    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>) const override;

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

SkImage_Raster::SkImage_Raster(const Info& info, sk_sp<SkData> data, size_t rowBytes,
                               SkColorTable* ctable)
    : INHERITED(info.width(), info.height(), kNeedNewImageUniqueID)
{
    void* addr = const_cast<void*>(data->data());

    fBitmap.installPixels(info, addr, rowBytes, ctable, release_data, data.release());
    fBitmap.setImmutable();
    fBitmap.lockPixels();
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

sk_sp<SkImage> SkImage::MakeRasterCopy(const SkPixmap& pmap) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(),
                                   pmap.ctable() != nullptr, &size) || !pmap.addr()) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    sk_sp<SkData> data(SkData::MakeWithCopy(pmap.addr(), size));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), pmap.ctable());
}


sk_sp<SkImage> SkImage::MakeRasterData(const SkImageInfo& info, sk_sp<SkData> data,
                                       size_t rowBytes) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(info, rowBytes, false, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    SkColorTable* ctable = nullptr;
    return sk_make_sp<SkImage_Raster>(info, std::move(data), rowBytes, ctable);
}

sk_sp<SkImage> SkImage::MakeFromRaster(const SkPixmap& pmap, RasterReleaseProc proc,
                                       ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Raster::ValidArgs(pmap.info(), pmap.rowBytes(), pmap.ctable(), &size) ||
        !pmap.addr())
    {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), pmap.ctable());
}

sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap& bm, SkCopyPixelsMode cpm) {
    bool hasColorTable = false;
    if (kIndex_8_SkColorType == bm.colorType()) {
        SkAutoLockPixels autoLockPixels(bm);
        hasColorTable = bm.getColorTable() != nullptr;
    }

    if (!SkImage_Raster::ValidArgs(bm.info(), bm.rowBytes(), hasColorTable, nullptr)) {
        return nullptr;
    }

    if (kAlways_SkCopyPixelsMode == cpm || (!bm.isImmutable() && kNever_SkCopyPixelsMode != cpm)) {
        SkBitmap tmp(bm);
        tmp.lockPixels();
        SkPixmap pmap;
        if (tmp.getPixels() && tmp.peekPixels(&pmap)) {
            return SkImage::MakeRasterCopy(pmap);
        }
    } else {
        return sk_make_sp<SkImage_Raster>(bm, kNever_SkCopyPixelsMode == cpm);
    }
    return sk_sp<SkImage>();
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

static inline void do_color_xform_non_linear_blending(SkBitmap* dst, const SkPixmap& src) {
    SkDEBUGCODE(SkColorSpaceTransferFn fn;);
    SkASSERT(dst->colorSpace()->isNumericalTransferFn(&fn) &&
             src.colorSpace()->isNumericalTransferFn(&fn));

    void* dstPixels = dst->getPixels();
    const void* srcPixels = src.addr();
    size_t dstRowBytes = dst->rowBytes();
    size_t srcRowBytes = src.rowBytes();
    if (kN32_SkColorType != src.colorType()) {
        SkAssertResult(src.readPixels(src.info().makeColorType(kN32_SkColorType), dstPixels,
                                      dstRowBytes, 0, 0));

        srcPixels = dstPixels;
        srcRowBytes = dstRowBytes;
    }

    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform_Base::New(
            src.colorSpace(), dst->colorSpace(), SkTransferFunctionBehavior::kIgnore);

    void* dstRow = dstPixels;
    const void* srcRow = srcPixels;
    for (int y = 0; y < dst->height(); y++) {
        // This function assumes non-linear blending.  Which means that we must start by
        // unpremultiplying in the gamma encoded space.
        const void* tmpRow = srcRow;
        if (kPremul_SkAlphaType == src.alphaType()) {
            SkUnpremultiplyRow<false>((uint32_t*) dstRow, (const uint32_t*) srcRow, dst->width());
            tmpRow = dstRow;
        }

        SkColorSpaceXform::ColorFormat fmt = select_xform_format(kN32_SkColorType);
        SkAssertResult(xform->apply(fmt, dstRow, fmt, tmpRow, dst->width(), dst->alphaType()));

        dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
        srcRow = SkTAddOffset<const void>(srcRow, srcRowBytes);
    }
}

sk_sp<SkImage> SkImage_Raster::onMakeColorSpace(sk_sp<SkColorSpace> target) const {
    // Force the color type of the new image to be kN32_SkColorType.
    // (1) This means we lose precision on F16 images.  This is necessary while this function is
    //     used to pre-transform inputs to a legacy canvas.  Legacy canvases do not handle F16.
    // (2) kIndex8 and kGray8 must be expanded in order perform a color space transformation.
    // (3) Seems reasonable to expand k565 and k4444.  It's nice to avoid these color types for
    //     clients who opt into color space support.
    SkImageInfo dstInfo = fBitmap.info().makeColorType(kN32_SkColorType).makeColorSpace(target);
    SkBitmap dst;
    dst.allocPixels(dstInfo);

    SkPixmap src;
    SkTLazy<SkBitmap> tmp;
    if (!fBitmap.peekPixels(&src)) {
        tmp.init(fBitmap);
        tmp.get()->lockPixels();
        SkAssertResult(tmp.get()->peekPixels(&src));
    }

    // Treat nullptr srcs as sRGB.
    if (!src.colorSpace()) {
        src.setColorSpace(SkColorSpace::MakeSRGB());
    }

    do_color_xform_non_linear_blending(&dst, src);
    dst.setImmutable();
    return SkImage::MakeFromBitmap(dst);
}

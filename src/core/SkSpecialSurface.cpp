/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "include/core/SkCanvas.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkSurfacePriv.h"

 ///////////////////////////////////////////////////////////////////////////////
class SkSpecialSurface_Base : public SkSpecialSurface {
public:
    SkSpecialSurface_Base(const SkIRect& subset, const SkSurfaceProps* props)
        : INHERITED(subset, props)
        , fCanvas(nullptr) {
    }

    virtual ~SkSpecialSurface_Base() { }

    // reset is called after an SkSpecialImage has been snapped
    void reset() { fCanvas.reset(); }

    // This can return nullptr if reset has already been called or something when wrong in the ctor
    SkCanvas* onGetCanvas() { return fCanvas.get(); }

    virtual sk_sp<SkSpecialImage> onMakeImageSnapshot() = 0;

protected:
    std::unique_ptr<SkCanvas> fCanvas;   // initialized by derived classes in ctors

private:
    typedef SkSpecialSurface INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
static SkSpecialSurface_Base* as_SB(SkSpecialSurface* surface) {
    return static_cast<SkSpecialSurface_Base*>(surface);
}

SkSpecialSurface::SkSpecialSurface(const SkIRect& subset,
                                   const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props).flags(), kUnknown_SkPixelGeometry)
    , fSubset(subset) {
    SkASSERT(fSubset.width() > 0);
    SkASSERT(fSubset.height() > 0);
}

SkCanvas* SkSpecialSurface::getCanvas() {
    return as_SB(this)->onGetCanvas();
}

sk_sp<SkSpecialImage> SkSpecialSurface::makeImageSnapshot() {
    sk_sp<SkSpecialImage> image(as_SB(this)->onMakeImageSnapshot());
    as_SB(this)->reset();
    return image;   // the caller gets the creation ref
}

///////////////////////////////////////////////////////////////////////////////
#include "include/core/SkMallocPixelRef.h"

class SkSpecialSurface_Raster : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Raster(const SkImageInfo& info,
                            sk_sp<SkPixelRef> pr,
                            const SkIRect& subset,
                            const SkSurfaceProps* props)
        : INHERITED(subset, props) {
        SkASSERT(info.width() == pr->width() && info.height() == pr->height());
        fBitmap.setInfo(info, info.minRowBytes());
        fBitmap.setPixelRef(std::move(pr), 0, 0);

        fCanvas.reset(new SkCanvas(fBitmap, this->props()));
        fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
        fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
    }

    ~SkSpecialSurface_Raster() override { }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        return SkSpecialImage::MakeFromRaster(this->subset(), fBitmap, &this->props());
    }

private:
    SkBitmap fBitmap;

    typedef SkSpecialSurface_Base INHERITED;
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeFromBitmap(const SkIRect& subset, SkBitmap& bm,
                                                         const SkSurfaceProps* props) {
    if (subset.isEmpty() || !SkSurfaceValidateRasterInfo(bm.info(), bm.rowBytes())) {
        return nullptr;
    }
    return sk_make_sp<SkSpecialSurface_Raster>(bm.info(), sk_ref_sp(bm.pixelRef()), subset, props);
}

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRaster(const SkImageInfo& info,
                                                     const SkSurfaceProps* props) {
    if (!SkSurfaceValidateRasterInfo(info)) {
        return nullptr;
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, 0);
    if (!pr) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(info.width(), info.height());

    return sk_make_sp<SkSpecialSurface_Raster>(info, std::move(pr), subset, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "include/private/GrRecordingContext.h"
#include "src/core/SkMakeUnique.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGpuDevice.h"

class SkSpecialSurface_Gpu : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Gpu(GrRecordingContext* context,
                         std::unique_ptr<GrRenderTargetContext> renderTargetContext,
                         int width, int height, const SkIRect& subset)
            : INHERITED(subset, &renderTargetContext->surfaceProps())
            , fProxy(renderTargetContext->asTextureProxyRef()) {
        // CONTEXT TODO: remove this use of 'backdoor' to create an SkGpuDevice
        auto device = SkGpuDevice::Make(context->priv().backdoor(), std::move(renderTargetContext),
                                        SkGpuDevice::kUninit_InitContents);
        if (!device) {
            return;
        }

        fCanvas.reset(new SkCanvas(std::move(device)));
        fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
        fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
    }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fProxy) {
            return nullptr;
        }
        GrColorType ct = SkColorTypeToGrColorType(fCanvas->imageInfo().colorType());

        return SkSpecialImage::MakeDeferredFromGpu(fCanvas->getGrContext(),
                                                   this->subset(),
                                                   kNeedNewImageUniqueID_SpecialImage,
                                                   std::move(fProxy), ct,
                                                   fCanvas->imageInfo().refColorSpace(),
                                                   &this->props());
    }

private:
    sk_sp<GrTextureProxy> fProxy;
    typedef SkSpecialSurface_Base INHERITED;
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRenderTarget(GrRecordingContext* context,
                                                           int width, int height,
                                                           GrColorType colorType,
                                                           sk_sp<SkColorSpace> colorSpace,
                                                           const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    auto renderTargetContext = context->priv().makeDeferredRenderTargetContext(
            SkBackingFit::kApprox, width, height, colorType, std::move(colorSpace), 1,
            GrMipMapped::kNo, kBottomLeft_GrSurfaceOrigin, props);
    if (!renderTargetContext) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(width, height);

    return sk_make_sp<SkSpecialSurface_Gpu>(context, std::move(renderTargetContext),
                                            width, height, subset);
}

#endif

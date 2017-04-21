/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "SkCanvas.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkSurfacePriv.h"

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
#include "SkMallocPixelRef.h"

class SkSpecialSurface_Raster : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Raster(sk_sp<SkPixelRef> pr,
                            const SkIRect& subset,
                            const SkSurfaceProps* props)
        : INHERITED(subset, props) {
        const SkImageInfo& info = pr->info();

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
    return sk_make_sp<SkSpecialSurface_Raster>(sk_ref_sp(bm.pixelRef()), subset, props);
}

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRaster(const SkImageInfo& info,
                                                     const SkSurfaceProps* props) {
    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeZeroed(info, 0, nullptr);
    if (!pr) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(pr->info().width(), pr->info().height());

    return sk_make_sp<SkSpecialSurface_Raster>(std::move(pr), subset, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "GrContext.h"
#include "SkGpuDevice.h"

class SkSpecialSurface_Gpu : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Gpu(GrContext* context, sk_sp<GrRenderTargetContext> renderTargetContext,
                         int width, int height, const SkIRect& subset)
        : INHERITED(subset, &renderTargetContext->surfaceProps())
        , fRenderTargetContext(std::move(renderTargetContext)) {

        sk_sp<SkBaseDevice> device(SkGpuDevice::Make(context, fRenderTargetContext, width, height,
                                                     SkGpuDevice::kUninit_InitContents));
        if (!device) {
            return;
        }

        fCanvas.reset(new SkCanvas(device.get()));
        fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
        fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
    }

    ~SkSpecialSurface_Gpu() override { }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fRenderTargetContext->asTextureProxy()) {
            return nullptr;
        }
        sk_sp<SkSpecialImage> tmp(SkSpecialImage::MakeDeferredFromGpu(
                                                   fCanvas->getGrContext(),
                                                   this->subset(),
                                                   kNeedNewImageUniqueID_SpecialImage,
                                                   fRenderTargetContext->asTextureProxyRef(),
                                                   fRenderTargetContext->refColorSpace(),
                                                   &this->props()));
        fRenderTargetContext = nullptr;
        return tmp;
    }

private:
    sk_sp<GrRenderTargetContext> fRenderTargetContext;

    typedef SkSpecialSurface_Base INHERITED;
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRenderTarget(GrContext* context,
                                                           int width, int height,
                                                           GrPixelConfig config,
                                                           sk_sp<SkColorSpace> colorSpace) {
    if (!context) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeRenderTargetContext(
        SkBackingFit::kApprox, width, height, config, std::move(colorSpace)));
    if (!renderTargetContext) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(width, height);

    return sk_make_sp<SkSpecialSurface_Gpu>(context, std::move(renderTargetContext),
                                            width, height, subset);
}

#endif

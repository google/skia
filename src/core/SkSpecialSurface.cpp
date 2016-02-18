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
    SkSpecialSurface_Base(SkImageFilter::Proxy* proxy,
                          const SkIRect& subset, const SkSurfaceProps* props)
        : INHERITED(proxy, subset, props)
        , fCanvas(nullptr) {
    }

    virtual ~SkSpecialSurface_Base() { }

    // reset is called after an SkSpecialImage has been snapped
    void reset() { fCanvas.reset(); }

    // This can return nullptr if reset has already been called or something when wrong in the ctor
    SkCanvas* onGetCanvas() { return fCanvas; }

    virtual SkSpecialImage* onNewImageSnapshot() = 0;

protected:
    SkAutoTUnref<SkCanvas> fCanvas;   // initialized by derived classes in ctors

private:
    typedef SkSpecialSurface INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
static SkSpecialSurface_Base* as_SB(SkSpecialSurface* surface) {
    return static_cast<SkSpecialSurface_Base*>(surface);
}

SkSpecialSurface::SkSpecialSurface(SkImageFilter::Proxy* proxy,
                                   const SkIRect& subset,
                                   const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props))
    , fSubset(subset)
    , fProxy(proxy) {
    SkASSERT(fSubset.width() > 0);
    SkASSERT(fSubset.height() > 0);
}

SkCanvas* SkSpecialSurface::getCanvas() {
    return as_SB(this)->onGetCanvas();
}

SkSpecialImage* SkSpecialSurface::newImageSnapshot() {
    SkSpecialImage* image = as_SB(this)->onNewImageSnapshot();
    as_SB(this)->reset();
    return image;   // the caller gets the creation ref
}

///////////////////////////////////////////////////////////////////////////////
#include "SkMallocPixelRef.h"

class SkSpecialSurface_Raster : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Raster(SkImageFilter::Proxy* proxy,
                            SkPixelRef* pr,
                            const SkIRect& subset,
                            const SkSurfaceProps* props)
        : INHERITED(proxy, subset, props) {   
        const SkImageInfo& info = pr->info();

        fBitmap.setInfo(info, info.minRowBytes());
        fBitmap.setPixelRef(pr);

        fCanvas.reset(new SkCanvas(fBitmap));
    }

    ~SkSpecialSurface_Raster() override { }

    SkSpecialImage* onNewImageSnapshot() override {
        return SkSpecialImage::NewFromRaster(this->proxy(), this->subset(), fBitmap);
    }

private:
    SkBitmap fBitmap;

    typedef SkSpecialSurface_Base INHERITED;
};

SkSpecialSurface* SkSpecialSurface::NewFromBitmap(SkImageFilter::Proxy* proxy,
                                                  const SkIRect& subset, SkBitmap& bm,
                                                  const SkSurfaceProps* props) {
    return new SkSpecialSurface_Raster(proxy, bm.pixelRef(), subset, props);
}

SkSpecialSurface* SkSpecialSurface::NewRaster(SkImageFilter::Proxy* proxy,
                                              const SkImageInfo& info,
                                              const SkSurfaceProps* props) {
    SkAutoTUnref<SkPixelRef> pr(SkMallocPixelRef::NewZeroed(info, 0, nullptr));
    if (nullptr == pr.get()) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(pr->info().width(), pr->info().height());

    return new SkSpecialSurface_Raster(proxy, pr, subset, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "GrContext.h"
#include "SkGpuDevice.h"

class SkSpecialSurface_Gpu : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Gpu(SkImageFilter::Proxy* proxy,
                         GrTexture* texture,
                         const SkIRect& subset,
                         const SkSurfaceProps* props)
        : INHERITED(proxy, subset, props)
        , fTexture(SkRef(texture)) {

        SkASSERT(fTexture->asRenderTarget());

        SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(fTexture->asRenderTarget(), props,
                                                             SkGpuDevice::kUninit_InitContents));
        if (!device) {
            return;
        }

        fCanvas.reset(new SkCanvas(device));
    }

    ~SkSpecialSurface_Gpu() override { }

    SkSpecialImage* onNewImageSnapshot() override {
        return SkSpecialImage::NewFromGpu(this->proxy(), this->subset(),
                                          kNeedNewImageUniqueID_SpecialImage, fTexture);
    }

private:
    SkAutoTUnref<GrTexture> fTexture;

    typedef SkSpecialSurface_Base INHERITED;
};

SkSpecialSurface* SkSpecialSurface::NewFromTexture(SkImageFilter::Proxy* proxy, 
                                                   const SkIRect& subset,
                                                   GrTexture* texture,
                                                   const SkSurfaceProps* props) {
    if (!texture->asRenderTarget()) {
        return nullptr;
    }

    return new SkSpecialSurface_Gpu(proxy, texture, subset, props);
}

SkSpecialSurface* SkSpecialSurface::NewRenderTarget(SkImageFilter::Proxy* proxy,
                                                    GrContext* context,
                                                    const GrSurfaceDesc& desc,
                                                    const SkSurfaceProps* props) {
    if (!context || !SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag)) {
        return nullptr;
    }

    SkAutoTUnref<GrTexture> temp(context->textureProvider()->createApproxTexture(desc));
    if (!temp) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(desc.fWidth, desc.fHeight);

    return new SkSpecialSurface_Gpu(proxy, temp, subset, props);
}

#else

SkSpecialSurface* SkSpecialSurface::NewFromTexture(SkImageFilter::Proxy* proxy,
                                                   const SkIRect& subset,
                                                   GrTexture*,
                                                   const SkSurfaceProps*) {
    return nullptr;
}

SkSpecialSurface* SkSpecialSurface::NewRenderTarget(SkImageFilter::Proxy* proxy,
                                                    GrContext* context,
                                                    const GrSurfaceDesc& desc,
                                                    const SkSurfaceProps* props) {
    return nullptr;
}

#endif

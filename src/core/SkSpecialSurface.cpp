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
    SkCanvas* onGetCanvas() { return fCanvas; }

    virtual sk_sp<SkSpecialImage> onMakeImageSnapshot() = 0;

protected:
    SkAutoTUnref<SkCanvas> fCanvas;   // initialized by derived classes in ctors

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
    SkSpecialSurface_Raster(SkPixelRef* pr,
                            const SkIRect& subset,
                            const SkSurfaceProps* props)
        : INHERITED(subset, props) {
        const SkImageInfo& info = pr->info();

        fBitmap.setInfo(info, info.minRowBytes());
        fBitmap.setPixelRef(pr);

        fCanvas.reset(new SkCanvas(fBitmap, this->props()));
        fCanvas->clipRect(SkRect::Make(subset));
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
    return sk_make_sp<SkSpecialSurface_Raster>(bm.pixelRef(), subset, props);
}

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRaster(const SkImageInfo& info,
                                                     const SkSurfaceProps* props) {
    SkAutoTUnref<SkPixelRef> pr(SkMallocPixelRef::NewZeroed(info, 0, nullptr));
    if (nullptr == pr.get()) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(pr->info().width(), pr->info().height());

    return sk_make_sp<SkSpecialSurface_Raster>(pr, subset, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "GrContext.h"
#include "SkGpuDevice.h"

class SkSpecialSurface_Gpu : public SkSpecialSurface_Base {
public:
    SkSpecialSurface_Gpu(sk_sp<GrTexture> texture,
                         int width, int height,
                         const SkIRect& subset,
                         const SkSurfaceProps* props)
        : INHERITED(subset, props)
        , fTexture(std::move(texture)) {

        SkASSERT(fTexture->asRenderTarget());

        sk_sp<SkGpuDevice> device(SkGpuDevice::Create(fTexture->asRenderTarget(), width, height,
                                                      props,
                                                      SkGpuDevice::kUninit_InitContents));
        if (!device) {
            return;
        }

        fCanvas.reset(new SkCanvas(device.get()));
        fCanvas->clipRect(SkRect::Make(subset));
    }

    ~SkSpecialSurface_Gpu() override { }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        // Note: we are intentionally zeroing out 'fTexture' here
        return SkSpecialImage::MakeFromGpu(this->subset(),
                                           kNeedNewImageUniqueID_SpecialImage,
                                           std::move(fTexture),
                                           &this->props());
    }

private:
    sk_sp<GrTexture> fTexture;

    typedef SkSpecialSurface_Base INHERITED;
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRenderTarget(GrContext* context,
                                                           int width, int height,
                                                           GrPixelConfig config) {
    if (!context) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    sk_sp<GrTexture> tex(context->textureProvider()->createApproxTexture(desc));
    if (!tex) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(width, height);

    return sk_make_sp<SkSpecialSurface_Gpu>(std::move(tex), width, height, subset, nullptr);
}

#endif

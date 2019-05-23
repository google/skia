/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <atomic>
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/core/SkFontLCDConfig.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImagePriv.h"
#include "src/image/SkSurface_Base.h"

static SkPixelGeometry compute_default_geometry() {
    SkFontLCDConfig::LCDOrder order = SkFontLCDConfig::GetSubpixelOrder();
    if (SkFontLCDConfig::kNONE_LCDOrder == order) {
        return kUnknown_SkPixelGeometry;
    } else {
        // Bit0 is RGB(0), BGR(1)
        // Bit1 is H(0), V(1)
        const SkPixelGeometry gGeo[] = {
            kRGB_H_SkPixelGeometry,
            kBGR_H_SkPixelGeometry,
            kRGB_V_SkPixelGeometry,
            kBGR_V_SkPixelGeometry,
        };
        int index = 0;
        if (SkFontLCDConfig::kBGR_LCDOrder == order) {
            index |= 1;
        }
        if (SkFontLCDConfig::kVertical_LCDOrientation == SkFontLCDConfig::GetSubpixelOrientation()){
            index |= 2;
        }
        return gGeo[index];
    }
}

SkSurfaceProps::SkSurfaceProps() : fFlags(0), fPixelGeometry(kUnknown_SkPixelGeometry) {}

SkSurfaceProps::SkSurfaceProps(InitType) : fFlags(0), fPixelGeometry(compute_default_geometry()) {}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, InitType)
    : fFlags(flags)
    , fPixelGeometry(compute_default_geometry())
{}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, SkPixelGeometry pg)
    : fFlags(flags), fPixelGeometry(pg)
{}

SkSurfaceProps::SkSurfaceProps(const SkSurfaceProps& other)
    : fFlags(other.fFlags)
    , fPixelGeometry(other.fPixelGeometry)
{}

///////////////////////////////////////////////////////////////////////////////

SkSurface_Base::SkSurface_Base(int width, int height, const SkSurfaceProps* props)
    : INHERITED(width, height, props) {
}

SkSurface_Base::SkSurface_Base(const SkImageInfo& info, const SkSurfaceProps* props)
    : INHERITED(info, props) {
}

SkSurface_Base::~SkSurface_Base() {
    // in case the canvas outsurvives us, we null the callback
    if (fCachedCanvas) {
        fCachedCanvas->setSurfaceBase(nullptr);
    }
}

GrBackendTexture SkSurface_Base::onGetBackendTexture(BackendHandleAccess) {
    return GrBackendTexture(); // invalid
}

GrBackendRenderTarget SkSurface_Base::onGetBackendRenderTarget(BackendHandleAccess) {
    return GrBackendRenderTarget(); // invalid
}

bool SkSurface_Base::onReplaceBackendTexture(const GrBackendTexture&,
                                             GrSurfaceOrigin,
                                             TextureReleaseProc,
                                             ReleaseContext) {
    return false;
}

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    auto image = this->makeImageSnapshot();
    if (image) {
        canvas->drawImage(image, x, y, paint);
    }
}

void SkSurface_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info, const SkIRect& srcRect,
                                                 SkSurface::RescaleGamma rescaleGamma,
                                                 SkFilterQuality rescaleQuality,
                                                 SkSurface::ReadPixelsCallback callback,
                                                 SkSurface::ReadPixelsContext context) {
    int srcW = srcRect.width();
    int srcH = srcRect.height();
    float sx = (float)info.width() / srcW;
    float sy = (float)info.height() / srcH;
    // How many bilerp/bicubic steps to do in X and Y. + means upscaling, - means downscaling.
    int stepsX;
    int stepsY;
    if (rescaleQuality > kNone_SkFilterQuality) {
        stepsX = static_cast<int>((sx > 1.f) ? std::ceil(std::log2f(sx))
                                             : std::floor(std::log2f(sx)));
        stepsY = static_cast<int>((sy > 1.f) ? std::ceil(std::log2f(sy))
                                             : std::floor(std::log2f(sy)));
    } else {
        stepsX = sx != 1.f;
        stepsY = sy != 1.f;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    if (stepsX < 0 || stepsY < 0) {
        // Don't trigger MIP generation. We don't currently have a way to trigger bicubic for
        // downscaling draws.
        rescaleQuality = std::min(rescaleQuality, kLow_SkFilterQuality);
    }
    paint.setFilterQuality(rescaleQuality);
    sk_sp<SkSurface> src(SkRef(this));
    int srcX = srcRect.fLeft;
    int srcY = srcRect.fTop;
    SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == SkSurface::RescaleGamma::kLinear &&
        this->getCanvas()->imageInfo().colorSpace() &&
        !this->getCanvas()->imageInfo().colorSpace()->gammaIsLinear()) {
        auto cs = this->getCanvas()->imageInfo().colorSpace()->makeLinearGamma();
        // Promote to F16 color type to preserve precision.
        auto ii = SkImageInfo::Make(srcW, srcH, kRGBA_F16_SkColorType,
                                    this->getCanvas()->imageInfo().alphaType(), std::move(cs));
        auto linearSurf = this->makeSurface(ii);
        if (!linearSurf) {
            // Maybe F16 isn't supported? Try again with original color type.
            ii = ii.makeColorType(this->getCanvas()->imageInfo().colorType());
            linearSurf = this->makeSurface(ii);
            if (!linearSurf) {
                callback(context, nullptr, 0);
                return;
            }
        }
        this->draw(linearSurf->getCanvas(), -srcX, -srcY, &paint);
        src = std::move(linearSurf);
        srcX = 0;
        srcY = 0;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    while (stepsX || stepsY) {
        int nextW = info.width();
        int nextH = info.height();
        if (stepsX < 0) {
            nextW = info.width() << (-stepsX - 1);
            stepsX++;
        } else if (stepsX != 0) {
            if (stepsX > 1) {
                nextW = srcW * 2;
            }
            --stepsX;
        }
        if (stepsY < 0) {
            nextH = info.height() << (-stepsY - 1);
            stepsY++;
        } else if (stepsY != 0) {
            if (stepsY > 1) {
                nextH = srcH * 2;
            }
            --stepsY;
        }
        auto ii = src->getCanvas()->imageInfo().makeWH(nextW, nextH);
        if (!stepsX && !stepsY) {
            // Might as well fold conversion to final info in the last step.
            ii = info;
        }
        auto next = this->makeSurface(ii);
        if (!next) {
            callback(context, nullptr, 0);
            return;
        }
        next->getCanvas()->drawImageRect(
                src->makeImageSnapshot(), SkIRect::MakeXYWH(srcX, srcY, srcW, srcH),
                SkRect::MakeWH((float)nextW, (float)nextH), &paint, constraint);
        src = std::move(next);
        srcX = srcY = 0;
        srcW = nextW;
        srcH = nextH;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }

    SkAutoPixmapStorage pm;
    pm.alloc(info);
    if (src->readPixels(pm, srcX, srcY)) {
        callback(context, pm.addr(), pm.rowBytes());
    } else {
        callback(context, nullptr, 0);
    }
}

bool SkSurface_Base::outstandingImageSnapshot() const {
    return fCachedImage && !fCachedImage->unique();
}

void SkSurface_Base::aboutToDraw(ContentChangeMode mode) {
    this->dirtyGenerationID();

    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);

    if (fCachedImage) {
        // the surface may need to fork its backend, if its sharing it with
        // the cached image. Note: we only call if there is an outstanding owner
        // on the image (besides us).
        bool unique = fCachedImage->unique();
        if (!unique) {
            this->onCopyOnWrite(mode);
        }

        // regardless of copy-on-write, we must drop our cached image now, so
        // that the next request will get our new contents.
        fCachedImage.reset();

        if (unique) {
            // Our content isn't held by any image now, so we can consider that content mutable.
            // Raster surfaces need to be told it's safe to consider its pixels mutable again.
            // We make this call after the ->unref() so the subclass can assert there are no images.
            this->onRestoreBackingMutability();
        }
    } else if (kDiscard_ContentChangeMode == mode) {
        this->onDiscard();
    }
}

uint32_t SkSurface_Base::newGenerationID() {
    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    static std::atomic<uint32_t> nextID{1};
    return nextID++;
}

static SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface::SkSurface(int width, int height, const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props)), fWidth(width), fHeight(height)
{
    SkASSERT(fWidth > 0);
    SkASSERT(fHeight > 0);
    fGenerationID = 0;
}

SkSurface::SkSurface(const SkImageInfo& info, const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props)), fWidth(info.width()), fHeight(info.height())
{
    SkASSERT(fWidth > 0);
    SkASSERT(fHeight > 0);
    fGenerationID = 0;
}

uint32_t SkSurface::generationID() {
    if (0 == fGenerationID) {
        fGenerationID = asSB(this)->newGenerationID();
    }
    return fGenerationID;
}

void SkSurface::notifyContentWillChange(ContentChangeMode mode) {
    asSB(this)->aboutToDraw(mode);
}

SkCanvas* SkSurface::getCanvas() {
    return asSB(this)->getCachedCanvas();
}

sk_sp<SkImage> SkSurface::makeImageSnapshot() {
    return asSB(this)->refCachedImage();
}

sk_sp<SkImage> SkSurface::makeImageSnapshot(const SkIRect& srcBounds) {
    const SkIRect surfBounds = { 0, 0, fWidth, fHeight };
    SkIRect bounds = srcBounds;
    if (!bounds.intersect(surfBounds)) {
        return nullptr;
    }
    SkASSERT(!bounds.isEmpty());
    if (bounds == surfBounds) {
        return this->makeImageSnapshot();
    } else {
        return asSB(this)->onNewImageSnapshot(&bounds);
    }
}

sk_sp<SkSurface> SkSurface::makeSurface(const SkImageInfo& info) {
    return asSB(this)->onNewSurface(info);
}

void SkSurface::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                     const SkPaint* paint) {
    return asSB(this)->onDraw(canvas, x, y, paint);
}

bool SkSurface::peekPixels(SkPixmap* pmap) {
    return this->getCanvas()->peekPixels(pmap);
}

bool SkSurface::readPixels(const SkPixmap& pm, int srcX, int srcY) {
    return this->getCanvas()->readPixels(pm, srcX, srcY);
}

bool SkSurface::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           int srcX, int srcY) {
    return this->readPixels({dstInfo, dstPixels, dstRowBytes}, srcX, srcY);
}

bool SkSurface::readPixels(const SkBitmap& bitmap, int srcX, int srcY) {
    SkPixmap pm;
    return bitmap.peekPixels(&pm) && this->readPixels(pm, srcX, srcY);
}

void SkSurface::asyncRescaleAndReadPixels(const SkImageInfo& info, const SkIRect& srcRect,
                                          RescaleGamma rescaleGamma, SkFilterQuality rescaleQuality,
                                          ReadPixelsCallback callback, ReadPixelsContext context) {
    if (!SkIRect::MakeWH(this->width(), this->height()).contains(srcRect) ||
        !SkImageInfoIsValid(info)) {
        callback(context, nullptr, 0);
        return;
    }
    asSB(this)->onAsyncRescaleAndReadPixels(info, srcRect, rescaleGamma, rescaleQuality, callback,
                                            context);
}

void SkSurface::writePixels(const SkPixmap& pmap, int x, int y) {
    if (pmap.addr() == nullptr || pmap.width() <= 0 || pmap.height() <= 0) {
        return;
    }

    const SkIRect srcR = SkIRect::MakeXYWH(x, y, pmap.width(), pmap.height());
    const SkIRect dstR = SkIRect::MakeWH(this->width(), this->height());
    if (SkIRect::Intersects(srcR, dstR)) {
        ContentChangeMode mode = kRetain_ContentChangeMode;
        if (srcR.contains(dstR)) {
            mode = kDiscard_ContentChangeMode;
        }
        asSB(this)->aboutToDraw(mode);
        asSB(this)->onWritePixels(pmap, x, y);
    }
}

void SkSurface::writePixels(const SkBitmap& src, int x, int y) {
    SkPixmap pm;
    if (src.peekPixels(&pm)) {
        this->writePixels(pm, x, y);
    }
}

GrBackendTexture SkSurface::getBackendTexture(BackendHandleAccess access) {
    return asSB(this)->onGetBackendTexture(access);
}

GrBackendRenderTarget SkSurface::getBackendRenderTarget(BackendHandleAccess access) {
    return asSB(this)->onGetBackendRenderTarget(access);
}

bool SkSurface::replaceBackendTexture(const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin,
                                      TextureReleaseProc textureReleaseProc,
                                      ReleaseContext releaseContext) {
    return asSB(this)->onReplaceBackendTexture(backendTexture, origin, textureReleaseProc,
                                               releaseContext);
}

void SkSurface::flush() {
    this->flush(BackendSurfaceAccess::kNoAccess, GrFlushInfo());
}

GrSemaphoresSubmitted SkSurface::flush(BackendSurfaceAccess access, const GrFlushInfo& flushInfo) {
    return asSB(this)->onFlush(access, flushInfo);
}

GrSemaphoresSubmitted SkSurface::flush(BackendSurfaceAccess access, GrFlushFlags flags,
                                       int numSemaphores, GrBackendSemaphore signalSemaphores[],
                                       GrGpuFinishedProc finishedProc,
                                       GrGpuFinishedContext finishedContext) {
    GrFlushInfo info;
    info.fFlags = flags;
    info.fNumSemaphores = numSemaphores;
    info.fSignalSemaphores = signalSemaphores;
    info.fFinishedProc = finishedProc;
    info.fFinishedContext = finishedContext;
    return this->flush(access, info);
}

GrSemaphoresSubmitted SkSurface::flush(BackendSurfaceAccess access, FlushFlags flags,
                                       int numSemaphores, GrBackendSemaphore signalSemaphores[]) {
    GrFlushFlags grFlags = flags == kSyncCpu_FlushFlag ? kSyncCpu_GrFlushFlag : kNone_GrFlushFlags;
    GrFlushInfo info;
    info.fFlags = grFlags;
    info.fNumSemaphores = numSemaphores;
    info.fSignalSemaphores = signalSemaphores;
    return this->flush(access, info);
}

GrSemaphoresSubmitted SkSurface::flushAndSignalSemaphores(int numSemaphores,
                                                          GrBackendSemaphore signalSemaphores[]) {
    GrFlushInfo info;
    info.fNumSemaphores = numSemaphores;
    info.fSignalSemaphores = signalSemaphores;
    return this->flush(BackendSurfaceAccess::kNoAccess, info);
}

bool SkSurface::wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores) {
    return asSB(this)->onWait(numSemaphores, waitSemaphores);
}

bool SkSurface::characterize(SkSurfaceCharacterization* characterization) const {
    return asSB(const_cast<SkSurface*>(this))->onCharacterize(characterization);
}

bool SkSurface::draw(SkDeferredDisplayList* ddl) {
    return asSB(this)->onDraw(ddl);
}

//////////////////////////////////////////////////////////////////////////////////////
#include "include/utils/SkNoDrawCanvas.h"

class SkNullSurface : public SkSurface_Base {
public:
    SkNullSurface(int width, int height) : SkSurface_Base(width, height, nullptr) {}

protected:
    SkCanvas* onNewCanvas() override {
        return new SkNoDrawCanvas(this->width(), this->height());
    }
    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info) override {
        return MakeNull(info.width(), info.height());
    }
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subsetOrNull) override { return nullptr; }
    void onWritePixels(const SkPixmap&, int x, int y) override {}
    void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) override {}
    void onCopyOnWrite(ContentChangeMode) override {}
};

sk_sp<SkSurface> SkSurface::MakeNull(int width, int height) {
    if (width < 1 || height < 1) {
        return nullptr;
    }
    return sk_sp<SkSurface>(new SkNullSurface(width, height));
}

//////////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrContext*, SkBudgeted, const SkImageInfo&, int,
                                             GrSurfaceOrigin, const SkSurfaceProps*, bool) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext*, const SkSurfaceCharacterization&,
                                             SkBudgeted) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrContext*, const GrBackendTexture&,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType, sk_sp<SkColorSpace>,
                                                   const SkSurfaceProps*,
                                                   TextureReleaseProc, ReleaseContext) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrContext*,
                                                        const GrBackendRenderTarget&,
                                                        GrSurfaceOrigin origin,
                                                        SkColorType,
                                                        sk_sp<SkColorSpace>,
                                                        const SkSurfaceProps*,
                                                        RenderTargetReleaseProc, ReleaseContext) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTextureAsRenderTarget(GrContext*,
                                                                 const GrBackendTexture&,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt,
                                                                 SkColorType,
                                                                 sk_sp<SkColorSpace>,
                                                                 const SkSurfaceProps*) {
    return nullptr;
}

#endif

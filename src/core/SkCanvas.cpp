/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"

#include "include/core/SkBlender.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkTOptional.h"
#include "include/private/SkTo.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMSAN.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTextFormatParams.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkVerticesPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkSurface_Base.h"
#include "src/utils/SkPatchUtils.h"

#include <memory>
#include <new>

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "include/private/chromium/GrSlug.h"
#include "src/gpu/BaseDevice.h"
#include "src/gpu/SkGr.h"
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
#   include "src/gpu/GrRenderTarget.h"
#   include "src/gpu/GrRenderTargetProxy.h"
#endif
#endif

#define RETURN_ON_NULL(ptr)     do { if (nullptr == (ptr)) return; } while (0)
#define RETURN_ON_FALSE(pred)   do { if (!(pred)) return; } while (0)

// This is a test: static_assert with no message is a c++17 feature,
// and std::max() is constexpr only since the c++14 stdlib.
static_assert(std::max(3,4) == 4);

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  Return true if the drawing this rect would hit every pixels in the canvas.
 *
 *  Returns false if
 *  - rect does not contain the canvas' bounds
 *  - paint is not fill
 *  - paint would blur or otherwise change the coverage of the rect
 */
bool SkCanvas::wouldOverwriteEntireSurface(const SkRect* rect, const SkPaint* paint,
                                           ShaderOverrideOpacity overrideOpacity) const {
    static_assert((int)SkPaintPriv::kNone_ShaderOverrideOpacity ==
                  (int)kNone_ShaderOverrideOpacity,
                  "need_matching_enums0");
    static_assert((int)SkPaintPriv::kOpaque_ShaderOverrideOpacity ==
                  (int)kOpaque_ShaderOverrideOpacity,
                  "need_matching_enums1");
    static_assert((int)SkPaintPriv::kNotOpaque_ShaderOverrideOpacity ==
                  (int)kNotOpaque_ShaderOverrideOpacity,
                  "need_matching_enums2");

    const SkISize size = this->getBaseLayerSize();
    const SkRect bounds = SkRect::MakeIWH(size.width(), size.height());

    // if we're clipped at all, we can't overwrite the entire surface
    {
        const SkBaseDevice* base = this->baseDevice();
        const SkBaseDevice* top = this->topDevice();
        if (base != top) {
            return false;   // we're in a saveLayer, so conservatively don't assume we'll overwrite
        }
        if (!base->clipIsWideOpen()) {
            return false;
        }
    }

    if (rect) {
        if (!this->getTotalMatrix().isScaleTranslate()) {
            return false; // conservative
        }

        SkRect devRect;
        this->getTotalMatrix().mapRectScaleTranslate(&devRect, *rect);
        if (!devRect.contains(bounds)) {
            return false;
        }
    }

    if (paint) {
        SkPaint::Style paintStyle = paint->getStyle();
        if (!(paintStyle == SkPaint::kFill_Style ||
              paintStyle == SkPaint::kStrokeAndFill_Style)) {
            return false;
        }
        if (paint->getMaskFilter() || paint->getPathEffect() || paint->getImageFilter()) {
            return false; // conservative
        }
    }
    return SkPaintPriv::Overwrites(paint, (SkPaintPriv::ShaderOverrideOpacity)overrideOpacity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// experimental for faster tiled drawing...
//#define SK_TRACE_SAVERESTORE

#ifdef SK_TRACE_SAVERESTORE
    static int gLayerCounter;
    static void inc_layer() { ++gLayerCounter; printf("----- inc layer %d\n", gLayerCounter); }
    static void dec_layer() { --gLayerCounter; printf("----- dec layer %d\n", gLayerCounter); }

    static int gRecCounter;
    static void inc_rec() { ++gRecCounter; printf("----- inc rec %d\n", gRecCounter); }
    static void dec_rec() { --gRecCounter; printf("----- dec rec %d\n", gRecCounter); }

    static int gCanvasCounter;
    static void inc_canvas() { ++gCanvasCounter; printf("----- inc canvas %d\n", gCanvasCounter); }
    static void dec_canvas() { --gCanvasCounter; printf("----- dec canvas %d\n", gCanvasCounter); }
#else
    #define inc_layer()
    #define dec_layer()
    #define inc_rec()
    #define dec_rec()
    #define inc_canvas()
    #define dec_canvas()
#endif

bool SkCanvas::predrawNotify(bool willOverwritesEntireSurface) {
    if (fSurfaceBase) {
        if (!fSurfaceBase->aboutToDraw(willOverwritesEntireSurface
                                       ? SkSurface::kDiscard_ContentChangeMode
                                       : SkSurface::kRetain_ContentChangeMode)) {
            return false;
        }
    }
    return true;
}

bool SkCanvas::predrawNotify(const SkRect* rect, const SkPaint* paint,
                             ShaderOverrideOpacity overrideOpacity) {
    if (fSurfaceBase) {
        SkSurface::ContentChangeMode mode = SkSurface::kRetain_ContentChangeMode;
        // Since willOverwriteAllPixels() may not be complete free to call, we only do so if
        // there is an outstanding snapshot, since w/o that, there will be no copy-on-write
        // and therefore we don't care which mode we're in.
        //
        if (fSurfaceBase->outstandingImageSnapshot()) {
            if (this->wouldOverwriteEntireSurface(rect, paint, overrideOpacity)) {
                mode = SkSurface::kDiscard_ContentChangeMode;
            }
        }
        if (!fSurfaceBase->aboutToDraw(mode)) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkCanvas::Layer::Layer(sk_sp<SkBaseDevice> device,
                       sk_sp<SkImageFilter> imageFilter,
                       const SkPaint& paint)
        : fDevice(std::move(device))
        , fImageFilter(std::move(imageFilter))
        , fPaint(paint)
        , fDiscard(false) {
    SkASSERT(fDevice);
    // Any image filter should have been pulled out and stored in 'imageFilter' so that 'paint'
    // can be used as-is to draw the result of the filter to the dst device.
    SkASSERT(!fPaint.getImageFilter());
}

SkCanvas::MCRec::MCRec(SkBaseDevice* device) : fDevice(device) {
    SkASSERT(fDevice);
    inc_rec();
}

SkCanvas::MCRec::MCRec(const MCRec* prev) : fDevice(prev->fDevice), fMatrix(prev->fMatrix) {
    SkASSERT(fDevice);
    inc_rec();
}

SkCanvas::MCRec::~MCRec() { dec_rec(); }

void SkCanvas::MCRec::newLayer(sk_sp<SkBaseDevice> layerDevice,
                               sk_sp<SkImageFilter> filter,
                               const SkPaint& restorePaint) {
    SkASSERT(!fBackImage);
    fLayer = std::make_unique<Layer>(std::move(layerDevice), std::move(filter), restorePaint);
    fDevice = fLayer->fDevice.get();
}

void SkCanvas::MCRec::reset(SkBaseDevice* device) {
    SkASSERT(!fLayer);
    SkASSERT(device);
    SkASSERT(fDeferredSaveCount == 0);
    fDevice = device;
    fMatrix.setIdentity();
}

class SkCanvas::AutoUpdateQRBounds {
public:
    explicit AutoUpdateQRBounds(SkCanvas* canvas) : fCanvas(canvas) {
        // pre-condition, fQuickRejectBounds and other state should be valid before anything
        // modifies the device's clip.
        fCanvas->validateClip();
    }
    ~AutoUpdateQRBounds() {
        fCanvas->fQuickRejectBounds = fCanvas->computeDeviceClipBounds();
        // post-condition, we should remain valid after re-computing the bounds
        fCanvas->validateClip();
    }

private:
    SkCanvas* fCanvas;

    AutoUpdateQRBounds(AutoUpdateQRBounds&&) = delete;
    AutoUpdateQRBounds(const AutoUpdateQRBounds&) = delete;
    AutoUpdateQRBounds& operator=(AutoUpdateQRBounds&&) = delete;
    AutoUpdateQRBounds& operator=(const AutoUpdateQRBounds&) = delete;
};

/////////////////////////////////////////////////////////////////////////////
// Attempts to convert an image filter to its equivalent color filter, which if possible, modifies
// the paint to compose the image filter's color filter into the paint's color filter slot.
// Returns true if the paint has been modified.
// Requires the paint to have an image filter and the copy-on-write be initialized.
static bool image_to_color_filter(SkPaint* paint) {
    SkASSERT(SkToBool(paint) && paint->getImageFilter());

    SkColorFilter* imgCFPtr;
    if (!paint->getImageFilter()->asAColorFilter(&imgCFPtr)) {
        return false;
    }
    sk_sp<SkColorFilter> imgCF(imgCFPtr);

    SkColorFilter* paintCF = paint->getColorFilter();
    if (paintCF) {
        // The paint has both a colorfilter(paintCF) and an imagefilter-that-is-a-colorfilter(imgCF)
        // and we need to combine them into a single colorfilter.
        imgCF = imgCF->makeComposed(sk_ref_sp(paintCF));
    }

    paint->setColorFilter(std::move(imgCF));
    paint->setImageFilter(nullptr);
    return true;
}

/**
 *  We implement ImageFilters for a given draw by creating a layer, then applying the
 *  imagefilter to the pixels of that layer (its backing surface/image), and then
 *  we call restore() to xfer that layer to the main canvas.
 *
 *  1. SaveLayer (with a paint containing the current imagefilter and xfermode)
 *  2. Generate the src pixels:
 *      Remove the imagefilter and the xfermode from the paint that we (AutoDrawLooper)
 *      return (fPaint). We then draw the primitive (using srcover) into a cleared
 *      buffer/surface.
 *  3. Restore the layer created in #1
 *      The imagefilter is passed the buffer/surface from the layer (now filled with the
 *      src pixels of the primitive). It returns a new "filtered" buffer, which we
 *      draw onto the previous layer using the xfermode from the original paint.
 */
class AutoLayerForImageFilter {
public:
    // "rawBounds" is the original bounds of the primitive about to be drawn, unmodified by the
    // paint. It's used to determine the size of the offscreen layer for filters.
    // If null, the clip will be used instead.
    //
    // Draw functions should use layer->paint() instead of the passed-in paint.
    AutoLayerForImageFilter(SkCanvas* canvas,
                            const SkPaint& paint,
                            const SkRect* rawBounds = nullptr)
            : fPaint(paint)
            , fCanvas(canvas)
            , fTempLayerForImageFilter(false) {
        SkDEBUGCODE(fSaveCount = canvas->getSaveCount();)

        if (fPaint.getImageFilter() && !image_to_color_filter(&fPaint)) {
            // The draw paint has an image filter that couldn't be simplified to an equivalent
            // color filter, so we have to inject an automatic saveLayer().
            SkPaint restorePaint;
            restorePaint.setImageFilter(fPaint.refImageFilter());
            restorePaint.setBlender(fPaint.refBlender());

            // Remove the restorePaint fields from our "working" paint
            fPaint.setImageFilter(nullptr);
            fPaint.setBlendMode(SkBlendMode::kSrcOver);

            SkRect storage;
            if (rawBounds && fPaint.canComputeFastBounds()) {
                // Make rawBounds include all paint outsets except for those due to image filters.
                // At this point, fPaint's image filter has been moved to 'restorePaint'.
                SkASSERT(!fPaint.getImageFilter());
                rawBounds = &fPaint.computeFastBounds(*rawBounds, &storage);
            }

            (void)canvas->internalSaveLayer(SkCanvas::SaveLayerRec(rawBounds, &restorePaint),
                                            SkCanvas::kFullLayer_SaveLayerStrategy);
            fTempLayerForImageFilter = true;
        }
    }

    AutoLayerForImageFilter(const AutoLayerForImageFilter&) = delete;
    AutoLayerForImageFilter& operator=(const AutoLayerForImageFilter&) = delete;
    AutoLayerForImageFilter(AutoLayerForImageFilter&&) = default;
    AutoLayerForImageFilter& operator=(AutoLayerForImageFilter&&) = default;

    ~AutoLayerForImageFilter() {
        if (fTempLayerForImageFilter) {
            fCanvas->internalRestore();
        }
        SkASSERT(fCanvas->getSaveCount() == fSaveCount);
    }

    const SkPaint& paint() const { return fPaint; }

private:
    SkPaint         fPaint;
    SkCanvas*       fCanvas;
    bool            fTempLayerForImageFilter;

    SkDEBUGCODE(int fSaveCount;)
};

skstd::optional<AutoLayerForImageFilter> SkCanvas::aboutToDraw(
    SkCanvas* canvas,
    const SkPaint& paint,
    const SkRect* rawBounds,
    CheckForOverwrite checkOverwrite,
    ShaderOverrideOpacity overrideOpacity)
{
    if (checkOverwrite == CheckForOverwrite::kYes) {
        if (!this->predrawNotify(rawBounds, &paint, overrideOpacity)) {
            return skstd::nullopt;
        }
    } else {
        if (!this->predrawNotify()) {
            return skstd::nullopt;
        }
    }
    return skstd::optional<AutoLayerForImageFilter>(canvas, paint, rawBounds);
}

////////////////////////////////////////////////////////////////////////////

void SkCanvas::resetForNextPicture(const SkIRect& bounds) {
    this->restoreToCount(1);

    // We're peering through a lot of structs here.  Only at this scope do we
    // know that the device is a SkNoPixelsDevice.
    SkASSERT(fBaseDevice->isNoPixelsDevice());
    static_cast<SkNoPixelsDevice*>(fBaseDevice.get())->resetForNextPicture(bounds);
    fMCRec->reset(fBaseDevice.get());
    fQuickRejectBounds = this->computeDeviceClipBounds();
}

void SkCanvas::init(sk_sp<SkBaseDevice> device) {
    // SkCanvas.h declares internal storage for the hidden struct MCRec, and this
    // assert ensure it's sufficient. <= is used because the struct has pointer fields, so the
    // declared size is an upper bound across architectures. When the size is smaller, more stack
    static_assert(sizeof(MCRec) <= kMCRecSize);

    if (!device) {
        device = sk_make_sp<SkNoPixelsDevice>(SkIRect::MakeEmpty(), fProps);
    }

    // From this point on, SkCanvas will always have a device
    SkASSERT(device);

    fSaveCount = 1;
    fMCRec = new (fMCStack.push_back()) MCRec(device.get());

    // The root device and the canvas should always have the same pixel geometry
    SkASSERT(fProps.pixelGeometry() == device->surfaceProps().pixelGeometry());

    fSurfaceBase = nullptr;
    fBaseDevice = std::move(device);
    fScratchGlyphRunBuilder = std::make_unique<SkGlyphRunBuilder>();
    fQuickRejectBounds = this->computeDeviceClipBounds();
}

SkCanvas::SkCanvas() : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    inc_canvas();
    this->init(nullptr);
}

SkCanvas::SkCanvas(int width, int height, const SkSurfaceProps* props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fProps(SkSurfacePropsCopyOrDefault(props)) {
    inc_canvas();
    this->init(sk_make_sp<SkNoPixelsDevice>(
            SkIRect::MakeWH(std::max(width, 0), std::max(height, 0)), fProps));
}

SkCanvas::SkCanvas(const SkIRect& bounds)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    inc_canvas();

    SkIRect r = bounds.isEmpty() ? SkIRect::MakeEmpty() : bounds;
    this->init(sk_make_sp<SkNoPixelsDevice>(r, fProps));
}

SkCanvas::SkCanvas(sk_sp<SkBaseDevice> device)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fProps(device->surfaceProps()) {
    inc_canvas();

    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)), fProps(props) {
    inc_canvas();

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps));
    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap,
                   std::unique_ptr<SkRasterHandleAllocator> alloc,
                   SkRasterHandleAllocator::Handle hndl)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fAllocator(std::move(alloc)) {
    inc_canvas();

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps, hndl));
    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap, nullptr, nullptr) {}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    inc_canvas();

    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
    sk_sp<SkBaseDevice> device(new SkBitmapDevice(tmp, fProps));
    this->init(device);
}
#endif

SkCanvas::~SkCanvas() {
    // Mark all pending layers to be discarded during restore (rather than drawn)
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kFront_IterStart);
    for (;;) {
        MCRec* rec = (MCRec*)iter.next();
        if (!rec) {
            break;
        }
        if (rec->fLayer) {
            rec->fLayer->fDiscard = true;
        }
    }

    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last
    this->internalRestore();    // restore the last, since we're going away

    dec_canvas();
}

///////////////////////////////////////////////////////////////////////////////

void SkCanvas::flush() {
    this->onFlush();
}

void SkCanvas::onFlush() {
#if SK_SUPPORT_GPU
    auto dContext = GrAsDirectContext(this->recordingContext());

    if (dContext) {
        dContext->flushAndSubmit();
    }
#endif
}

SkSurface* SkCanvas::getSurface() const {
    return fSurfaceBase;
}

SkISize SkCanvas::getBaseLayerSize() const {
    return this->baseDevice()->imageInfo().dimensions();
}

SkBaseDevice* SkCanvas::topDevice() const {
    SkASSERT(fMCRec->fDevice);
    return fMCRec->fDevice;
}

bool SkCanvas::readPixels(const SkPixmap& pm, int x, int y) {
    return pm.addr() && this->baseDevice()->readPixels(pm, x, y);
}

bool SkCanvas::readPixels(const SkImageInfo& dstInfo, void* dstP, size_t rowBytes, int x, int y) {
    return this->readPixels({ dstInfo, dstP, rowBytes}, x, y);
}

bool SkCanvas::readPixels(const SkBitmap& bm, int x, int y) {
    SkPixmap pm;
    return bm.peekPixels(&pm) && this->readPixels(pm, x, y);
}

bool SkCanvas::writePixels(const SkBitmap& bitmap, int x, int y) {
    SkPixmap pm;
    if (bitmap.peekPixels(&pm)) {
        return this->writePixels(pm.info(), pm.addr(), pm.rowBytes(), x, y);
    }
    return false;
}

bool SkCanvas::writePixels(const SkImageInfo& srcInfo, const void* pixels, size_t rowBytes,
                           int x, int y) {
    SkBaseDevice* device = this->baseDevice();

    // This check gives us an early out and prevents generation ID churn on the surface.
    // This is purely optional: it is a subset of the checks performed by SkWritePixelsRec.
    SkIRect srcRect = SkIRect::MakeXYWH(x, y, srcInfo.width(), srcInfo.height());
    if (!srcRect.intersect({0, 0, device->width(), device->height()})) {
        return false;
    }

    // Tell our owning surface to bump its generation ID.
    const bool completeOverwrite = srcRect.size() == device->imageInfo().dimensions();
    if (!this->predrawNotify(completeOverwrite)) {
        return false;
    }

    // This can still fail, most notably in the case of a invalid color type or alpha type
    // conversion.  We could pull those checks into this function and avoid the unnecessary
    // generation ID bump.  But then we would be performing those checks twice, since they
    // are also necessary at the bitmap/pixmap entry points.
    return device->writePixels({srcInfo, pixels, rowBytes}, x, y);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::checkForDeferredSave() {
    if (fMCRec->fDeferredSaveCount > 0) {
        this->doSave();
    }
}

int SkCanvas::getSaveCount() const {
#ifdef SK_DEBUG
    int count = 0;
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kFront_IterStart);
    for (;;) {
        const MCRec* rec = (const MCRec*)iter.next();
        if (!rec) {
            break;
        }
        count += 1 + rec->fDeferredSaveCount;
    }
    SkASSERT(count == fSaveCount);
#endif
    return fSaveCount;
}

int SkCanvas::save() {
    fSaveCount += 1;
    fMCRec->fDeferredSaveCount += 1;
    return this->getSaveCount() - 1;  // return our prev value
}

void SkCanvas::doSave() {
    this->willSave();

    SkASSERT(fMCRec->fDeferredSaveCount > 0);
    fMCRec->fDeferredSaveCount -= 1;
    this->internalSave();
}

void SkCanvas::restore() {
    if (fMCRec->fDeferredSaveCount > 0) {
        SkASSERT(fSaveCount > 1);
        fSaveCount -= 1;
        fMCRec->fDeferredSaveCount -= 1;
    } else {
        // check for underflow
        if (fMCStack.count() > 1) {
            this->willRestore();
            SkASSERT(fSaveCount > 1);
            fSaveCount -= 1;
            this->internalRestore();
            this->didRestore();
        }
    }
}

void SkCanvas::restoreToCount(int count) {
    // safety check
    if (count < 1) {
        count = 1;
    }

    int n = this->getSaveCount() - count;
    for (int i = 0; i < n; ++i) {
        this->restore();
    }
}

void SkCanvas::internalSave() {
    fMCRec = new (fMCStack.push_back()) MCRec(fMCRec);

    this->topDevice()->save();
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint) {
    return this->saveLayer(SaveLayerRec(bounds, paint, 0));
}

int SkCanvas::saveLayer(const SaveLayerRec& rec) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (rec.fPaint && rec.fPaint->nothingToDraw()) {
        // no need for the layer (or any of the draws until the matching restore()
        this->save();
        this->clipRect({0,0,0,0});
    } else {
        SaveLayerStrategy strategy = this->getSaveLayerStrategy(rec);
        fSaveCount += 1;
        this->internalSaveLayer(rec, strategy);
    }
    return this->getSaveCount() - 1;
}

int SkCanvas::only_axis_aligned_saveBehind(const SkRect* bounds) {
    if (bounds && !this->getLocalClipBounds().intersects(*bounds)) {
        // Assuming clips never expand, if the request bounds is outside of the current clip
        // there is no need to copy/restore the area, so just devolve back to a regular save.
        this->save();
    } else {
        bool doTheWork = this->onDoSaveBehind(bounds);
        fSaveCount += 1;
        this->internalSave();
        if (doTheWork) {
            this->internalSaveBehind(bounds);
        }
    }
    return this->getSaveCount() - 1;
}

// In our current design/features, we should never have a layer (src) in a different colorspace
// than its parent (dst), so we assert that here. This is called out from other asserts, in case
// we add some feature in the future to allow a given layer/imagefilter to operate in a specific
// colorspace.
static void check_drawdevice_colorspaces(SkColorSpace* src, SkColorSpace* dst) {
    SkASSERT(src == dst);
}

// Helper function to compute the center reference point used for scale decomposition under
// non-linear transformations.
static skif::ParameterSpace<SkPoint> compute_decomposition_center(
        const SkMatrix& dstToLocal,
        const skif::ParameterSpace<SkRect>* contentBounds,
        const skif::DeviceSpace<SkIRect>& targetOutput) {
    // Will use the inverse and center of the device bounds if the content bounds aren't provided.
    SkRect rect = contentBounds ? SkRect(*contentBounds) : SkRect::Make(SkIRect(targetOutput));
    SkPoint center = {rect.centerX(), rect.centerY()};
    if (!contentBounds) {
        // Theoretically, the inverse transform could put center's homogeneous coord behind W = 0,
        // but that case is handled automatically in Mapping::decomposeCTM later.
        dstToLocal.mapPoints(&center, 1);
    }

    return skif::ParameterSpace<SkPoint>(center);
}

// Compute suitable transformations and layer bounds for a new layer that will be used as the source
// input into 'filter'  before being drawn into 'dst' via the returned skif::Mapping.
// Null filters are permitted and act as the identity. The returned mapping will be compatible with
// the image filter.
//
// Returns an empty rect if the layer wouldn't draw anything after filtering.
static std::pair<skif::Mapping, skif::LayerSpace<SkIRect>> get_layer_mapping_and_bounds(
        const SkImageFilter* filter,
        const SkMatrix& localToDst,
        const skif::DeviceSpace<SkIRect>& targetOutput,
        const skif::ParameterSpace<SkRect>* contentBounds = nullptr,
        bool mustCoverDst = true,
        SkScalar scaleFactor = 1.0f) {
    auto failedMapping = []() {
        return std::make_pair<skif::Mapping, skif::LayerSpace<SkIRect>>(
                {}, skif::LayerSpace<SkIRect>::Empty());
    };

    SkMatrix dstToLocal;
    if (!localToDst.isFinite() ||
        !localToDst.invert(&dstToLocal)) {
        return failedMapping();
    }

    skif::ParameterSpace<SkPoint> center =
            compute_decomposition_center(dstToLocal, contentBounds, targetOutput);
    // *after* possibly getting a representative point from the provided content bounds, it might
    // be necessary to discard the bounds for subsequent layer calculations.
    if (mustCoverDst) {
        contentBounds = nullptr;
    }

    // Determine initial mapping and a reasonable maximum dimension to prevent layer-to-device
    // transforms with perspective and skew from triggering excessive buffer allocations.
    skif::Mapping mapping;
    if (!mapping.decomposeCTM(localToDst, filter, center)) {
        return failedMapping();
    }
    // Push scale factor into layer matrix and device matrix (net no change, but the layer will have
    // its resolution adjusted in comparison to the final device).
    if (scaleFactor != 1.0f &&
        !mapping.adjustLayerSpace(SkMatrix::Scale(scaleFactor, scaleFactor))) {
        return failedMapping();
    }

    // Perspective and skew could exceed this since mapping.deviceToLayer(targetOutput) is
    // theoretically unbounded under those conditions. Under a 45 degree rotation, a layer needs to
    // be 2X larger per side of the prior device in order to fully cover it. We use the max of that
    // and 2048 for a reasonable upper limit (this allows small layers under extreme transforms to
    // use more relative resolution than a larger layer).
    static const int kMinDimThreshold = 2048;
    int maxLayerDim = std::max(Sk64_pin_to_s32(2 * std::max(SkIRect(targetOutput).width64(),
                                                            SkIRect(targetOutput).height64())),
                               kMinDimThreshold);

    skif::LayerSpace<SkIRect> layerBounds;
    if (filter) {
        layerBounds = as_IFB(filter)->getInputBounds(mapping, targetOutput, contentBounds);
        // When a filter is involved, the layer size may be larger than the default maxLayerDim due
        // to required inputs for filters (e.g. a displacement map with a large radius).
        if (layerBounds.width() > maxLayerDim || layerBounds.height() > maxLayerDim) {
            skif::Mapping idealMapping(SkMatrix::I(), mapping.layerMatrix());
            auto idealLayerBounds = as_IFB(filter)->getInputBounds(idealMapping, targetOutput,
                                                                   contentBounds);
            maxLayerDim = std::max(std::max(idealLayerBounds.width(), idealLayerBounds.height()),
                                   maxLayerDim);
        }
    } else {
        layerBounds = mapping.deviceToLayer(targetOutput);
        if (contentBounds) {
            // For better or for worse, user bounds currently act as a hard clip on the layer's
            // extent (i.e., they implement the CSS filter-effects 'filter region' feature).
            skif::LayerSpace<SkIRect> knownBounds = mapping.paramToLayer(*contentBounds).roundOut();
            if (!layerBounds.intersect(knownBounds)) {
                return failedMapping();
            }
        }
    }

    if (layerBounds.width() > maxLayerDim || layerBounds.height() > maxLayerDim) {
        skif::LayerSpace<SkIRect> newLayerBounds(
                SkIRect::MakeWH(std::min(layerBounds.width(), maxLayerDim),
                                std::min(layerBounds.height(), maxLayerDim)));
        SkMatrix adjust = SkMatrix::MakeRectToRect(SkRect::Make(SkIRect(layerBounds)),
                                                   SkRect::Make(SkIRect(newLayerBounds)),
                                                   SkMatrix::kFill_ScaleToFit);
        if (!mapping.adjustLayerSpace(adjust)) {
            return failedMapping();
        } else {
            layerBounds = newLayerBounds;
        }
    }

    return {mapping, layerBounds};
}

static SkImageInfo make_layer_info(const SkImageInfo& prev, int w, int h, bool f16) {
    SkColorType ct = f16 ? SkColorType::kRGBA_F16_SkColorType : prev.colorType();
    if (!f16 &&
        prev.bytesPerPixel() <= 4 &&
        prev.colorType() != kRGBA_8888_SkColorType &&
        prev.colorType() != kBGRA_8888_SkColorType) {
        // "Upgrade" A8, G8, 565, 4444, 1010102, 101010x, and 888x to 8888,
        // ensuring plenty of alpha bits for the layer, perhaps losing some color bits in return.
        ct = kN32_SkColorType;
    }
    return SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType, prev.refColorSpace());
}

static bool draw_layer_as_sprite(const SkMatrix& matrix, const SkISize& size) {
    // Assume anti-aliasing and highest valid filter mode (linear) for drawing layers and image
    // filters. If the layer can be drawn as a sprite, these can be downgraded.
    SkPaint paint;
    paint.setAntiAlias(true);
    SkSamplingOptions sampling{SkFilterMode::kLinear};
    return SkTreatAsSprite(matrix, size, sampling, paint);
}

void SkCanvas::internalDrawDeviceWithFilter(SkBaseDevice* src,
                                            SkBaseDevice* dst,
                                            const SkImageFilter* filter,
                                            const SkPaint& paint,
                                            DeviceCompatibleWithFilter compat,
                                            SkScalar scaleFactor) {
    check_drawdevice_colorspaces(dst->imageInfo().colorSpace(),
                                 src->imageInfo().colorSpace());

    // 'filter' sees the src device's buffer as the implicit input image, and processes the image
    // in this device space (referred to as the "layer" space). However, the filter
    // parameters need to respect the current matrix, which is not necessarily the local matrix that
    // was set on 'src' (e.g. because we've popped src off the stack already).
    // TODO (michaelludwig): Stay in SkM44 once skif::Mapping supports SkM44 instead of SkMatrix.
    SkMatrix localToSrc = (src->globalToDevice() * fMCRec->fMatrix).asM33();
    SkISize srcDims = src->imageInfo().dimensions();

    // Whether or not we need to make a transformed tmp image from 'src', and what that transform is
    bool needsIntermediateImage = false;
    SkMatrix srcToIntermediate;

    skif::Mapping mapping;
    skif::LayerSpace<SkIRect> requiredInput;
    if (compat == DeviceCompatibleWithFilter::kYes) {
        // Just use the relative transform from src to dst and the src's whole image, since
        // internalSaveLayer should have already determined what was necessary.
        SkASSERT(scaleFactor == 1.0f);
        mapping = skif::Mapping(src->getRelativeTransform(*dst), localToSrc);
        requiredInput = skif::LayerSpace<SkIRect>(SkIRect::MakeSize(srcDims));
        SkASSERT(!requiredInput.isEmpty());
    } else {
        // Compute the image filter mapping by decomposing the local->device matrix of dst and
        // re-determining the required input.
        std::tie(mapping, requiredInput) = get_layer_mapping_and_bounds(
                filter, dst->localToDevice(), skif::DeviceSpace<SkIRect>(dst->devClipBounds()),
                nullptr, true, SkTPin(scaleFactor, 0.f, 1.f));
        if (requiredInput.isEmpty()) {
            return;
        }

        // The above mapping transforms from local to dst's device space, where the layer space
        // represents the intermediate buffer. Now we need to determine the transform from src to
        // intermediate to prepare the input to the filter.
        if (!localToSrc.invert(&srcToIntermediate)) {
            return;
        }
        srcToIntermediate.postConcat(mapping.layerMatrix());
        if (draw_layer_as_sprite(srcToIntermediate, srcDims)) {
            // src differs from intermediate by just an integer translation, so it can be applied
            // automatically when taking a subset of src if we update the mapping.
            skif::LayerSpace<SkIPoint> srcOrigin({(int) srcToIntermediate.getTranslateX(),
                                                  (int) srcToIntermediate.getTranslateY()});
            mapping.applyOrigin(srcOrigin);
            requiredInput.offset(-srcOrigin);
        } else {
            // The contents of 'src' will be drawn to an intermediate buffer using srcToIntermediate
            // and that buffer will be the input to the image filter.
            needsIntermediateImage = true;
        }
    }

    sk_sp<SkSpecialImage> filterInput;
    if (!needsIntermediateImage) {
        // The src device can be snapped directly
        skif::LayerSpace<SkIRect> srcSubset(SkIRect::MakeSize(srcDims));
        if (srcSubset.intersect(requiredInput)) {
            filterInput = src->snapSpecial(SkIRect(srcSubset));

            // TODO: For now image filter input images need to have a (0,0) origin. The required
            // input's top left has been baked into srcSubset so we use that as the image origin.
            mapping.applyOrigin(srcSubset.topLeft());
        }
    } else {
        // We need to produce a temporary image that is equivalent to 'src' but transformed to
        // a coordinate space compatible with the image filter

        // TODO: If the srcToIntermediate is scale+translate, can we use the framebuffer blit
        // extensions to handle doing the copy and scale at the same time?

        SkASSERT(compat == DeviceCompatibleWithFilter::kUnknown);
        SkRect srcRect;
        if (!SkMatrixPriv::InverseMapRect(srcToIntermediate, &srcRect,
                                          SkRect::Make(SkIRect(requiredInput)))) {
            return;
        }

        SkIRect srcSubset = srcRect.roundOut();
        sk_sp<SkSpecialImage> srcImage;
        if (srcSubset.intersect(SkIRect::MakeSize(srcDims)) &&
            (srcImage = src->snapSpecial(srcSubset))) {
            // Make a new surface and draw 'srcImage' into it with the srcToIntermediate transform
            // to produce the final input image for the filter
            SkBaseDevice::CreateInfo info(make_layer_info(src->imageInfo(), requiredInput.width(),
                                                          requiredInput.height(), false),
                                          SkPixelGeometry::kUnknown_SkPixelGeometry,
                                          SkBaseDevice::TileUsage::kNever_TileUsage,
                                          fAllocator.get());
            sk_sp<SkBaseDevice> intermediateDevice(src->onCreateDevice(info, &paint));
            if (!intermediateDevice) {
                return;
            }
            intermediateDevice->setOrigin(SkM44(srcToIntermediate),
                                          requiredInput.left(), requiredInput.top());

            SkMatrix offsetLocalToDevice = intermediateDevice->localToDevice();
            offsetLocalToDevice.preTranslate(srcSubset.left(), srcSubset.top());
            // We draw with non-AA bilinear since we cover the destination but definitely don't have
            // a pixel-aligned transform.
            intermediateDevice->drawSpecial(srcImage.get(), offsetLocalToDevice,
                                            SkSamplingOptions{SkFilterMode::kLinear}, {});
            filterInput = intermediateDevice->snapSpecial();

            // TODO: Like the non-intermediate case, we need to apply the image origin.
            mapping.applyOrigin(requiredInput.topLeft());
        }
    }

    if (filterInput) {
        const bool use_nn = draw_layer_as_sprite(mapping.deviceMatrix(),
                                                 filterInput->subset().size());
        SkSamplingOptions sampling{use_nn ? SkFilterMode::kNearest : SkFilterMode::kLinear};
        if (filter) {
            dst->drawFilteredImage(mapping, filterInput.get(), filter, sampling, paint);
        } else {
            dst->drawSpecial(filterInput.get(), mapping.deviceMatrix(), sampling, paint);
        }
    }
}

// This is similar to image_to_color_filter used by AutoLayerForImageFilter, but with key changes:
//  - image_to_color_filter requires the entire image filter DAG to be represented as a color filter
//    that does not affect transparent black (SkImageFilter::asAColorFilter)
//  - when that is met, the image filter's CF is composed around any CF that was on the draw's paint
//    since for a draw, the color filtering happens before any image filtering
//  - optimize_layer_filter only applies to the last node and does not care about transparent black
//    since a layer is being made regardless (SkImageFilter::isColorFilterNode)
//  - any extracted CF is composed inside the restore paint's CF because image filters are evaluated
//    before the color filter of a restore paint for layers.
//
// Assumes that 'filter', and thus its inputs, will remain owned by the caller. Modifies 'paint'
// to have the updated color filter and returns the image filter to evaluate on restore.
//
// FIXME: skbug.com/12083 - we modify 'coversDevice' here because for now, only the color filter
// produced from an image filter node is checked for affecting transparent black, even though it's
// better in the long run to have any CF that affects transparent black expand to the clip.
static const SkImageFilter* optimize_layer_filter(const SkImageFilter* filter, SkPaint* paint,
                                                  bool* coversDevice=nullptr) {
    SkASSERT(paint);
    SkColorFilter* cf;
    if (filter && filter->isColorFilterNode(&cf)) {
        sk_sp<SkColorFilter> inner(cf);
        if (paint->getAlphaf() < 1.f) {
            // The paint's alpha is applied after the image filter but before the paint's color
            // filter. If there is transparency, we have to apply it between the two filters.
            // FIXME: The Blend CF should allow composing directly at construction.
            inner = SkColorFilters::Compose(
                    SkColorFilters::Blend(/* src */ paint->getColor(), SkBlendMode::kDstIn),
                                          /* dst */ std::move(inner));
            paint->setAlphaf(1.f);
        }

        // Check if the once-wrapped color filter affects transparent black *before* we combine
        // it with any original color filter on the paint.
        if (coversDevice) {
            *coversDevice = as_CFB(inner)->affectsTransparentBlack();
        }

        paint->setColorFilter(SkColorFilters::Compose(paint->refColorFilter(), std::move(inner)));
        SkASSERT(filter->countInputs() == 1);
        return filter->getInput(0);
    } else {
        if (coversDevice) {
            *coversDevice = false;
        }
        return filter;
    }
}

// If there is a backdrop filter, or if the restore paint has a color filter that affects
// transparent black, then the new layer must be sized such that it covers the entire device
// clip bounds of the prior device (otherwise edges of the temporary layer would be visible).
// See skbug.com/8783
static bool must_cover_prior_device(const SkImageFilter* backdrop,
                                    const SkPaint& restorePaint) {
    // FIXME(michaelludwig) - see skbug.com/12083, once clients do not depend on user bounds for
    // clipping a layer visually, we can respect the fact that the color filter affects transparent
    // black and should cover the device.
    return SkToBool(backdrop); // ||
           // (restorePaint.getColorFilter() &&
           // as_CFB(restorePaint.getColorFilter())->affectsTransparentBlack());
}

void SkCanvas::internalSaveLayer(const SaveLayerRec& rec, SaveLayerStrategy strategy) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Do this before we create the layer. We don't call the public save() since that would invoke a
    // possibly overridden virtual.
    this->internalSave();

    if (this->isClipEmpty()) {
        // Early out if the layer wouldn't draw anything
        return;
    }

    // Build up the paint for restoring the layer, taking only the pieces of rec.fPaint that are
    // relevant. Filtering is automatically chosen in internalDrawDeviceWithFilter based on the
    // device's coordinate space.
    SkPaint restorePaint(rec.fPaint ? *rec.fPaint : SkPaint());
    restorePaint.setMaskFilter(nullptr);  // mask filters are ignored for saved layers
    restorePaint.setImageFilter(nullptr); // the image filter is held separately
    // Smooth non-axis-aligned layer edges; this automatically downgrades to non-AA for aligned
    // layer restores. This is done to match legacy behavior where the post-applied MatrixTransform
    // bilerp also smoothed cropped edges. See skbug.com/11252
    restorePaint.setAntiAlias(true);

    bool optimizedCFAffectsTransparent;
    const SkImageFilter* filter = optimize_layer_filter(
            rec.fPaint ? rec.fPaint->getImageFilter() : nullptr, &restorePaint,
            &optimizedCFAffectsTransparent);

    // Size the new layer relative to the prior device, which may already be aligned for filters.
    SkBaseDevice* priorDevice = this->topDevice();
    skif::Mapping newLayerMapping;
    skif::LayerSpace<SkIRect> layerBounds;
    std::tie(newLayerMapping, layerBounds) = get_layer_mapping_and_bounds(
            filter, priorDevice->localToDevice(),
            skif::DeviceSpace<SkIRect>(priorDevice->devClipBounds()),
            skif::ParameterSpace<SkRect>::Optional(rec.fBounds),
            must_cover_prior_device(rec.fBackdrop, restorePaint) || optimizedCFAffectsTransparent);

    auto abortLayer = [this]() {
        // The filtered content would not draw anything, or the new device space has an invalid
        // coordinate system, in which case we mark the current top device as empty so that nothing
        // draws until the canvas is restored past this saveLayer.
        AutoUpdateQRBounds aqr(this);
        this->topDevice()->clipRect(SkRect::MakeEmpty(), SkClipOp::kIntersect, /* aa */ false);
    };

    if (layerBounds.isEmpty()) {
        abortLayer();
        return;
    }

    sk_sp<SkBaseDevice> newDevice;
    if (strategy == kFullLayer_SaveLayerStrategy) {
        SkASSERT(!layerBounds.isEmpty());
        SkImageInfo info = make_layer_info(priorDevice->imageInfo(),
                                           layerBounds.width(), layerBounds.height(),
                                           SkToBool(rec.fSaveLayerFlags & kF16ColorType));
        if (rec.fSaveLayerFlags & kF16ColorType) {
            info = info.makeColorType(kRGBA_F16_SkColorType);
        }
        SkASSERT(info.alphaType() != kOpaque_SkAlphaType);

        SkPixelGeometry geo = rec.fSaveLayerFlags & kPreserveLCDText_SaveLayerFlag
                                      ? fProps.pixelGeometry()
                                      : kUnknown_SkPixelGeometry;
        const auto createInfo = SkBaseDevice::CreateInfo(info, geo, SkBaseDevice::kNever_TileUsage,
                                                         fAllocator.get());
        // Use the original paint as a hint so that it includes the image filter
        newDevice.reset(priorDevice->onCreateDevice(createInfo, rec.fPaint));
    }

    bool initBackdrop = (rec.fSaveLayerFlags & kInitWithPrevious_SaveLayerFlag) || rec.fBackdrop;
    if (!newDevice) {
        // Either we weren't meant to allocate a full layer, or the full layer creation failed.
        // Using an explicit NoPixelsDevice lets us reflect what the layer state would have been
        // on success (or kFull_LayerStrategy) while squashing draw calls that target something that
        // doesn't exist.
        newDevice = sk_make_sp<SkNoPixelsDevice>(SkIRect::MakeWH(layerBounds.width(),
                                                                 layerBounds.height()),
                                                 fProps, this->imageInfo().refColorSpace());
        initBackdrop = false;
    }

    // Configure device to match determined mapping for any image filters.
    // The setDeviceCoordinateSystem applies the prior device's global transform since
    // 'newLayerMapping' only defines the transforms between the two devices and it must be updated
    // to the global coordinate system.
    if (!newDevice->setDeviceCoordinateSystem(priorDevice->deviceToGlobal() *
                                              SkM44(newLayerMapping.deviceMatrix()),
                                              SkM44(newLayerMapping.layerMatrix()),
                                              layerBounds.left(), layerBounds.top())) {
        // If we made it this far and the coordinate system is invalid, we most likely had a valid
        // mapping until being combined with the previous device-to-global matrix, at which point
        // it overflowed or floating point rounding caused it to no longer be invertible. There's
        // not much we can do but clean up the layer and mark the clip as empty. This tends to come
        // up in fuzzer-generated inputs, so this policy is not unreasonable and helps avoids UB.
        newDevice = nullptr;
        abortLayer();
        return;
    }

    if (initBackdrop) {
        SkPaint backdropPaint;
        const SkImageFilter* backdropFilter = optimize_layer_filter(rec.fBackdrop, &backdropPaint);
        // The new device was constructed to be compatible with 'filter', not necessarily
        // 'rec.fBackdrop', so allow DrawDeviceWithFilter to transform the prior device contents
        // if necessary to evaluate the backdrop filter. If no filters are involved, then the
        // devices differ by integer translations and are always compatible.
        bool scaleBackdrop = rec.fExperimentalBackdropScale != 1.0f;
        auto compat = (filter || backdropFilter || scaleBackdrop)
                ? DeviceCompatibleWithFilter::kUnknown : DeviceCompatibleWithFilter::kYes;
        this->internalDrawDeviceWithFilter(priorDevice,     // src
                                           newDevice.get(), // dst
                                           backdropFilter,
                                           backdropPaint,
                                           compat,
                                           rec.fExperimentalBackdropScale);
    }

    fMCRec->newLayer(std::move(newDevice), sk_ref_sp(filter), restorePaint);
    fQuickRejectBounds = this->computeDeviceClipBounds();
}

int SkCanvas::saveLayerAlpha(const SkRect* bounds, U8CPU alpha) {
    if (0xFF == alpha) {
        return this->saveLayer(bounds, nullptr);
    } else {
        SkPaint tmpPaint;
        tmpPaint.setAlpha(alpha);
        return this->saveLayer(bounds, &tmpPaint);
    }
}

void SkCanvas::internalSaveBehind(const SkRect* localBounds) {
    SkBaseDevice* device = this->topDevice();

    // Map the local bounds into the top device's coordinate space (this is not
    // necessarily the full global CTM transform).
    SkIRect devBounds;
    if (localBounds) {
        SkRect tmp;
        device->localToDevice().mapRect(&tmp, *localBounds);
        if (!devBounds.intersect(tmp.round(), device->devClipBounds())) {
            devBounds.setEmpty();
        }
    } else {
        devBounds = device->devClipBounds();
    }
    if (devBounds.isEmpty()) {
        return;
    }

    // This is getting the special image from the current device, which is then drawn into (both by
    // a client, and the drawClippedToSaveBehind below). Since this is not saving a layer, with its
    // own device, we need to explicitly copy the back image contents so that its original content
    // is available when we splat it back later during restore.
    auto backImage = device->snapSpecial(devBounds, /* copy */ true);
    if (!backImage) {
        return;
    }

    // we really need the save, so we can wack the fMCRec
    this->checkForDeferredSave();

    fMCRec->fBackImage =
            std::make_unique<BackImage>(BackImage{std::move(backImage), devBounds.topLeft()});

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kClear);
    this->drawClippedToSaveBehind(paint);
}

void SkCanvas::internalRestore() {
    SkASSERT(!fMCStack.empty());

    // now detach these from fMCRec so we can pop(). Gets freed after its drawn
    std::unique_ptr<Layer> layer = std::move(fMCRec->fLayer);
    std::unique_ptr<BackImage> backImage = std::move(fMCRec->fBackImage);

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*) fMCStack.back();

    if (!fMCRec) {
        // This was the last record, restored during the destruction of the SkCanvas
        return;
    }

    this->topDevice()->restore(fMCRec->fMatrix);

    if (backImage) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kDstOver);
        this->topDevice()->drawSpecial(backImage->fImage.get(),
                                       SkMatrix::Translate(backImage->fLoc),
                                       SkSamplingOptions(),
                                       paint);
    }

    // Draw the layer's device contents into the now-current older device. We can't call public
    // draw functions since we don't want to record them.
    if (layer && !layer->fDevice->isNoPixelsDevice() && !layer->fDiscard) {
        layer->fDevice->setImmutable();

        // Don't go through AutoLayerForImageFilter since device draws are so closely tied to
        // internalSaveLayer and internalRestore.
        if (this->predrawNotify()) {
            SkBaseDevice* dstDev = this->topDevice();
            if (layer->fImageFilter) {
                this->internalDrawDeviceWithFilter(layer->fDevice.get(), // src
                                                   dstDev,               // dst
                                                   layer->fImageFilter.get(),
                                                   layer->fPaint,
                                                   DeviceCompatibleWithFilter::kYes);
            } else {
                // NOTE: We don't just call internalDrawDeviceWithFilter with a null filter
                // because we want to take advantage of overridden drawDevice functions for
                // document-based devices.
                SkSamplingOptions sampling;
                dstDev->drawDevice(layer->fDevice.get(), sampling, layer->fPaint);
            }
        }
    }

    // Reset the clip restriction if the restore went past the save point that had added it.
    if (this->getSaveCount() < fClipRestrictionSaveCount) {
        fClipRestrictionRect.setEmpty();
        fClipRestrictionSaveCount = -1;
    }
    // Update the quick-reject bounds in case the restore changed the top device or the
    // removed save record had included modifications to the clip stack.
    fQuickRejectBounds = this->computeDeviceClipBounds();
    this->validateClip();
}

sk_sp<SkSurface> SkCanvas::makeSurface(const SkImageInfo& info, const SkSurfaceProps* props) {
    if (nullptr == props) {
        props = &fProps;
    }
    return this->onNewSurface(info, *props);
}

sk_sp<SkSurface> SkCanvas::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return this->baseDevice()->makeSurface(info, props);
}

SkImageInfo SkCanvas::imageInfo() const {
    return this->onImageInfo();
}

SkImageInfo SkCanvas::onImageInfo() const {
    return this->baseDevice()->imageInfo();
}

bool SkCanvas::getProps(SkSurfaceProps* props) const {
    return this->onGetProps(props);
}

bool SkCanvas::onGetProps(SkSurfaceProps* props) const {
    if (props) {
        *props = fProps;
    }
    return true;
}

bool SkCanvas::peekPixels(SkPixmap* pmap) {
    return this->onPeekPixels(pmap);
}

bool SkCanvas::onPeekPixels(SkPixmap* pmap) {
    return this->baseDevice()->peekPixels(pmap);
}

void* SkCanvas::accessTopLayerPixels(SkImageInfo* info, size_t* rowBytes, SkIPoint* origin) {
    SkPixmap pmap;
    if (!this->onAccessTopLayerPixels(&pmap)) {
        return nullptr;
    }
    if (info) {
        *info = pmap.info();
    }
    if (rowBytes) {
        *rowBytes = pmap.rowBytes();
    }
    if (origin) {
        // If the caller requested the origin, they presumably are expecting the returned pixels to
        // be axis-aligned with the root canvas. If the top level device isn't axis aligned, that's
        // not the case. Until we update accessTopLayerPixels() to accept a coord space matrix
        // instead of an origin, just don't expose the pixels in that case. Note that this means
        // that layers with complex coordinate spaces can still report their pixels if the caller
        // does not ask for the origin (e.g. just to dump its output to a file, etc).
        if (this->topDevice()->isPixelAlignedToGlobal()) {
            *origin = this->topDevice()->getOrigin();
        } else {
            return nullptr;
        }
    }
    return pmap.writable_addr();
}

bool SkCanvas::onAccessTopLayerPixels(SkPixmap* pmap) {
    return this->topDevice()->accessPixels(pmap);
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preTranslate(dx, dy);

        this->topDevice()->setGlobalCTM(fMCRec->fMatrix);

        this->didTranslate(dx,dy);
    }
}

void SkCanvas::scale(SkScalar sx, SkScalar sy) {
    if (sx != 1 || sy != 1) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preScale(sx, sy);

        this->topDevice()->setGlobalCTM(fMCRec->fMatrix);

        this->didScale(sx, sy);
    }
}

void SkCanvas::rotate(SkScalar degrees) {
    SkMatrix m;
    m.setRotate(degrees);
    this->concat(m);
}

void SkCanvas::rotate(SkScalar degrees, SkScalar px, SkScalar py) {
    SkMatrix m;
    m.setRotate(degrees, px, py);
    this->concat(m);
}

void SkCanvas::skew(SkScalar sx, SkScalar sy) {
    SkMatrix m;
    m.setSkew(sx, sy);
    this->concat(m);
}

void SkCanvas::concat(const SkMatrix& matrix) {
    if (matrix.isIdentity()) {
        return;
    }
    this->concat(SkM44(matrix));
}

void SkCanvas::internalConcat44(const SkM44& m) {
    this->checkForDeferredSave();

    fMCRec->fMatrix.preConcat(m);

    this->topDevice()->setGlobalCTM(fMCRec->fMatrix);
}

void SkCanvas::concat(const SkM44& m) {
    this->internalConcat44(m);
    // notify subclasses
    this->didConcat44(m);
}

void SkCanvas::internalSetMatrix(const SkM44& m) {
    fMCRec->fMatrix = m;

    this->topDevice()->setGlobalCTM(fMCRec->fMatrix);
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    this->setMatrix(SkM44(matrix));
}

void SkCanvas::setMatrix(const SkM44& m) {
    this->checkForDeferredSave();
    this->internalSetMatrix(m);
    this->didSetM44(m);
}

void SkCanvas::resetMatrix() {
    this->setMatrix(SkM44());
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect, SkClipOp op, bool doAA) {
    if (!rect.isFinite()) {
        return;
    }
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    this->onClipRect(rect.makeSorted(), op, edgeStyle);
}

void SkCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    SkASSERT(rect.isSorted());
    const bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipRect(rect, op, isAA);
}

void SkCanvas::androidFramework_setDeviceClipRestriction(const SkIRect& rect) {
    // The device clip restriction is a surface-space rectangular intersection that cannot be
    // drawn outside of. The rectangle is remembered so that subsequent resetClip calls still
    // respect the restriction. Other than clip resetting, all clip operations restrict the set
    // of renderable pixels, so once set, the restriction will be respected until the canvas
    // save stack is restored past the point this function was invoked. Unfortunately, the current
    // implementation relies on the clip stack of the underyling SkDevices, which leads to some
    // awkward behavioral interactions (see skbug.com/12252).
    //
    // Namely, a canvas restore() could undo the clip restriction's rect, and if
    // setDeviceClipRestriction were called at a nested save level, there's no way to undo just the
    // prior restriction and re-apply the new one. It also only makes sense to apply to the base
    // device; any other device for a saved layer will be clipped back to the base device during its
    // matched restore. As such, we:
    // - Remember the save count that added the clip restriction and reset the rect to empty when
    //   we've restored past that point to keep our state in sync with the device's clip stack.
    // - We assert that we're on the base device when this is invoked.
    // - We assert that setDeviceClipRestriction() is only called when there was no prior
    //   restriction (cannot re-restrict, and prior state must have been reset by restoring the
    //   canvas state).
    // - Historically, the empty rect would reset the clip restriction but it only could do so
    //   partially since the device's clips wasn't adjusted. Resetting is now handled
    //   automatically via SkCanvas::restore(), so empty input rects are skipped.
    SkASSERT(this->topDevice() == this->baseDevice()); // shouldn't be in a nested layer
    // and shouldn't already have a restriction
    SkASSERT(fClipRestrictionSaveCount < 0 && fClipRestrictionRect.isEmpty());

    if (fClipRestrictionSaveCount < 0 && !rect.isEmpty()) {
        fClipRestrictionRect = rect;
        fClipRestrictionSaveCount = this->getSaveCount();

        // A non-empty clip restriction immediately applies an intersection op (ignoring the ctm).
        // so we have to resolve the save.
        this->checkForDeferredSave();
        AutoUpdateQRBounds aqr(this);
        // Use clipRegion() since that operates in canvas-space, whereas clipRect() would apply the
        // device's current transform first.
        this->topDevice()->clipRegion(SkRegion(rect), SkClipOp::kIntersect);
    }
}

void SkCanvas::internal_private_resetClip() {
    this->checkForDeferredSave();
    this->onResetClip();
}

void SkCanvas::onResetClip() {
    SkIRect deviceRestriction = this->topDevice()->imageInfo().bounds();
    if (fClipRestrictionSaveCount >= 0 && this->topDevice() == this->baseDevice()) {
        // Respect the device clip restriction when resetting the clip if we're on the base device.
        // If we're not on the base device, then the "reset" applies to the top device's clip stack,
        // and the clip restriction will be respected automatically during a restore of the layer.
        if (!deviceRestriction.intersect(fClipRestrictionRect)) {
            deviceRestriction = SkIRect::MakeEmpty();
        }
    }

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->replaceClip(deviceRestriction);
}

void SkCanvas::clipRRect(const SkRRect& rrect, SkClipOp op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    if (rrect.isRect()) {
        this->onClipRect(rrect.getBounds(), op, edgeStyle);
    } else {
        this->onClipRRect(rrect, op, edgeStyle);
    }
}

void SkCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipRRect(rrect, op, isAA);
}

void SkCanvas::clipPath(const SkPath& path, SkClipOp op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;

    if (!path.isInverseFillType() && fMCRec->fMatrix.asM33().rectStaysRect()) {
        SkRect r;
        if (path.isRect(&r)) {
            this->onClipRect(r, op, edgeStyle);
            return;
        }
        SkRRect rrect;
        if (path.isOval(&r)) {
            rrect.setOval(r);
            this->onClipRRect(rrect, op, edgeStyle);
            return;
        }
        if (path.isRRect(&rrect)) {
            this->onClipRRect(rrect, op, edgeStyle);
            return;
        }
    }

    this->onClipPath(path, op, edgeStyle);
}

void SkCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipPath(path, op, isAA);
}

void SkCanvas::clipShader(sk_sp<SkShader> sh, SkClipOp op) {
    if (sh) {
        if (sh->isOpaque()) {
            if (op == SkClipOp::kIntersect) {
                // we don't occlude anything, so skip this call
            } else {
                SkASSERT(op == SkClipOp::kDifference);
                // we occlude everything, so set the clip to empty
                this->clipRect({0,0,0,0});
            }
        } else {
            this->checkForDeferredSave();
            this->onClipShader(std::move(sh), op);
        }
    }
}

void SkCanvas::onClipShader(sk_sp<SkShader> sh, SkClipOp op) {
    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipShader(sh, op);
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkClipOp op) {
    this->checkForDeferredSave();
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipRegion(rgn, op);
}

void SkCanvas::validateClip() const {
#ifdef SK_DEBUG
    SkRect tmp = this->computeDeviceClipBounds();
    if (this->isClipEmpty()) {
        SkASSERT(fQuickRejectBounds.isEmpty());
    } else {
        SkASSERT(tmp == fQuickRejectBounds);
    }
#endif
}

bool SkCanvas::androidFramework_isClipAA() const {
    return this->topDevice()->onClipIsAA();
}

void SkCanvas::temporary_internal_getRgnClip(SkRegion* rgn) {
    rgn->setEmpty();
    SkBaseDevice* device = this->topDevice();
    if (device && device->isPixelAlignedToGlobal()) {
        device->onAsRgnClip(rgn);
        SkIPoint origin = device->getOrigin();
        if (origin.x() | origin.y()) {
            rgn->translate(origin.x(), origin.y());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkCanvas::isClipEmpty() const {
    return this->topDevice()->onGetClipType() == SkBaseDevice::ClipType::kEmpty;
}

bool SkCanvas::isClipRect() const {
    return this->topDevice()->onGetClipType() == SkBaseDevice::ClipType::kRect;
}

bool SkCanvas::quickReject(const SkRect& src) const {
#ifdef SK_DEBUG
    // Verify that fQuickRejectBounds are set properly.
    this->validateClip();
#endif

    SkRect devRect = SkMatrixPriv::MapRect(fMCRec->fMatrix, src);
    return !devRect.isFinite() || !devRect.intersects(fQuickRejectBounds);
}

bool SkCanvas::quickReject(const SkPath& path) const {
    return path.isEmpty() || this->quickReject(path.getBounds());
}

bool SkCanvas::internalQuickReject(const SkRect& bounds, const SkPaint& paint,
                                   const SkMatrix* matrix) {
    if (!bounds.isFinite() || paint.nothingToDraw()) {
        return true;
    }

    if (paint.canComputeFastBounds()) {
        SkRect tmp = matrix ? matrix->mapRect(bounds) : bounds;
        return this->quickReject(paint.computeFastBounds(tmp, &tmp));
    }

    return false;
}


SkRect SkCanvas::getLocalClipBounds() const {
    SkIRect ibounds = this->getDeviceClipBounds();
    if (ibounds.isEmpty()) {
        return SkRect::MakeEmpty();
    }

    SkMatrix inverse;
    // if we can't invert the CTM, we can't return local clip bounds
    if (!fMCRec->fMatrix.asM33().invert(&inverse)) {
        return SkRect::MakeEmpty();
    }

    SkRect bounds;
    // adjust it outwards in case we are antialiasing
    const int margin = 1;

    SkRect r = SkRect::Make(ibounds.makeOutset(margin, margin));
    inverse.mapRect(&bounds, r);
    return bounds;
}

SkIRect SkCanvas::getDeviceClipBounds() const {
    return this->computeDeviceClipBounds(/*outsetForAA=*/false).roundOut();
}

SkRect SkCanvas::computeDeviceClipBounds(bool outsetForAA) const {
    const SkBaseDevice* dev = this->topDevice();
    if (dev->onGetClipType() == SkBaseDevice::ClipType::kEmpty) {
        return SkRect::MakeEmpty();
    } else {
        SkRect devClipBounds =
                SkMatrixPriv::MapRect(dev->deviceToGlobal(), SkRect::Make(dev->devClipBounds()));
        if (outsetForAA) {
            // Expand bounds out by 1 in case we are anti-aliasing.  We store the
            // bounds as floats to enable a faster quick reject implementation.
            devClipBounds.outset(1.f, 1.f);
        }
        return devClipBounds;
    }
}

///////////////////////////////////////////////////////////////////////

SkMatrix SkCanvas::getTotalMatrix() const {
    return fMCRec->fMatrix.asM33();
}

SkM44 SkCanvas::getLocalToDevice() const {
    return fMCRec->fMatrix;
}

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && SK_SUPPORT_GPU

SkIRect SkCanvas::topLayerBounds() const {
    return this->topDevice()->getGlobalBounds();
}

GrBackendRenderTarget SkCanvas::topLayerBackendRenderTarget() const {
    auto proxy = SkCanvasPriv::TopDeviceTargetProxy(const_cast<SkCanvas*>(this));
    if (!proxy) {
        return {};
    }
    const GrRenderTarget* renderTarget = proxy->peekRenderTarget();
    return renderTarget ? renderTarget->getBackendRenderTarget() : GrBackendRenderTarget();
}
#endif

GrRecordingContext* SkCanvas::recordingContext() {
#if SK_SUPPORT_GPU
    if (auto gpuDevice = this->topDevice()->asGpuDevice()) {
        return gpuDevice->recordingContext();
    }
#endif

    return nullptr;
}

void SkCanvas::drawDRRect(const SkRRect& outer, const SkRRect& inner,
                          const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (outer.isEmpty()) {
        return;
    }
    if (inner.isEmpty()) {
        this->drawRRect(outer, paint);
        return;
    }

    // We don't have this method (yet), but technically this is what we should
    // be able to return ...
    // if (!outer.contains(inner))) {
    //
    // For now at least check for containment of bounds
    if (!outer.getBounds().contains(inner.getBounds())) {
        return;
    }

    this->onDrawDRRect(outer, inner, paint);
}

void SkCanvas::drawPaint(const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawPaint(paint);
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // To avoid redundant logic in our culling code and various backends, we always sort rects
    // before passing them along.
    this->onDrawRect(r.makeSorted(), paint);
}

void SkCanvas::drawClippedToSaveBehind(const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawBehind(paint);
}

void SkCanvas::drawRegion(const SkRegion& region, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (region.isEmpty()) {
        return;
    }

    if (region.isRect()) {
        return this->drawIRect(region.getBounds(), paint);
    }

    this->onDrawRegion(region, paint);
}

void SkCanvas::drawOval(const SkRect& r, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // To avoid redundant logic in our culling code and various backends, we always sort rects
    // before passing them along.
    this->onDrawOval(r.makeSorted(), paint);
}

void SkCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawRRect(rrect, paint);
}

void SkCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawPoints(mode, count, pts, paint);
}

void SkCanvas::drawVertices(const sk_sp<SkVertices>& vertices, SkBlendMode mode,
                            const SkPaint& paint) {
    this->drawVertices(vertices.get(), mode, paint);
}

void SkCanvas::drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(vertices);

    // We expect fans to be converted to triangles when building or deserializing SkVertices.
    SkASSERT(vertices->priv().mode() != SkVertices::kTriangleFan_VertexMode);

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // Preserve legacy behavior for Android: ignore the SkShader if there are no texCoords present
    if (paint.getShader() && !vertices->priv().hasTexCoords()) {
        SkPaint noShaderPaint(paint);
        noShaderPaint.setShader(nullptr);
        this->onDrawVerticesObject(vertices, mode, noShaderPaint);
        return;
    }
#endif
    this->onDrawVerticesObject(vertices, mode, paint);
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawPath(path, paint);
}

// Returns true if the rect can be "filled" : non-empty and finite
static bool fillable(const SkRect& r) {
    SkScalar w = r.width();
    SkScalar h = r.height();
    return SkScalarIsFinite(w) && w > 0 && SkScalarIsFinite(h) && h > 0;
}

static SkPaint clean_paint_for_lattice(const SkPaint* paint) {
    SkPaint cleaned;
    if (paint) {
        cleaned = *paint;
        cleaned.setMaskFilter(nullptr);
        cleaned.setAntiAlias(false);
    }
    return cleaned;
}

void SkCanvas::drawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                             SkFilterMode filter, const SkPaint* paint) {
    RETURN_ON_NULL(image);

    const int xdivs[] = {center.fLeft, center.fRight};
    const int ydivs[] = {center.fTop, center.fBottom};

    Lattice lat;
    lat.fXDivs = xdivs;
    lat.fYDivs = ydivs;
    lat.fRectTypes = nullptr;
    lat.fXCount = lat.fYCount = 2;
    lat.fBounds = nullptr;
    lat.fColors = nullptr;
    this->drawImageLattice(image, lat, dst, filter, paint);
}

void SkCanvas::drawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                SkFilterMode filter, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);
    if (dst.isEmpty()) {
        return;
    }

    SkIRect bounds;
    Lattice latticePlusBounds = lattice;
    if (!latticePlusBounds.fBounds) {
        bounds = SkIRect::MakeWH(image->width(), image->height());
        latticePlusBounds.fBounds = &bounds;
    }

    if (SkLatticeIter::Valid(image->width(), image->height(), latticePlusBounds)) {
        SkPaint latticePaint = clean_paint_for_lattice(paint);
        this->onDrawImageLattice2(image, latticePlusBounds, dst, filter, &latticePaint);
    } else {
        this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst,
                            SkSamplingOptions(filter), paint, kStrict_SrcRectConstraint);
    }
}

void SkCanvas::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                         const SkColor colors[], int count, SkBlendMode mode,
                         const SkSamplingOptions& sampling, const SkRect* cull,
                         const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(atlas);
    if (count <= 0) {
        return;
    }
    SkASSERT(atlas);
    SkASSERT(tex);
    this->onDrawAtlas2(atlas, xform, tex, colors, count, mode, sampling, cull, paint);
}

void SkCanvas::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (key) {
        this->onDrawAnnotation(rect, key, value);
    }
}

void SkCanvas::private_draw_shadow_rec(const SkPath& path, const SkDrawShadowRec& rec) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawShadowRec(path, rec);
}

void SkCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    // We don't test quickReject because the shadow outsets the path's bounds.
    // TODO(michaelludwig): Is it worth calling SkDrawShadowMetrics::GetLocalBounds here?
    if (!this->predrawNotify()) {
        return;
    }
    this->topDevice()->drawShadow(path, rec);
}

void SkCanvas::experimental_DrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                           QuadAAFlags aaFlags, const SkColor4f& color,
                                           SkBlendMode mode) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Make sure the rect is sorted before passing it along
    this->onDrawEdgeAAQuad(rect.makeSorted(), clip, aaFlags, color, mode);
}

void SkCanvas::experimental_DrawEdgeAAImageSet(const ImageSetEntry imageSet[], int cnt,
                                               const SkPoint dstClips[],
                                               const SkMatrix preViewMatrices[],
                                               const SkSamplingOptions& sampling,
                                               const SkPaint* paint,
                                               SrcRectConstraint constraint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawEdgeAAImageSet2(imageSet, cnt, dstClips, preViewMatrices, sampling, paint,
                                constraint);
}

//////////////////////////////////////////////////////////////////////////////
//  These are the virtual drawing methods
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::onDiscard() {
    if (fSurfaceBase) {
        sk_ignore_unused_variable(fSurfaceBase->aboutToDraw(SkSurface::kDiscard_ContentChangeMode));
    }
}

void SkCanvas::onDrawPaint(const SkPaint& paint) {
    this->internalDrawPaint(paint);
}

void SkCanvas::internalDrawPaint(const SkPaint& paint) {
    // drawPaint does not call internalQuickReject() because computing its geometry is not free
    // (see getLocalClipBounds(), and the two conditions below are sufficient.
    if (paint.nothingToDraw() || this->isClipEmpty()) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, nullptr, CheckForOverwrite::kYes);
    if (layer) {
        this->topDevice()->drawPaint(layer->paint());
    }
}

void SkCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) {
    if ((long)count <= 0 || paint.nothingToDraw()) {
        return;
    }
    SkASSERT(pts != nullptr);

    SkRect bounds;
    // Compute bounds from points (common for drawing a single line)
    if (count == 2) {
        bounds.set(pts[0], pts[1]);
    } else {
        bounds.setBounds(pts, SkToInt(count));
    }

    // Enforce paint style matches implicit behavior of drawPoints
    SkPaint strokePaint = paint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    if (this->internalQuickReject(bounds, strokePaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, strokePaint, &bounds);
    if (layer) {
        this->topDevice()->drawPoints(mode, count, pts, layer->paint());
    }
}

void SkCanvas::onDrawRect(const SkRect& r, const SkPaint& paint) {
    SkASSERT(r.isSorted());
    if (this->internalQuickReject(r, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &r, CheckForOverwrite::kYes);
    if (layer) {
        this->topDevice()->drawRect(r, layer->paint());
    }
}

void SkCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    const SkRect bounds = SkRect::Make(region.getBounds());
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        this->topDevice()->drawRegion(region, layer->paint());
    }
}

void SkCanvas::onDrawBehind(const SkPaint& paint) {
    SkBaseDevice* dev = this->topDevice();
    if (!dev) {
        return;
    }

    SkIRect bounds;
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kBack_IterStart);
    for (;;) {
        const MCRec* rec = (const MCRec*)iter.prev();
        if (!rec) {
            return; // no backimages, so nothing to draw
        }
        if (rec->fBackImage) {
            // drawBehind should only have been called when the saveBehind record is active;
            // if this fails, it means a real saveLayer was made w/o being restored first.
            SkASSERT(dev == rec->fDevice);
            bounds = SkIRect::MakeXYWH(rec->fBackImage->fLoc.fX, rec->fBackImage->fLoc.fY,
                                       rec->fBackImage->fImage->width(),
                                       rec->fBackImage->fImage->height());
            break;
        }
    }

    // The backimage location (and thus bounds) were defined in the device's space, so mark it
    // as a clip. We use a clip instead of just drawing a rect in case the paint has an image
    // filter on it (which is applied before any auto-layer so the filter is clipped).
    dev->save();
    {
        // We also have to temporarily whack the device matrix since clipRegion is affected by the
        // global-to-device matrix and clipRect is affected by the local-to-device.
        SkAutoDeviceTransformRestore adtr(dev, SkMatrix::I());
        dev->clipRect(SkRect::Make(bounds), SkClipOp::kIntersect, /* aa */ false);
        // ~adtr will reset the local-to-device matrix so that drawPaint() shades correctly.
    }

    auto layer = this->aboutToDraw(this, paint);
    if (layer) {
        this->topDevice()->drawPaint(layer->paint());
    }

    dev->restore(fMCRec->fMatrix);
}

void SkCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    SkASSERT(oval.isSorted());
    if (this->internalQuickReject(oval, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &oval);
    if (layer) {
        this->topDevice()->drawOval(oval, layer->paint());
    }
}

void SkCanvas::onDrawArc(const SkRect& oval, SkScalar startAngle,
                         SkScalar sweepAngle, bool useCenter,
                         const SkPaint& paint) {
    SkASSERT(oval.isSorted());
    if (this->internalQuickReject(oval, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &oval);
    if (layer) {
        this->topDevice()->drawArc(oval, startAngle, sweepAngle, useCenter, layer->paint());
    }
}

void SkCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    const SkRect& bounds = rrect.getBounds();

    // Delegating to simpler draw operations
    if (rrect.isRect()) {
        // call the non-virtual version
        this->SkCanvas::drawRect(bounds, paint);
        return;
    } else if (rrect.isOval()) {
        // call the non-virtual version
        this->SkCanvas::drawOval(bounds, paint);
        return;
    }

    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        this->topDevice()->drawRRect(rrect, layer->paint());
    }
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    const SkRect& bounds = outer.getBounds();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        this->topDevice()->drawDRRect(outer, inner, layer->paint());
    }
}

void SkCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    if (!path.isFinite()) {
        return;
    }

    const SkRect& pathBounds = path.getBounds();
    if (!path.isInverseFillType() && this->internalQuickReject(pathBounds, paint)) {
        return;
    }
    if (path.isInverseFillType() && pathBounds.width() <= 0 && pathBounds.height() <= 0) {
        this->internalDrawPaint(paint);
        return;
    }

    auto layer = this->aboutToDraw(this, paint, &pathBounds);
    if (layer) {
        this->topDevice()->drawPath(path, layer->paint());
    }
}

bool SkCanvas::canDrawBitmapAsSprite(SkScalar x, SkScalar y, int w, int h,
                                     const SkSamplingOptions& sampling, const SkPaint& paint) {
    if (!paint.getImageFilter()) {
        return false;
    }

    const SkMatrix& ctm = this->getTotalMatrix();
    if (!SkTreatAsSprite(ctm, SkISize::Make(w, h), sampling, paint)) {
        return false;
    }

    // The other paint effects need to be applied before the image filter, but the sprite draw
    // applies the filter explicitly first.
    if (paint.getAlphaf() < 1.f || paint.getColorFilter() || paint.getMaskFilter()) {
        return false;
    }
    // Currently we can only use the filterSprite code if we are clipped to the bitmap's bounds.
    // Once we can filter and the filter will return a result larger than itself, we should be
    // able to remove this constraint.
    // skbug.com/4526
    //
    SkPoint pt;
    ctm.mapXY(x, y, &pt);
    SkIRect ir = SkIRect::MakeXYWH(SkScalarRoundToInt(pt.x()), SkScalarRoundToInt(pt.y()), w, h);
    // quick bounds have been outset by 1px compared to overall device bounds, so this makes the
    // contains check equivalent to between ir and device bounds
    ir.outset(1, 1);
    return ir.contains(fQuickRejectBounds);
}

// Clean-up the paint to match the drawing semantics for drawImage et al. (skbug.com/7804).
static SkPaint clean_paint_for_drawImage(const SkPaint* paint) {
    SkPaint cleaned;
    if (paint) {
        cleaned = *paint;
        cleaned.setStyle(SkPaint::kFill_Style);
        cleaned.setPathEffect(nullptr);
    }
    return cleaned;
}

// drawVertices fills triangles and ignores mask filter and path effect,
// so canonicalize the paint before checking quick reject.
static SkPaint clean_paint_for_drawVertices(SkPaint paint) {
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(nullptr);
    paint.setPathEffect(nullptr);
    return paint;
}

void SkCanvas::onDrawImage2(const SkImage* image, SkScalar x, SkScalar y,
                            const SkSamplingOptions& sampling, const SkPaint* paint) {
    SkPaint realPaint = clean_paint_for_drawImage(paint);

    SkRect bounds = SkRect::MakeXYWH(x, y, image->width(), image->height());
    if (this->internalQuickReject(bounds, realPaint)) {
        return;
    }

    if (realPaint.getImageFilter() &&
        this->canDrawBitmapAsSprite(x, y, image->width(), image->height(), sampling, realPaint) &&
        !image_to_color_filter(&realPaint)) {
        // Evaluate the image filter directly on the input image and then draw the result, instead
        // of first drawing the image to a temporary layer and filtering.
        SkBaseDevice* device = this->topDevice();
        sk_sp<SkSpecialImage> special;
        if ((special = device->makeSpecial(image))) {
            sk_sp<SkImageFilter> filter = realPaint.refImageFilter();
            realPaint.setImageFilter(nullptr);

            // TODO(michaelludwig) - Many filters could probably be evaluated like this even if the
            // CTM is not translate-only; the post-transformation of the filtered image by the CTM
            // will probably look just as good and not require an extra layer.
            // TODO(michaelludwig) - Once image filter implementations can support source images
            // with non-(0,0) origins, we can just mark the origin as (x,y) instead of doing a
            // pre-concat here.
            SkMatrix layerToDevice = device->localToDevice();
            layerToDevice.preTranslate(x, y);
            skif::Mapping mapping(layerToDevice, SkMatrix::Translate(-x, -y));

            if (this->predrawNotify()) {
                device->drawFilteredImage(mapping, special.get(), filter.get(), sampling,realPaint);
            }
            return;
        } // else fall through to regular drawing path
    }

    auto layer = this->aboutToDraw(this, realPaint, &bounds);
    if (layer) {
        this->topDevice()->drawImageRect(image, nullptr, bounds, sampling,
                                         layer->paint(), kStrict_SrcRectConstraint);
    }
}

void SkCanvas::onDrawImageRect2(const SkImage* image, const SkRect& src, const SkRect& dst,
                                const SkSamplingOptions& sampling, const SkPaint* paint,
                                SrcRectConstraint constraint) {
    SkPaint realPaint = clean_paint_for_drawImage(paint);

    if (this->internalQuickReject(dst, realPaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, realPaint, &dst, CheckForOverwrite::kYes,
                                   image->isOpaque() ? kOpaque_ShaderOverrideOpacity
                                                     : kNotOpaque_ShaderOverrideOpacity);
    if (layer) {
        this->topDevice()->drawImageRect(image, &src, dst, sampling, layer->paint(), constraint);
    }
}

void SkCanvas::onDrawImageLattice2(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                   SkFilterMode filter, const SkPaint* paint) {
    SkPaint realPaint = clean_paint_for_drawImage(paint);

    if (this->internalQuickReject(dst, realPaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, realPaint, &dst);
    if (layer) {
        this->topDevice()->drawImageLattice(image, lattice, dst, filter, layer->paint());
    }
}

void SkCanvas::drawImage(const SkImage* image, SkScalar x, SkScalar y,
                         const SkSamplingOptions& sampling, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);
    this->onDrawImage2(image, x, y, sampling, paint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& src, const SkRect& dst,
                             const SkSamplingOptions& sampling, const SkPaint* paint,
                             SrcRectConstraint constraint) {
    RETURN_ON_NULL(image);
    if (!fillable(dst) || !fillable(src)) {
        return;
    }
    this->onDrawImageRect2(image, src, dst, sampling, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& dst,
                             const SkSamplingOptions& sampling, const SkPaint* paint) {
    RETURN_ON_NULL(image);
    this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst, sampling,
                        paint, kFast_SrcRectConstraint);
}

void SkCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                              const SkPaint& paint) {
    auto glyphRunList = fScratchGlyphRunBuilder->blobToGlyphRunList(*blob, {x, y});
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) {
    SkRect bounds = glyphRunList.sourceBounds();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }
    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        this->topDevice()->drawGlyphRunList(glyphRunList, layer->paint());
    }
}

#if SK_SUPPORT_GPU
sk_sp<GrSlug> SkCanvas::convertBlobToSlug(
        const SkTextBlob& blob, SkPoint origin, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    return this->doConvertBlobToSlug(blob, origin, paint);
}

sk_sp<GrSlug>
SkCanvas::doConvertBlobToSlug(const SkTextBlob& blob, SkPoint origin, const SkPaint& paint) {
    auto glyphRunList = fScratchGlyphRunBuilder->blobToGlyphRunList(blob, origin);
    SkRect bounds = glyphRunList.sourceBounds();
    if (bounds.isEmpty() || !bounds.isFinite() || paint.nothingToDraw()) {
        return nullptr;
    }
    auto layer = this->aboutToDraw(this, paint, &bounds);
    if (layer) {
        return this->topDevice()->convertGlyphRunListToSlug(glyphRunList, layer->paint());
    }
    return nullptr;
}

void SkCanvas::drawSlug(GrSlug* slug) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (slug) {
        this->doDrawSlug(slug);
    }
}

void SkCanvas::doDrawSlug(GrSlug* slug) {
    SkRect bounds = slug->sourceBounds();
    if (this->internalQuickReject(bounds, slug->paint())) {
        return;
    }

    this->topDevice()->drawSlug(slug);
}
#endif

// These call the (virtual) onDraw... method
void SkCanvas::drawSimpleText(const void* text, size_t byteLength, SkTextEncoding encoding,
                              SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (byteLength) {
        sk_msan_assert_initialized(text, SkTAddOffset<const void>(text, byteLength));
        const SkGlyphRunList& glyphRunList =
            fScratchGlyphRunBuilder->textToGlyphRunList(
                    font, paint, text, byteLength, {x, y}, encoding);
        if (!glyphRunList.empty()) {
            this->onDrawGlyphRunList(glyphRunList, paint);
        }
    }
}

void SkCanvas::drawGlyphs(int count, const SkGlyphID* glyphs, const SkPoint* positions,
                          const uint32_t* clusters, int textByteCount, const char* utf8text,
                          SkPoint origin, const SkFont& font, const SkPaint& paint) {
    if (count <= 0) { return; }

    SkGlyphRun glyphRun {
            font,
            SkMakeSpan(positions, count),
            SkMakeSpan(glyphs, count),
            SkMakeSpan(utf8text, textByteCount),
            SkMakeSpan(clusters, count),
            SkSpan<SkVector>()
    };
    SkGlyphRunList glyphRunList {
            glyphRun,
            glyphRun.sourceBounds(paint).makeOffset(origin),
            origin
    };
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::drawGlyphs(int count, const SkGlyphID glyphs[], const SkPoint positions[],
                          SkPoint origin, const SkFont& font, const SkPaint& paint) {
    if (count <= 0) { return; }

    SkGlyphRun glyphRun {
        font,
        SkMakeSpan(positions, count),
        SkMakeSpan(glyphs, count),
        SkSpan<const char>(),
        SkSpan<const uint32_t>(),
        SkSpan<SkVector>()
    };
    SkGlyphRunList glyphRunList {
        glyphRun,
        glyphRun.sourceBounds(paint).makeOffset(origin),
        origin
    };
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::drawGlyphs(int count, const SkGlyphID glyphs[], const SkRSXform xforms[],
                          SkPoint origin, const SkFont& font, const SkPaint& paint) {
    if (count <= 0) { return; }

    auto [positions, rotateScales] =
            fScratchGlyphRunBuilder->convertRSXForm(SkMakeSpan(xforms, count));

    SkGlyphRun glyphRun {
            font,
            positions,
            SkMakeSpan(glyphs, count),
            SkSpan<const char>(),
            SkSpan<const uint32_t>(),
            rotateScales
    };
    SkGlyphRunList glyphRunList {
            glyphRun,
            glyphRun.sourceBounds(paint).makeOffset(origin),
            origin
    };
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(blob);
    RETURN_ON_FALSE(blob->bounds().makeOffset(x, y).isFinite());

    // Overflow if more than 2^21 glyphs stopping a buffer overflow latter in the stack.
    // See chromium:1080481
    // TODO: can consider unrolling a few at a time if this limit becomes a problem.
    int totalGlyphCount = 0;
    constexpr int kMaxGlyphCount = 1 << 21;
    SkTextBlob::Iter i(*blob);
    SkTextBlob::Iter::Run r;
    while (i.next(&r)) {
        int glyphsLeft = kMaxGlyphCount - totalGlyphCount;
        RETURN_ON_FALSE(r.fGlyphCount <= glyphsLeft);
        totalGlyphCount += r.fGlyphCount;
    }
    this->onDrawTextBlob(blob, x, y, paint);
}

void SkCanvas::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                    const SkPaint& paint) {
    SkPaint simplePaint = clean_paint_for_drawVertices(paint);

    const SkRect& bounds = vertices->bounds();
    if (this->internalQuickReject(bounds, simplePaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, simplePaint, &bounds);
    if (layer) {
        this->topDevice()->drawVertices(vertices, SkBlender::Mode(bmode), layer->paint());
    }
}

void SkCanvas::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                         const SkPoint texCoords[4], SkBlendMode bmode,
                         const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (nullptr == cubics) {
        return;
    }

    this->onDrawPatch(cubics, colors, texCoords, bmode, paint);
}

void SkCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkBlendMode bmode,
                           const SkPaint& paint) {
    // drawPatch has the same behavior restrictions as drawVertices
    SkPaint simplePaint = clean_paint_for_drawVertices(paint);

    // Since a patch is always within the convex hull of the control points, we discard it when its
    // bounding rectangle is completely outside the current clip.
    SkRect bounds;
    bounds.setBounds(cubics, SkPatchUtils::kNumCtrlPts);
    if (this->internalQuickReject(bounds, simplePaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, simplePaint, &bounds);
    if (layer) {
        this->topDevice()->drawPatch(cubics, colors, texCoords, SkBlender::Mode(bmode),
                                     layer->paint());
    }
}

void SkCanvas::drawDrawable(SkDrawable* dr, SkScalar x, SkScalar y) {
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    TRACE_EVENT0("skia", TRACE_FUNC);
#endif
    RETURN_ON_NULL(dr);
    if (x || y) {
        SkMatrix matrix = SkMatrix::Translate(x, y);
        this->onDrawDrawable(dr, &matrix);
    } else {
        this->onDrawDrawable(dr, nullptr);
    }
}

void SkCanvas::drawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    TRACE_EVENT0("skia", TRACE_FUNC);
#endif
    RETURN_ON_NULL(dr);
    if (matrix && matrix->isIdentity()) {
        matrix = nullptr;
    }
    this->onDrawDrawable(dr, matrix);
}

void SkCanvas::onDrawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    // drawable bounds are no longer reliable (e.g. android displaylist)
    // so don't use them for quick-reject
    if (this->predrawNotify()) {
        this->topDevice()->drawDrawable(dr, matrix, this);
    }
}

void SkCanvas::onDrawAtlas2(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                            const SkColor colors[], int count, SkBlendMode bmode,
                            const SkSamplingOptions& sampling, const SkRect* cull,
                            const SkPaint* paint) {
    // drawAtlas is a combination of drawVertices and drawImage...
    SkPaint realPaint = clean_paint_for_drawVertices(clean_paint_for_drawImage(paint));
    realPaint.setShader(atlas->makeShader(sampling));

    if (cull && this->internalQuickReject(*cull, realPaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, realPaint);
    if (layer) {
        this->topDevice()->drawAtlas(xform, tex, colors, count, SkBlender::Mode(bmode),
                                     layer->paint());
    }
}

void SkCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    SkASSERT(key);

    if (this->predrawNotify()) {
        this->topDevice()->drawAnnotation(rect, key, value);
    }
}

void SkCanvas::onDrawEdgeAAQuad(const SkRect& r, const SkPoint clip[4], QuadAAFlags edgeAA,
                                const SkColor4f& color, SkBlendMode mode) {
    SkASSERT(r.isSorted());

    SkPaint paint{color};
    paint.setBlendMode(mode);
    if (this->internalQuickReject(r, paint)) {
        return;
    }

    if (this->predrawNotify()) {
        this->topDevice()->drawEdgeAAQuad(r, clip, edgeAA, color, mode);
    }
}

void SkCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry imageSet[], int count,
                                     const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                     const SkSamplingOptions& sampling, const SkPaint* paint,
                                     SrcRectConstraint constraint) {
    if (count <= 0) {
        // Nothing to draw
        return;
    }

    SkPaint realPaint = clean_paint_for_drawImage(paint);

    // We could calculate the set's dstRect union to always check quickReject(), but we can't reject
    // individual entries and Chromium's occlusion culling already makes it likely that at least one
    // entry will be visible. So, we only calculate the draw bounds when it's trivial (count == 1),
    // or we need it for the autolooper (since it greatly improves image filter perf).
    bool needsAutoLayer = SkToBool(realPaint.getImageFilter());
    bool setBoundsValid = count == 1 || needsAutoLayer;
    SkRect setBounds = imageSet[0].fDstRect;
    if (imageSet[0].fMatrixIndex >= 0) {
        // Account for the per-entry transform that is applied prior to the CTM when drawing
        preViewMatrices[imageSet[0].fMatrixIndex].mapRect(&setBounds);
    }
    if (needsAutoLayer) {
        for (int i = 1; i < count; ++i) {
            SkRect entryBounds = imageSet[i].fDstRect;
            if (imageSet[i].fMatrixIndex >= 0) {
                preViewMatrices[imageSet[i].fMatrixIndex].mapRect(&entryBounds);
            }
            setBounds.joinPossiblyEmptyRect(entryBounds);
        }
    }

    // If we happen to have the draw bounds, though, might as well check quickReject().
    if (setBoundsValid && this->internalQuickReject(setBounds, realPaint)) {
        return;
    }

    auto layer = this->aboutToDraw(this, realPaint, setBoundsValid ? &setBounds : nullptr);
    if (layer) {
        this->topDevice()->drawEdgeAAImageSet(imageSet, count, dstClips, preViewMatrices, sampling,
                                              layer->paint(), constraint);
    }
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawColor(const SkColor4f& c, SkBlendMode mode) {
    SkPaint paint;
    paint.setColor(c);
    paint.setBlendMode(mode);
    this->drawPaint(paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, const SkPaint& paint) {
    const SkPoint pt = { x, y };
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const SkPaint& paint) {
    SkPoint pts[2];
    pts[0].set(x0, y0);
    pts[1].set(x1, y1);
    this->drawPoints(kLines_PointMode, 2, pts, paint);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius, const SkPaint& paint) {
    if (radius < 0) {
        radius = 0;
    }

    SkRect  r;
    r.setLTRB(cx - radius, cy - radius, cx + radius, cy + radius);
    this->drawOval(r, paint);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry,
                             const SkPaint& paint) {
    if (rx > 0 && ry > 0) {
        SkRRect rrect;
        rrect.setRectXY(r, rx, ry);
        this->drawRRect(rrect, paint);
    } else {
        this->drawRect(r, paint);
    }
}

void SkCanvas::drawArc(const SkRect& oval, SkScalar startAngle,
                       SkScalar sweepAngle, bool useCenter,
                       const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (oval.isEmpty() || !sweepAngle) {
        return;
    }
    this->onDrawArc(oval, startAngle, sweepAngle, useCenter, paint);
}

///////////////////////////////////////////////////////////////////////////////
#ifdef SK_DISABLE_SKPICTURE
void SkCanvas::drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {}


void SkCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                             const SkPaint* paint) {}
#else

void SkCanvas::drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(picture);

    if (matrix && matrix->isIdentity()) {
        matrix = nullptr;
    }
    if (picture->approximateOpCount() <= kMaxPictureOpsToUnrollInsteadOfRef) {
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
        picture->playback(this);
    } else {
        this->onDrawPicture(picture, matrix, paint);
    }
}

void SkCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                             const SkPaint* paint) {
    if (this->internalQuickReject(picture->cullRect(), paint ? *paint : SkPaint{}, matrix)) {
        return;
    }

    SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
    picture->playback(this);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkCanvas::ImageSetEntry::ImageSetEntry() = default;
SkCanvas::ImageSetEntry::~ImageSetEntry() = default;
SkCanvas::ImageSetEntry::ImageSetEntry(const ImageSetEntry&) = default;
SkCanvas::ImageSetEntry& SkCanvas::ImageSetEntry::operator=(const ImageSetEntry&) = default;

SkCanvas::ImageSetEntry::ImageSetEntry(sk_sp<const SkImage> image, const SkRect& srcRect,
                                       const SkRect& dstRect, int matrixIndex, float alpha,
                                       unsigned aaFlags, bool hasClip)
                : fImage(std::move(image))
                , fSrcRect(srcRect)
                , fDstRect(dstRect)
                , fMatrixIndex(matrixIndex)
                , fAlpha(alpha)
                , fAAFlags(aaFlags)
                , fHasClip(hasClip) {}

SkCanvas::ImageSetEntry::ImageSetEntry(sk_sp<const SkImage> image, const SkRect& srcRect,
                                       const SkRect& dstRect, float alpha, unsigned aaFlags)
                : fImage(std::move(image))
                , fSrcRect(srcRect)
                , fDstRect(dstRect)
                , fAlpha(alpha)
                , fAAFlags(aaFlags) {}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkCanvas> SkCanvas::MakeRasterDirect(const SkImageInfo& info, void* pixels,
                                                     size_t rowBytes, const SkSurfaceProps* props) {
    if (!SkSurfaceValidateRasterInfo(info, rowBytes)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return nullptr;
    }

    return props ?
        std::make_unique<SkCanvas>(bitmap, *props) :
        std::make_unique<SkCanvas>(bitmap);
}

///////////////////////////////////////////////////////////////////////////////

SkNoDrawCanvas::SkNoDrawCanvas(int width, int height)
    : INHERITED(SkIRect::MakeWH(width, height)) {}

SkNoDrawCanvas::SkNoDrawCanvas(const SkIRect& bounds)
    : INHERITED(bounds) {}

SkNoDrawCanvas::SkNoDrawCanvas(sk_sp<SkBaseDevice> device)
    : INHERITED(device) {}

SkCanvas::SaveLayerStrategy SkNoDrawCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    (void)this->INHERITED::getSaveLayerStrategy(rec);
    return kNoLayer_SaveLayerStrategy;
}

bool SkNoDrawCanvas::onDoSaveBehind(const SkRect*) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static_assert((int)SkRegion::kDifference_Op == (int)SkClipOp::kDifference, "");
static_assert((int)SkRegion::kIntersect_Op  == (int)SkClipOp::kIntersect, "");

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRasterHandleAllocator::Handle SkCanvas::accessTopRasterHandle() const {
    const SkBaseDevice* dev = this->topDevice();
    if (fAllocator) {
        SkRasterHandleAllocator::Handle handle = dev->getRasterHandle();
        SkIRect clip = dev->devClipBounds();
        if (!clip.intersect({0, 0, dev->width(), dev->height()})) {
            clip.setEmpty();
        }

        fAllocator->updateHandle(handle, dev->localToDevice(), clip);
        return handle;
    }
    return nullptr;
}

static bool install(SkBitmap* bm, const SkImageInfo& info,
                    const SkRasterHandleAllocator::Rec& rec) {
    return bm->installPixels(info, rec.fPixels, rec.fRowBytes, rec.fReleaseProc, rec.fReleaseCtx);
}

SkRasterHandleAllocator::Handle SkRasterHandleAllocator::allocBitmap(const SkImageInfo& info,
                                                                     SkBitmap* bm) {
    SkRasterHandleAllocator::Rec rec;
    if (!this->allocHandle(info, &rec) || !install(bm, info, rec)) {
        return nullptr;
    }
    return rec.fHandle;
}

std::unique_ptr<SkCanvas>
SkRasterHandleAllocator::MakeCanvas(std::unique_ptr<SkRasterHandleAllocator> alloc,
                                    const SkImageInfo& info, const Rec* rec) {
    if (!alloc || !SkSurfaceValidateRasterInfo(info, rec ? rec->fRowBytes : kIgnoreRowBytesValue)) {
        return nullptr;
    }

    SkBitmap bm;
    Handle hndl;

    if (rec) {
        hndl = install(&bm, info, *rec) ? rec->fHandle : nullptr;
    } else {
        hndl = alloc->allocBitmap(info, &bm);
    }
    return hndl ? std::unique_ptr<SkCanvas>(new SkCanvas(bm, std::move(alloc), hndl)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

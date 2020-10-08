/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"

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
#include "include/private/SkNx.h"
#include "include/private/SkTo.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMSAN.h"
#include "src/core/SkMarkerStack.h"
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
#include "src/gpu/SkGr.h"
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
        SkBaseDevice* base = this->getDevice();
        SkBaseDevice* top = this->getTopDevice();
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

void SkCanvas::predrawNotify(bool willOverwritesEntireSurface) {
    if (fSurfaceBase) {
        fSurfaceBase->aboutToDraw(willOverwritesEntireSurface
                                  ? SkSurface::kDiscard_ContentChangeMode
                                  : SkSurface::kRetain_ContentChangeMode);
    }
}

void SkCanvas::predrawNotify(const SkRect* rect, const SkPaint* paint,
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
        fSurfaceBase->aboutToDraw(mode);
    }
}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*  This is the record we keep for each SkBaseDevice that the user installs.
    The clip/matrix/proc are fields that reflect the top of the save/restore
    stack. Whenever the canvas changes, it marks a dirty flag, and then before
    these are used (assuming we're not on a layer) we rebuild these cache
    values: they reflect the top of the save stack, but translated and clipped
    by the device's XY offset and bitmap-bounds.
*/
struct DeviceCM {
    DeviceCM*                      fNext;
    sk_sp<SkBaseDevice>            fDevice;
    std::unique_ptr<const SkPaint> fPaint; // may be null (in the future)
    SkMatrix                       fStashedMatrix; // original CTM; used by imagefilter in saveLayer

    DeviceCM(sk_sp<SkBaseDevice> device, const SkPaint* paint, const SkMatrix& stashed)
        : fNext(nullptr)
        , fDevice(std::move(device))
        , fPaint(paint ? std::make_unique<SkPaint>(*paint) : nullptr)
        , fStashedMatrix(stashed)
    {}

    void validate() {
        SkASSERT(!fPaint);
        SkASSERT(!fNext);
        SkASSERT(fDevice);
    }
};

// Encapsulate state needed to restore from saveBehind()
struct BackImage {
    sk_sp<SkSpecialImage> fImage;
    SkIPoint              fLoc;
};
}  // namespace

/*  This is the record we keep for each save/restore level in the stack.
    Since a level optionally copies the matrix and/or stack, we have pointers
    for these fields. If the value is copied for this level, the copy is
    stored in the ...Storage field, and the pointer points to that. If the
    value is not copied for this level, we ignore ...Storage, and just point
    at the corresponding value in the previous level in the stack.
*/
class SkCanvas::MCRec {
public:
    DeviceCM* fLayer;
    /*  If there are any layers in the stack, this points to the top-most
        one that is at or below this level in the stack (so we know what
        bitmap/device to draw into from this level. This value is NOT
        reference counted, since the real owner is either our fLayer field,
        or a previous one in a lower level.)
    */
    DeviceCM* fTopLayer;
    std::unique_ptr<BackImage> fBackImage;
    SkConservativeClip fRasterClip;
    SkM44 fMatrix;
    int fDeferredSaveCount;

    MCRec() {
        fLayer      = nullptr;
        fTopLayer   = nullptr;
        fMatrix.setIdentity();
        fDeferredSaveCount = 0;

        // don't bother initializing fNext
        inc_rec();
    }
    MCRec(const MCRec& prev) : fRasterClip(prev.fRasterClip), fMatrix(prev.fMatrix) {
        fLayer = nullptr;
        fTopLayer = prev.fTopLayer;
        fDeferredSaveCount = 0;

        // don't bother initializing fNext
        inc_rec();
    }
    ~MCRec() {
        delete fLayer;
        dec_rec();
    }

    void reset(const SkIRect& bounds) {
        SkASSERT(fLayer);
        SkASSERT(fDeferredSaveCount == 0);
        fLayer->validate();

        fMatrix.setIdentity();
        fRasterClip.setRect(bounds);
    }
};

class SkDrawIter {
public:
    SkDrawIter(SkCanvas* canvas)
        : fDevice(nullptr), fCurrLayer(canvas->fMCRec->fTopLayer), fPaint(nullptr)
    {}

    bool next() {
        const DeviceCM* rec = fCurrLayer;
        if (rec && rec->fDevice) {
            fDevice = rec->fDevice.get();
            fPaint  = rec->fPaint.get();
            fCurrLayer = rec->fNext;
            // fCurrLayer may be nullptr now
            return true;
        }
        return false;
    }

    const SkPaint* getPaint() const { return fPaint; }

    SkBaseDevice*   fDevice;

private:
    const DeviceCM* fCurrLayer;
    const SkPaint*  fPaint;     // May be null.
};

#define FOR_EACH_TOP_DEVICE( code )                       \
    do {                                                  \
        DeviceCM* layer = fMCRec->fTopLayer;              \
        while (layer) {                                   \
            SkBaseDevice* device = layer->fDevice.get();  \
            if (device) {                                 \
                code;                                     \
            }                                             \
            layer = layer->fNext;                         \
        }                                                 \
    } while (0)

/////////////////////////////////////////////////////////////////////////////

/**
 *  If the paint has an imagefilter, but it can be simplified to just a colorfilter, return that
 *  colorfilter, else return nullptr.
 */
static sk_sp<SkColorFilter> image_to_color_filter(const SkPaint& paint) {
    SkImageFilter* imgf = paint.getImageFilter();
    if (!imgf) {
        return nullptr;
    }

    SkColorFilter* imgCFPtr;
    if (!imgf->asAColorFilter(&imgCFPtr)) {
        return nullptr;
    }
    sk_sp<SkColorFilter> imgCF(imgCFPtr);

    SkColorFilter* paintCF = paint.getColorFilter();
    if (nullptr == paintCF) {
        // there is no existing paint colorfilter, so we can just return the imagefilter's
        return imgCF;
    }

    // The paint has both a colorfilter(paintCF) and an imagefilter-which-is-a-colorfilter(imgCF)
    // and we need to combine them into a single colorfilter.
    return imgCF->makeComposed(sk_ref_sp(paintCF));
}

/**
 * There are many bounds in skia. A circle's bounds is just its center extended by its radius.
 * However, if we stroke a circle, then the "bounds" of that is larger, since it will now draw
 * outside of its raw-bounds by 1/2 the stroke width.  SkPaint has lots of optional
 * effects/attributes that can modify the effective bounds of a given primitive -- maskfilters,
 * patheffects, stroking, etc.  This function takes a raw bounds and a paint, and returns the
 * conservative "effective" bounds based on the settings in the paint... with one exception. This
 * function does *not* look at the imagefilter, which can also modify the effective bounds. It is
 * deliberately ignored.
 */
static const SkRect& apply_paint_to_bounds_sans_imagefilter(const SkPaint& paint,
                                                            const SkRect& rawBounds,
                                                            SkRect* storage) {
    SkPaint tmpUnfiltered(paint);
    tmpUnfiltered.setImageFilter(nullptr);
    if (tmpUnfiltered.canComputeFastBounds()) {
        return tmpUnfiltered.computeFastBounds(rawBounds, storage);
    } else {
        return rawBounds;
    }
}

class AutoLayerForImageFilter {
public:
    // "rawBounds" is the original bounds of the primitive about to be drawn, unmodified by the
    // paint. It's used to determine the size of the offscreen layer for filters.
    // If null, the clip will be used instead.
    AutoLayerForImageFilter(SkCanvas* canvas, const SkPaint& paint,
                            bool skipLayerForImageFilter = false,
                            const SkRect* rawBounds = nullptr)
            : fPaint(paint)
            , fCanvas(canvas)
            , fTempLayerForImageFilter(false) {
        SkDEBUGCODE(fSaveCount = canvas->getSaveCount();)

        if (auto simplifiedCF = image_to_color_filter(*fPaint)) {
            // The image filter that would have triggered an auto-layer has been converted into
            // a color filter (possibly composed with the paint's original color filter), so no
            // layer is necessary.
            fPaint.writable()->setColorFilter(std::move(simplifiedCF));
            fPaint.writable()->setImageFilter(nullptr);
        } else if (!skipLayerForImageFilter && fPaint->getImageFilter()) {
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

            SkPaint restorePaint;
            restorePaint.setImageFilter(fPaint->refImageFilter());
            restorePaint.setBlendMode(fPaint->getBlendMode());

            SkRect storage;
            if (rawBounds) {
                // Make rawBounds include all paint outsets except for those due to image filters.
                rawBounds = &apply_paint_to_bounds_sans_imagefilter(*fPaint, *rawBounds, &storage);
            }
            (void)canvas->internalSaveLayer(SkCanvas::SaveLayerRec(rawBounds, &restorePaint),
                                            SkCanvas::kFullLayer_SaveLayerStrategy);
            fTempLayerForImageFilter = true;

            // Remove the restorePaint fields from our "working" paint. If we got here, fPaint
            // should not have been modified from the original (up to this point).
            SkASSERT(fPaint.get() == &paint);
            fPaint.writable()->setImageFilter(nullptr);
            fPaint.writable()->setBlendMode(SkBlendMode::kSrcOver);
        }
    }

    ~AutoLayerForImageFilter() {
        if (fTempLayerForImageFilter) {
            fCanvas->internalRestore();
        }
        SkASSERT(fCanvas->getSaveCount() == fSaveCount);
    }

    const SkPaint& paint() const { return *fPaint; }

private:
    SkTCopyOnFirstWrite<SkPaint> fPaint;
    SkCanvas*                    fCanvas;
    bool                         fTempLayerForImageFilter;

    SkDEBUGCODE(int              fSaveCount;)
};

////////// macros to place around the internal draw calls //////////////////

#define DRAW_BEGIN_DRAWBITMAP(paint, skipLayerForFilter, bounds)    \
    this->predrawNotify();                                          \
    AutoLayerForImageFilter draw(this, paint, skipLayerForFilter, bounds); \
    {   SkDrawIter iter(this);


#define DRAW_BEGIN_DRAWDEVICE(paint)                                \
    this->predrawNotify();                                          \
    AutoLayerForImageFilter draw(this, paint, true);                \
    {   SkDrawIter iter(this);

#define DRAW_BEGIN(paint, bounds)                                   \
    this->predrawNotify();                                          \
    AutoLayerForImageFilter draw(this, paint, false, bounds);       \
    {   SkDrawIter iter(this);

#define DRAW_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, bounds, auxOpaque)  \
    this->predrawNotify(bounds, &paint, auxOpaque);                 \
    AutoLayerForImageFilter draw(this, paint, false, bounds);       \
    {   SkDrawIter iter(this);

#define DRAW_END    }

////////////////////////////////////////////////////////////////////////////

static inline SkRect qr_clip_bounds(const SkIRect& bounds) {
    if (bounds.isEmpty()) {
        return SkRect::MakeEmpty();
    }

    // Expand bounds out by 1 in case we are anti-aliasing.  We store the
    // bounds as floats to enable a faster quick reject implementation.
    SkRect dst;
    SkNx_cast<float>(Sk4i::Load(&bounds.fLeft) + Sk4i(-1,-1,1,1)).store(&dst.fLeft);
    return dst;
}

void SkCanvas::resetForNextPicture(const SkIRect& bounds) {
    this->restoreToCount(1);
    fMCRec->reset(bounds);

    // We're peering through a lot of structs here.  Only at this scope do we
    // know that the device is a SkNoPixelsDevice.
    static_cast<SkNoPixelsDevice*>(fMCRec->fLayer->fDevice.get())->resetForNextPicture(bounds);
    fDeviceClipBounds = qr_clip_bounds(bounds);
    fIsScaleTranslate = true;
}

void SkCanvas::init(sk_sp<SkBaseDevice> device) {
    fMarkerStack = sk_make_sp<SkMarkerStack>();

    fSaveCount = 1;

    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec;
    fMCRec->fRasterClip.setDeviceClipRestriction(&fClipRestrictionRect);
    fIsScaleTranslate = true;

    SkASSERT(sizeof(DeviceCM) <= sizeof(fDeviceCMStorage));
    fMCRec->fLayer = (DeviceCM*)fDeviceCMStorage;
    new (fDeviceCMStorage) DeviceCM(device, nullptr, fMCRec->fMatrix.asM33());

    fMCRec->fTopLayer = fMCRec->fLayer;

    fSurfaceBase = nullptr;
    fDeviceClipBounds = {0, 0, 0, 0};

    if (device) {
        // The root device and the canvas should always have the same pixel geometry
        SkASSERT(fProps.pixelGeometry() == device->surfaceProps().pixelGeometry());
        fMCRec->fRasterClip.setRect(device->getGlobalBounds());
        fDeviceClipBounds = qr_clip_bounds(device->getGlobalBounds());

        device->androidFramework_setDeviceClipRestriction(&fClipRestrictionRect);

        device->setMarkerStack(fMarkerStack.get());
    }

    fScratchGlyphRunBuilder = std::make_unique<SkGlyphRunBuilder>();
}

SkCanvas::SkCanvas()
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
#ifdef SK_LEGACY_SURFACE_PROPS
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
#else
    , fProps()
#endif
{
    inc_canvas();

    this->init(nullptr);
}

SkCanvas::SkCanvas(int width, int height, const SkSurfaceProps* props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfacePropsCopyOrDefault(props))
{
    inc_canvas();
    this->init(sk_make_sp<SkNoPixelsDevice>(
            SkIRect::MakeWH(std::max(width, 0), std::max(height, 0)), fProps));
}

SkCanvas::SkCanvas(const SkIRect& bounds)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
#ifdef SK_LEGACY_SURFACE_PROPS
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
#else
    , fProps()
#endif
{
    inc_canvas();

    SkIRect r = bounds.isEmpty() ? SkIRect::MakeEmpty() : bounds;
    this->init(sk_make_sp<SkNoPixelsDevice>(r, fProps));
}

SkCanvas::SkCanvas(sk_sp<SkBaseDevice> device)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(device->surfaceProps())
{
    inc_canvas();

    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(props)
{
    inc_canvas();

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps, nullptr, nullptr));
    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, std::unique_ptr<SkRasterHandleAllocator> alloc,
                   SkRasterHandleAllocator::Handle hndl)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
#ifdef SK_LEGACY_SURFACE_PROPS
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
#else
    , fProps()
#endif
    , fAllocator(std::move(alloc))
{
    inc_canvas();

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps, hndl, nullptr));
    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap, nullptr, nullptr) {}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)), fProps(), fAllocator(nullptr)
{
    inc_canvas();

    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
    sk_sp<SkBaseDevice> device(new SkBitmapDevice(tmp, fProps, nullptr, nullptr));
    this->init(device);
}
#endif

SkCanvas::~SkCanvas() {
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
    SkBaseDevice* device = this->getDevice();
    if (device) {
        device->flush();
    }
}

SkSurface* SkCanvas::getSurface() const {
    return fSurfaceBase;
}

SkISize SkCanvas::getBaseLayerSize() const {
    SkBaseDevice* d = this->getDevice();
    return d ? SkISize::Make(d->width(), d->height()) : SkISize::Make(0, 0);
}

SkIRect SkCanvas::getTopLayerBounds() const {
    SkBaseDevice* d = this->getTopDevice();
    if (!d) {
        return SkIRect::MakeEmpty();
    }
    return d->getGlobalBounds();
}

SkBaseDevice* SkCanvas::getDevice() const {
    // return root device
    MCRec* rec = (MCRec*) fMCStack.front();
    SkASSERT(rec && rec->fLayer);
    return rec->fLayer->fDevice.get();
}

SkBaseDevice* SkCanvas::getTopDevice() const {
    return fMCRec->fTopLayer->fDevice.get();
}

bool SkCanvas::readPixels(const SkPixmap& pm, int x, int y) {
    SkBaseDevice* device = this->getDevice();
    return device && pm.addr() && device->readPixels(pm, x, y);
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
    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }

    // This check gives us an early out and prevents generation ID churn on the surface.
    // This is purely optional: it is a subset of the checks performed by SkWritePixelsRec.
    SkIRect srcRect = SkIRect::MakeXYWH(x, y, srcInfo.width(), srcInfo.height());
    if (!srcRect.intersect({0, 0, device->width(), device->height()})) {
        return false;
    }

    // Tell our owning surface to bump its generation ID.
    const bool completeOverwrite =
            srcRect.size() == SkISize::Make(device->width(), device->height());
    this->predrawNotify(completeOverwrite);

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
    MCRec* newTop = (MCRec*)fMCStack.push_back();
    new (newTop) MCRec(*fMCRec);    // balanced in restore()
    fMCRec = newTop;

    FOR_EACH_TOP_DEVICE(device->save());
}

bool SkCanvas::BoundsAffectsClip(SaveLayerFlags saveLayerFlags) {
    return !(saveLayerFlags & SkCanvasPriv::kDontClipToLayer_SaveLayerFlag);
}

bool SkCanvas::clipRectBounds(const SkRect* bounds, SaveLayerFlags saveLayerFlags,
                              SkIRect* intersection, const SkImageFilter* imageFilter) {
    // clipRectBounds() is called to determine the input layer size needed for a given image filter.
    // The coordinate space of the rectangle passed to filterBounds(kReverse) is meant to be in the
    // filtering layer space. Here, 'clipBounds' is always in the true device space. When an image
    // filter does not require a decomposed CTM matrix, the filter space and device space are the
    // same. When it has been decomposed, we want the original image filter node to process the
    // bounds in the layer space represented by the decomposed scale matrix. 'imageFilter' is no
    // longer the original filter, but has the remainder matrix baked into it, and passing in the
    // the true device clip bounds ensures that the matrix image filter provides a layer clip bounds
    // to the original filter node (barring inflation from consecutive calls to mapRect). While
    // initially counter-intuitive given the apparent inconsistency of coordinate spaces, always
    // passing getDeviceClipBounds() to 'imageFilter' is correct.
    // FIXME (michaelludwig) - When the remainder matrix is instead applied as a final draw, it will
    // be important to more accurately calculate the clip bounds in the layer space for the original
    // image filter (similar to how matrix image filter does it, but ideally without the inflation).
    SkIRect clipBounds = this->getDeviceClipBounds();
    if (clipBounds.isEmpty()) {
        return false;
    }

    const SkMatrix& ctm = fMCRec->fMatrix.asM33();  // this->getTotalMatrix()

    if (imageFilter && bounds && !imageFilter->canComputeFastBounds()) {
        // If the image filter DAG affects transparent black then we will need to render
        // out to the clip bounds
        bounds = nullptr;
    }

    SkIRect inputSaveLayerBounds;
    if (bounds) {
        SkRect r;
        ctm.mapRect(&r, *bounds);
        r.roundOut(&inputSaveLayerBounds);
    } else {    // no user bounds, so just use the clip
        inputSaveLayerBounds = clipBounds;
    }

    if (imageFilter) {
        // expand the clip bounds by the image filter DAG to include extra content that might
        // be required by the image filters.
        clipBounds = imageFilter->filterBounds(clipBounds, ctm,
                                               SkImageFilter::kReverse_MapDirection,
                                               &inputSaveLayerBounds);
    }

    SkIRect clippedSaveLayerBounds;
    if (bounds) {
        // For better or for worse, user bounds currently act as a hard clip on the layer's
        // extent (i.e., they implement the CSS filter-effects 'filter region' feature).
        clippedSaveLayerBounds = inputSaveLayerBounds;
    } else {
        // If there are no user bounds, we don't want to artificially restrict the resulting
        // layer bounds, so allow the expanded clip bounds free reign.
        clippedSaveLayerBounds = clipBounds;
    }

    // early exit if the layer's bounds are clipped out
    if (!clippedSaveLayerBounds.intersect(clipBounds)) {
        if (BoundsAffectsClip(saveLayerFlags)) {
            fMCRec->fTopLayer->fDevice->clipRegion(SkRegion(), SkClipOp::kIntersect); // empty
            fMCRec->fRasterClip.setEmpty();
            fDeviceClipBounds.setEmpty();
        }
        return false;
    }
    SkASSERT(!clippedSaveLayerBounds.isEmpty());

    if (BoundsAffectsClip(saveLayerFlags)) {
        // Simplify the current clips since they will be applied properly during restore()
        fMCRec->fRasterClip.setRect(clippedSaveLayerBounds);
        fDeviceClipBounds = qr_clip_bounds(clippedSaveLayerBounds);
    }

    if (intersection) {
        *intersection = clippedSaveLayerBounds;
    }

    return true;
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

void SkCanvas::DrawDeviceWithFilter(SkBaseDevice* src, const SkImageFilter* filter,
                                    SkBaseDevice* dst, const SkIPoint& dstOrigin,
                                    const SkMatrix& ctm) {
    // The local bounds of the src device; all the bounds passed to snapSpecial must be intersected
    // with this rect.
    const SkIRect srcDevRect = SkIRect::MakeWH(src->width(), src->height());
    // TODO(michaelludwig) - Update this function to use the relative transforms between src and
    // dst; for now, since devices never have complex transforms, we can keep using getOrigin().
    if (!filter) {
        // All non-filtered devices are currently axis aligned, so they only differ by their origin.
        // This means that we only have to copy a dst-sized block of pixels out of src and translate
        // it to the matching position relative to dst's origin.
        SkIRect snapBounds = SkIRect::MakeXYWH(dstOrigin.x() - src->getOrigin().x(),
                                               dstOrigin.y() - src->getOrigin().y(),
                                               dst->width(), dst->height());
        if (!snapBounds.intersect(srcDevRect)) {
            return;
        }

        auto special = src->snapSpecial(snapBounds);
        if (special) {
            // The image is drawn at 1-1 scale with integer translation, so no filtering is needed.
            SkPaint p;
            dst->drawSpecial(special.get(), SkMatrix::I(), p);
        }
        return;
    }

    // First decompose the ctm into a post-filter transform and a filter matrix that is supported
    // by the backdrop filter.
    SkMatrix toRoot, layerMatrix;
    SkSize scale;
    if (ctm.isScaleTranslate() || as_IFB(filter)->canHandleComplexCTM()) {
        toRoot = SkMatrix::I();
        layerMatrix = ctm;
    } else if (ctm.decomposeScale(&scale, &toRoot)) {
        layerMatrix = SkMatrix::Scale(scale.fWidth, scale.fHeight);
    } else {
        // Perspective, for now, do no scaling of the layer itself.
        // TODO (michaelludwig) - perhaps it'd be better to explore a heuristic scale pulled from
        // the matrix, e.g. based on the midpoint of the near/far planes?
        toRoot = ctm;
        layerMatrix = SkMatrix::I();
    }

    // We have to map the dst bounds from the root space into the layer space where filtering will
    // occur. If we knew the input bounds of the content that defined the original dst bounds, we
    // could map that forward by layerMatrix and have tighter bounds, but toRoot^-1 * dst bounds
    // is a safe, conservative estimate.
    SkMatrix fromRoot;
    if (!toRoot.invert(&fromRoot)) {
        return;
    }

    // This represents what the backdrop filter needs to produce in the layer space, and is sized
    // such that drawing it into dst with the toRoot transform will cover the actual dst device.
    SkIRect layerTargetBounds = fromRoot.mapRect(
            SkRect::MakeXYWH(dstOrigin.x(), dstOrigin.y(), dst->width(), dst->height())).roundOut();
    // While layerTargetBounds is what needs to be output by the filter, the filtering process may
    // require some extra input pixels.
    SkIRect layerInputBounds = filter->filterBounds(
            layerTargetBounds, layerMatrix, SkImageFilter::kReverse_MapDirection,
            &layerTargetBounds);

    // Map the required input into the root space, then make relative to the src device. This will
    // be the conservative contents required to fill a layerInputBounds-sized surface with the
    // backdrop content (transformed back into the layer space using fromRoot).
    SkIRect backdropBounds = toRoot.mapRect(SkRect::Make(layerInputBounds)).roundOut();
    backdropBounds.offset(-src->getOrigin().x(), -src->getOrigin().y());
    if (!backdropBounds.intersect(srcDevRect)) {
        return;
    }

    auto special = src->snapSpecial(backdropBounds);
    if (!special) {
        return;
    }

    SkColorType colorType = src->imageInfo().colorType();
    if (colorType == kUnknown_SkColorType) {
        colorType = kRGBA_8888_SkColorType;
    }
    SkColorSpace* colorSpace = src->imageInfo().colorSpace();

    SkPaint p;
    if (!toRoot.isIdentity()) {
        // Drawing the temporary and final filtered image requires a higher filter quality if the
        // 'toRoot' transformation is not identity, in order to minimize the impact on already
        // rendered edges/content.
        // TODO (michaelludwig) - Explore reducing this quality, identify visual tradeoffs
        p.setFilterQuality(kHigh_SkFilterQuality);

        // The snapped backdrop content needs to be transformed by fromRoot into the layer space,
        // and stored in a temporary surface, which is then used as the input to the actual filter.
        auto tmpSurface = special->makeSurface(colorType, colorSpace, layerInputBounds.size());
        if (!tmpSurface) {
            return;
        }

        auto tmpCanvas = tmpSurface->getCanvas();
        tmpCanvas->clear(SK_ColorTRANSPARENT);
        // Reading in reverse, this takes the backdrop bounds from src device space into the root
        // space, then maps from root space into the layer space, then maps it so the input layer's
        // top left corner is (0, 0). This transformation automatically accounts for any cropping
        // performed on backdropBounds.
        tmpCanvas->translate(-layerInputBounds.fLeft, -layerInputBounds.fTop);
        tmpCanvas->concat(fromRoot);
        tmpCanvas->translate(src->getOrigin().x(), src->getOrigin().y());

        tmpCanvas->drawImageRect(special->asImage(), special->subset(),
                                 SkRect::Make(backdropBounds), &p, kStrict_SrcRectConstraint);
        special = tmpSurface->makeImageSnapshot();
    } else {
        // Since there is no extra transform that was done, update the input bounds to reflect
        // cropping of the snapped backdrop image. In this case toRoot = I, so layerInputBounds
        // was equal to backdropBounds before it was made relative to the src device and cropped.
        // When we use the original snapped image directly, just map the update backdrop bounds
        // back into the shared layer space
        layerInputBounds = backdropBounds;
        layerInputBounds.offset(src->getOrigin().x(), src->getOrigin().y());

        // Similar to the unfiltered case above, when toRoot is the identity, then the final
        // draw will be 1-1 so there is no need to increase filter quality.
        p.setFilterQuality(kNone_SkFilterQuality);
    }

    // Now evaluate the filter on 'special', which contains the backdrop content mapped back into
    // layer space. This has to further offset everything so that filter evaluation thinks the
    // source image's top left corner is (0, 0).
    // TODO (michaelludwig) - Once image filters are robust to non-(0,0) image origins for inputs,
    // this can be simplified.
    layerTargetBounds.offset(-layerInputBounds.fLeft, -layerInputBounds.fTop);
    SkMatrix filterCTM = layerMatrix;
    filterCTM.postTranslate(-layerInputBounds.fLeft, -layerInputBounds.fTop);
    skif::Context ctx(filterCTM, layerTargetBounds, nullptr, colorType, colorSpace, special.get());

    SkIPoint offset;
    special = as_IFB(filter)->filterImage(ctx).imageAndOffset(&offset);
    if (special) {
        // Draw the filtered backdrop content into the dst device. We add layerInputBounds origin
        // to offset because the original value in 'offset' was relative to 'filterCTM'. 'filterCTM'
        // had subtracted the layerInputBounds origin, so adding that back makes 'offset' relative
        // to 'layerMatrix' (what we need it to be when drawing the image by 'toRoot').
        offset += layerInputBounds.topLeft();

        // Manually setting the device's CTM requires accounting for the device's origin.
        // TODO (michaelludwig) - This could be simpler if the dst device had its origin configured
        // before filtering the backdrop device and we use skif::Mapping instead.
        SkMatrix dstCTM = toRoot;
        dstCTM.postTranslate(-dstOrigin.x(), -dstOrigin.y());
        dstCTM.preTranslate(offset.fX, offset.fY);
        dst->drawSpecial(special.get(), dstCTM,  p);
    }
}

static SkImageInfo make_layer_info(const SkImageInfo& prev, int w, int h, const SkPaint* paint) {
    SkColorType ct = prev.colorType();
    if (prev.bytesPerPixel() <= 4 &&
        prev.colorType() != kRGBA_8888_SkColorType &&
        prev.colorType() != kBGRA_8888_SkColorType) {
        // "Upgrade" A8, G8, 565, 4444, 1010102, 101010x, and 888x to 8888,
        // ensuring plenty of alpha bits for the layer, perhaps losing some color bits in return.
        ct = kN32_SkColorType;
    }
    return SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType, prev.refColorSpace());
}

void SkCanvas::internalSaveLayer(const SaveLayerRec& rec, SaveLayerStrategy strategy) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    const SkRect* bounds = rec.fBounds;
    SaveLayerFlags saveLayerFlags = rec.fSaveLayerFlags;

    SkTCopyOnFirstWrite<SkPaint> paint(rec.fPaint);
    // saveLayer ignores mask filters, so force it to null
    if (paint.get() && paint->getMaskFilter()) {
        paint.writable()->setMaskFilter(nullptr);
    }

    // If we have a backdrop filter, then we must apply it to the entire layer (clip-bounds)
    // regardless of any hint-rect from the caller. skbug.com/8783
    if (rec.fBackdrop) {
        bounds = nullptr;
    }

    SkImageFilter* imageFilter = paint.get() ? paint->getImageFilter() : nullptr;
    SkMatrix stashedMatrix = fMCRec->fMatrix.asM33();
    MCRec* modifiedRec = nullptr;

    /*
     *  Many ImageFilters (so far) do not (on their own) correctly handle matrices (CTM) that
     *  contain rotation/skew/etc. We rely on applyCTM to create a new image filter DAG as needed to
     *  accommodate this, but it requires update the CTM we use when drawing into the layer.
     *
     *  1. Stash off the current CTM
     *  2. Apply the CTM to imagefilter, which decomposes it into simple and complex transforms
     *     if necessary.
     *  3. Wack the CTM to be the remaining scale matrix and use the modified imagefilter, which
     *     is a MatrixImageFilter that contains the complex matrix.
     *  4. Proceed as usual, allowing the client to draw into the layer (now with a scale-only CTM)
     *  5. During restore, the MatrixImageFilter automatically applies complex stage to the output
     *     of the original imagefilter, and draw that (via drawSprite)
     *  6. Unwack the CTM to its original state (i.e. stashedMatrix)
     *
     *  Perhaps in the future we could augment #5 to apply REMAINDER as part of the draw (no longer
     *  a sprite operation) to avoid the extra buffer/overhead of MatrixImageFilter.
     */
    if (imageFilter) {
        SkMatrix modifiedCTM;
        sk_sp<SkImageFilter> modifiedFilter = as_IFB(imageFilter)->applyCTM(stashedMatrix,
                                                                            &modifiedCTM);
        if (as_IFB(modifiedFilter)->uniqueID() != as_IFB(imageFilter)->uniqueID()) {
            // The original filter couldn't support the CTM entirely
            SkASSERT(modifiedCTM.isScaleTranslate() || as_IFB(imageFilter)->canHandleComplexCTM());
            modifiedRec = fMCRec;
            this->internalSetMatrix(modifiedCTM);
            imageFilter = modifiedFilter.get();
            paint.writable()->setImageFilter(std::move(modifiedFilter));
        }
        // Else the filter didn't change, so modifiedCTM == stashedMatrix and there's nothing
        // left to do since the stack already has that as the CTM.
    }

    // do this before we create the layer. We don't call the public save() since
    // that would invoke a possibly overridden virtual
    this->internalSave();

    SkIRect ir;
    if (!this->clipRectBounds(bounds, saveLayerFlags, &ir, imageFilter)) {
        if (modifiedRec) {
            // In this case there will be no layer in which to stash the matrix so we need to
            // revert the prior MCRec to its earlier state.
            modifiedRec->fMatrix = SkM44(stashedMatrix);
        }
        return;
    }

    // FIXME: do willSaveLayer() overriders returning kNoLayer_SaveLayerStrategy really care about
    // the clipRectBounds() call above?
    if (kNoLayer_SaveLayerStrategy == strategy) {
        return;
    }

    SkBaseDevice* priorDevice = this->getTopDevice();
    if (nullptr == priorDevice) {   // Do we still need this check???
        SkDebugf("Unable to find device for layer.");
        return;
    }

    SkImageInfo info = make_layer_info(priorDevice->imageInfo(), ir.width(), ir.height(), paint);
    if (rec.fSaveLayerFlags & kF16ColorType) {
        info = info.makeColorType(kRGBA_F16_SkColorType);
    }

    sk_sp<SkBaseDevice> newDevice;
    {
        SkASSERT(info.alphaType() != kOpaque_SkAlphaType);

        SkPixelGeometry geo = saveLayerFlags & kPreserveLCDText_SaveLayerFlag
                                      ? fProps.pixelGeometry()
                                      : kUnknown_SkPixelGeometry;
        const bool trackCoverage =
                SkToBool(saveLayerFlags & kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag);
        const auto createInfo = SkBaseDevice::CreateInfo(info,
                                                         geo,
                                                         SkBaseDevice::kNever_TileUsage,
                                                         trackCoverage,
                                                         fAllocator.get());
        newDevice.reset(priorDevice->onCreateDevice(createInfo, paint));
        if (!newDevice) {
            return;
        }
        newDevice->setMarkerStack(fMarkerStack.get());
    }
    DeviceCM* layer = new DeviceCM(newDevice, paint, stashedMatrix);

    // only have a "next" if this new layer doesn't affect the clip (rare)
    layer->fNext = BoundsAffectsClip(saveLayerFlags) ? nullptr : fMCRec->fTopLayer;
    fMCRec->fLayer = layer;
    fMCRec->fTopLayer = layer;    // this field is NOT an owner of layer

    if ((rec.fSaveLayerFlags & kInitWithPrevious_SaveLayerFlag) || rec.fBackdrop) {
        DrawDeviceWithFilter(priorDevice, rec.fBackdrop, newDevice.get(), { ir.fLeft, ir.fTop },
                             fMCRec->fMatrix.asM33());
    }

    newDevice->setOrigin(fMCRec->fMatrix, ir.fLeft, ir.fTop);

    newDevice->androidFramework_setDeviceClipRestriction(&fClipRestrictionRect);
    if (layer->fNext) {
        // need to punch a hole in the previous device, so we don't draw there, given that
        // the new top-layer will allow drawing to happen "below" it.
        SkRegion hole(ir);
        do {
            layer = layer->fNext;
            layer->fDevice->clipRegion(hole, SkClipOp::kDifference);
        } while (layer->fNext);
    }
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
    SkBaseDevice* device = this->getTopDevice();
    if (nullptr == device) {   // Do we still need this check???
        return;
    }

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
    SkASSERT(fMCStack.count() != 0);

    // reserve our layer (if any)
    DeviceCM* layer = fMCRec->fLayer;   // may be null
    // now detach it from fMCRec so we can pop(). Gets freed after its drawn
    fMCRec->fLayer = nullptr;

    // move this out before we do the actual restore
    auto backImage = std::move(fMCRec->fBackImage);

    fMarkerStack->restore(fMCRec);

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*)fMCStack.back();

    if (fMCRec) {
        FOR_EACH_TOP_DEVICE(device->restore(fMCRec->fMatrix));
    }

    if (backImage) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kDstOver);
        this->getTopDevice()->drawSpecial(backImage->fImage.get(),
                                          SkMatrix::Translate(backImage->fLoc.x(),
                                                              backImage->fLoc.y()), paint);
    }

    /*  Time to draw the layer's offscreen. We can't call the public drawSprite,
        since if we're being recorded, we don't want to record this (the
        recorder will have already recorded the restore).
    */
    if (layer) {
        if (fMCRec) {
            layer->fDevice->setImmutable();
            // At this point, 'layer' has been removed from the device stack, so the devices that
            // internalDrawDevice sees are the destinations that 'layer' is drawn into.
            this->internalDrawDevice(layer->fDevice.get(), layer->fPaint.get());
            // restore what we smashed in internalSaveLayer
            this->internalSetMatrix(layer->fStashedMatrix);
            delete layer;
        } else {
            // we're at the root
            SkASSERT(layer == (void*)fDeviceCMStorage);
            layer->~DeviceCM();
            // no need to update fMCRec, 'cause we're killing the canvas
        }
    }

    if (fMCRec) {
        fIsScaleTranslate = SkMatrixPriv::IsScaleTranslateAsM33(fMCRec->fMatrix);
        fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
    }
}

sk_sp<SkSurface> SkCanvas::makeSurface(const SkImageInfo& info, const SkSurfaceProps* props) {
    if (nullptr == props) {
        props = &fProps;
    }
    return this->onNewSurface(info, *props);
}

sk_sp<SkSurface> SkCanvas::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    SkBaseDevice* dev = this->getDevice();
    return dev ? dev->makeSurface(info, props) : nullptr;
}

SkImageInfo SkCanvas::imageInfo() const {
    return this->onImageInfo();
}

SkImageInfo SkCanvas::onImageInfo() const {
    SkBaseDevice* dev = this->getDevice();
    if (dev) {
        return dev->imageInfo();
    } else {
        return SkImageInfo::MakeUnknown(0, 0);
    }
}

bool SkCanvas::getProps(SkSurfaceProps* props) const {
    return this->onGetProps(props);
}

bool SkCanvas::onGetProps(SkSurfaceProps* props) const {
    SkBaseDevice* dev = this->getDevice();
    if (dev) {
        if (props) {
            *props = fProps;
        }
        return true;
    } else {
        return false;
    }
}

bool SkCanvas::peekPixels(SkPixmap* pmap) {
    return this->onPeekPixels(pmap);
}

bool SkCanvas::onPeekPixels(SkPixmap* pmap) {
    SkBaseDevice* dev = this->getDevice();
    return dev && dev->peekPixels(pmap);
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
        if (this->getTopDevice()->isPixelAlignedToGlobal()) {
            *origin = this->getTopDevice()->getOrigin();
        } else {
            return nullptr;
        }
    }
    return pmap.writable_addr();
}

bool SkCanvas::onAccessTopLayerPixels(SkPixmap* pmap) {
    SkBaseDevice* dev = this->getTopDevice();
    return dev && dev->accessPixels(pmap);
}

/////////////////////////////////////////////////////////////////////////////

// In our current design/features, we should never have a layer (src) in a different colorspace
// than its parent (dst), so we assert that here. This is called out from other asserts, in case
// we add some feature in the future to allow a given layer/imagefilter to operate in a specific
// colorspace.
static void check_drawdevice_colorspaces(SkColorSpace* src, SkColorSpace* dst) {
    SkASSERT(src == dst);
}

void SkCanvas::internalDrawDevice(SkBaseDevice* srcDev, const SkPaint* paint) {
    SkPaint tmp;
    if (nullptr == paint) {
        paint = &tmp;
    }

    DRAW_BEGIN_DRAWDEVICE(*paint)

    while (iter.next()) {
        SkBaseDevice* dstDev = iter.fDevice;
        check_drawdevice_colorspaces(dstDev->imageInfo().colorSpace(),
                                     srcDev->imageInfo().colorSpace());

        SkTCopyOnFirstWrite<SkPaint> noFilterPaint(draw.paint());
        SkImageFilter* filter = draw.paint().getImageFilter();
        if (filter) {
            noFilterPaint.writable()->setImageFilter(nullptr);
        }

        if (!filter) {
            // Can draw the src device's buffer w/o any extra image filter evaluation
            // (although this draw may include color filter processing extracted from the IF DAG).
            dstDev->drawDevice(srcDev, *noFilterPaint);
        } else {
            // Use the whole device buffer, presumably it was sized appropriately to match the
            // desired output size of the destination when the layer was first saved.
            sk_sp<SkSpecialImage> srcBuffer = srcDev->snapSpecial();
            if (!srcBuffer) {
                return;
            }

            // Evaluate the image filter DAG on the src device's buffer. The filter processes an
            // image in the src's device space. However, the filter parameters need to respect the
            // dst's local matrix (this reflects the CTM that was set when the layer was first
            // saved). We can achieve this by concatenating the dst's local-to-device matrix with
            // the relative transform from dst to src. Then the final result is drawn to dst using
            // the relative transform from src to dst.
            SkMatrix srcToDst = srcDev->getRelativeTransform(*dstDev);
            SkMatrix dstToSrc = dstDev->getRelativeTransform(*srcDev);
            skif::Mapping mapping(srcToDst, SkMatrix::Concat(dstToSrc, dstDev->localToDevice()));
            dstDev->drawFilteredImage(mapping, srcBuffer.get(), filter, *noFilterPaint);
        }
    }

    DRAW_END
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preTranslate(dx, dy);

        // Translate shouldn't affect the is-scale-translateness of the matrix.
        // However, if either is non-finite, we might still complicate the matrix type,
        // so we still have to compute this.
        fIsScaleTranslate = SkMatrixPriv::IsScaleTranslateAsM33(fMCRec->fMatrix);

        FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));

        this->didTranslate(dx,dy);
    }
}

void SkCanvas::scale(SkScalar sx, SkScalar sy) {
    if (sx != 1 || sy != 1) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preScale(sx, sy);

        // shouldn't need to do this (theoretically), as the state shouldn't have changed,
        // but pre-scaling by a non-finite does change it, so we have to recompute.
        fIsScaleTranslate = SkMatrixPriv::IsScaleTranslateAsM33(fMCRec->fMatrix);

        FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));

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

    this->checkForDeferredSave();
    fMCRec->fMatrix.preConcat(matrix);

    fIsScaleTranslate = SkMatrixPriv::IsScaleTranslateAsM33(fMCRec->fMatrix);

    FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));

    this->didConcat(matrix);
}

void SkCanvas::internalConcat44(const SkM44& m) {
    this->checkForDeferredSave();

    fMCRec->fMatrix.preConcat(m);

    fIsScaleTranslate = SkMatrixPriv::IsScaleTranslateAsM33(fMCRec->fMatrix);

    FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));
}

void SkCanvas::concat(const SkM44& m) {
    this->internalConcat44(m);
    // notify subclasses
    this->didConcat44(m);
}

void SkCanvas::internalSetMatrix(const SkMatrix& matrix) {
    fMCRec->fMatrix = SkM44(matrix);
    fIsScaleTranslate = matrix.isScaleTranslate();

    FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    this->checkForDeferredSave();
    this->internalSetMatrix(matrix);
    this->didSetMatrix(matrix);
}

void SkCanvas::resetMatrix() {
    this->setMatrix(SkMatrix::I());
}

void SkCanvas::markCTM(const char* name) {
    if (SkCanvasPriv::ValidateMarker(name)) {
        fMarkerStack->setMarker(SkOpts::hash_fn(name, strlen(name), 0),
                                this->getLocalToDevice(), fMCRec);
        this->onMarkCTM(name);
    }
}

bool SkCanvas::findMarkedCTM(const char* name, SkM44* mx) const {
    return SkCanvasPriv::ValidateMarker(name) &&
           fMarkerStack->findMarker(SkOpts::hash_fn(name, strlen(name), 0), mx);
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

    FOR_EACH_TOP_DEVICE(device->clipRect(rect, op, isAA));

    AutoValidateClip avc(this);
    fMCRec->fRasterClip.opRect(rect, fMCRec->fMatrix.asM33(), this->getTopLayerBounds(),
                               (SkRegion::Op)op, isAA);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
}

void SkCanvas::androidFramework_setDeviceClipRestriction(const SkIRect& rect) {
    fClipRestrictionRect = rect;
    if (fClipRestrictionRect.isEmpty()) {
        // we notify the device, but we *dont* resolve deferred saves (since we're just
        // removing the restriction if the rect is empty. how I hate this api.
        FOR_EACH_TOP_DEVICE(device->androidFramework_setDeviceClipRestriction(&fClipRestrictionRect));
    } else {
        this->checkForDeferredSave();
        FOR_EACH_TOP_DEVICE(device->androidFramework_setDeviceClipRestriction(&fClipRestrictionRect));
        AutoValidateClip avc(this);
        fMCRec->fRasterClip.opIRect(fClipRestrictionRect, SkRegion::kIntersect_Op);
        fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
    }
}

void SkCanvas::androidFramework_replaceClip(const SkIRect& rect) {
    this->checkForDeferredSave();
    FOR_EACH_TOP_DEVICE(device->replaceClip(rect));
    AutoValidateClip avc(this);
    fMCRec->fRasterClip.setRect(rect);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
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
    AutoValidateClip avc(this);

    bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    FOR_EACH_TOP_DEVICE(device->clipRRect(rrect, op, isAA));

    fMCRec->fRasterClip.opRRect(rrect, fMCRec->fMatrix.asM33(), this->getTopLayerBounds(),
                                (SkRegion::Op)op, isAA);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
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
    AutoValidateClip avc(this);

    bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    FOR_EACH_TOP_DEVICE(device->clipPath(path, op, isAA));

    const SkPath* rasterClipPath = &path;
    fMCRec->fRasterClip.opPath(*rasterClipPath, fMCRec->fMatrix.asM33(), this->getTopLayerBounds(),
                               (SkRegion::Op)op, isAA);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
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
    AutoValidateClip avc(this);

    FOR_EACH_TOP_DEVICE(device->clipShader(sh, op));

    // we don't know how to mutate our conservative bounds, so we don't
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkClipOp op) {
    this->checkForDeferredSave();
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    FOR_EACH_TOP_DEVICE(device->clipRegion(rgn, op));

    AutoValidateClip avc(this);

    fMCRec->fRasterClip.opRegion(rgn, (SkRegion::Op)op);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
}

#ifdef SK_DEBUG
void SkCanvas::validateClip() const {
    // construct clipRgn from the clipstack
    const SkBaseDevice* device = this->getDevice();
    if (!device) {
        SkASSERT(this->isClipEmpty());
        return;
    }
}
#endif

bool SkCanvas::androidFramework_isClipAA() const {
    bool containsAA = false;

    FOR_EACH_TOP_DEVICE(containsAA |= device->onClipIsAA());

    return containsAA;
}

class RgnAccumulator {
    SkRegion* fRgn;
public:
    RgnAccumulator(SkRegion* total) : fRgn(total) {}
    void accumulate(SkBaseDevice* device, SkRegion* rgn) {
        SkIPoint origin = device->getOrigin();
        if (origin.x() | origin.y()) {
            rgn->translate(origin.x(), origin.y());
        }
        fRgn->op(*rgn, SkRegion::kUnion_Op);
    }
};

void SkCanvas::temporary_internal_getRgnClip(SkRegion* rgn) {
    RgnAccumulator accum(rgn);
    SkRegion tmp;

    rgn->setEmpty();
    FOR_EACH_TOP_DEVICE(device->onAsRgnClip(&tmp); accum.accumulate(device, &tmp));
}

///////////////////////////////////////////////////////////////////////////////

bool SkCanvas::isClipEmpty() const {
    return fMCRec->fRasterClip.isEmpty();

    // TODO: should we only use the conservative answer in a recording canvas?
#if 0
    SkBaseDevice* dev = this->getTopDevice();
    // if no device we return true
    return !dev || dev->onGetClipType() == SkBaseDevice::kEmpty_ClipType;
#endif
}

bool SkCanvas::isClipRect() const {
    SkBaseDevice* dev = this->getTopDevice();
    // if no device we return false
    return dev && dev->onGetClipType() == SkBaseDevice::ClipType::kRect;
}

static inline bool is_nan_or_clipped(const Sk4f& devRect, const Sk4f& devClip) {
#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    __m128 lLtT = _mm_unpacklo_ps(devRect.fVec, devClip.fVec);
    __m128 RrBb = _mm_unpackhi_ps(devClip.fVec, devRect.fVec);
    __m128 mask = _mm_cmplt_ps(lLtT, RrBb);
    return 0xF != _mm_movemask_ps(mask);
#elif !defined(SKNX_NO_SIMD) && defined(SK_ARM_HAS_NEON)
    float32x4_t lLtT = vzipq_f32(devRect.fVec, devClip.fVec).val[0];
    float32x4_t RrBb = vzipq_f32(devClip.fVec, devRect.fVec).val[1];
    uint32x4_t mask = vcltq_f32(lLtT, RrBb);
    return 0xFFFFFFFFFFFFFFFF != (uint64_t) vmovn_u32(mask);
#else
    SkRect devRectAsRect;
    SkRect devClipAsRect;
    devRect.store(&devRectAsRect.fLeft);
    devClip.store(&devClipAsRect.fLeft);
    return !devRectAsRect.isFinite() || !devRectAsRect.intersect(devClipAsRect);
#endif
}

// It's important for this function to not be inlined.  Otherwise the compiler will share code
// between the fast path and the slow path, resulting in two slow paths.
static SK_NEVER_INLINE bool quick_reject_slow_path(const SkRect& src, const SkRect& deviceClip,
                                                   const SkMatrix& matrix) {
    SkRect deviceRect;
    matrix.mapRect(&deviceRect, src);
    return !deviceRect.isFinite() || !deviceRect.intersect(deviceClip);
}

bool SkCanvas::quickReject(const SkRect& src) const {
#ifdef SK_DEBUG
    // Verify that fDeviceClipBounds are set properly.
    SkRect tmp = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
    if (fMCRec->fRasterClip.isEmpty()) {
        SkASSERT(fDeviceClipBounds.isEmpty());
    } else {
        SkASSERT(tmp == fDeviceClipBounds);
    }

    // Verify that fIsScaleTranslate is set properly.
    SkASSERT(fIsScaleTranslate == SkMatrixPriv::IsScaleTranslateAsM33(fMCRec->fMatrix));
#endif

    if (!fIsScaleTranslate) {
        return quick_reject_slow_path(src, fDeviceClipBounds, fMCRec->fMatrix.asM33());
    }

    // We inline the implementation of mapScaleTranslate() for the fast path.
    float sx = fMCRec->fMatrix.rc(0, 0);
    float sy = fMCRec->fMatrix.rc(1, 1);
    float tx = fMCRec->fMatrix.rc(0, 3);
    float ty = fMCRec->fMatrix.rc(1, 3);
    Sk4f scale(sx, sy, sx, sy);
    Sk4f trans(tx, ty, tx, ty);

    // Apply matrix.
    Sk4f ltrb = Sk4f::Load(&src.fLeft) * scale + trans;

    // Make sure left < right, top < bottom.
    Sk4f rblt(ltrb[2], ltrb[3], ltrb[0], ltrb[1]);
    Sk4f min = Sk4f::Min(ltrb, rblt);
    Sk4f max = Sk4f::Max(ltrb, rblt);
    // We can extract either pair [0,1] or [2,3] from min and max and be correct, but on
    // ARM this sequence generates the fastest (a single instruction).
    Sk4f devRect = Sk4f(min[2], min[3], max[0], max[1]);

    // Check if the device rect is NaN or outside the clip.
    return is_nan_or_clipped(devRect, Sk4f::Load(&fDeviceClipBounds.fLeft));
}

bool SkCanvas::quickReject(const SkPath& path) const {
    return path.isEmpty() || this->quickReject(path.getBounds());
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
    return fMCRec->fRasterClip.getBounds();
}

///////////////////////////////////////////////////////////////////////

SkMatrix SkCanvas::getTotalMatrix() const {
    return fMCRec->fMatrix.asM33();
}

SkM44 SkCanvas::getLocalToDevice() const {
    return fMCRec->fMatrix;
}

GrRenderTargetContext* SkCanvas::internal_private_accessTopLayerRenderTargetContext() {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessRenderTargetContext() : nullptr;
}

GrRecordingContext* SkCanvas::recordingContext() {
    SkBaseDevice* device = this->getTopDevice();
    return device ? device->recordingContext() : nullptr;
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

    // If the vertices contain custom attributes, ensure they line up with the paint's shader.
    const SkRuntimeEffect* effect =
            paint.getShader() ? as_SB(paint.getShader())->asRuntimeEffect() : nullptr;
    if ((size_t)vertices->priv().attributeCount() != (effect ? effect->varyings().count() : 0)) {
        return;
    }
    if (effect) {
        int attrIndex = 0;
        for (const auto& v : effect->varyings()) {
            const SkVertices::Attribute& attr(vertices->priv().attributes()[attrIndex++]);
            // Mismatch between the SkSL varying and the vertex shader output for this attribute
            if (attr.channelCount() != v.fWidth) {
                return;
            }
            // If we can't provide any of the asked-for matrices, we can't draw this
            if (attr.fMarkerID && !fMarkerStack->findMarker(attr.fMarkerID, nullptr)) {
                return;
            }
        }
    }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // Preserve legacy behavior for Android: ignore the SkShader if there are no texCoords present
    if (paint.getShader() &&
        !(vertices->priv().hasTexCoords() || vertices->priv().hasCustomData())) {
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

void SkCanvas::drawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);
    this->onDrawImage(image, x, y, paint);
}

// Returns true if the rect can be "filled" : non-empty and finite
static bool fillable(const SkRect& r) {
    SkScalar w = r.width();
    SkScalar h = r.height();
    return SkScalarIsFinite(w) && w > 0 && SkScalarIsFinite(h) && h > 0;
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& src, const SkRect& dst,
                             const SkPaint* paint, SrcRectConstraint constraint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);
    if (!fillable(dst) || !fillable(src)) {
        return;
    }
    this->onDrawImageRect(image, &src, dst, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkIRect& isrc, const SkRect& dst,
                             const SkPaint* paint, SrcRectConstraint constraint) {
    RETURN_ON_NULL(image);
    this->drawImageRect(image, SkRect::Make(isrc), dst, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& dst, const SkPaint* paint) {
    RETURN_ON_NULL(image);
    this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst, paint,
                        kFast_SrcRectConstraint);
}

namespace {
class LatticePaint : SkNoncopyable {
public:
    LatticePaint(const SkPaint* origPaint) : fPaint(origPaint) {
        if (!origPaint) {
            return;
        }
        if (origPaint->getFilterQuality() > kLow_SkFilterQuality) {
            fPaint.writable()->setFilterQuality(kLow_SkFilterQuality);
        }
        if (origPaint->getMaskFilter()) {
            fPaint.writable()->setMaskFilter(nullptr);
        }
        if (origPaint->isAntiAlias()) {
            fPaint.writable()->setAntiAlias(false);
        }
    }

    const SkPaint* get() const {
        return fPaint;
    }

private:
    SkTCopyOnFirstWrite<SkPaint> fPaint;
};
} // namespace

void SkCanvas::drawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                             const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);
    if (dst.isEmpty()) {
        return;
    }
    if (SkLatticeIter::Valid(image->width(), image->height(), center)) {
        LatticePaint latticePaint(paint);
        this->onDrawImageNine(image, center, dst, latticePaint.get());
    } else {
        this->drawImageRect(image, dst, paint);
    }
}

void SkCanvas::drawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                const SkPaint* paint) {
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
        LatticePaint latticePaint(paint);
        this->onDrawImageLattice(image, latticePlusBounds, dst, latticePaint.get());
    } else {
        this->drawImageRect(image, dst, paint);
    }
}

static sk_sp<SkImage> bitmap_as_image(const SkBitmap& bitmap) {
    if (bitmap.drawsNothing()) {
        return nullptr;
    }
    return SkImage::MakeFromBitmap(bitmap);
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar dx, SkScalar dy, const SkPaint* paint) {
    this->drawImage(bitmap_as_image(bitmap), dx, dy, paint);
}

void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkRect& src, const SkRect& dst,
                              const SkPaint* paint, SrcRectConstraint constraint) {
    this->drawImageRect(bitmap_as_image(bitmap), src, dst, paint, constraint);
}

void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkIRect& isrc, const SkRect& dst,
                              const SkPaint* paint, SrcRectConstraint constraint) {
    this->drawBitmapRect(bitmap, SkRect::Make(isrc), dst, paint, constraint);
}

void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkRect& dst, const SkPaint* paint,
                              SrcRectConstraint constraint) {
    this->drawBitmapRect(bitmap, SkRect::MakeIWH(bitmap.width(), bitmap.height()), dst, paint,
                         constraint);
}

void SkCanvas::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                         const SkColor colors[], int count, SkBlendMode mode,
                         const SkRect* cull, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(atlas);
    if (count <= 0) {
        return;
    }
    SkASSERT(atlas);
    SkASSERT(tex);
    this->onDrawAtlas(atlas, xform, tex, colors, count, mode, cull, paint);
}

void SkCanvas::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (key) {
        this->onDrawAnnotation(rect, key, value);
    }
}

void SkCanvas::legacy_drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    if (src) {
        this->drawImageRect(image, *src, dst, paint, constraint);
    } else {
        this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()),
                            dst, paint, constraint);
    }
}

void SkCanvas::private_draw_shadow_rec(const SkPath& path, const SkDrawShadowRec& rec) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawShadowRec(path, rec);
}

void SkCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    SkPaint paint;
    const SkRect& pathBounds = path.getBounds();

    DRAW_BEGIN(paint, &pathBounds)
    while (iter.next()) {
        iter.fDevice->drawShadow(path, rec);
    }
    DRAW_END
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
                                               const SkPaint* paint,
                                               SrcRectConstraint constraint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawEdgeAAImageSet(imageSet, cnt, dstClips, preViewMatrices, paint, constraint);
}

//////////////////////////////////////////////////////////////////////////////
//  These are the virtual drawing methods
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::onDiscard() {
    if (fSurfaceBase) {
        fSurfaceBase->aboutToDraw(SkSurface::kDiscard_ContentChangeMode);
    }
}

void SkCanvas::onDrawPaint(const SkPaint& paint) {
    this->internalDrawPaint(paint);
}

void SkCanvas::internalDrawPaint(const SkPaint& paint) {
    DRAW_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, nullptr, false)

    while (iter.next()) {
        iter.fDevice->drawPaint(draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) {
    if ((long)count <= 0) {
        return;
    }

    SkRect r;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        // special-case 2 points (common for drawing a single line)
        if (2 == count) {
            r.set(pts[0], pts[1]);
        } else {
            r.setBounds(pts, SkToInt(count));
        }
        if (!r.isFinite()) {
            return;
        }
        SkRect storage;
        if (this->quickReject(paint.computeFastStrokeBounds(r, &storage))) {
            return;
        }
        bounds = &r;
    }

    SkASSERT(pts != nullptr);

    DRAW_BEGIN(paint, bounds)

    while (iter.next()) {
        iter.fDevice->drawPoints(mode, count, pts, draw.paint());
    }

    DRAW_END
}

static bool needs_autodrawlooper(SkCanvas* canvas, const SkPaint& paint) {
    return paint.getImageFilter() != nullptr;
}

void SkCanvas::onDrawRect(const SkRect& r, const SkPaint& paint) {
    SkASSERT(r.isSorted());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(r, &storage))) {
            return;
        }
    }

    if (needs_autodrawlooper(this, paint)) {
        DRAW_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, &r, false)

        while (iter.next()) {
            iter.fDevice->drawRect(r, draw.paint());
        }

        DRAW_END
    } else if (!paint.nothingToDraw()) {
        this->predrawNotify(&r, &paint, false);
        SkDrawIter iter(this);
        while (iter.next()) {
            iter.fDevice->drawRect(r, paint);
        }
    }
}

void SkCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    SkRect regionRect = SkRect::Make(region.getBounds());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(regionRect, &storage))) {
            return;
        }
    }

    DRAW_BEGIN(paint, &regionRect)

    while (iter.next()) {
        iter.fDevice->drawRegion(region, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawBehind(const SkPaint& paint) {
    SkIRect bounds;
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kBack_IterStart);
    for (;;) {
        const MCRec* rec = (const MCRec*)iter.prev();
        if (!rec) {
            return; // no backimages, so nothing to draw
        }
        if (rec->fBackImage) {
            bounds = SkIRect::MakeXYWH(rec->fBackImage->fLoc.fX, rec->fBackImage->fLoc.fY,
                                       rec->fBackImage->fImage->width(),
                                       rec->fBackImage->fImage->height());
            break;
        }
    }

    DRAW_BEGIN(paint, nullptr)

    while (iter.next()) {
        SkBaseDevice* dev = iter.fDevice;

        dev->save();
        // We use clipRegion because it is already defined to operate in dev-space
        // (i.e. ignores the ctm). However, it is going to first translate by -origin,
        // but we don't want that, so we undo that before calling in.
        SkRegion rgn(bounds.makeOffset(dev->getOrigin()));
        dev->clipRegion(rgn, SkClipOp::kIntersect);
        dev->drawPaint(draw.paint());
        dev->restore(fMCRec->fMatrix);
    }

    DRAW_END
}

void SkCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    SkASSERT(oval.isSorted());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(oval, &storage))) {
            return;
        }
    }

    DRAW_BEGIN(paint, &oval)

    while (iter.next()) {
        iter.fDevice->drawOval(oval, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawArc(const SkRect& oval, SkScalar startAngle,
                         SkScalar sweepAngle, bool useCenter,
                         const SkPaint& paint) {
    SkASSERT(oval.isSorted());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        // Note we're using the entire oval as the bounds.
        if (this->quickReject(paint.computeFastBounds(oval, &storage))) {
            return;
        }
    }

    DRAW_BEGIN(paint, &oval)

    while (iter.next()) {
        iter.fDevice->drawArc(oval, startAngle, sweepAngle, useCenter, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(rrect.getBounds(), &storage))) {
            return;
        }
    }

    if (rrect.isRect()) {
        // call the non-virtual version
        this->SkCanvas::drawRect(rrect.getBounds(), paint);
        return;
    } else if (rrect.isOval()) {
        // call the non-virtual version
        this->SkCanvas::drawOval(rrect.getBounds(), paint);
        return;
    }

    DRAW_BEGIN(paint, &rrect.getBounds())

    while (iter.next()) {
        iter.fDevice->drawRRect(rrect, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(outer.getBounds(), &storage))) {
            return;
        }
    }

    DRAW_BEGIN(paint, &outer.getBounds())

    while (iter.next()) {
        iter.fDevice->drawDRRect(outer, inner, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    if (!path.isFinite()) {
        return;
    }

    const SkRect& pathBounds = path.getBounds();
    if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(pathBounds, &storage))) {
            return;
        }
    }

    if (pathBounds.width() <= 0 && pathBounds.height() <= 0) {
        if (path.isInverseFillType()) {
            this->internalDrawPaint(paint);
            return;
        }
    }

    DRAW_BEGIN(paint, &pathBounds)

    while (iter.next()) {
        iter.fDevice->drawPath(path, draw.paint());
    }

    DRAW_END
}

bool SkCanvas::canDrawBitmapAsSprite(SkScalar x, SkScalar y, int w, int h, const SkPaint& paint) {
    if (!paint.getImageFilter()) {
        return false;
    }

    const SkMatrix& ctm = this->getTotalMatrix();
    if (!SkTreatAsSprite(ctm, SkISize::Make(w, h), paint)) {
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
    return ir.contains(fMCRec->fRasterClip.getBounds());
}

// Given storage for a real paint, and an optional paint parameter, clean-up the param (if non-null)
// given the drawing semantics for drawImage/bitmap (skbug.com/7804) and return it, or the original
// null.
static const SkPaint* init_image_paint(SkPaint* real, const SkPaint* paintParam) {
    if (paintParam) {
        *real = *paintParam;
        real->setStyle(SkPaint::kFill_Style);
        real->setPathEffect(nullptr);
        paintParam = real;
    }
    return paintParam;
}

void SkCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    SkPaint realPaint;
    paint = init_image_paint(&realPaint, paint);

    SkRect bounds = SkRect::MakeXYWH(x, y,
                                     SkIntToScalar(image->width()), SkIntToScalar(image->height()));
    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect tmp = bounds;
        if (paint) {
            paint->computeFastBounds(tmp, &tmp);
        }
        if (this->quickReject(tmp)) {
            return;
        }
    }
    // At this point we need a real paint object. If the caller passed null, then we should
    // use realPaint (in its default state). If the caller did pass a paint, then we have copied
    // (and modified) it in realPaint. Thus either way, "realPaint" is what we want to use.
    paint = &realPaint;

    sk_sp<SkSpecialImage> special;
    sk_sp<SkImageFilter> filter;
    bool drawAsSprite = this->canDrawBitmapAsSprite(x, y, image->width(), image->height(),
                                                    *paint);
    if (drawAsSprite && paint->getImageFilter()) {
        special = this->getDevice()->makeSpecial(image);
        if (special) {
            filter = paint->refImageFilter();
            realPaint.setImageFilter(nullptr); // This modifies 'paint' but is not const
        } else {
            drawAsSprite = false;
        }
    }

    DRAW_BEGIN_DRAWBITMAP(*paint, drawAsSprite, &bounds)

    while (iter.next()) {
        const SkPaint& pnt = draw.paint();
        if (special) {
            SkASSERT(!pnt.getImageFilter());

            // TODO(michaelludwig) - Many filters could probably be evaluated like this even if the
            // CTM is not translate-only; the post-transformation of the filtered image by the CTM
            // will probably look just as good and not require an extra layer.
            // TODO(michaelludwig) - Once image filter implementations can support source images
            // with non-(0,0) origins, we can just mark the origin as (x,y) instead of doing a
            // pre-concat here.
            SkMatrix layerToDevice = iter.fDevice->localToDevice();
            layerToDevice.preTranslate(x, y);
            skif::Mapping mapping(layerToDevice, SkMatrix::Translate(-x, -y));
            iter.fDevice->drawFilteredImage(mapping, special.get(), filter.get(), pnt);
        } else {
            iter.fDevice->drawImageRect(
                    image, nullptr, SkRect::MakeXYWH(x, y, image->width(), image->height()), pnt,
                    kStrict_SrcRectConstraint);
        }
    }

    DRAW_END
}

void SkCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                               const SkPaint* paint, SrcRectConstraint constraint) {
    SkPaint realPaint;
    paint = init_image_paint(&realPaint, paint);

    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage = dst;
        if (paint) {
            paint->computeFastBounds(dst, &storage);
        }
        if (this->quickReject(storage)) {
            return;
        }
    }
    paint = &realPaint;

    DRAW_BEGIN_CHECK_COMPLETE_OVERWRITE(*paint, &dst, image->isOpaque())

    while (iter.next()) {
        iter.fDevice->drawImageRect(image, src, dst, draw.paint(), constraint);
    }

    DRAW_END
}

void SkCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                               const SkPaint* paint) {
    SkPaint realPaint;
    paint = init_image_paint(&realPaint, paint);

    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }
    paint = &realPaint;

    DRAW_BEGIN(*paint, &dst)

    while (iter.next()) {
        iter.fDevice->drawImageNine(image, center, dst, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                  const SkPaint* paint) {
    SkPaint realPaint;
    paint = init_image_paint(&realPaint, paint);

    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }
    paint = &realPaint;

    DRAW_BEGIN(*paint, &dst)

    while (iter.next()) {
        iter.fDevice->drawImageLattice(image, lattice, dst, draw.paint());
    }

    DRAW_END
}

void SkCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                              const SkPaint& paint) {
    SkRect storage;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        storage = blob->bounds().makeOffset(x, y);
        SkRect tmp;
        if (this->quickReject(paint.computeFastBounds(storage, &tmp))) {
            return;
        }
        bounds = &storage;
    }

    // We cannot filter in the looper as we normally do, because the paint is
    // incomplete at this point (text-related attributes are embedded within blob run paints).
    DRAW_BEGIN(paint, bounds)

    while (iter.next()) {
        fScratchGlyphRunBuilder->drawTextBlob(draw.paint(), *blob, {x, y}, iter.fDevice);
    }

    DRAW_END
}

// These call the (virtual) onDraw... method
void SkCanvas::drawSimpleText(const void* text, size_t byteLength, SkTextEncoding encoding,
                              SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (byteLength) {
        sk_msan_assert_initialized(text, SkTAddOffset<const void>(text, byteLength));
        this->drawTextBlob(SkTextBlob::MakeFromText(text, byteLength, font, encoding), x, y, paint);
    }
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
    DRAW_BEGIN(paint, nullptr)

    while (iter.next()) {
        // In the common case of one iteration we could std::move vertices here.
        iter.fDevice->drawVertices(vertices, bmode, draw.paint());
    }

    DRAW_END
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
    // Since a patch is always within the convex hull of the control points, we discard it when its
    // bounding rectangle is completely outside the current clip.
    SkRect bounds;
    bounds.setBounds(cubics, SkPatchUtils::kNumCtrlPts);
    if (this->quickReject(bounds)) {
        return;
    }

    DRAW_BEGIN(paint, nullptr)

    while (iter.next()) {
        iter.fDevice->drawPatch(cubics, colors, texCoords, bmode, draw.paint());
    }

    DRAW_END
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
    this->getDevice()->drawDrawable(dr, matrix, this);
}

void SkCanvas::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                           const SkColor colors[], int count, SkBlendMode bmode,
                           const SkRect* cull, const SkPaint* paint) {
    if (cull && this->quickReject(*cull)) {
        return;
    }

    SkPaint pnt;
    if (paint) {
        pnt = *paint;
    }

    DRAW_BEGIN(pnt, nullptr)
    while (iter.next()) {
        iter.fDevice->drawAtlas(atlas, xform, tex, colors, count, bmode, draw.paint());
    }
    DRAW_END
}

void SkCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    SkASSERT(key);

    SkPaint paint;
    DRAW_BEGIN(paint, nullptr)
    while (iter.next()) {
        iter.fDevice->drawAnnotation(rect, key, value);
    }
    DRAW_END
}

void SkCanvas::onDrawEdgeAAQuad(const SkRect& r, const SkPoint clip[4], QuadAAFlags edgeAA,
                                const SkColor4f& color, SkBlendMode mode) {
    SkASSERT(r.isSorted());

    // If this used a paint, it would be a filled color with blend mode, which does not
    // need to use an autodraw loop, so use SkDrawIter directly.
    if (this->quickReject(r)) {
        return;
    }

    this->predrawNotify(&r, nullptr, false);
    SkDrawIter iter(this);
    while(iter.next()) {
        iter.fDevice->drawEdgeAAQuad(r, clip, edgeAA, color, mode);
    }
}

void SkCanvas::onDrawEdgeAAImageSet(const ImageSetEntry imageSet[], int count,
                                    const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    if (count <= 0) {
        // Nothing to draw
        return;
    }

    SkPaint realPaint;
    init_image_paint(&realPaint, paint);

    // We could calculate the set's dstRect union to always check quickReject(), but we can't reject
    // individual entries and Chromium's occlusion culling already makes it likely that at least one
    // entry will be visible. So, we only calculate the draw bounds when it's trivial (count == 1),
    // or we need it for the autolooper (since it greatly improves image filter perf).
    bool needsAutoLooper = needs_autodrawlooper(this, realPaint);
    bool setBoundsValid = count == 1 || needsAutoLooper;
    SkRect setBounds = imageSet[0].fDstRect;
    if (imageSet[0].fMatrixIndex >= 0) {
        // Account for the per-entry transform that is applied prior to the CTM when drawing
        preViewMatrices[imageSet[0].fMatrixIndex].mapRect(&setBounds);
    }
    if (needsAutoLooper) {
        for (int i = 1; i < count; ++i) {
            SkRect entryBounds = imageSet[i].fDstRect;
            if (imageSet[i].fMatrixIndex >= 0) {
                preViewMatrices[imageSet[i].fMatrixIndex].mapRect(&entryBounds);
            }
            setBounds.joinPossiblyEmptyRect(entryBounds);
        }
    }

    // If we happen to have the draw bounds, though, might as well check quickReject().
    if (setBoundsValid && realPaint.canComputeFastBounds()) {
        SkRect tmp;
        if (this->quickReject(realPaint.computeFastBounds(setBounds, &tmp))) {
            return;
        }
    }

    if (needsAutoLooper) {
        SkASSERT(setBoundsValid);
        DRAW_BEGIN(realPaint, &setBounds)
        while (iter.next()) {
            iter.fDevice->drawEdgeAAImageSet(
                imageSet, count, dstClips, preViewMatrices, draw.paint(), constraint);
        }
        DRAW_END
    } else {
        this->predrawNotify();
        SkDrawIter iter(this);
        while(iter.next()) {
            iter.fDevice->drawEdgeAAImageSet(
                imageSet, count, dstClips, preViewMatrices, realPaint, constraint);
        }
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
    if (!paint || paint->canComputeFastBounds()) {
        SkRect bounds = picture->cullRect();
        if (paint) {
            paint->computeFastBounds(bounds, &bounds);
        }
        if (matrix) {
            matrix->mapRect(&bounds);
        }
        if (this->quickReject(bounds)) {
            return;
        }
    }

    SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
    picture->playback(this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkCanvas::LayerIter::LayerIter(SkCanvas* canvas) {
    static_assert(sizeof(fStorage) >= sizeof(SkDrawIter), "fStorage_too_small");

    SkASSERT(canvas);

    fImpl = new (fStorage) SkDrawIter(canvas);
    // This advances the base iterator to the first device and caches its origin,
    // correctly handling the case where there are no devices.
    this->next();
}

SkCanvas::LayerIter::~LayerIter() {
    fImpl->~SkDrawIter();
}

void SkCanvas::LayerIter::next() {
    fDone = !fImpl->next();
    if (!fDone) {
        // Cache the device origin. LayerIter is only used in Android, which doesn't use image
        // filters, so its devices will always be able to report the origin exactly.
        fDeviceOrigin = fImpl->fDevice->getOrigin();
    }
}

SkBaseDevice* SkCanvas::LayerIter::device() const {
    return fImpl->fDevice;
}

const SkMatrix& SkCanvas::LayerIter::matrix() const {
    return fImpl->fDevice->localToDevice();
}

const SkPaint& SkCanvas::LayerIter::paint() const {
    const SkPaint* paint = fImpl->getPaint();
    if (nullptr == paint) {
        paint = &fDefaultPaint;
    }
    return *paint;
}

SkIRect SkCanvas::LayerIter::clipBounds() const {
    return fImpl->fDevice->getGlobalBounds();
}

int SkCanvas::LayerIter::x() const { return fDeviceOrigin.fX; }
int SkCanvas::LayerIter::y() const { return fDeviceOrigin.fY; }

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

static_assert((int)SkRegion::kDifference_Op         == (int)kDifference_SkClipOp, "");
static_assert((int)SkRegion::kIntersect_Op          == (int)kIntersect_SkClipOp, "");
static_assert((int)SkRegion::kUnion_Op              == (int)kUnion_SkClipOp, "");
static_assert((int)SkRegion::kXOR_Op                == (int)kXOR_SkClipOp, "");
static_assert((int)SkRegion::kReverseDifference_Op  == (int)kReverseDifference_SkClipOp, "");
static_assert((int)SkRegion::kReplace_Op            == (int)kReplace_SkClipOp, "");

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRasterHandleAllocator::Handle SkCanvas::accessTopRasterHandle() const {
    if (fAllocator && fMCRec->fTopLayer->fDevice) {
        const auto& dev = fMCRec->fTopLayer->fDevice;
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

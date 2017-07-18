/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkCanvasPriv.h"
#include "SkClipStack.h"
#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkDrawable.h"
#include "SkDrawFilter.h"
#include "SkDrawLooper.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkImageFilter.h"
#include "SkImageFilterCache.h"
#include "SkLatticeIter.h"
#include "SkLights.h"
#include "SkMakeUnique.h"
#include "SkMatrixUtils.h"
#include "SkMetaData.h"
#include "SkNoDrawCanvas.h"
#include "SkNx.h"
#include "SkPaintPriv.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkRasterClip.h"
#include "SkRasterHandleAllocator.h"
#include "SkRRect.h"
#include "SkSpecialImage.h"
#include "SkString.h"
#include "SkSurface_Base.h"
#include "SkTextBlob.h"
#include "SkTextFormatParams.h"
#include "SkTLazy.h"
#include "SkTraceEvent.h"
#include <new>

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkGr.h"

#endif
#include "SkClipOpPriv.h"
#include "SkVertices.h"

#define RETURN_ON_NULL(ptr)     do { if (nullptr == (ptr)) return; } while (0)

class SkNoPixelsDevice : public SkBaseDevice {
public:
    SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props)
        : SkBaseDevice(SkImageInfo::MakeUnknown(bounds.width(), bounds.height()), props)
    {
        // this fails if we enable this assert: DiscardableImageMapTest.GetDiscardableImagesInRectMaxImage
        //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);
    }

    void resetForNextPicture(const SkIRect& bounds) {
        //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);
        this->privateResize(bounds.width(), bounds.height());
    }

protected:
    // We don't track the clip at all (for performance), but we have to respond to some queries.
    // We pretend to be wide-open. We could pretend to always be empty, but that *seems* worse.
    void onSave() override {}
    void onRestore() override {}
    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override {}
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override {}
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override {}
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override {}
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override {}
    bool onClipIsAA() const override { return false; }
    void onAsRgnClip(SkRegion* rgn) const override {
        rgn->setRect(SkIRect::MakeWH(this->width(), this->height()));
    }
    ClipType onGetClipType() const override {
        return kRect_ClipType;
    }

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void drawRect(const SkRect&, const SkPaint&) override {}
    void drawOval(const SkRect&, const SkPaint&) override {}
    void drawRRect(const SkRRect&, const SkPaint&) override {}
    void drawPath(const SkPath&, const SkPaint&, const SkMatrix*, bool) override {}
    void drawBitmap(const SkBitmap&, SkScalar x, SkScalar y, const SkPaint&) override {}
    void drawSprite(const SkBitmap&, int, int, const SkPaint&) override {}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint&,
                        SkCanvas::SrcRectConstraint) override {}
    void drawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&) override {}
    void drawPosText(const void*, size_t, const SkScalar[], int, const SkPoint&,
                     const SkPaint&) override {}
    void drawDevice(SkBaseDevice*, int, int, const SkPaint&) override {}
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override {}

private:
    typedef SkBaseDevice INHERITED;
};

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
        if (paint->getMaskFilter() || paint->getLooper()
            || paint->getPathEffect() || paint->getImageFilter()) {
            return false; // conservative
        }
    }
    return SkPaintPriv::Overwrites(paint, (SkPaintPriv::ShaderOverrideOpacity)overrideOpacity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool gIgnoreSaveLayerBounds;
void SkCanvas::Internal_Private_SetIgnoreSaveLayerBounds(bool ignore) {
    gIgnoreSaveLayerBounds = ignore;
}
bool SkCanvas::Internal_Private_GetIgnoreSaveLayerBounds() {
    return gIgnoreSaveLayerBounds;
}

static bool gTreatSpriteAsBitmap;
void SkCanvas::Internal_Private_SetTreatSpriteAsBitmap(bool spriteAsBitmap) {
    gTreatSpriteAsBitmap = spriteAsBitmap;
}
bool SkCanvas::Internal_Private_GetTreatSpriteAsBitmap() {
    return gTreatSpriteAsBitmap;
}

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

typedef SkTLazy<SkPaint> SkLazyPaint;

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
    SkRasterClip                   fClip;
    std::unique_ptr<const SkPaint> fPaint; // may be null (in the future)
    SkMatrix                       fStashedMatrix; // original CTM; used by imagefilter in saveLayer
    sk_sp<SkImage>                 fClipImage;
    SkMatrix                       fClipMatrix;

    DeviceCM(sk_sp<SkBaseDevice> device, const SkPaint* paint, const SkMatrix& stashed,
             const SkImage* clipImage, const SkMatrix* clipMatrix)
        : fNext(nullptr)
        , fDevice(std::move(device))
        , fPaint(paint ? skstd::make_unique<SkPaint>(*paint) : nullptr)
        , fStashedMatrix(stashed)
        , fClipImage(sk_ref_sp(const_cast<SkImage*>(clipImage)))
        , fClipMatrix(clipMatrix ? *clipMatrix : SkMatrix::I())
    {}

    void reset(const SkIRect& bounds) {
        SkASSERT(!fPaint);
        SkASSERT(!fNext);
        SkASSERT(fDevice);
        fClip.setRect(bounds);
    }
};

/*  This is the record we keep for each save/restore level in the stack.
    Since a level optionally copies the matrix and/or stack, we have pointers
    for these fields. If the value is copied for this level, the copy is
    stored in the ...Storage field, and the pointer points to that. If the
    value is not copied for this level, we ignore ...Storage, and just point
    at the corresponding value in the previous level in the stack.
*/
class SkCanvas::MCRec {
public:
    SkDrawFilter*   fFilter;    // the current filter (or null)
    DeviceCM*       fLayer;
    /*  If there are any layers in the stack, this points to the top-most
        one that is at or below this level in the stack (so we know what
        bitmap/device to draw into from this level. This value is NOT
        reference counted, since the real owner is either our fLayer field,
        or a previous one in a lower level.)
    */
    DeviceCM*           fTopLayer;
    SkConservativeClip  fRasterClip;
    SkMatrix            fMatrix;
    int                 fDeferredSaveCount;

    MCRec() {
        fFilter     = nullptr;
        fLayer      = nullptr;
        fTopLayer   = nullptr;
        fMatrix.reset();
        fDeferredSaveCount = 0;

        // don't bother initializing fNext
        inc_rec();
    }
    MCRec(const MCRec& prev) : fRasterClip(prev.fRasterClip), fMatrix(prev.fMatrix) {
        fFilter = SkSafeRef(prev.fFilter);
        fLayer = nullptr;
        fTopLayer = prev.fTopLayer;
        fDeferredSaveCount = 0;

        // don't bother initializing fNext
        inc_rec();
    }
    ~MCRec() {
        SkSafeUnref(fFilter);
        delete fLayer;
        dec_rec();
    }

    void reset(const SkIRect& bounds) {
        SkASSERT(fLayer);
        SkASSERT(fDeferredSaveCount == 0);

        fMatrix.reset();
        fRasterClip.setRect(bounds);
        fLayer->reset(bounds);
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

    int getX() const { return fDevice->getOrigin().x(); }
    int getY() const { return fDevice->getOrigin().y(); }
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

static SkPaint* set_if_needed(SkLazyPaint* lazy, const SkPaint& orig) {
    return lazy->isValid() ? lazy->get() : lazy->set(orig);
}

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
    return SkColorFilter::MakeComposeFilter(std::move(imgCF), sk_ref_sp(paintCF));
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

class AutoDrawLooper {
public:
    // "rawBounds" is the original bounds of the primitive about to be drawn, unmodified by the
    // paint. It's used to determine the size of the offscreen layer for filters.
    // If null, the clip will be used instead.
    AutoDrawLooper(SkCanvas* canvas, const SkPaint& paint, bool skipLayerForImageFilter = false,
                   const SkRect* rawBounds = nullptr) : fOrigPaint(paint) {
        fCanvas = canvas;
#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
        fFilter = canvas->getDrawFilter();
#else
        fFilter = nullptr;
#endif
        fPaint = &fOrigPaint;
        fSaveCount = canvas->getSaveCount();
        fTempLayerForImageFilter = false;
        fDone = false;

        auto simplifiedCF = image_to_color_filter(fOrigPaint);
        if (simplifiedCF) {
            SkPaint* paint = set_if_needed(&fLazyPaintInit, fOrigPaint);
            paint->setColorFilter(std::move(simplifiedCF));
            paint->setImageFilter(nullptr);
            fPaint = paint;
        }

        if (!skipLayerForImageFilter && fPaint->getImageFilter()) {
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
            SkPaint tmp;
            tmp.setImageFilter(fPaint->refImageFilter());
            tmp.setBlendMode(fPaint->getBlendMode());
            SkRect storage;
            if (rawBounds) {
                // Make rawBounds include all paint outsets except for those due to image filters.
                rawBounds = &apply_paint_to_bounds_sans_imagefilter(*fPaint, *rawBounds, &storage);
            }
            (void)canvas->internalSaveLayer(SkCanvas::SaveLayerRec(rawBounds, &tmp),
                                            SkCanvas::kFullLayer_SaveLayerStrategy);
            fTempLayerForImageFilter = true;
            // we remove the imagefilter/xfermode inside doNext()
        }

        if (SkDrawLooper* looper = paint.getLooper()) {
            fLooperContext = looper->makeContext(canvas, &fAlloc);
            fIsSimple = false;
        } else {
            fLooperContext = nullptr;
            // can we be marked as simple?
            fIsSimple = !fFilter && !fTempLayerForImageFilter;
        }
    }

    ~AutoDrawLooper() {
        if (fTempLayerForImageFilter) {
            fCanvas->internalRestore();
        }
        SkASSERT(fCanvas->getSaveCount() == fSaveCount);
    }

    const SkPaint& paint() const {
        SkASSERT(fPaint);
        return *fPaint;
    }

    bool next(SkDrawFilter::Type drawType) {
        if (fDone) {
            return false;
        } else if (fIsSimple) {
            fDone = true;
            return !fPaint->nothingToDraw();
        } else {
            return this->doNext(drawType);
        }
    }

private:
    SkLazyPaint     fLazyPaintInit;       // base paint storage in case we need to modify it
    SkLazyPaint     fLazyPaintPerLooper;  // per-draw-looper storage, so the looper can modify it
    SkCanvas*       fCanvas;
    const SkPaint&  fOrigPaint;
    SkDrawFilter*   fFilter;
    const SkPaint*  fPaint;
    int             fSaveCount;
    bool            fTempLayerForImageFilter;
    bool            fDone;
    bool            fIsSimple;
    SkDrawLooper::Context* fLooperContext;
    SkSTArenaAlloc<48>     fAlloc;

    bool doNext(SkDrawFilter::Type drawType);
};

bool AutoDrawLooper::doNext(SkDrawFilter::Type drawType) {
    fPaint = nullptr;
    SkASSERT(!fIsSimple);
    SkASSERT(fLooperContext || fFilter || fTempLayerForImageFilter);

    SkPaint* paint = fLazyPaintPerLooper.set(fLazyPaintInit.isValid() ?
                                             *fLazyPaintInit.get() : fOrigPaint);

    if (fTempLayerForImageFilter) {
        paint->setImageFilter(nullptr);
        paint->setBlendMode(SkBlendMode::kSrcOver);
    }

    if (fLooperContext && !fLooperContext->next(fCanvas, paint)) {
        fDone = true;
        return false;
    }
    if (fFilter) {
        if (!fFilter->filter(paint, drawType)) {
            fDone = true;
            return false;
        }
        if (nullptr == fLooperContext) {
            // no looper means we only draw once
            fDone = true;
        }
    }
    fPaint = paint;

    // if we only came in here for the imagefilter, mark us as done
    if (!fLooperContext && !fFilter) {
        fDone = true;
    }

    // call this after any possible paint modifiers
    if (fPaint->nothingToDraw()) {
        fPaint = nullptr;
        return false;
    }
    return true;
}

////////// macros to place around the internal draw calls //////////////////

#define LOOPER_BEGIN_DRAWBITMAP(paint, skipLayerForFilter, bounds)  \
    this->predrawNotify();                                          \
    AutoDrawLooper looper(this, paint, skipLayerForFilter, bounds); \
    while (looper.next(SkDrawFilter::kBitmap_Type)) {               \
        SkDrawIter iter(this);


#define LOOPER_BEGIN_DRAWDEVICE(paint, type)                        \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, paint, true);                      \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_BEGIN(paint, type, bounds)                           \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, paint, false, bounds);             \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, type, bounds, auxOpaque)  \
    this->predrawNotify(bounds, &paint, auxOpaque);                 \
    AutoDrawLooper  looper(this, paint, false, bounds);             \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_END    }

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

SkBaseDevice* SkCanvas::init(SkBaseDevice* device, InitFlags flags) {
    if (device && device->forceConservativeRasterClip()) {
        flags = InitFlags(flags | kConservativeRasterClip_InitFlag);
    }

    fAllowSimplifyClip = false;
    fSaveCount = 1;
    fMetaData = nullptr;

    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec;
    fMCRec->fRasterClip.setDeviceClipRestriction(&fClipRestrictionRect);
    fIsScaleTranslate = true;

    SkASSERT(sizeof(DeviceCM) <= sizeof(fDeviceCMStorage));
    fMCRec->fLayer = (DeviceCM*)fDeviceCMStorage;
    new (fDeviceCMStorage) DeviceCM(sk_ref_sp(device), nullptr, fMCRec->fMatrix, nullptr, nullptr);

    fMCRec->fTopLayer = fMCRec->fLayer;

    fSurfaceBase = nullptr;

    if (device) {
        // The root device and the canvas should always have the same pixel geometry
        SkASSERT(fProps.pixelGeometry() == device->surfaceProps().pixelGeometry());
        fMCRec->fRasterClip.setRect(device->getGlobalBounds());
        fDeviceClipBounds = qr_clip_bounds(device->getGlobalBounds());

        device->androidFramework_setDeviceClipRestriction(&fClipRestrictionRect);
    }

    return device;
}

SkCanvas::SkCanvas()
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    this->init(nullptr, kDefault_InitFlags);
}

SkCanvas::SkCanvas(int width, int height, const SkSurfaceProps* props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfacePropsCopyOrDefault(props))
{
    inc_canvas();

    this->init(new SkNoPixelsDevice(SkIRect::MakeWH(SkTMax(width, 0), SkTMax(height, 0)), fProps),
               kDefault_InitFlags)->unref();
}

SkCanvas::SkCanvas(const SkIRect& bounds, InitFlags flags)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    SkIRect r = bounds.isEmpty() ? SkIRect::MakeEmpty() : bounds;
    this->init(new SkNoPixelsDevice(r, fProps), flags)->unref();
}

SkCanvas::SkCanvas(SkBaseDevice* device)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(device->surfaceProps())
{
    inc_canvas();

    this->init(device, kDefault_InitFlags);
}

SkCanvas::SkCanvas(SkBaseDevice* device, InitFlags flags)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(device->surfaceProps())
{
    inc_canvas();

    this->init(device, flags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(props)
{
    inc_canvas();

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps));
    this->init(device.get(), kDefault_InitFlags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, std::unique_ptr<SkRasterHandleAllocator> alloc,
                   SkRasterHandleAllocator::Handle hndl)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
    , fAllocator(std::move(alloc))
{
    inc_canvas();

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps, hndl));
    this->init(device.get(), kDefault_InitFlags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap, nullptr, nullptr) {}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
    , fAllocator(nullptr)
{
    inc_canvas();

    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
    sk_sp<SkBaseDevice> device(new SkBitmapDevice(tmp, fProps, nullptr));
    this->init(device.get(), kDefault_InitFlags);
}
#endif

SkCanvas::~SkCanvas() {
    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last

    this->internalRestore();    // restore the last, since we're going away

    delete fMetaData;

    dec_canvas();
}

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
SkDrawFilter* SkCanvas::getDrawFilter() const {
    return fMCRec->fFilter;
}

SkDrawFilter* SkCanvas::setDrawFilter(SkDrawFilter* filter) {
    this->checkForDeferredSave();
    SkRefCnt_SafeAssign(fMCRec->fFilter, filter);
    return filter;
}
#endif

SkMetaData& SkCanvas::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (nullptr == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
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

SkISize SkCanvas::getBaseLayerSize() const {
    SkBaseDevice* d = this->getDevice();
    return d ? SkISize::Make(d->width(), d->height()) : SkISize::Make(0, 0);
}

SkIRect SkCanvas::getTopLayerBounds() const {
    SkBaseDevice* d = this->getTopDevice();
    if (!d) {
        return SkIRect::MakeEmpty();
    }
    return SkIRect::MakeXYWH(d->getOrigin().x(), d->getOrigin().y(), d->width(), d->height());
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

bool SkCanvas::readPixels(const SkImageInfo& dstInfo, void* dstP, size_t rowBytes, int x, int y) {
    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }

    return device->readPixels(dstInfo, dstP, rowBytes, x, y);
}

bool SkCanvas::readPixels(const SkPixmap& pm, int x, int y) {
    return pm.addr() && this->readPixels(pm.info(), pm.writable_addr(), pm.rowBytes(), x, y);
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
    if (!srcRect.intersect(0, 0, device->width(), device->height())) {
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
    return device->writePixels(srcInfo, pixels, rowBytes, x, y);
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
    // sanity check
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
    return !(saveLayerFlags & SkCanvas::kDontClipToLayer_PrivateSaveLayerFlag);
}

bool SkCanvas::clipRectBounds(const SkRect* bounds, SaveLayerFlags saveLayerFlags,
                              SkIRect* intersection, const SkImageFilter* imageFilter) {
    SkIRect clipBounds = this->getDeviceClipBounds();
    if (clipBounds.isEmpty()) {
        return false;
    }

    const SkMatrix& ctm = fMCRec->fMatrix;  // this->getTotalMatrix()

    if (imageFilter) {
        clipBounds = imageFilter->filterBounds(clipBounds, ctm);
        if (bounds && !imageFilter->canComputeFastBounds()) {
            bounds = nullptr;
        }
    }
    SkIRect ir;
    if (bounds) {
        SkRect r;

        ctm.mapRect(&r, *bounds);
        r.roundOut(&ir);
        // early exit if the layer's bounds are clipped out
        if (!ir.intersect(clipBounds)) {
            if (BoundsAffectsClip(saveLayerFlags)) {
                fMCRec->fTopLayer->fDevice->clipRegion(SkRegion(), SkClipOp::kIntersect); // empty
                fMCRec->fRasterClip.setEmpty();
                fDeviceClipBounds.setEmpty();
            }
            return false;
        }
    } else {    // no user bounds, so just use the clip
        ir = clipBounds;
    }
    SkASSERT(!ir.isEmpty());

    if (BoundsAffectsClip(saveLayerFlags)) {
        // Simplify the current clips since they will be applied properly during restore()
        fMCRec->fRasterClip.setRect(ir);
        fDeviceClipBounds = qr_clip_bounds(ir);
    }

    if (intersection) {
        *intersection = ir;
    }
    return true;
}


int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint) {
    return this->saveLayer(SaveLayerRec(bounds, paint, 0));
}

int SkCanvas::saveLayerPreserveLCDTextRequests(const SkRect* bounds, const SkPaint* paint) {
    return this->saveLayer(SaveLayerRec(bounds, paint, kPreserveLCDText_SaveLayerFlag));
}

int SkCanvas::saveLayer(const SaveLayerRec& origRec) {
    SkTCopyOnFirstWrite<SaveLayerRec> rec(origRec);
    if (gIgnoreSaveLayerBounds) {
        rec.writable()->fBounds = nullptr;
    }

    SaveLayerStrategy strategy = this->getSaveLayerStrategy(*rec);
    fSaveCount += 1;
    this->internalSaveLayer(*rec, strategy);
    return this->getSaveCount() - 1;
}

void SkCanvas::DrawDeviceWithFilter(SkBaseDevice* src, const SkImageFilter* filter,
                                    SkBaseDevice* dst, const SkIPoint& dstOrigin,
                                    const SkMatrix& ctm) {
    SkDraw draw;
    SkRasterClip rc;
    rc.setRect(SkIRect::MakeWH(dst->width(), dst->height()));
    if (!dst->accessPixels(&draw.fDst)) {
        draw.fDst.reset(dst->imageInfo(), nullptr, 0);
    }
    draw.fMatrix = &SkMatrix::I();
    draw.fRC = &rc;

    SkPaint p;
    if (filter) {
        p.setImageFilter(filter->makeWithLocalMatrix(ctm));
    }

    int x = src->getOrigin().x() - dstOrigin.x();
    int y = src->getOrigin().y() - dstOrigin.y();
    auto special = src->snapSpecial();
    if (special) {
        dst->drawSpecial(special.get(), x, y, p, nullptr, SkMatrix::I());
    }
}

static SkImageInfo make_layer_info(const SkImageInfo& prev, int w, int h, bool isOpaque,
                                   const SkPaint* paint) {
    // need to force L32 for now if we have an image filter. Once filters support other colortypes
    // e.g. sRGB or F16, we can remove this check
    // SRGBTODO: Can we remove this check now?
    const bool hasImageFilter = paint && paint->getImageFilter();

    SkAlphaType alphaType = isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    if ((prev.bytesPerPixel() < 4) || hasImageFilter) {
        // force to L32
        return SkImageInfo::MakeN32(w, h, alphaType);
    } else {
        // keep the same characteristics as the prev
        return SkImageInfo::Make(w, h, prev.colorType(), alphaType, prev.refColorSpace());
    }
}

void SkCanvas::internalSaveLayer(const SaveLayerRec& rec, SaveLayerStrategy strategy) {
    const SkRect* bounds = rec.fBounds;
    const SkPaint* paint = rec.fPaint;
    SaveLayerFlags saveLayerFlags = rec.fSaveLayerFlags;

    SkLazyPaint lazyP;
    SkImageFilter* imageFilter = paint ? paint->getImageFilter() : NULL;
    SkMatrix stashedMatrix = fMCRec->fMatrix;
    SkMatrix remainder;
    SkSize scale;
    /*
     *  ImageFilters (so far) do not correctly handle matrices (CTM) that contain rotation/skew/etc.
     *  but they do handle scaling. To accommodate this, we do the following:
     *
     *  1. Stash off the current CTM
     *  2. Decompose the CTM into SCALE and REMAINDER
     *  3. Wack the CTM to be just SCALE, and wrap the imagefilter with a MatrixImageFilter that
     *     contains the REMAINDER
     *  4. Proceed as usual, allowing the client to draw into the layer (now with a scale-only CTM)
     *  5. During restore, we process the MatrixImageFilter, which applies REMAINDER to the output
     *     of the original imagefilter, and draw that (via drawSprite)
     *  6. Unwack the CTM to its original state (i.e. stashedMatrix)
     *
     *  Perhaps in the future we could augment #5 to apply REMAINDER as part of the draw (no longer
     *  a sprite operation) to avoid the extra buffer/overhead of MatrixImageFilter.
     */
    if (imageFilter && !stashedMatrix.isScaleTranslate() && !imageFilter->canHandleComplexCTM() &&
        stashedMatrix.decomposeScale(&scale, &remainder))
    {
        // We will restore the matrix (which we are overwriting here) in restore via fStashedMatrix
        this->internalSetMatrix(SkMatrix::MakeScale(scale.width(), scale.height()));
        SkPaint* p = lazyP.set(*paint);
        p->setImageFilter(SkImageFilter::MakeMatrixFilter(remainder,
                                                          SkFilterQuality::kLow_SkFilterQuality,
                                                          sk_ref_sp(imageFilter)));
        imageFilter = p->getImageFilter();
        paint = p;
    }

    // do this before we create the layer. We don't call the public save() since
    // that would invoke a possibly overridden virtual
    this->internalSave();

    SkIRect ir;
    if (!this->clipRectBounds(bounds, saveLayerFlags, &ir, imageFilter)) {
        return;
    }

    // FIXME: do willSaveLayer() overriders returning kNoLayer_SaveLayerStrategy really care about
    // the clipRectBounds() call above?
    if (kNoLayer_SaveLayerStrategy == strategy) {
        return;
    }

    bool isOpaque = SkToBool(saveLayerFlags & kIsOpaque_SaveLayerFlag);
    SkPixelGeometry geo = fProps.pixelGeometry();
    if (paint) {
        // TODO: perhaps add a query to filters so we might preserve opaqueness...
        if (paint->getImageFilter() || paint->getColorFilter()) {
            isOpaque = false;
            geo = kUnknown_SkPixelGeometry;
        }
    }

    SkBaseDevice* priorDevice = this->getTopDevice();
    if (nullptr == priorDevice) {   // Do we still need this check???
        SkDebugf("Unable to find device for layer.");
        return;
    }

    SkImageInfo info = make_layer_info(priorDevice->imageInfo(), ir.width(), ir.height(), isOpaque,
                                       paint);

    sk_sp<SkBaseDevice> newDevice;
    {
        const bool preserveLCDText = kOpaque_SkAlphaType == info.alphaType() ||
                                     (saveLayerFlags & kPreserveLCDText_SaveLayerFlag);
        const SkBaseDevice::TileUsage usage = SkBaseDevice::kNever_TileUsage;
        const SkBaseDevice::CreateInfo createInfo = SkBaseDevice::CreateInfo(info, usage, geo,
                                                                             preserveLCDText,
                                                                             fAllocator.get());
        newDevice.reset(priorDevice->onCreateDevice(createInfo, paint));
        if (!newDevice) {
            return;
        }
    }
    DeviceCM* layer = new DeviceCM(newDevice, paint, stashedMatrix, rec.fClipMask, rec.fClipMatrix);

    // only have a "next" if this new layer doesn't affect the clip (rare)
    layer->fNext = BoundsAffectsClip(saveLayerFlags) ? nullptr : fMCRec->fTopLayer;
    fMCRec->fLayer = layer;
    fMCRec->fTopLayer = layer;    // this field is NOT an owner of layer

    if ((rec.fSaveLayerFlags & kInitWithPrevious_SaveLayerFlag) || rec.fBackdrop) {
        DrawDeviceWithFilter(priorDevice, rec.fBackdrop, newDevice.get(), { ir.fLeft, ir.fTop },
                             fMCRec->fMatrix);
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

void SkCanvas::internalRestore() {
    SkASSERT(fMCStack.count() != 0);

    // reserve our layer (if any)
    DeviceCM* layer = fMCRec->fLayer;   // may be null
    // now detach it from fMCRec so we can pop(). Gets freed after its drawn
    fMCRec->fLayer = nullptr;

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*)fMCStack.back();

    if (fMCRec) {
        FOR_EACH_TOP_DEVICE(device->restore(fMCRec->fMatrix));
    }

    /*  Time to draw the layer's offscreen. We can't call the public drawSprite,
        since if we're being recorded, we don't want to record this (the
        recorder will have already recorded the restore).
    */
    if (layer) {
        if (fMCRec) {
            const SkIPoint& origin = layer->fDevice->getOrigin();
            this->internalDrawDevice(layer->fDevice.get(), origin.x(), origin.y(),
                                     layer->fPaint.get(),
                                     layer->fClipImage.get(), layer->fClipMatrix);
            // restore what we smashed in internalSaveLayer
            fMCRec->fMatrix = layer->fStashedMatrix;
            // reset this, since internalDrawDevice will have set it to true
            delete layer;
        } else {
            // we're at the root
            SkASSERT(layer == (void*)fDeviceCMStorage);
            layer->~DeviceCM();
            // no need to update fMCRec, 'cause we're killing the canvas
        }
    }

    if (fMCRec) {
        fIsScaleTranslate = fMCRec->fMatrix.isScaleTranslate();
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
        *origin = this->getTopDevice()->getOrigin();
    }
    return pmap.writable_addr();
}

bool SkCanvas::onAccessTopLayerPixels(SkPixmap* pmap) {
    SkBaseDevice* dev = this->getTopDevice();
    return dev && dev->accessPixels(pmap);
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::internalDrawDevice(SkBaseDevice* srcDev, int x, int y, const SkPaint* paint,
                                  SkImage* clipImage, const SkMatrix& clipMatrix) {
    SkPaint tmp;
    if (nullptr == paint) {
        paint = &tmp;
    }

    LOOPER_BEGIN_DRAWDEVICE(*paint, SkDrawFilter::kBitmap_Type)

    while (iter.next()) {
        SkBaseDevice* dstDev = iter.fDevice;
        paint = &looper.paint();
        SkImageFilter* filter = paint->getImageFilter();
        SkIPoint pos = { x - iter.getX(), y - iter.getY() };
        if (filter || clipImage) {
            sk_sp<SkSpecialImage> specialImage = srcDev->snapSpecial();
            if (specialImage) {
                dstDev->drawSpecial(specialImage.get(), pos.x(), pos.y(), *paint,
                                    clipImage, clipMatrix);
            }
        } else {
            dstDev->drawDevice(srcDev, pos.x(), pos.y(), *paint);
        }
    }

    LOOPER_END
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preTranslate(dx,dy);

        // Translate shouldn't affect the is-scale-translateness of the matrix.
        SkASSERT(fIsScaleTranslate == fMCRec->fMatrix.isScaleTranslate());

        FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));

        this->didTranslate(dx,dy);
    }
}

void SkCanvas::scale(SkScalar sx, SkScalar sy) {
    SkMatrix m;
    m.setScale(sx, sy);
    this->concat(m);
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
    fIsScaleTranslate = fMCRec->fMatrix.isScaleTranslate();

    FOR_EACH_TOP_DEVICE(device->setGlobalCTM(fMCRec->fMatrix));

    this->didConcat(matrix);
}

void SkCanvas::internalSetMatrix(const SkMatrix& matrix) {
    fMCRec->fMatrix = matrix;
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

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect, SkClipOp op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    this->onClipRect(rect, op, edgeStyle);
}

void SkCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    const bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    FOR_EACH_TOP_DEVICE(device->clipRect(rect, op, isAA));

    AutoValidateClip avc(this);
    fMCRec->fRasterClip.op(rect, fMCRec->fMatrix, this->getTopLayerBounds(), (SkRegion::Op)op,
                           isAA);
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
        fMCRec->fRasterClip.op(fClipRestrictionRect, SkRegion::kIntersect_Op);
        fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
    }
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

    fMCRec->fRasterClip.op(rrect, fMCRec->fMatrix, this->getTopLayerBounds(), (SkRegion::Op)op,
                           isAA);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
}

void SkCanvas::clipPath(const SkPath& path, SkClipOp op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;

    if (!path.isInverseFillType() && fMCRec->fMatrix.rectStaysRect()) {
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
    const SkMatrix* matrix = &fMCRec->fMatrix;
    fMCRec->fRasterClip.op(*rasterClipPath, *matrix, this->getTopLayerBounds(), (SkRegion::Op)op,
                           isAA);
    fDeviceClipBounds = qr_clip_bounds(fMCRec->fRasterClip.getBounds());
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkClipOp op) {
    this->checkForDeferredSave();
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    FOR_EACH_TOP_DEVICE(device->clipRegion(rgn, op));

    AutoValidateClip avc(this);

    fMCRec->fRasterClip.op(rgn, (SkRegion::Op)op);
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
    return dev && dev->onGetClipType() == SkBaseDevice::kRect_ClipType;
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
    SkASSERT(fIsScaleTranslate == fMCRec->fMatrix.isScaleTranslate());
#endif

    if (!fIsScaleTranslate) {
        return quick_reject_slow_path(src, fDeviceClipBounds, fMCRec->fMatrix);
    }

    // We inline the implementation of mapScaleTranslate() for the fast path.
    float sx = fMCRec->fMatrix.getScaleX();
    float sy = fMCRec->fMatrix.getScaleY();
    float tx = fMCRec->fMatrix.getTranslateX();
    float ty = fMCRec->fMatrix.getTranslateY();
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

SkRect SkCanvas::onGetLocalClipBounds() const {
    SkIRect ibounds = this->onGetDeviceClipBounds();
    if (ibounds.isEmpty()) {
        return SkRect::MakeEmpty();
    }

    SkMatrix inverse;
    // if we can't invert the CTM, we can't return local clip bounds
    if (!fMCRec->fMatrix.invert(&inverse)) {
        return SkRect::MakeEmpty();
    }

    SkRect bounds;
    SkRect r;
    // adjust it outwards in case we are antialiasing
    const int inset = 1;

    r.iset(ibounds.fLeft - inset, ibounds.fTop - inset,
           ibounds.fRight + inset, ibounds.fBottom + inset);
    inverse.mapRect(&bounds, r);
    return bounds;
}

SkIRect SkCanvas::onGetDeviceClipBounds() const {
    return fMCRec->fRasterClip.getBounds();
}

const SkMatrix& SkCanvas::getTotalMatrix() const {
    return fMCRec->fMatrix;
}

GrRenderTargetContext* SkCanvas::internal_private_accessTopLayerRenderTargetContext() {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessRenderTargetContext() : nullptr;
}

GrContext* SkCanvas::getGrContext() {
    SkBaseDevice* device = this->getTopDevice();
    return device ? device->context() : nullptr;
}

void SkCanvas::drawDRRect(const SkRRect& outer, const SkRRect& inner,
                          const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawDRRect()");
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

// These need to stop being virtual -- clients need to override the onDraw... versions

void SkCanvas::drawPaint(const SkPaint& paint) {
    this->onDrawPaint(paint);
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    // To avoid redundant logic in our culling code and various backends, we always sort rects
    // before passing them along.
    this->onDrawRect(r.makeSorted(), paint);
}

void SkCanvas::drawRegion(const SkRegion& region, const SkPaint& paint) {
    if (region.isEmpty()) {
        return;
    }

    if (region.isRect()) {
        return this->drawIRect(region.getBounds(), paint);
    }

    this->onDrawRegion(region, paint);
}

void SkCanvas::drawOval(const SkRect& r, const SkPaint& paint) {
    // To avoid redundant logic in our culling code and various backends, we always sort rects
    // before passing them along.
    this->onDrawOval(r.makeSorted(), paint);
}

void SkCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->onDrawRRect(rrect, paint);
}

void SkCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint) {
    this->onDrawPoints(mode, count, pts, paint);
}

void SkCanvas::drawVertices(const sk_sp<SkVertices>& vertices, SkBlendMode mode,
                            const SkPaint& paint) {
    RETURN_ON_NULL(vertices);
    this->onDrawVerticesObject(vertices.get(), mode, paint);
}

void SkCanvas::drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint) {
    RETURN_ON_NULL(vertices);
    this->onDrawVerticesObject(vertices, mode, paint);
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    this->onDrawPath(path, paint);
}

void SkCanvas::drawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    RETURN_ON_NULL(image);
    this->onDrawImage(image, x, y, paint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& src, const SkRect& dst,
                             const SkPaint* paint, SrcRectConstraint constraint) {
    RETURN_ON_NULL(image);
    if (dst.isEmpty() || src.isEmpty()) {
        return;
    }
    this->onDrawImageRect(image, &src, dst, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkIRect& isrc, const SkRect& dst,
                             const SkPaint* paint, SrcRectConstraint constraint) {
    RETURN_ON_NULL(image);
    this->drawImageRect(image, SkRect::Make(isrc), dst, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& dst, const SkPaint* paint,
                             SrcRectConstraint constraint) {
    RETURN_ON_NULL(image);
    this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst, paint,
                        constraint);
}

void SkCanvas::drawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                             const SkPaint* paint) {
    RETURN_ON_NULL(image);
    if (dst.isEmpty()) {
        return;
    }
    if (SkLatticeIter::Valid(image->width(), image->height(), center)) {
        this->onDrawImageNine(image, center, dst, paint);
    } else {
        this->drawImageRect(image, dst, paint);
    }
}

void SkCanvas::drawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                const SkPaint* paint) {
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
        this->onDrawImageLattice(image, latticePlusBounds, dst, paint);
    } else {
        this->drawImageRect(image, dst, paint);
    }
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar dx, SkScalar dy, const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }
    this->onDrawBitmap(bitmap, dx, dy, paint);
}

void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkRect& src, const SkRect& dst,
                              const SkPaint* paint, SrcRectConstraint constraint) {
    if (bitmap.drawsNothing() || dst.isEmpty() || src.isEmpty()) {
        return;
    }
    this->onDrawBitmapRect(bitmap, &src, dst, paint, constraint);
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

void SkCanvas::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                              const SkPaint* paint) {
    if (bitmap.drawsNothing() || dst.isEmpty()) {
        return;
    }
    if (SkLatticeIter::Valid(bitmap.width(), bitmap.height(), center)) {
        this->onDrawBitmapNine(bitmap, center, dst, paint);
    } else {
        this->drawBitmapRect(bitmap, dst, paint);
    }
}

void SkCanvas::drawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice, const SkRect& dst,
                                 const SkPaint* paint) {
    if (bitmap.drawsNothing() || dst.isEmpty()) {
        return;
    }

    SkIRect bounds;
    Lattice latticePlusBounds = lattice;
    if (!latticePlusBounds.fBounds) {
        bounds = SkIRect::MakeWH(bitmap.width(), bitmap.height());
        latticePlusBounds.fBounds = &bounds;
    }

    if (SkLatticeIter::Valid(bitmap.width(), bitmap.height(), latticePlusBounds)) {
        this->onDrawBitmapLattice(bitmap, latticePlusBounds, dst, paint);
    } else {
        this->drawBitmapRect(bitmap, dst, paint);
    }
}

void SkCanvas::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                         const SkColor colors[], int count, SkBlendMode mode,
                         const SkRect* cull, const SkPaint* paint) {
    RETURN_ON_NULL(atlas);
    if (count <= 0) {
        return;
    }
    SkASSERT(atlas);
    SkASSERT(tex);
    this->onDrawAtlas(atlas, xform, tex, colors, count, mode, cull, paint);
}

void SkCanvas::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
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
void SkCanvas::legacy_drawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                     const SkPaint* paint, SrcRectConstraint constraint) {
    if (src) {
        this->drawBitmapRect(bitmap, *src, dst, paint, constraint);
    } else {
        this->drawBitmapRect(bitmap, SkRect::MakeIWH(bitmap.width(), bitmap.height()),
                             dst, paint, constraint);
    }
}

void SkCanvas::private_draw_shadow_rec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->onDrawShadowRec(path, rec);
}

void SkCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    SkPaint paint;
    const SkRect& pathBounds = path.getBounds();

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, &pathBounds)
    while (iter.next()) {
        iter.fDevice->drawShadow(path, rec);
    }
    LOOPER_END
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
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPaint()");
    this->internalDrawPaint(paint);
}

void SkCanvas::internalDrawPaint(const SkPaint& paint) {
    LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, SkDrawFilter::kPaint_Type, nullptr, false)

    while (iter.next()) {
        iter.fDevice->drawPaint(looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) {
    TRACE_EVENT1("disabled-by-default-skia", "SkCanvas::drawPoints()", "count", static_cast<uint64_t>(count));
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
            r.set(pts, SkToInt(count));
        }
        SkRect storage;
        if (this->quickReject(paint.computeFastStrokeBounds(r, &storage))) {
            return;
        }
        bounds = &r;
    }

    SkASSERT(pts != nullptr);

    LOOPER_BEGIN(paint, SkDrawFilter::kPoint_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPoints(mode, count, pts, looper.paint());
    }

    LOOPER_END
}

static bool needs_autodrawlooper(SkCanvas* canvas, const SkPaint& paint) {
    return ((intptr_t)paint.getImageFilter()    |
#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
            (intptr_t)canvas->getDrawFilter()   |
#endif
            (intptr_t)paint.getLooper()         ) != 0;
}

void SkCanvas::onDrawRect(const SkRect& r, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawRect()");
    SkASSERT(r.isSorted());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(r, &storage))) {
            return;
        }
    }

    if (needs_autodrawlooper(this, paint)) {
        LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, SkDrawFilter::kRect_Type, &r, false)

        while (iter.next()) {
            iter.fDevice->drawRect(r, looper.paint());
        }

        LOOPER_END
    } else {
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

    LOOPER_BEGIN(paint, SkDrawFilter::kRect_Type, &regionRect)

    while (iter.next()) {
        iter.fDevice->drawRegion(region, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawOval()");
    SkASSERT(oval.isSorted());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(oval, &storage))) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kOval_Type, &oval)

    while (iter.next()) {
        iter.fDevice->drawOval(oval, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawArc(const SkRect& oval, SkScalar startAngle,
                         SkScalar sweepAngle, bool useCenter,
                         const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawArc()");
    SkASSERT(oval.isSorted());
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        // Note we're using the entire oval as the bounds.
        if (this->quickReject(paint.computeFastBounds(oval, &storage))) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kOval_Type, &oval)

    while (iter.next()) {
        iter.fDevice->drawArc(oval, startAngle, sweepAngle, useCenter, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawRRect()");
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

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, &rrect.getBounds())

    while (iter.next()) {
        iter.fDevice->drawRRect(rrect, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(outer.getBounds(), &storage))) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, &outer.getBounds())

    while (iter.next()) {
        iter.fDevice->drawDRRect(outer, inner, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPath()");
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

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, &pathBounds)

    while (iter.next()) {
        iter.fDevice->drawPath(path, looper.paint());
    }

    LOOPER_END
}

bool SkCanvas::canDrawBitmapAsSprite(SkScalar x, SkScalar y, int w, int h, const SkPaint& paint) {
    if (!paint.getImageFilter()) {
        return false;
    }

    const SkMatrix& ctm = this->getTotalMatrix();
    if (!SkTreatAsSprite(ctm, SkISize::Make(w, h), paint)) {
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

void SkCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawImage()");
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

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    sk_sp<SkSpecialImage> special;
    bool drawAsSprite = this->canDrawBitmapAsSprite(x, y, image->width(), image->height(),
                                                    *paint);
    if (drawAsSprite && paint->getImageFilter()) {
        special = this->getDevice()->makeSpecial(image);
        if (!special) {
            drawAsSprite = false;
        }
    }

    LOOPER_BEGIN_DRAWBITMAP(*paint, drawAsSprite, &bounds)

    while (iter.next()) {
        const SkPaint& pnt = looper.paint();
        if (special) {
            SkPoint pt;
            iter.fDevice->ctm().mapXY(x, y, &pt);
            iter.fDevice->drawSpecial(special.get(),
                                      SkScalarRoundToInt(pt.fX),
                                      SkScalarRoundToInt(pt.fY), pnt,
                                      nullptr, SkMatrix::I());
        } else {
            iter.fDevice->drawImage(image, x, y, pnt);
        }
    }

    LOOPER_END
}

void SkCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                               const SkPaint* paint, SrcRectConstraint constraint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawImageRect()");
    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage = dst;
        if (paint) {
            paint->computeFastBounds(dst, &storage);
        }
        if (this->quickReject(storage)) {
            return;
        }
    }
    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(*paint, SkDrawFilter::kBitmap_Type, &dst,
                                          image->isOpaque())

    while (iter.next()) {
        iter.fDevice->drawImageRect(image, src, dst, looper.paint(), constraint);
    }

    LOOPER_END
}

void SkCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y, const SkPaint* paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawBitmap()");
    SkDEBUGCODE(bitmap.validate();)

    if (bitmap.drawsNothing()) {
        return;
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    SkRect bounds;
    bitmap.getBounds(&bounds);
    bounds.offset(x, y);
    bool canFastBounds = paint->canComputeFastBounds();
    if (canFastBounds) {
        SkRect storage;
        if (this->quickReject(paint->computeFastBounds(bounds, &storage))) {
            return;
        }
    }

    sk_sp<SkSpecialImage> special;
    bool drawAsSprite = canFastBounds && this->canDrawBitmapAsSprite(x, y, bitmap.width(),
                                                                     bitmap.height(), *paint);
    if (drawAsSprite && paint->getImageFilter()) {
        special = this->getDevice()->makeSpecial(bitmap);
        if (!special) {
            drawAsSprite = false;
        }
    }

    LOOPER_BEGIN_DRAWBITMAP(*paint, drawAsSprite, &bounds)

    while (iter.next()) {
        const SkPaint& pnt = looper.paint();
        if (special) {
            SkPoint pt;
            iter.fDevice->ctm().mapXY(x, y, &pt);
            iter.fDevice->drawSpecial(special.get(),
                                      SkScalarRoundToInt(pt.fX),
                                      SkScalarRoundToInt(pt.fY), pnt,
                                      nullptr, SkMatrix::I());
        } else {
            iter.fDevice->drawBitmap(bitmap, x, y, looper.paint());
        }
    }

    LOOPER_END
}

// this one is non-virtual, so it can be called safely by other canvas apis
void SkCanvas::internalDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      SrcRectConstraint constraint) {
    if (bitmap.drawsNothing() || dst.isEmpty()) {
        return;
    }

    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(*paint, SkDrawFilter::kBitmap_Type, &dst,
                                          bitmap.isOpaque())

    while (iter.next()) {
        iter.fDevice->drawBitmapRect(bitmap, src, dst, looper.paint(), constraint);
    }

    LOOPER_END
}

void SkCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                const SkPaint* paint, SrcRectConstraint constraint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawBitmapRectToRect()");
    SkDEBUGCODE(bitmap.validate();)
    this->internalDrawBitmapRect(bitmap, src, dst, paint, constraint);
}

void SkCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                               const SkPaint* paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawImageNine()");

    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, &dst)

    while (iter.next()) {
        iter.fDevice->drawImageNine(image, center, dst, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                                const SkPaint* paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawBitmapNine()");
    SkDEBUGCODE(bitmap.validate();)

    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, &dst)

    while (iter.next()) {
        iter.fDevice->drawBitmapNine(bitmap, center, dst, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                  const SkPaint* paint) {
    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, &dst)

    while (iter.next()) {
        iter.fDevice->drawImageLattice(image, lattice, dst, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                   const SkRect& dst, const SkPaint* paint) {
    if (nullptr == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, &dst)

    while (iter.next()) {
        iter.fDevice->drawBitmapLattice(bitmap, lattice, dst, looper.paint());
    }

    LOOPER_END
}

class SkDeviceFilteredPaint {
public:
    SkDeviceFilteredPaint(SkBaseDevice* device, const SkPaint& paint) {
        uint32_t filteredFlags = device->filterTextFlags(paint);
        if (filteredFlags != paint.getFlags()) {
            SkPaint* newPaint = fLazy.set(paint);
            newPaint->setFlags(filteredFlags);
            fPaint = newPaint;
        } else {
            fPaint = &paint;
        }
    }

    const SkPaint& paint() const { return *fPaint; }

private:
    const SkPaint*  fPaint;
    SkLazyPaint     fLazy;
};

void SkCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawText(text, byteLength, x, y, dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                             const SkPaint& paint) {
    SkPoint textOffset = SkPoint::Make(0, 0);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(text, byteLength, &pos->fX, 2, textOffset,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                              SkScalar constY, const SkPaint& paint) {

    SkPoint textOffset = SkPoint::Make(0, constY);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(text, byteLength, xpos, 1, textOffset,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                const SkMatrix* matrix, const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        iter.fDevice->drawTextOnPath(text, byteLength, path,
                                     matrix, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                                 const SkRect* cullRect, const SkPaint& paint) {
    if (cullRect && this->quickReject(*cullRect)) {
        return;
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        iter.fDevice->drawTextRSXform(text, byteLength, xform, looper.paint());
    }

    LOOPER_END
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
    SkDrawFilter* drawFilter = fMCRec->fFilter;
    fMCRec->fFilter = nullptr;

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, bounds)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawTextBlob(blob, x, y, dfp.paint(), drawFilter);
    }

    LOOPER_END

    fMCRec->fFilter = drawFilter;
}

void SkCanvas::drawString(const SkString& string, SkScalar x, SkScalar y, const SkPaint& paint) {
    this->drawText(string.c_str(), string.size(), x, y, paint);
}

// These will become non-virtual, so they always call the (virtual) onDraw... method
void SkCanvas::drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                        const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawText()");
    if (byteLength) {
        this->onDrawText(text, byteLength, x, y, paint);
    }
}
void SkCanvas::drawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                           const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPosText()");
    if (byteLength) {
        this->onDrawPosText(text, byteLength, pos, paint);
    }
}
void SkCanvas::drawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                            SkScalar constY, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPosTextH()");
    if (byteLength) {
        this->onDrawPosTextH(text, byteLength, xpos, constY, paint);
    }
}
void SkCanvas::drawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                              const SkMatrix* matrix, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawTextOnPath()");
    if (byteLength) {
        this->onDrawTextOnPath(text, byteLength, path, matrix, paint);
    }
}
void SkCanvas::drawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                               const SkRect* cullRect, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawTextRSXform()");
    if (byteLength) {
        this->onDrawTextRSXform(text, byteLength, xform, cullRect, paint);
    }
}
void SkCanvas::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    RETURN_ON_NULL(blob);
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawTextBlob()");
    this->onDrawTextBlob(blob, x, y, paint);
}

void SkCanvas::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                    const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawVertices()");
    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, nullptr)

    while (iter.next()) {
        // In the common case of one iteration we could std::move vertices here.
        iter.fDevice->drawVertices(vertices, bmode, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                         const SkPoint texCoords[4], SkBlendMode bmode,
                         const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPatch()");
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
    bounds.set(cubics, SkPatchUtils::kNumCtrlPts);
    if (this->quickReject(bounds)) {
        return;
    }

    const bool interpColorsLinearly = (this->imageInfo().colorSpace() != nullptr);

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, nullptr)

    while (iter.next()) {
        iter.fDevice->drawPatch(cubics, colors, texCoords, bmode, interpColorsLinearly, paint);
    }

    LOOPER_END
}

void SkCanvas::drawDrawable(SkDrawable* dr, SkScalar x, SkScalar y) {
    RETURN_ON_NULL(dr);
    if (x || y) {
        SkMatrix matrix = SkMatrix::MakeTrans(x, y);
        this->onDrawDrawable(dr, &matrix);
    } else {
        this->onDrawDrawable(dr, nullptr);
    }
}

void SkCanvas::drawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    RETURN_ON_NULL(dr);
    if (matrix && matrix->isIdentity()) {
        matrix = nullptr;
    }
    this->onDrawDrawable(dr, matrix);
}

void SkCanvas::onDrawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    // drawable bounds are no longer reliable (e.g. android displaylist)
    // so don't use them for quick-reject
    dr->draw(this, matrix);
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

    LOOPER_BEGIN(pnt, SkDrawFilter::kPath_Type, nullptr)
    while (iter.next()) {
        iter.fDevice->drawAtlas(atlas, xform, tex, colors, count, bmode, pnt);
    }
    LOOPER_END
}

void SkCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    SkASSERT(key);

    SkPaint paint;
    LOOPER_BEGIN(paint, SkDrawFilter::kRect_Type, nullptr)
    while (iter.next()) {
        iter.fDevice->drawAnnotation(rect, key, value);
    }
    LOOPER_END
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawColor(SkColor c, SkBlendMode mode) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawColor()");
    SkPaint paint;

    paint.setColor(c);
    paint.setBlendMode(mode);
    this->drawPaint(paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPoint(SkPaint)");
    const SkPoint pt = { x, y };
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawLine()");
    SkPoint pts[2];

    pts[0].set(x0, y0);
    pts[1].set(x1, y1);
    this->drawPoints(kLines_PointMode, 2, pts, paint);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawCircle()");
    if (radius < 0) {
        radius = 0;
    }

    SkRect  r;
    r.set(cx - radius, cy - radius, cx + radius, cy + radius);
    this->drawOval(r, paint);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry,
                             const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawRoundRect()");
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
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawArc()");
    if (oval.isEmpty() || !sweepAngle) {
        return;
    }
    this->onDrawArc(oval, startAngle, sweepAngle, useCenter, paint);
}

void SkCanvas::drawTextOnPathHV(const void* text, size_t byteLength,
                                const SkPath& path, SkScalar hOffset,
                                SkScalar vOffset, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawTextOnPathHV()");
    SkMatrix    matrix;

    matrix.setTranslate(hOffset, vOffset);
    this->drawTextOnPath(text, byteLength, path, &matrix, paint);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  This constant is trying to balance the speed of ref'ing a subpicture into a parent picture,
 *  against the playback cost of recursing into the subpicture to get at its actual ops.
 *
 *  For now we pick a conservatively small value, though measurement (and other heuristics like
 *  the type of ops contained) may justify changing this value.
 */
#define kMaxPictureOpsToUnrollInsteadOfRef  1

void SkCanvas::drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {
    RETURN_ON_NULL(picture);

    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPicture()");
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkCanvas::LayerIter::LayerIter(SkCanvas* canvas) {
    static_assert(sizeof(fStorage) >= sizeof(SkDrawIter), "fStorage_too_small");

    SkASSERT(canvas);

    fImpl = new (fStorage) SkDrawIter(canvas);
    fDone = !fImpl->next();
}

SkCanvas::LayerIter::~LayerIter() {
    fImpl->~SkDrawIter();
}

void SkCanvas::LayerIter::next() {
    fDone = !fImpl->next();
}

SkBaseDevice* SkCanvas::LayerIter::device() const {
    return fImpl->fDevice;
}

const SkMatrix& SkCanvas::LayerIter::matrix() const {
    return fImpl->fDevice->ctm();
}

const SkPaint& SkCanvas::LayerIter::paint() const {
    const SkPaint* paint = fImpl->getPaint();
    if (nullptr == paint) {
        paint = &fDefaultPaint;
    }
    return *paint;
}

void SkCanvas::LayerIter::clip(SkRegion* rgn) const {
    return fImpl->fDevice->onAsRgnClip(rgn);
}

int SkCanvas::LayerIter::x() const { return fImpl->getX(); }
int SkCanvas::LayerIter::y() const { return fImpl->getY(); }

///////////////////////////////////////////////////////////////////////////////

static bool supported_for_raster_canvas(const SkImageInfo& info) {
    switch (info.alphaType()) {
        case kPremul_SkAlphaType:
        case kOpaque_SkAlphaType:
            break;
        default:
            return false;
    }

    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kN32_SkColorType:
        case kRGBA_F16_SkColorType:
            break;
        default:
            return false;
    }

    return true;
}

std::unique_ptr<SkCanvas> SkCanvas::MakeRasterDirect(const SkImageInfo& info, void* pixels,
                                                     size_t rowBytes) {
    if (!supported_for_raster_canvas(info)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return nullptr;
    }
    return skstd::make_unique<SkCanvas>(bitmap);
}

///////////////////////////////////////////////////////////////////////////////

SkAutoCanvasMatrixPaint::SkAutoCanvasMatrixPaint(SkCanvas* canvas, const SkMatrix* matrix,
                                                 const SkPaint* paint, const SkRect& bounds)
    : fCanvas(canvas)
    , fSaveCount(canvas->getSaveCount())
{
    if (paint) {
        SkRect newBounds = bounds;
        if (matrix) {
            matrix->mapRect(&newBounds);
        }
        canvas->saveLayer(&newBounds, paint);
    } else if (matrix) {
        canvas->save();
    }

    if (matrix) {
        canvas->concat(*matrix);
    }
}

SkAutoCanvasMatrixPaint::~SkAutoCanvasMatrixPaint() {
    fCanvas->restoreToCount(fSaveCount);
}

///////////////////////////////////////////////////////////////////////////////

SkNoDrawCanvas::SkNoDrawCanvas(int width, int height)
    : INHERITED(SkIRect::MakeWH(width, height), kConservativeRasterClip_InitFlag) {}

SkNoDrawCanvas::SkNoDrawCanvas(const SkIRect& bounds)
    : INHERITED(bounds, kConservativeRasterClip_InitFlag) {}

SkCanvas::SaveLayerStrategy SkNoDrawCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    (void)this->INHERITED::getSaveLayerStrategy(rec);
    return kNoLayer_SaveLayerStrategy;
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
        SkIPoint origin = dev->getOrigin();
        SkMatrix ctm = this->getTotalMatrix();
        ctm.preTranslate(SkIntToScalar(-origin.x()), SkIntToScalar(-origin.y()));

        SkIRect clip = fMCRec->fRasterClip.getBounds();
        clip.offset(-origin.x(), -origin.y());
        if (!clip.intersect(0, 0, dev->width(), dev->height())) {
            clip.setEmpty();
        }

        fAllocator->updateHandle(handle, ctm, clip);
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
    if (!alloc || !supported_for_raster_canvas(info)) {
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

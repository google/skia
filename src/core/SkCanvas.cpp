/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkCanvasPriv.h"
#include "SkClipStack.h"
#include "SkColorFilter.h"
#include "SkDevice.h"
#include "SkDraw.h"
#include "SkDrawable.h"
#include "SkDrawFilter.h"
#include "SkDrawLooper.h"
#include "SkErrorInternals.h"
#include "SkImage.h"
#include "SkMetaData.h"
#include "SkNinePatchIter.h"
#include "SkPaintPriv.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkRasterClip.h"
#include "SkReadPixelsRec.h"
#include "SkRRect.h"
#include "SkSmallAllocator.h"
#include "SkSurface_Base.h"
#include "SkTextBlob.h"
#include "SkTextFormatParams.h"
#include "SkTLazy.h"
#include "SkTraceEvent.h"

#include <new>

#if SK_SUPPORT_GPU
#include "GrRenderTarget.h"
#endif

#define SK_SUPPORT_SRC_BOUNDS_BLOAT_FOR_IMAGEFILTERS

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
    if (!this->getClipStack()->quickContains(bounds)) {
        return false;
    }

    if (rect) {
        if (!this->getTotalMatrix().rectStaysRect()) {
            return false; // conservative
        }

        SkRect devRect;
        this->getTotalMatrix().mapRect(&devRect, *rect);
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
//#define SK_ENABLE_CLIP_QUICKREJECT

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

static uint32_t filter_paint_flags(const SkSurfaceProps& props, uint32_t flags) {
    const uint32_t propFlags = props.flags();
    if (propFlags & SkSurfaceProps::kDisallowDither_Flag) {
        flags &= ~SkPaint::kDither_Flag;
    }
    if (propFlags & SkSurfaceProps::kDisallowAntiAlias_Flag) {
        flags &= ~SkPaint::kAntiAlias_Flag;
    }
    return flags;
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
    DeviceCM*           fNext;
    SkBaseDevice*       fDevice;
    SkRasterClip        fClip;
    SkPaint*            fPaint; // may be null (in the future)
    const SkMatrix*     fMatrix;
    SkMatrix            fMatrixStorage;
    const bool          fDeviceIsBitmapDevice;

    DeviceCM(SkBaseDevice* device, const SkPaint* paint, SkCanvas* canvas,
             bool conservativeRasterClip, bool deviceIsBitmapDevice)
        : fNext(nullptr)
        , fClip(conservativeRasterClip)
        , fDeviceIsBitmapDevice(deviceIsBitmapDevice)
    {
        if (nullptr != device) {
            device->ref();
            device->onAttachToCanvas(canvas);
        }
        fDevice = device;
        fPaint = paint ? new SkPaint(*paint) : nullptr;
    }

    ~DeviceCM() {
        if (fDevice) {
            fDevice->onDetachFromCanvas();
            fDevice->unref();
        }
        delete fPaint;
    }

    void reset(const SkIRect& bounds) {
        SkASSERT(!fPaint);
        SkASSERT(!fNext);
        SkASSERT(fDevice);
        fClip.setRect(bounds);
    }

    void updateMC(const SkMatrix& totalMatrix, const SkRasterClip& totalClip,
                  const SkClipStack& clipStack, SkRasterClip* updateClip) {
        int x = fDevice->getOrigin().x();
        int y = fDevice->getOrigin().y();
        int width = fDevice->width();
        int height = fDevice->height();

        if ((x | y) == 0) {
            fMatrix = &totalMatrix;
            fClip = totalClip;
        } else {
            fMatrixStorage = totalMatrix;
            fMatrixStorage.postTranslate(SkIntToScalar(-x),
                                         SkIntToScalar(-y));
            fMatrix = &fMatrixStorage;

            totalClip.translate(-x, -y, &fClip);
        }

        fClip.op(SkIRect::MakeWH(width, height), SkRegion::kIntersect_Op);

        // intersect clip, but don't translate it (yet)

        if (updateClip) {
            updateClip->op(SkIRect::MakeXYWH(x, y, width, height),
                           SkRegion::kDifference_Op);
        }

        fDevice->setMatrixClip(*fMatrix, fClip.forceGetBW(), clipStack);

#ifdef SK_DEBUG
        if (!fClip.isEmpty()) {
            SkIRect deviceR;
            deviceR.set(0, 0, width, height);
            SkASSERT(deviceR.contains(fClip.getBounds()));
        }
#endif
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
    DeviceCM*       fTopLayer;
    SkRasterClip    fRasterClip;
    SkMatrix        fMatrix;
    int             fDeferredSaveCount;

    MCRec(bool conservativeRasterClip) : fRasterClip(conservativeRasterClip) {
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

class SkDrawIter : public SkDraw {
public:
    SkDrawIter(SkCanvas* canvas, bool skipEmptyClips = true) {
        canvas = canvas->canvasForDrawIter();
        fCanvas = canvas;
        canvas->updateDeviceCMCache();

        fClipStack = canvas->fClipStack;
        fCurrLayer = canvas->fMCRec->fTopLayer;
        fSkipEmptyClips = skipEmptyClips;
    }

    bool next() {
        // skip over recs with empty clips
        if (fSkipEmptyClips) {
            while (fCurrLayer && fCurrLayer->fClip.isEmpty()) {
                fCurrLayer = fCurrLayer->fNext;
            }
        }

        const DeviceCM* rec = fCurrLayer;
        if (rec && rec->fDevice) {

            fMatrix = rec->fMatrix;
            fClip   = &((SkRasterClip*)&rec->fClip)->forceGetBW();
            fRC     = &rec->fClip;
            fDevice = rec->fDevice;
            if (!fDevice->accessPixels(&fDst)) {
                fDst.reset(fDevice->imageInfo(), nullptr, 0);
            }
            fPaint  = rec->fPaint;
            SkDEBUGCODE(this->validate();)

            fCurrLayer = rec->fNext;
            // fCurrLayer may be nullptr now

            return true;
        }
        return false;
    }

    SkBaseDevice* getDevice() const { return fDevice; }
    int getX() const { return fDevice->getOrigin().x(); }
    int getY() const { return fDevice->getOrigin().y(); }
    const SkMatrix& getMatrix() const { return *fMatrix; }
    const SkRegion& getClip() const { return *fClip; }
    const SkPaint* getPaint() const { return fPaint; }

private:
    SkCanvas*       fCanvas;
    const DeviceCM* fCurrLayer;
    const SkPaint*  fPaint;     // May be null.
    SkBool8         fSkipEmptyClips;

    typedef SkDraw INHERITED;
};

/////////////////////////////////////////////////////////////////////////////

static SkPaint* set_if_needed(SkLazyPaint* lazy, const SkPaint& orig) {
    return lazy->isValid() ? lazy->get() : lazy->set(orig);
}

/**
 *  If the paint has an imagefilter, but it can be simplified to just a colorfilter, return that
 *  colorfilter, else return nullptr.
 */
static SkColorFilter* image_to_color_filter(const SkPaint& paint) {
    SkImageFilter* imgf = paint.getImageFilter();
    if (!imgf) {
        return nullptr;
    }

    SkColorFilter* imgCF;
    if (!imgf->asAColorFilter(&imgCF)) {
        return nullptr;
    }

    SkColorFilter* paintCF = paint.getColorFilter();
    if (nullptr == paintCF) {
        // there is no existing paint colorfilter, so we can just return the imagefilter's
        return imgCF;
    }

    // The paint has both a colorfilter(paintCF) and an imagefilter-which-is-a-colorfilter(imgCF)
    // and we need to combine them into a single colorfilter.
    SkAutoTUnref<SkColorFilter> autoImgCF(imgCF);
    return SkColorFilter::CreateComposeFilter(imgCF, paintCF);
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
    AutoDrawLooper(SkCanvas* canvas, const SkSurfaceProps& props, const SkPaint& paint,
                   bool skipLayerForImageFilter = false,
                   const SkRect* rawBounds = nullptr) : fOrigPaint(paint) {
        fCanvas = canvas;
        fFilter = canvas->getDrawFilter();
        fPaint = &fOrigPaint;
        fSaveCount = canvas->getSaveCount();
        fTempLayerForImageFilter = false;
        fDone = false;

        SkColorFilter* simplifiedCF = image_to_color_filter(fOrigPaint);
        if (simplifiedCF) {
            SkPaint* paint = set_if_needed(&fLazyPaintInit, fOrigPaint);
            paint->setColorFilter(simplifiedCF)->unref();
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
            tmp.setImageFilter(fPaint->getImageFilter());
            tmp.setXfermode(fPaint->getXfermode());
            SkRect storage;
            if (rawBounds) {
                // Make rawBounds include all paint outsets except for those due to image filters.
                rawBounds = &apply_paint_to_bounds_sans_imagefilter(*fPaint, *rawBounds, &storage);
            }
            (void)canvas->internalSaveLayer(rawBounds, &tmp, SkCanvas::kARGB_ClipLayer_SaveFlag,
                                            SkCanvas::kFullLayer_SaveLayerStrategy);
            fTempLayerForImageFilter = true;
            // we remove the imagefilter/xfermode inside doNext()
        }

        if (SkDrawLooper* looper = paint.getLooper()) {
            void* buffer = fLooperContextAllocator.reserveT<SkDrawLooper::Context>(
                    looper->contextSize());
            fLooperContext = looper->createContext(canvas, buffer);
            fIsSimple = false;
        } else {
            fLooperContext = nullptr;
            // can we be marked as simple?
            fIsSimple = !fFilter && !fTempLayerForImageFilter;
        }

        uint32_t oldFlags = paint.getFlags();
        fNewPaintFlags = filter_paint_flags(props, oldFlags);
        if (fIsSimple && (fNewPaintFlags != oldFlags)) {
            SkPaint* paint = set_if_needed(&fLazyPaintInit, fOrigPaint);
            paint->setFlags(fNewPaintFlags);
            fPaint = paint;
            // if we're not simple, doNext() will take care of calling setFlags()
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
    SkLazyPaint     fLazyPaintInit; // base paint storage in case we need to modify it
    SkLazyPaint     fLazyPaintPerLooper;  // per-draw-looper storage, so the looper can modify it
    SkCanvas*       fCanvas;
    const SkPaint&  fOrigPaint;
    SkDrawFilter*   fFilter;
    const SkPaint*  fPaint;
    int             fSaveCount;
    uint32_t        fNewPaintFlags;
    bool            fTempLayerForImageFilter;
    bool            fDone;
    bool            fIsSimple;
    SkDrawLooper::Context* fLooperContext;
    SkSmallAllocator<1, 32> fLooperContextAllocator;

    bool doNext(SkDrawFilter::Type drawType);
};

bool AutoDrawLooper::doNext(SkDrawFilter::Type drawType) {
    fPaint = nullptr;
    SkASSERT(!fIsSimple);
    SkASSERT(fLooperContext || fFilter || fTempLayerForImageFilter);

    SkPaint* paint = fLazyPaintPerLooper.set(fLazyPaintInit.isValid() ?
                                             *fLazyPaintInit.get() : fOrigPaint);
    paint->setFlags(fNewPaintFlags);

    if (fTempLayerForImageFilter) {
        paint->setImageFilter(nullptr);
        paint->setXfermode(nullptr);
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

#define LOOPER_BEGIN_DRAWDEVICE(paint, type)                        \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, fProps, paint, true);              \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_BEGIN(paint, type, bounds)                           \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, fProps, paint, false, bounds);     \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, type, bounds, auxOpaque)  \
    this->predrawNotify(bounds, &paint, auxOpaque);                 \
    AutoDrawLooper  looper(this, fProps, paint, false, bounds);     \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_END    }

////////////////////////////////////////////////////////////////////////////

void SkCanvas::resetForNextPicture(const SkIRect& bounds) {
    this->restoreToCount(1);
    fCachedLocalClipBounds.setEmpty();
    fCachedLocalClipBoundsDirty = true;
    fClipStack->reset();
    fMCRec->reset(bounds);

    // We're peering through a lot of structs here.  Only at this scope do we
    // know that the device is an SkBitmapDevice (really an SkNoPixelsBitmapDevice).
    static_cast<SkBitmapDevice*>(fMCRec->fLayer->fDevice)->setNewSize(bounds.size());
}

SkBaseDevice* SkCanvas::init(SkBaseDevice* device, InitFlags flags) {
    if (device && device->forceConservativeRasterClip()) {
        flags = InitFlags(flags | kConservativeRasterClip_InitFlag);
    }
    // Since init() is only called once by our constructors, it is safe to perform this
    // const-cast.
    *const_cast<bool*>(&fConservativeRasterClip) = SkToBool(flags & kConservativeRasterClip_InitFlag);

    fCachedLocalClipBounds.setEmpty();
    fCachedLocalClipBoundsDirty = true;
    fAllowSoftClip = true;
    fAllowSimplifyClip = false;
    fDeviceCMDirty = true;
    fSaveCount = 1;
    fMetaData = nullptr;

    fClipStack.reset(new SkClipStack);

    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec(fConservativeRasterClip);

    SkASSERT(sizeof(DeviceCM) <= sizeof(fDeviceCMStorage));
    fMCRec->fLayer = (DeviceCM*)fDeviceCMStorage;
    new (fDeviceCMStorage) DeviceCM(nullptr, nullptr, nullptr, fConservativeRasterClip, false);

    fMCRec->fTopLayer = fMCRec->fLayer;

    fSurfaceBase = nullptr;

    if (device) {
        // The root device and the canvas should always have the same pixel geometry
        SkASSERT(fProps.pixelGeometry() == device->surfaceProps().pixelGeometry());
        device->onAttachToCanvas(this);
        fMCRec->fLayer->fDevice = SkRef(device);
        fMCRec->fRasterClip.setRect(device->getGlobalBounds());
    }
    return device;
}

SkCanvas::SkCanvas()
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
    , fConservativeRasterClip(false)
{
    inc_canvas();

    this->init(nullptr, kDefault_InitFlags);
}

static SkBitmap make_nopixels(int width, int height) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeUnknown(width, height));
    return bitmap;
}

class SkNoPixelsBitmapDevice : public SkBitmapDevice {
public:
    SkNoPixelsBitmapDevice(const SkIRect& bounds, const SkSurfaceProps& surfaceProps)
        : INHERITED(make_nopixels(bounds.width(), bounds.height()), surfaceProps)
    {
        this->setOrigin(bounds.x(), bounds.y());
    }

private:

    typedef SkBitmapDevice INHERITED;
};

SkCanvas::SkCanvas(int width, int height, const SkSurfaceProps* props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfacePropsCopyOrDefault(props))
    , fConservativeRasterClip(false)
{
    inc_canvas();

    this->init(new SkNoPixelsBitmapDevice(SkIRect::MakeWH(width, height), fProps),
               kDefault_InitFlags)->unref();
}

SkCanvas::SkCanvas(const SkIRect& bounds, InitFlags flags)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
    , fConservativeRasterClip(false)
{
    inc_canvas();

    this->init(new SkNoPixelsBitmapDevice(bounds, fProps), flags)->unref();
}

SkCanvas::SkCanvas(SkBaseDevice* device)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(device->surfaceProps())
    , fConservativeRasterClip(false)
{
    inc_canvas();

    this->init(device, kDefault_InitFlags);
}

SkCanvas::SkCanvas(SkBaseDevice* device, InitFlags flags)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(device->surfaceProps())
    , fConservativeRasterClip(false)
{
    inc_canvas();

    this->init(device, flags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(props)
    , fConservativeRasterClip(false)
{
    inc_canvas();

    SkAutoTUnref<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps));
    this->init(device, kDefault_InitFlags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
    , fConservativeRasterClip(false)
{
    inc_canvas();

    SkAutoTUnref<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps));
    this->init(device, kDefault_InitFlags);
}

SkCanvas::~SkCanvas() {
    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last

    this->internalRestore();    // restore the last, since we're going away

    delete fMetaData;

    dec_canvas();
}

SkDrawFilter* SkCanvas::getDrawFilter() const {
    return fMCRec->fFilter;
}

SkDrawFilter* SkCanvas::setDrawFilter(SkDrawFilter* filter) {
    this->checkForDeferredSave();
    SkRefCnt_SafeAssign(fMCRec->fFilter, filter);
    return filter;
}

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
    SkBaseDevice* device = this->getDevice();
    if (device) {
        device->flush();
    }
}

SkISize SkCanvas::getTopLayerSize() const {
    SkBaseDevice* d = this->getTopDevice();
    return d ? SkISize::Make(d->width(), d->height()) : SkISize::Make(0, 0);
}

SkIPoint SkCanvas::getTopLayerOrigin() const {
    SkBaseDevice* d = this->getTopDevice();
    return d ? d->getOrigin() : SkIPoint::Make(0, 0);
}

SkISize SkCanvas::getBaseLayerSize() const {
    SkBaseDevice* d = this->getDevice();
    return d ? SkISize::Make(d->width(), d->height()) : SkISize::Make(0, 0);
}

SkBaseDevice* SkCanvas::getDevice() const {
    // return root device
    MCRec* rec = (MCRec*) fMCStack.front();
    SkASSERT(rec && rec->fLayer);
    return rec->fLayer->fDevice;
}

SkBaseDevice* SkCanvas::getTopDevice(bool updateMatrixClip) const {
    if (updateMatrixClip) {
        const_cast<SkCanvas*>(this)->updateDeviceCMCache();
    }
    return fMCRec->fTopLayer->fDevice;
}

bool SkCanvas::readPixels(SkBitmap* bitmap, int x, int y) {
    if (kUnknown_SkColorType == bitmap->colorType() || bitmap->getTexture()) {
        return false;
    }

    bool weAllocated = false;
    if (nullptr == bitmap->pixelRef()) {
        if (!bitmap->tryAllocPixels()) {
            return false;
        }
        weAllocated = true;
    }

    SkAutoPixmapUnlock unlocker;
    if (bitmap->requestLock(&unlocker)) {
        const SkPixmap& pm = unlocker.pixmap();
        if (this->readPixels(pm.info(), pm.writable_addr(), pm.rowBytes(), x, y)) {
            return true;
        }
    }

    if (weAllocated) {
        bitmap->setPixelRef(nullptr);
    }
    return false;
}

bool SkCanvas::readPixels(const SkIRect& srcRect, SkBitmap* bitmap) {
    SkIRect r = srcRect;
    const SkISize size = this->getBaseLayerSize();
    if (!r.intersect(0, 0, size.width(), size.height())) {
        bitmap->reset();
        return false;
    }

    if (!bitmap->tryAllocN32Pixels(r.width(), r.height())) {
        // bitmap will already be reset.
        return false;
    }
    if (!this->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), r.x(), r.y())) {
        bitmap->reset();
        return false;
    }
    return true;
}

bool SkCanvas::readPixels(const SkImageInfo& dstInfo, void* dstP, size_t rowBytes, int x, int y) {
    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }
    const SkISize size = this->getBaseLayerSize();

    SkReadPixelsRec rec(dstInfo, dstP, rowBytes, x, y);
    if (!rec.trim(size.width(), size.height())) {
        return false;
    }

    // The device can assert that the requested area is always contained in its bounds
    return device->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY);
}

bool SkCanvas::writePixels(const SkBitmap& bitmap, int x, int y) {
    if (bitmap.getTexture()) {
        return false;
    }

    SkAutoPixmapUnlock unlocker;
    if (bitmap.requestLock(&unlocker)) {
        const SkPixmap& pm = unlocker.pixmap();
        return this->writePixels(pm.info(), pm.addr(), pm.rowBytes(), x, y);
    }
    return false;
}

bool SkCanvas::writePixels(const SkImageInfo& origInfo, const void* pixels, size_t rowBytes,
                           int x, int y) {
    switch (origInfo.colorType()) {
        case kUnknown_SkColorType:
        case kIndex_8_SkColorType:
            return false;
        default:
            break;
    }
    if (nullptr == pixels || rowBytes < origInfo.minRowBytes()) {
        return false;
    }

    const SkISize size = this->getBaseLayerSize();
    SkIRect target = SkIRect::MakeXYWH(x, y, origInfo.width(), origInfo.height());
    if (!target.intersect(0, 0, size.width(), size.height())) {
        return false;
    }

    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }

    // the intersect may have shrunk info's logical size
    const SkImageInfo info = origInfo.makeWH(target.width(), target.height());

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    pixels = ((const char*)pixels - y * rowBytes - x * info.bytesPerPixel());

    // Tell our owning surface to bump its generation ID
    const bool completeOverwrite = info.dimensions() == size;
    this->predrawNotify(completeOverwrite);

    // The device can assert that the requested area is always contained in its bounds
    return device->writePixels(info, pixels, rowBytes, target.x(), target.y());
}

SkCanvas* SkCanvas::canvasForDrawIter() {
    return this;
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::updateDeviceCMCache() {
    if (fDeviceCMDirty) {
        const SkMatrix& totalMatrix = this->getTotalMatrix();
        const SkRasterClip& totalClip = fMCRec->fRasterClip;
        DeviceCM*       layer = fMCRec->fTopLayer;

        if (nullptr == layer->fNext) {   // only one layer
            layer->updateMC(totalMatrix, totalClip, *fClipStack, nullptr);
        } else {
            SkRasterClip clip(totalClip);
            do {
                layer->updateMC(totalMatrix, clip, *fClipStack, &clip);
            } while ((layer = layer->fNext) != nullptr);
        }
        fDeviceCMDirty = false;
    }
}

///////////////////////////////////////////////////////////////////////////////

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

    fClipStack->save();
}

static bool bounds_affects_clip(SkCanvas::SaveFlags flags) {
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
    return (flags & SkCanvas::kClipToLayer_SaveFlag) != 0;
#else
    return true;
#endif
}

bool SkCanvas::clipRectBounds(const SkRect* bounds, SaveFlags flags,
                              SkIRect* intersection, const SkImageFilter* imageFilter) {
    SkIRect clipBounds;
    if (!this->getClipDeviceBounds(&clipBounds)) {
        return false;
    }

    const SkMatrix& ctm = fMCRec->fMatrix;  // this->getTotalMatrix()

// This is a temporary hack, until individual filters can do their own
// bloating, when this will be removed.
#ifdef SK_SUPPORT_SRC_BOUNDS_BLOAT_FOR_IMAGEFILTERS
    SkRect storage;
#endif
    if (imageFilter) {
        imageFilter->filterBounds(clipBounds, ctm, &clipBounds);
#ifdef SK_SUPPORT_SRC_BOUNDS_BLOAT_FOR_IMAGEFILTERS
        if (bounds && imageFilter->canComputeFastBounds()) {
            imageFilter->computeFastBounds(*bounds, &storage);
            bounds = &storage;
        } else {
            bounds = nullptr;
        }
#endif
    }
    SkIRect ir;
    if (bounds) {
        SkRect r;

        ctm.mapRect(&r, *bounds);
        r.roundOut(&ir);
        // early exit if the layer's bounds are clipped out
        if (!ir.intersect(clipBounds)) {
            if (bounds_affects_clip(flags)) {
                fCachedLocalClipBoundsDirty = true;
                fMCRec->fRasterClip.setEmpty();
            }
            return false;
        }
    } else {    // no user bounds, so just use the clip
        ir = clipBounds;
    }
    SkASSERT(!ir.isEmpty());

    if (bounds_affects_clip(flags)) {
        // Simplify the current clips since they will be applied properly during restore()
        fCachedLocalClipBoundsDirty = true;
        fClipStack->clipDevRect(ir, SkRegion::kReplace_Op);
        fMCRec->fRasterClip.setRect(ir);
    }

    if (intersection) {
        *intersection = ir;
    }
    return true;
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint) {
    if (gIgnoreSaveLayerBounds) {
        bounds = nullptr;
    }
    SaveLayerStrategy strategy = this->willSaveLayer(bounds, paint, kARGB_ClipLayer_SaveFlag);
    fSaveCount += 1;
    this->internalSaveLayer(bounds, paint, kARGB_ClipLayer_SaveFlag, strategy);
    return this->getSaveCount() - 1;
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags) {
    if (gIgnoreSaveLayerBounds) {
        bounds = nullptr;
    }
    SaveLayerStrategy strategy = this->willSaveLayer(bounds, paint, flags);
    fSaveCount += 1;
    this->internalSaveLayer(bounds, paint, flags, strategy);
    return this->getSaveCount() - 1;
}

void SkCanvas::internalSaveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags,
                                 SaveLayerStrategy strategy) {
#ifndef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
    flags |= kClipToLayer_SaveFlag;
#endif

    // do this before we create the layer. We don't call the public save() since
    // that would invoke a possibly overridden virtual
    this->internalSave();

    fDeviceCMDirty = true;

    SkIRect ir;
    if (!this->clipRectBounds(bounds, flags, &ir, paint ? paint->getImageFilter() : nullptr)) {
        return;
    }

    // FIXME: do willSaveLayer() overriders returning kNoLayer_SaveLayerStrategy really care about
    // the clipRectBounds() call above?
    if (kNoLayer_SaveLayerStrategy == strategy) {
        return;
    }

    bool isOpaque = !SkToBool(flags & kHasAlphaLayer_SaveFlag);
    SkPixelGeometry geo = fProps.pixelGeometry();
    if (paint) {
        // TODO: perhaps add a query to filters so we might preserve opaqueness...
        if (paint->getImageFilter() || paint->getColorFilter()) {
            isOpaque = false;
            geo = kUnknown_SkPixelGeometry;
        }
    }
    SkImageInfo info = SkImageInfo::MakeN32(ir.width(), ir.height(),
                        isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);

    SkBaseDevice* device = this->getTopDevice();
    if (nullptr == device) {
        SkDebugf("Unable to find device for layer.");
        return;
    }

    bool forceSpriteOnRestore = false;
    {
        const SkBaseDevice::TileUsage usage = SkBaseDevice::kNever_TileUsage;
        const SkBaseDevice::CreateInfo createInfo = SkBaseDevice::CreateInfo(info, usage, geo);
        SkBaseDevice* newDev = device->onCreateDevice(createInfo, paint);
        if (nullptr == newDev) {
            // If onCreateDevice didn't succeed, try raster (e.g. PDF couldn't handle the paint)
            const SkSurfaceProps surfaceProps(fProps.flags(), createInfo.fPixelGeometry);
            newDev = SkBitmapDevice::Create(createInfo.fInfo, surfaceProps);
            if (nullptr == newDev) {
                SkErrorInternals::SetError(kInternalError_SkError,
                                           "Unable to create device for layer.");
                return;
            }
            forceSpriteOnRestore = true;
        }
        device = newDev;
    }

    device->setOrigin(ir.fLeft, ir.fTop);
    DeviceCM* layer =
            new DeviceCM(device, paint, this, fConservativeRasterClip, forceSpriteOnRestore);
    device->unref();

    layer->fNext = fMCRec->fTopLayer;
    fMCRec->fLayer = layer;
    fMCRec->fTopLayer = layer;    // this field is NOT an owner of layer
}

int SkCanvas::saveLayerAlpha(const SkRect* bounds, U8CPU alpha) {
    return this->saveLayerAlpha(bounds, alpha, kARGB_ClipLayer_SaveFlag);
}

int SkCanvas::saveLayerAlpha(const SkRect* bounds, U8CPU alpha,
                             SaveFlags flags) {
    if (0xFF == alpha) {
        return this->saveLayer(bounds, nullptr, flags);
    } else {
        SkPaint tmpPaint;
        tmpPaint.setAlpha(alpha);
        return this->saveLayer(bounds, &tmpPaint, flags);
    }
}

void SkCanvas::internalRestore() {
    SkASSERT(fMCStack.count() != 0);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    fClipStack->restore();

    // reserve our layer (if any)
    DeviceCM* layer = fMCRec->fLayer;   // may be null
    // now detach it from fMCRec so we can pop(). Gets freed after its drawn
    fMCRec->fLayer = nullptr;

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*)fMCStack.back();

    /*  Time to draw the layer's offscreen. We can't call the public drawSprite,
        since if we're being recorded, we don't want to record this (the
        recorder will have already recorded the restore).
    */
    if (layer) {
        if (layer->fNext) {
            const SkIPoint& origin = layer->fDevice->getOrigin();
            this->internalDrawDevice(layer->fDevice, origin.x(), origin.y(),
                                     layer->fPaint, layer->fDeviceIsBitmapDevice);
            // reset this, since internalDrawDevice will have set it to true
            fDeviceCMDirty = true;
            delete layer;
        } else {
            // we're at the root
            SkASSERT(layer == (void*)fDeviceCMStorage);
            layer->~DeviceCM();
        }
    }
}

SkSurface* SkCanvas::newSurface(const SkImageInfo& info, const SkSurfaceProps* props) {
    if (nullptr == props) {
        props = &fProps;
    }
    return this->onNewSurface(info, *props);
}

SkSurface* SkCanvas::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    SkBaseDevice* dev = this->getDevice();
    return dev ? dev->newSurface(info, props) : nullptr;
}

SkImageInfo SkCanvas::imageInfo() const {
    SkBaseDevice* dev = this->getDevice();
    if (dev) {
        return dev->imageInfo();
    } else {
        return SkImageInfo::MakeUnknown(0, 0);
    }
}

const void* SkCanvas::peekPixels(SkImageInfo* info, size_t* rowBytes) {
    SkPixmap pmap;
    if (!this->onPeekPixels(&pmap)) {
        return nullptr;
    }
    if (info) {
        *info = pmap.info();
    }
    if (rowBytes) {
        *rowBytes = pmap.rowBytes();
    }
    return pmap.addr();
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
        *origin = this->getTopDevice(false)->getOrigin();
    }
    return pmap.writable_addr();
}

bool SkCanvas::onAccessTopLayerPixels(SkPixmap* pmap) {
    SkBaseDevice* dev = this->getTopDevice();
    return dev && dev->accessPixels(pmap);
}

SkAutoROCanvasPixels::SkAutoROCanvasPixels(SkCanvas* canvas) {
    fAddr = canvas->peekPixels(&fInfo, &fRowBytes);
    if (nullptr == fAddr) {
        fInfo = canvas->imageInfo();
        if (kUnknown_SkColorType == fInfo.colorType() || !fBitmap.tryAllocPixels(fInfo)) {
            return; // failure, fAddr is nullptr
        }
        if (!canvas->readPixels(&fBitmap, 0, 0)) {
            return; // failure, fAddr is nullptr
        }
        fAddr = fBitmap.getPixels();
        fRowBytes = fBitmap.rowBytes();
    }
    SkASSERT(fAddr);    // success
}

bool SkAutoROCanvasPixels::asROBitmap(SkBitmap* bitmap) const {
    if (fAddr) {
        return bitmap->installPixels(fInfo, const_cast<void*>(fAddr), fRowBytes);
    } else {
        bitmap->reset();
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::internalDrawDevice(SkBaseDevice* srcDev, int x, int y,
                                  const SkPaint* paint, bool deviceIsBitmapDevice) {
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
        if (filter && !dstDev->canHandleImageFilter(filter)) {
            SkImageFilter::DeviceProxy proxy(dstDev);
            SkBitmap dst;
            SkIPoint offset = SkIPoint::Make(0, 0);
            const SkBitmap& src = srcDev->accessBitmap(false);
            SkMatrix matrix = *iter.fMatrix;
            matrix.postTranslate(SkIntToScalar(-pos.x()), SkIntToScalar(-pos.y()));
            SkIRect clipBounds = SkIRect::MakeWH(srcDev->width(), srcDev->height());
            SkAutoTUnref<SkImageFilter::Cache> cache(dstDev->getImageFilterCache());
            SkImageFilter::Context ctx(matrix, clipBounds, cache.get(),
                                       SkImageFilter::kApprox_SizeConstraint);
            if (filter->filterImage(&proxy, src, ctx, &dst, &offset)) {
                SkPaint tmpUnfiltered(*paint);
                tmpUnfiltered.setImageFilter(nullptr);
                dstDev->drawSprite(iter, dst, pos.x() + offset.x(), pos.y() + offset.y(),
                                   tmpUnfiltered);
            }
        } else if (deviceIsBitmapDevice) {
            const SkBitmap& src = static_cast<SkBitmapDevice*>(srcDev)->fBitmap;
            dstDev->drawSprite(iter, src, pos.x(), pos.y(), *paint);
        } else {
            dstDev->drawDevice(iter, srcDev, pos.x(), pos.y(), *paint);
        }
    }
    LOOPER_END
}

void SkCanvas::onDrawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint* paint) {
    if (gTreatSpriteAsBitmap) {
        this->save();
        this->resetMatrix();
        this->drawBitmap(bitmap, SkIntToScalar(x), SkIntToScalar(y), paint);
        this->restore();
        return;
    }

    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawSprite()");
    if (bitmap.drawsNothing()) {
        return;
    }
    SkDEBUGCODE(bitmap.validate();)

    SkPaint tmp;
    if (nullptr == paint) {
        paint = &tmp;
    }

    LOOPER_BEGIN_DRAWDEVICE(*paint, SkDrawFilter::kBitmap_Type)

    while (iter.next()) {
        paint = &looper.paint();
        SkImageFilter* filter = paint->getImageFilter();
        SkIPoint pos = { x - iter.getX(), y - iter.getY() };
        if (filter && !iter.fDevice->canHandleImageFilter(filter)) {
            SkImageFilter::DeviceProxy proxy(iter.fDevice);
            SkBitmap dst;
            SkIPoint offset = SkIPoint::Make(0, 0);
            SkMatrix matrix = *iter.fMatrix;
            matrix.postTranslate(SkIntToScalar(-pos.x()), SkIntToScalar(-pos.y()));
            const SkIRect clipBounds = bitmap.bounds();
            SkAutoTUnref<SkImageFilter::Cache> cache(iter.fDevice->getImageFilterCache());
            SkImageFilter::Context ctx(matrix, clipBounds, cache.get(),
                                       SkImageFilter::kApprox_SizeConstraint);
            if (filter->filterImage(&proxy, bitmap, ctx, &dst, &offset)) {
                SkPaint tmpUnfiltered(*paint);
                tmpUnfiltered.setImageFilter(nullptr);
                iter.fDevice->drawSprite(iter, dst, pos.x() + offset.x(), pos.y() + offset.y(),
                                         tmpUnfiltered);
            }
        } else {
            iter.fDevice->drawSprite(iter, bitmap, pos.x(), pos.y(), *paint);
        }
    }
    LOOPER_END
}

/////////////////////////////////////////////////////////////////////////////
void SkCanvas::translate(SkScalar dx, SkScalar dy) {
    SkMatrix m;
    m.setTranslate(dx, dy);
    this->concat(m);
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
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    fMCRec->fMatrix.preConcat(matrix);

    this->didConcat(matrix);
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    this->checkForDeferredSave();
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    fMCRec->fMatrix = matrix;
    this->didSetMatrix(matrix);
}

void SkCanvas::resetMatrix() {
    SkMatrix matrix;

    matrix.reset();
    this->setMatrix(matrix);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    this->onClipRect(rect, op, edgeStyle);
}

void SkCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
#ifdef SK_ENABLE_CLIP_QUICKREJECT
    if (SkRegion::kIntersect_Op == op) {
        if (fMCRec->fRasterClip.isEmpty()) {
            return false;
        }

        if (this->quickReject(rect)) {
            fDeviceCMDirty = true;
            fCachedLocalClipBoundsDirty = true;

            fClipStack->clipEmpty();
            return fMCRec->fRasterClip.setEmpty();
        }
    }
#endif

    if (!fAllowSoftClip) {
        edgeStyle = kHard_ClipEdgeStyle;
    }

    const bool rectStaysRect = fMCRec->fMatrix.rectStaysRect();
    SkRect devR;
    if (rectStaysRect) {
        fMCRec->fMatrix.mapRect(&devR, rect);
    }

    // Check if we can quick-accept the clip call (and do nothing)
    //
    // TODO: investigate if a (conservative) version of this could be done in ::clipRect,
    //       so that subclasses (like PictureRecording) didn't see unnecessary clips, which in turn
    //       might allow lazy save/restores to eliminate entire save/restore blocks.
    //
    if (SkRegion::kIntersect_Op == op &&
        kHard_ClipEdgeStyle == edgeStyle
        && rectStaysRect)
    {
        if (devR.round().contains(fMCRec->fRasterClip.getBounds())) {
#if 0
            SkDebugf("------- ignored clipRect [%g %g %g %g]\n",
                     rect.left(), rect.top(), rect.right(), rect.bottom());
#endif
            return;
        }
    }

    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    if (rectStaysRect) {
        const bool isAA = kSoft_ClipEdgeStyle == edgeStyle;
        fClipStack->clipDevRect(devR, op, isAA);
        fMCRec->fRasterClip.op(devR, this->getBaseLayerSize(), op, isAA);
    } else {
        // since we're rotated or some such thing, we convert the rect to a path
        // and clip against that, since it can handle any matrix. However, to
        // avoid recursion in the case where we are subclassed (e.g. Pictures)
        // we explicitly call "our" version of clipPath.
        SkPath  path;

        path.addRect(rect);
        this->SkCanvas::onClipPath(path, op, edgeStyle);
    }
}

static void rasterclip_path(SkRasterClip* rc, const SkCanvas* canvas, const SkPath& devPath,
                            SkRegion::Op op, bool doAA) {
    rc->op(devPath, canvas->getBaseLayerSize(), op, doAA);
}

void SkCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    if (rrect.isRect()) {
        this->onClipRect(rrect.getBounds(), op, edgeStyle);
    } else {
        this->onClipRRect(rrect, op, edgeStyle);
    }
}

void SkCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    SkRRect transformedRRect;
    if (rrect.transform(fMCRec->fMatrix, &transformedRRect)) {
        AutoValidateClip avc(this);

        fDeviceCMDirty = true;
        fCachedLocalClipBoundsDirty = true;
        if (!fAllowSoftClip) {
            edgeStyle = kHard_ClipEdgeStyle;
        }

        fClipStack->clipDevRRect(transformedRRect, op, kSoft_ClipEdgeStyle == edgeStyle);

        fMCRec->fRasterClip.op(transformedRRect, this->getBaseLayerSize(), op,
                               kSoft_ClipEdgeStyle == edgeStyle);
        return;
    }

    SkPath path;
    path.addRRect(rrect);
    // call the non-virtual version
    this->SkCanvas::onClipPath(path, op, edgeStyle);
}

void SkCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
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

void SkCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
#ifdef SK_ENABLE_CLIP_QUICKREJECT
    if (SkRegion::kIntersect_Op == op && !path.isInverseFillType()) {
        if (fMCRec->fRasterClip.isEmpty()) {
            return false;
        }

        if (this->quickReject(path.getBounds())) {
            fDeviceCMDirty = true;
            fCachedLocalClipBoundsDirty = true;

            fClipStack->clipEmpty();
            return fMCRec->fRasterClip.setEmpty();
        }
    }
#endif

    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    if (!fAllowSoftClip) {
        edgeStyle = kHard_ClipEdgeStyle;
    }

    SkPath devPath;
    path.transform(fMCRec->fMatrix, &devPath);

    // Check if the transfomation, or the original path itself
    // made us empty. Note this can also happen if we contained NaN
    // values. computing the bounds detects this, and will set our
    // bounds to empty if that is the case. (see SkRect::set(pts, count))
    if (devPath.getBounds().isEmpty()) {
        // resetting the path will remove any NaN or other wanky values
        // that might upset our scan converter.
        devPath.reset();
    }

    // if we called path.swap() we could avoid a deep copy of this path
    fClipStack->clipDevPath(devPath, op, kSoft_ClipEdgeStyle == edgeStyle);

    if (fAllowSimplifyClip) {
        bool clipIsAA = getClipStack()->asPath(&devPath);
        if (clipIsAA) {
            edgeStyle = kSoft_ClipEdgeStyle;
        }

        op = SkRegion::kReplace_Op;
    }

    rasterclip_path(&fMCRec->fRasterClip, this, devPath, op, edgeStyle);
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkRegion::Op op) {
    this->checkForDeferredSave();
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkRegion::Op op) {
    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    // todo: signal fClipStack that we have a region, and therefore (I guess)
    // we have to ignore it, and use the region directly?
    fClipStack->clipDevRect(rgn.getBounds(), op);

    fMCRec->fRasterClip.op(rgn, op);
}

#ifdef SK_DEBUG
void SkCanvas::validateClip() const {
    // construct clipRgn from the clipstack
    const SkBaseDevice* device = this->getDevice();
    if (!device) {
        SkASSERT(this->isClipEmpty());
        return;
    }

    SkIRect ir;
    ir.set(0, 0, device->width(), device->height());
    SkRasterClip tmpClip(ir, fConservativeRasterClip);

    SkClipStack::B2TIter                iter(*fClipStack);
    const SkClipStack::Element* element;
    while ((element = iter.next()) != nullptr) {
        switch (element->getType()) {
            case SkClipStack::Element::kRect_Type:
                element->getRect().round(&ir);
                tmpClip.op(ir, element->getOp());
                break;
            case SkClipStack::Element::kEmpty_Type:
                tmpClip.setEmpty();
                break;
            default: {
                SkPath path;
                element->asPath(&path);
                rasterclip_path(&tmpClip, this, path, element->getOp(), element->isAA());
                break;
            }
        }
    }
}
#endif

void SkCanvas::replayClips(ClipVisitor* visitor) const {
    SkClipStack::B2TIter                iter(*fClipStack);
    const SkClipStack::Element*         element;

    while ((element = iter.next()) != nullptr) {
        element->replay(visitor);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkCanvas::isClipEmpty() const {
    return fMCRec->fRasterClip.isEmpty();
}

bool SkCanvas::isClipRect() const {
    return fMCRec->fRasterClip.isRect();
}

bool SkCanvas::quickReject(const SkRect& rect) const {
    if (!rect.isFinite())
        return true;

    if (fMCRec->fRasterClip.isEmpty()) {
        return true;
    }

    if (fMCRec->fMatrix.hasPerspective()) {
        SkRect dst;
        fMCRec->fMatrix.mapRect(&dst, rect);
        return !SkIRect::Intersects(dst.roundOut(), fMCRec->fRasterClip.getBounds());
    } else {
        const SkRect& clipR = this->getLocalClipBounds();

        // for speed, do the most likely reject compares first
        // TODO: should we use | instead, or compare all 4 at once?
        if (rect.fTop >= clipR.fBottom || rect.fBottom <= clipR.fTop) {
            return true;
        }
        if (rect.fLeft >= clipR.fRight || rect.fRight <= clipR.fLeft) {
            return true;
        }
        return false;
    }
}

bool SkCanvas::quickReject(const SkPath& path) const {
    return path.isEmpty() || this->quickReject(path.getBounds());
}

bool SkCanvas::getClipBounds(SkRect* bounds) const {
    SkIRect ibounds;
    if (!this->getClipDeviceBounds(&ibounds)) {
        return false;
    }

    SkMatrix inverse;
    // if we can't invert the CTM, we can't return local clip bounds
    if (!fMCRec->fMatrix.invert(&inverse)) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    if (bounds) {
        SkRect r;
        // adjust it outwards in case we are antialiasing
        const int inset = 1;

        r.iset(ibounds.fLeft - inset, ibounds.fTop - inset,
               ibounds.fRight + inset, ibounds.fBottom + inset);
        inverse.mapRect(bounds, r);
    }
    return true;
}

bool SkCanvas::getClipDeviceBounds(SkIRect* bounds) const {
    const SkRasterClip& clip = fMCRec->fRasterClip;
    if (clip.isEmpty()) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    if (bounds) {
        *bounds = clip.getBounds();
    }
    return true;
}

const SkMatrix& SkCanvas::getTotalMatrix() const {
    return fMCRec->fMatrix;
}

const SkRegion& SkCanvas::internal_private_getTotalClip() const {
    return fMCRec->fRasterClip.forceGetBW();
}

GrRenderTarget* SkCanvas::internal_private_accessTopLayerRenderTarget() {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessRenderTarget() : nullptr;
}

GrContext* SkCanvas::getGrContext() {
#if SK_SUPPORT_GPU
    SkBaseDevice* device = this->getTopDevice();
    if (device) {
        GrRenderTarget* renderTarget = device->accessRenderTarget();
        if (renderTarget) {
            return renderTarget->getContext();
        }
    }
#endif

    return nullptr;

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
    // be able to assert...
    // SkASSERT(outer.contains(inner));
    //
    // For now at least check for containment of bounds
    SkASSERT(outer.getBounds().contains(inner.getBounds()));

    this->onDrawDRRect(outer, inner, paint);
}

// These need to stop being virtual -- clients need to override the onDraw... versions

void SkCanvas::drawPaint(const SkPaint& paint) {
    this->onDrawPaint(paint);
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    this->onDrawRect(r, paint);
}

void SkCanvas::drawOval(const SkRect& r, const SkPaint& paint) {
    this->onDrawOval(r, paint);
}

void SkCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->onDrawRRect(rrect, paint);
}

void SkCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint) {
    this->onDrawPoints(mode, count, pts, paint);
}

void SkCanvas::drawVertices(VertexMode vmode, int vertexCount, const SkPoint vertices[],
                            const SkPoint texs[], const SkColor colors[], SkXfermode* xmode,
                            const uint16_t indices[], int indexCount, const SkPaint& paint) {
    this->onDrawVertices(vmode, vertexCount, vertices, texs, colors, xmode,
                         indices, indexCount, paint);
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    this->onDrawPath(path, paint);
}

void SkCanvas::drawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    this->onDrawImage(image, x, y, paint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& src, const SkRect& dst,
                             const SkPaint* paint, SrcRectConstraint constraint) {
    if (dst.isEmpty() || src.isEmpty()) {
        return;
    }
    this->onDrawImageRect(image, &src, dst, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkIRect& isrc, const SkRect& dst,
                             const SkPaint* paint, SrcRectConstraint constraint) {
    this->drawImageRect(image, SkRect::Make(isrc), dst, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& dst, const SkPaint* paint,
                             SrcRectConstraint constraint) {
    this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst, paint,
                        constraint);
}

void SkCanvas::drawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                             const SkPaint* paint) {
    if (dst.isEmpty()) {
        return;
    }
    if (!SkNinePatchIter::Valid(image->width(), image->height(), center)) {
        this->drawImageRect(image, dst, paint);
    }
    this->onDrawImageNine(image, center, dst, paint);
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
    if (!SkNinePatchIter::Valid(bitmap.width(), bitmap.height(), center)) {
        this->drawBitmapRect(bitmap, dst, paint);
    }
    this->onDrawBitmapNine(bitmap, center, dst, paint);
}

void SkCanvas::drawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }
    this->onDrawSprite(bitmap, left, top, paint);
}

void SkCanvas::drawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                         const SkColor colors[], int count, SkXfermode::Mode mode,
                         const SkRect* cull, const SkPaint* paint) {
    if (count <= 0) {
        return;
    }
    SkASSERT(atlas);
    SkASSERT(xform);
    SkASSERT(tex);
    this->onDrawAtlas(atlas, xform, tex, colors, count, mode, cull, paint);
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
        iter.fDevice->drawPaint(iter, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) {
    TRACE_EVENT1("disabled-by-default-skia", "SkCanvas::drawPoints()", "count", static_cast<uint64_t>(count));
    if ((long)count <= 0) {
        return;
    }

    SkRect r, storage;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        // special-case 2 points (common for drawing a single line)
        if (2 == count) {
            r.set(pts[0], pts[1]);
        } else {
            r.set(pts, SkToInt(count));
        }
        if (this->quickReject(paint.computeFastStrokeBounds(r, &storage))) {
            return;
        }
        bounds = &r;
    }

    SkASSERT(pts != nullptr);

    LOOPER_BEGIN(paint, SkDrawFilter::kPoint_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPoints(iter, mode, count, pts, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawRect(const SkRect& r, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawRect()");
    SkRect storage;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        // Skia will draw an inverted rect, because it explicitly "sorts" it downstream.
        // To prevent accidental rejecting at this stage, we have to sort it before we check.
        SkRect tmp(r);
        tmp.sort();

        if (this->quickReject(paint.computeFastBounds(tmp, &storage))) {
            return;
        }
        bounds = &r;
    }

    LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(paint, SkDrawFilter::kRect_Type, bounds, false)

    while (iter.next()) {
        iter.fDevice->drawRect(iter, r, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawOval()");
    SkRect storage;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        if (this->quickReject(paint.computeFastBounds(oval, &storage))) {
            return;
        }
        bounds = &oval;
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kOval_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawOval(iter, oval, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawRRect()");
    SkRect storage;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        if (this->quickReject(paint.computeFastBounds(rrect.getBounds(), &storage))) {
            return;
        }
        bounds = &rrect.getBounds();
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

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawRRect(iter, rrect, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                            const SkPaint& paint) {
    SkRect storage;
    const SkRect* bounds = nullptr;
    if (paint.canComputeFastBounds()) {
        if (this->quickReject(paint.computeFastBounds(outer.getBounds(), &storage))) {
            return;
        }
        bounds = &outer.getBounds();
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawDRRect(iter, outer, inner, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPath()");
    if (!path.isFinite()) {
        return;
    }

    SkRect storage;
    const SkRect* bounds = nullptr;
    if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
        const SkRect& pathBounds = path.getBounds();
        if (this->quickReject(paint.computeFastBounds(pathBounds, &storage))) {
            return;
        }
        bounds = &pathBounds;
    }

    const SkRect& r = path.getBounds();
    if (r.width() <= 0 && r.height() <= 0) {
        if (path.isInverseFillType()) {
            this->internalDrawPaint(paint);
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPath(iter, path, looper.paint());
    }

    LOOPER_END
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
    
    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, &bounds)
    
    while (iter.next()) {
        iter.fDevice->drawImage(iter, image, x, y, looper.paint());
    }
    
    LOOPER_END
}

void SkCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                               const SkPaint* paint, SrcRectConstraint constraint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawImageRect()");
    SkRect storage;
    const SkRect* bounds = &dst;
    if (nullptr == paint || paint->canComputeFastBounds()) {
        storage = dst;
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
    
    LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(*paint, SkDrawFilter::kBitmap_Type, bounds,
                                          image->isOpaque())
    
    while (iter.next()) {
        iter.fDevice->drawImageRect(iter, image, src, dst, looper.paint(), constraint);
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

    const SkMatrix matrix = SkMatrix::MakeTrans(x, y);

    SkRect storage;
    const SkRect* bounds = nullptr;
    if (paint->canComputeFastBounds()) {
        bitmap.getBounds(&storage);
        matrix.mapRect(&storage);
        SkRect tmp = storage;
        if (this->quickReject(paint->computeFastBounds(tmp, &tmp))) {
            return;
        }
        bounds = &storage;
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawBitmap(iter, bitmap, matrix, looper.paint());
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

    SkRect storage;
    const SkRect* bounds = &dst;
    if (nullptr == paint || paint->canComputeFastBounds()) {
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN_CHECK_COMPLETE_OVERWRITE(*paint, SkDrawFilter::kBitmap_Type, bounds,
                                          bitmap.isOpaque())

    while (iter.next()) {
        iter.fDevice->drawBitmapRect(iter, bitmap, src, dst, looper.paint(), constraint);
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
    
    SkRect storage;
    const SkRect* bounds = &dst;
    if (nullptr == paint || paint->canComputeFastBounds()) {
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }
    
    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }
    
    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)
    
    while (iter.next()) {
        iter.fDevice->drawImageNine(iter, image, center, dst, looper.paint());
    }
    
    LOOPER_END
}

void SkCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                                const SkPaint* paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawBitmapNine()");
    SkDEBUGCODE(bitmap.validate();)

    SkRect storage;
    const SkRect* bounds = &dst;
    if (nullptr == paint || paint->canComputeFastBounds()) {
        if (this->quickReject(paint ? paint->computeFastBounds(dst, &storage) : dst)) {
            return;
        }
    }
    
    SkLazyPaint lazy;
    if (nullptr == paint) {
        paint = lazy.init();
    }
    
    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)
    
    while (iter.next()) {
        iter.fDevice->drawBitmapNine(iter, bitmap, center, dst, looper.paint());
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

void SkCanvas::DrawRect(const SkDraw& draw, const SkPaint& paint,
                        const SkRect& r, SkScalar textSize) {
    if (paint.getStyle() == SkPaint::kFill_Style) {
        draw.fDevice->drawRect(draw, r, paint);
    } else {
        SkPaint p(paint);
        p.setStrokeWidth(SkScalarMul(textSize, paint.getStrokeWidth()));
        draw.fDevice->drawRect(draw, r, p);
    }
}

void SkCanvas::DrawTextDecorations(const SkDraw& draw, const SkPaint& paint,
                                   const char text[], size_t byteLength,
                                   SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0 ||
        draw.fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == nullptr)) {
        return;
    }

    SkScalar    width = 0;
    SkPoint     start;

    start.set(0, 0);    // to avoid warning
    if (paint.getFlags() & (SkPaint::kUnderlineText_Flag |
                            SkPaint::kStrikeThruText_Flag)) {
        width = paint.measureText(text, byteLength);

        SkScalar offsetX = 0;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            offsetX = SkScalarHalf(width);
        } else if (paint.getTextAlign() == SkPaint::kRight_Align) {
            offsetX = width;
        }
        start.set(x - offsetX, y);
    }

    if (0 == width) {
        return;
    }

    uint32_t flags = paint.getFlags();

    if (flags & (SkPaint::kUnderlineText_Flag |
                 SkPaint::kStrikeThruText_Flag)) {
        SkScalar textSize = paint.getTextSize();
        SkScalar height = SkScalarMul(textSize, kStdUnderline_Thickness);
        SkRect   r;

        r.fLeft = start.fX;
        r.fRight = start.fX + width;

        if (flags & SkPaint::kUnderlineText_Flag) {
            SkScalar offset = SkScalarMulAdd(textSize, kStdUnderline_Offset,
                                             start.fY);
            r.fTop = offset;
            r.fBottom = offset + height;
            DrawRect(draw, paint, r, textSize);
        }
        if (flags & SkPaint::kStrikeThruText_Flag) {
            SkScalar offset = SkScalarMulAdd(textSize, kStdStrikeThru_Offset,
                                             start.fY);
            r.fTop = offset;
            r.fBottom = offset + height;
            DrawRect(draw, paint, r, textSize);
        }
    }
}

void SkCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawText(iter, text, byteLength, x, y, dfp.paint());
        DrawTextDecorations(iter, dfp.paint(),
                            static_cast<const char*>(text), byteLength, x, y);
    }

    LOOPER_END
}

void SkCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                             const SkPaint& paint) {
    SkPoint textOffset = SkPoint::Make(0, 0);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(iter, text, byteLength, &pos->fX, 2, textOffset,
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
        iter.fDevice->drawPosText(iter, text, byteLength, xpos, 1, textOffset,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                const SkMatrix* matrix, const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, nullptr)

    while (iter.next()) {
        iter.fDevice->drawTextOnPath(iter, text, byteLength, path,
                                     matrix, looper.paint());
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
        iter.fDevice->drawTextBlob(iter, blob, x, y, dfp.paint(), drawFilter);
    }

    LOOPER_END

    fMCRec->fFilter = drawFilter;
}

// These will become non-virtual, so they always call the (virtual) onDraw... method
void SkCanvas::drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                        const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawText()");
    this->onDrawText(text, byteLength, x, y, paint);
}
void SkCanvas::drawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                           const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPosText()");
    this->onDrawPosText(text, byteLength, pos, paint);
}
void SkCanvas::drawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                            SkScalar constY, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPosTextH()");
    this->onDrawPosTextH(text, byteLength, xpos, constY, paint);
}
void SkCanvas::drawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                              const SkMatrix* matrix, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawTextOnPath()");
    this->onDrawTextOnPath(text, byteLength, path, matrix, paint);
}
void SkCanvas::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawTextBlob()");
    if (blob) {
        this->onDrawTextBlob(blob, x, y, paint);
    }
}

void SkCanvas::onDrawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawVertices()");
    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, nullptr)

    while (iter.next()) {
        iter.fDevice->drawVertices(iter, vmode, vertexCount, verts, texs,
                                   colors, xmode, indices, indexCount,
                                   looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                         const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPatch()");
    if (nullptr == cubics) {
        return;
    }

    // Since a patch is always within the convex hull of the control points, we discard it when its
    // bounding rectangle is completely outside the current clip.
    SkRect bounds;
    bounds.set(cubics, SkPatchUtils::kNumCtrlPts);
    if (this->quickReject(bounds)) {
        return;
    }

    this->onDrawPatch(cubics, colors, texCoords, xmode, paint);
}

void SkCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) {

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, nullptr)

    while (iter.next()) {
        iter.fDevice->drawPatch(iter, cubics, colors, texCoords, xmode, paint);
    }

    LOOPER_END
}

void SkCanvas::drawDrawable(SkDrawable* dr, SkScalar x, SkScalar y) {
    if (dr) {
        if (x || y) {
            SkMatrix matrix = SkMatrix::MakeTrans(x, y);
            this->onDrawDrawable(dr, &matrix);
        } else {
            this->onDrawDrawable(dr, nullptr);
        }
    }
}

void SkCanvas::drawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    if (dr) {
        if (matrix && matrix->isIdentity()) {
            matrix = nullptr;
        }
        this->onDrawDrawable(dr, matrix);
    }
}

void SkCanvas::onDrawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    SkRect bounds = dr->getBounds();
    if (matrix) {
        matrix->mapRect(&bounds);
    }
    if (this->quickReject(bounds)) {
        return;
    }
    dr->draw(this, matrix);
}

void SkCanvas::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                           const SkColor colors[], int count, SkXfermode::Mode mode,
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
        iter.fDevice->drawAtlas(iter, atlas, xform, tex, colors, count, mode, pnt);
    }
    LOOPER_END
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b,
                        SkXfermode::Mode mode) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawARGB()");
    SkPaint paint;

    paint.setARGB(a, r, g, b);
    if (SkXfermode::kSrcOver_Mode != mode) {
        paint.setXfermodeMode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawColor(SkColor c, SkXfermode::Mode mode) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawColor()");
    SkPaint paint;

    paint.setColor(c);
    if (SkXfermode::kSrcOver_Mode != mode) {
        paint.setXfermodeMode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPoint(SkPaint)");
    SkPoint pt;

    pt.set(x, y);
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, SkColor color) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPoint(SkColor)");
    SkPoint pt;
    SkPaint paint;

    pt.set(x, y);
    paint.setColor(color);
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1,
                        const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawLine()");
    SkPoint pts[2];

    pts[0].set(x0, y0);
    pts[1].set(x1, y1);
    this->drawPoints(kLines_PointMode, 2, pts, paint);
}

void SkCanvas::drawRectCoords(SkScalar left, SkScalar top,
                              SkScalar right, SkScalar bottom,
                              const SkPaint& paint) {
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawRectCoords()");
    SkRect  r;

    r.set(left, top, right, bottom);
    this->drawRect(r, paint);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius,
                          const SkPaint& paint) {
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
        if (paint.canComputeFastBounds()) {
            SkRect storage;
            if (this->quickReject(paint.computeFastBounds(r, &storage))) {
                return;
            }
        }
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
    if (SkScalarAbs(sweepAngle) >= SkIntToScalar(360)) {
        this->drawOval(oval, paint);
    } else {
        SkPath  path;
        if (useCenter) {
            path.moveTo(oval.centerX(), oval.centerY());
        }
        path.arcTo(oval, startAngle, sweepAngle, !useCenter);
        if (useCenter) {
            path.close();
        }
        this->drawPath(path, paint);
    }
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
    TRACE_EVENT0("disabled-by-default-skia", "SkCanvas::drawPicture()");
    if (picture) {
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

    SkBaseDevice* device = this->getTopDevice();
    if (device) {
        // Canvas has to first give the device the opportunity to render
        // the picture itself.
        if (device->EXPERIMENTAL_drawPicture(this, picture, matrix, paint)) {
            return; // the device has rendered the entire picture
        }
    }

    SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
    picture->playback(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkCanvas::LayerIter::LayerIter(SkCanvas* canvas, bool skipEmptyClips) {
    static_assert(sizeof(fStorage) >= sizeof(SkDrawIter), "fStorage_too_small");

    SkASSERT(canvas);

    fImpl = new (fStorage) SkDrawIter(canvas, skipEmptyClips);
    fDone = !fImpl->next();
}

SkCanvas::LayerIter::~LayerIter() {
    fImpl->~SkDrawIter();
}

void SkCanvas::LayerIter::next() {
    fDone = !fImpl->next();
}

SkBaseDevice* SkCanvas::LayerIter::device() const {
    return fImpl->getDevice();
}

const SkMatrix& SkCanvas::LayerIter::matrix() const {
    return fImpl->getMatrix();
}

const SkPaint& SkCanvas::LayerIter::paint() const {
    const SkPaint* paint = fImpl->getPaint();
    if (nullptr == paint) {
        paint = &fDefaultPaint;
    }
    return *paint;
}

const SkRegion& SkCanvas::LayerIter::clip() const { return fImpl->getClip(); }
int SkCanvas::LayerIter::x() const { return fImpl->getX(); }
int SkCanvas::LayerIter::y() const { return fImpl->getY(); }

///////////////////////////////////////////////////////////////////////////////

SkCanvasClipVisitor::~SkCanvasClipVisitor() { }

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
            break;
        default:
            return false;
    }

    return true;
}

SkCanvas* SkCanvas::NewRasterDirect(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    if (!supported_for_raster_canvas(info)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return nullptr;
    }
    return new SkCanvas(bitmap);
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


/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvas.h"
#include "SkBitmapDevice.h"
#include "SkBounder.h"
#include "SkDeviceImageFilterProxy.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawLooper.h"
#include "SkMetaData.h"
#include "SkPathOps.h"
#include "SkPicture.h"
#include "SkRasterClip.h"
#include "SkRRect.h"
#include "SkSmallAllocator.h"
#include "SkSurface_Base.h"
#include "SkTemplates.h"
#include "SkTextFormatParams.h"
#include "SkTLazy.h"
#include "SkUtils.h"

#if SK_SUPPORT_GPU
#include "GrRenderTarget.h"
#endif

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

#ifdef SK_DEBUG
#include "SkPixelRef.h"

/*
 *  Some pixelref subclasses can support being "locked" from another thread
 *  during the lock-scope of skia calling them. In these instances, this balance
 *  check will fail, but may not be indicative of a problem, so we allow a build
 *  flag to disable this check.
 *
 *  Potentially another fix would be to have a (debug-only) virtual or flag on
 *  pixelref, which could tell us at runtime if this check is valid. That would
 *  eliminate the need for this heavy-handed build check.
 */
#ifdef SK_DISABLE_PIXELREF_LOCKCOUNT_BALANCE_CHECK
class AutoCheckLockCountBalance {
public:
    AutoCheckLockCountBalance(const SkBitmap&) { /* do nothing */ }
};
#else
class AutoCheckLockCountBalance {
public:
    AutoCheckLockCountBalance(const SkBitmap& bm) : fPixelRef(bm.pixelRef()) {
        fLockCount = fPixelRef ? fPixelRef->getLockCount() : 0;
    }
    ~AutoCheckLockCountBalance() {
        const int count = fPixelRef ? fPixelRef->getLockCount() : 0;
        SkASSERT(count == fLockCount);
    }

private:
    const SkPixelRef* fPixelRef;
    int               fLockCount;
};
#endif

class AutoCheckNoSetContext {
public:
    AutoCheckNoSetContext(const SkPaint& paint) : fPaint(paint) {
        this->assertNoSetContext(fPaint);
    }
    ~AutoCheckNoSetContext() {
        this->assertNoSetContext(fPaint);
    }

private:
    const SkPaint& fPaint;

    void assertNoSetContext(const SkPaint& paint) {
        SkShader* s = paint.getShader();
        if (s) {
            SkASSERT(!s->setContextHasBeenCalled());
        }
    }
};

#define CHECK_LOCKCOUNT_BALANCE(bitmap)  AutoCheckLockCountBalance clcb(bitmap)
#define CHECK_SHADER_NOSETCONTEXT(paint) AutoCheckNoSetContext     cshsc(paint)

#else
    #define CHECK_LOCKCOUNT_BALANCE(bitmap)
    #define CHECK_SHADER_NOSETCONTEXT(paint)
#endif

typedef SkTLazy<SkPaint> SkLazyPaint;

void SkCanvas::predrawNotify() {
    if (fSurfaceBase) {
        fSurfaceBase->aboutToDraw(SkSurface::kRetain_ContentChangeMode);
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
    DeviceCM*           fNext;
    SkBaseDevice*       fDevice;
    SkRasterClip        fClip;
    const SkMatrix*     fMatrix;
    SkPaint*            fPaint; // may be null (in the future)

    DeviceCM(SkBaseDevice* device, int x, int y, const SkPaint* paint, SkCanvas* canvas)
            : fNext(NULL) {
        if (NULL != device) {
            device->ref();
            device->onAttachToCanvas(canvas);
        }
        fDevice = device;
        fPaint = paint ? SkNEW_ARGS(SkPaint, (*paint)) : NULL;
    }

    ~DeviceCM() {
        if (NULL != fDevice) {
            fDevice->onDetachFromCanvas();
            fDevice->unref();
        }
        SkDELETE(fPaint);
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

private:
    SkMatrix    fMatrixStorage;
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
    int             fFlags;
    SkMatrix*       fMatrix;        // points to either fMatrixStorage or prev MCRec
    SkRasterClip*   fRasterClip;    // points to either fRegionStorage or prev MCRec
    SkDrawFilter*   fFilter;        // the current filter (or null)

    DeviceCM*   fLayer;
    /*  If there are any layers in the stack, this points to the top-most
        one that is at or below this level in the stack (so we know what
        bitmap/device to draw into from this level. This value is NOT
        reference counted, since the real owner is either our fLayer field,
        or a previous one in a lower level.)
    */
    DeviceCM*   fTopLayer;

    MCRec(const MCRec* prev, int flags) : fFlags(flags) {
        if (NULL != prev) {
            if (flags & SkCanvas::kMatrix_SaveFlag) {
                fMatrixStorage = *prev->fMatrix;
                fMatrix = &fMatrixStorage;
            } else {
                fMatrix = prev->fMatrix;
            }

            if (flags & SkCanvas::kClip_SaveFlag) {
                fRasterClipStorage = *prev->fRasterClip;
                fRasterClip = &fRasterClipStorage;
            } else {
                fRasterClip = prev->fRasterClip;
            }

            fFilter = prev->fFilter;
            SkSafeRef(fFilter);

            fTopLayer = prev->fTopLayer;
        } else {   // no prev
            fMatrixStorage.reset();

            fMatrix     = &fMatrixStorage;
            fRasterClip = &fRasterClipStorage;
            fFilter     = NULL;
            fTopLayer   = NULL;
        }
        fLayer = NULL;

        // don't bother initializing fNext
        inc_rec();
    }
    ~MCRec() {
        SkSafeUnref(fFilter);
        SkDELETE(fLayer);
        dec_rec();
    }

private:
    SkMatrix        fMatrixStorage;
    SkRasterClip    fRasterClipStorage;
};

class SkDrawIter : public SkDraw {
public:
    SkDrawIter(SkCanvas* canvas, bool skipEmptyClips = true) {
        canvas = canvas->canvasForDrawIter();
        fCanvas = canvas;
        canvas->updateDeviceCMCache();

        fClipStack = &canvas->fClipStack;
        fBounder = canvas->getBounder();
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
            fBitmap = &fDevice->accessBitmap(true);
            fPaint  = rec->fPaint;
            SkDEBUGCODE(this->validate();)

            fCurrLayer = rec->fNext;
            if (fBounder) {
                fBounder->setClip(fClip);
            }
            // fCurrLayer may be NULL now

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

class AutoDrawLooper {
public:
    AutoDrawLooper(SkCanvas* canvas, const SkPaint& paint,
                   bool skipLayerForImageFilter = false,
                   const SkRect* bounds = NULL) : fOrigPaint(paint) {
        fCanvas = canvas;
        fFilter = canvas->getDrawFilter();
        fPaint = NULL;
        fSaveCount = canvas->getSaveCount();
        fDoClearImageFilter = false;
        fDone = false;

        if (!skipLayerForImageFilter && fOrigPaint.getImageFilter()) {
            SkPaint tmp;
            tmp.setImageFilter(fOrigPaint.getImageFilter());
            (void)canvas->internalSaveLayer(bounds, &tmp, SkCanvas::kARGB_ClipLayer_SaveFlag,
                                            true, SkCanvas::kFullLayer_SaveLayerStrategy);
            // we'll clear the imageFilter for the actual draws in next(), so
            // it will only be applied during the restore().
            fDoClearImageFilter = true;
        }

        if (SkDrawLooper* looper = paint.getLooper()) {
            void* buffer = fLooperContextAllocator.reserveT<SkDrawLooper::Context>(
                    looper->contextSize());
            fLooperContext = looper->createContext(canvas, buffer);
            fIsSimple = false;
        } else {
            fLooperContext = NULL;
            // can we be marked as simple?
            fIsSimple = !fFilter && !fDoClearImageFilter;
        }
    }

    ~AutoDrawLooper() {
        if (fDoClearImageFilter) {
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
            fPaint = &fOrigPaint;
            return !fPaint->nothingToDraw();
        } else {
            return this->doNext(drawType);
        }
    }

private:
    SkLazyPaint     fLazyPaint;
    SkCanvas*       fCanvas;
    const SkPaint&  fOrigPaint;
    SkDrawFilter*   fFilter;
    const SkPaint*  fPaint;
    int             fSaveCount;
    bool            fDoClearImageFilter;
    bool            fDone;
    bool            fIsSimple;
    SkDrawLooper::Context* fLooperContext;
    SkSmallAllocator<1, 32> fLooperContextAllocator;

    bool doNext(SkDrawFilter::Type drawType);
};

bool AutoDrawLooper::doNext(SkDrawFilter::Type drawType) {
    fPaint = NULL;
    SkASSERT(!fIsSimple);
    SkASSERT(fLooperContext || fFilter || fDoClearImageFilter);

    SkPaint* paint = fLazyPaint.set(fOrigPaint);

    if (fDoClearImageFilter) {
        paint->setImageFilter(NULL);
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
        if (NULL == fLooperContext) {
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
        fPaint = NULL;
        return false;
    }
    return true;
}

/*  Stack helper for managing a SkBounder. In the destructor, if we were
    given a bounder, we call its commit() method, signifying that we are
    done accumulating bounds for that draw.
*/
class SkAutoBounderCommit {
public:
    SkAutoBounderCommit(SkBounder* bounder) : fBounder(bounder) {}
    ~SkAutoBounderCommit() {
        if (NULL != fBounder) {
            fBounder->commit();
        }
    }
private:
    SkBounder*  fBounder;
};
#define SkAutoBounderCommit(...) SK_REQUIRE_LOCAL_VAR(SkAutoBounderCommit)

#include "SkColorPriv.h"

////////// macros to place around the internal draw calls //////////////////

#define LOOPER_BEGIN_DRAWDEVICE(paint, type)                        \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, paint, true);                      \
    while (looper.next(type)) {                                     \
        SkAutoBounderCommit ac(fBounder);                           \
        SkDrawIter          iter(this);

#define LOOPER_BEGIN(paint, type, bounds)                           \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, paint, false, bounds);             \
    while (looper.next(type)) {                                     \
        SkAutoBounderCommit ac(fBounder);                           \
        SkDrawIter          iter(this);

#define LOOPER_END    }

////////////////////////////////////////////////////////////////////////////

SkBaseDevice* SkCanvas::init(SkBaseDevice* device) {
    fBounder = NULL;
    fCachedLocalClipBounds.setEmpty();
    fCachedLocalClipBoundsDirty = true;
    fAllowSoftClip = true;
    fAllowSimplifyClip = false;
    fDeviceCMDirty = false;
    fSaveLayerCount = 0;
    fCullCount = 0;
    fMetaData = NULL;

    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec(NULL, 0);

    fMCRec->fLayer = SkNEW_ARGS(DeviceCM, (NULL, 0, 0, NULL, NULL));
    fMCRec->fTopLayer = fMCRec->fLayer;

    fSurfaceBase = NULL;

    return this->setRootDevice(device);
}

SkCanvas::SkCanvas()
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
{
    inc_canvas();

    this->init(NULL);
}

SkCanvas::SkCanvas(int width, int height)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
{
    inc_canvas();

    SkBitmap bitmap;
    bitmap.setConfig(SkImageInfo::MakeUnknown(width, height));
    this->init(SkNEW_ARGS(SkBitmapDevice, (bitmap)))->unref();
}

SkCanvas::SkCanvas(SkBaseDevice* device)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
{
    inc_canvas();

    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
{
    inc_canvas();

    this->init(SkNEW_ARGS(SkBitmapDevice, (bitmap)))->unref();
}

SkCanvas::~SkCanvas() {
    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last
    SkASSERT(0 == fSaveLayerCount);

    this->internalRestore();    // restore the last, since we're going away

    SkSafeUnref(fBounder);
    SkDELETE(fMetaData);

    dec_canvas();
}

SkBounder* SkCanvas::setBounder(SkBounder* bounder) {
    SkRefCnt_SafeAssign(fBounder, bounder);
    return bounder;
}

SkDrawFilter* SkCanvas::getDrawFilter() const {
    return fMCRec->fFilter;
}

SkDrawFilter* SkCanvas::setDrawFilter(SkDrawFilter* filter) {
    SkRefCnt_SafeAssign(fMCRec->fFilter, filter);
    return filter;
}

SkMetaData& SkCanvas::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
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

SkBaseDevice* SkCanvas::setRootDevice(SkBaseDevice* device) {
    // return root device
    SkDeque::F2BIter iter(fMCStack);
    MCRec*           rec = (MCRec*)iter.next();
    SkASSERT(rec && rec->fLayer);
    SkBaseDevice*    rootDevice = rec->fLayer->fDevice;

    if (rootDevice == device) {
        return device;
    }

    if (device) {
        device->onAttachToCanvas(this);
    }
    if (rootDevice) {
        rootDevice->onDetachFromCanvas();
    }

    SkRefCnt_SafeAssign(rec->fLayer->fDevice, device);
    rootDevice = device;

    fDeviceCMDirty = true;

    /*  Now we update our initial region to have the bounds of the new device,
        and then intersect all of the clips in our stack with these bounds,
        to ensure that we can't draw outside of the device's bounds (and trash
                                                                     memory).

    NOTE: this is only a partial-fix, since if the new device is larger than
        the previous one, we don't know how to "enlarge" the clips in our stack,
        so drawing may be artificially restricted. Without keeping a history of
        all calls to canvas->clipRect() and canvas->clipPath(), we can't exactly
        reconstruct the correct clips, so this approximation will have to do.
        The caller really needs to restore() back to the base if they want to
        accurately take advantage of the new device bounds.
    */

    SkIRect bounds;
    if (device) {
        bounds.set(0, 0, device->width(), device->height());
    } else {
        bounds.setEmpty();
    }
    // now jam our 1st clip to be bounds, and intersect the rest with that
    rec->fRasterClip->setRect(bounds);
    while ((rec = (MCRec*)iter.next()) != NULL) {
        (void)rec->fRasterClip->op(bounds, SkRegion::kIntersect_Op);
    }

    return device;
}

bool SkCanvas::readPixels(SkBitmap* bitmap,
                          int x, int y,
                          Config8888 config8888) {
    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }
    return device->readPixels(bitmap, x, y, config8888);
}

bool SkCanvas::readPixels(const SkIRect& srcRect, SkBitmap* bitmap) {
    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }

    SkIRect bounds;
    bounds.set(0, 0, device->width(), device->height());
    if (!bounds.intersect(srcRect)) {
        return false;
    }

    SkBitmap tmp;
    tmp.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(),
                                               bounds.height());
    if (this->readPixels(&tmp, bounds.fLeft, bounds.fTop)) {
        bitmap->swap(tmp);
        return true;
    } else {
        return false;
    }
}

#ifdef SK_SUPPORT_LEGACY_WRITEPIXELSCONFIG
void SkCanvas::writePixels(const SkBitmap& bitmap, int x, int y,
                           Config8888 config8888) {
    SkBaseDevice* device = this->getDevice();
    if (device) {
        if (SkIRect::Intersects(SkIRect::MakeSize(this->getDeviceSize()),
                                SkIRect::MakeXYWH(x, y, bitmap.width(), bitmap.height()))) {
            device->accessBitmap(true);
            device->writePixels(bitmap, x, y, config8888);
        }
    }
}
#endif

bool SkCanvas::writePixels(const SkBitmap& bitmap, int x, int y) {
    if (bitmap.getTexture()) {
        return false;
    }
    SkBitmap bm(bitmap);
    bm.lockPixels();
    if (bm.getPixels()) {
        return this->writePixels(bm.info(), bm.getPixels(), bm.rowBytes(), x, y);
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
    if (NULL == pixels || rowBytes < origInfo.minRowBytes()) {
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

    SkImageInfo info = origInfo;
    // the intersect may have shrunk info's logical size
    info.fWidth = target.width();
    info.fHeight = target.height();

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    pixels = ((const char*)pixels - y * rowBytes - x * info.bytesPerPixel());

    // The device can assert that the requested area is always contained in its bounds
    return device->writePixelsDirect(info, pixels, rowBytes, target.x(), target.y());
}

SkCanvas* SkCanvas::canvasForDrawIter() {
    return this;
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::updateDeviceCMCache() {
    if (fDeviceCMDirty) {
        const SkMatrix& totalMatrix = this->getTotalMatrix();
        const SkRasterClip& totalClip = *fMCRec->fRasterClip;
        DeviceCM*       layer = fMCRec->fTopLayer;

        if (NULL == layer->fNext) {   // only one layer
            layer->updateMC(totalMatrix, totalClip, fClipStack, NULL);
        } else {
            SkRasterClip clip(totalClip);
            do {
                layer->updateMC(totalMatrix, clip, fClipStack, &clip);
            } while ((layer = layer->fNext) != NULL);
        }
        fDeviceCMDirty = false;
    }
}

///////////////////////////////////////////////////////////////////////////////

int SkCanvas::internalSave(SaveFlags flags) {
    int saveCount = this->getSaveCount(); // record this before the actual save

    MCRec* newTop = (MCRec*)fMCStack.push_back();
    new (newTop) MCRec(fMCRec, flags);    // balanced in restore()

    fMCRec = newTop;

    if (SkCanvas::kClip_SaveFlag & flags) {
        fClipStack.save();
    }

    return saveCount;
}

void SkCanvas::willSave(SaveFlags) {
    // Do nothing. Subclasses may do something.
}

int SkCanvas::save(SaveFlags flags) {
    this->willSave(flags);
    // call shared impl
    return this->internalSave(flags);
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
    SkRegion::Op op = SkRegion::kIntersect_Op;
    if (!this->getClipDeviceBounds(&clipBounds)) {
        return false;
    }

    if (imageFilter) {
        imageFilter->filterBounds(clipBounds, *fMCRec->fMatrix, &clipBounds);
        // Filters may grow the bounds beyond the device bounds.
        op = SkRegion::kReplace_Op;
    }
    SkIRect ir;
    if (NULL != bounds) {
        SkRect r;

        this->getTotalMatrix().mapRect(&r, *bounds);
        r.roundOut(&ir);
        // early exit if the layer's bounds are clipped out
        if (!ir.intersect(clipBounds)) {
            if (bounds_affects_clip(flags)) {
                fMCRec->fRasterClip->setEmpty();
            }
            return false;
        }
    } else {    // no user bounds, so just use the clip
        ir = clipBounds;
    }

    if (bounds_affects_clip(flags)) {
        fClipStack.clipDevRect(ir, op);
        // early exit if the clip is now empty
        if (!fMCRec->fRasterClip->op(ir, op)) {
            return false;
        }
    }

    if (intersection) {
        *intersection = ir;
    }
    return true;
}

SkCanvas::SaveLayerStrategy SkCanvas::willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) {

    // Do nothing. Subclasses may do something.
    return kFullLayer_SaveLayerStrategy;
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                        SaveFlags flags) {
    // Overriding classes may return false to signal that we don't need to create a layer.
    SaveLayerStrategy strategy = this->willSaveLayer(bounds, paint, flags);
    return this->internalSaveLayer(bounds, paint, flags, false, strategy);
}

static SkBaseDevice* createCompatibleDevice(SkCanvas* canvas,
                                            const SkImageInfo& info) {
    SkBaseDevice* device = canvas->getDevice();
    return device ? device->createCompatibleDevice(info) : NULL;
}

int SkCanvas::internalSaveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags,
                                bool justForImageFilter, SaveLayerStrategy strategy) {
#ifndef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
    flags = (SaveFlags)(flags | kClipToLayer_SaveFlag);
#endif

    // do this before we create the layer. We don't call the public save() since
    // that would invoke a possibly overridden virtual
    int count = this->internalSave(flags);

    fDeviceCMDirty = true;

    SkIRect ir;
    if (!this->clipRectBounds(bounds, flags, &ir, paint ? paint->getImageFilter() : NULL)) {
        return count;
    }

    // FIXME: do willSaveLayer() overriders returning kNoLayer_SaveLayerStrategy really care about
    // the clipRectBounds() call above?
    if (kNoLayer_SaveLayerStrategy == strategy) {
        return count;
    }

    // Kill the imagefilter if our device doesn't allow it
    SkLazyPaint lazyP;
    if (paint && paint->getImageFilter()) {
        if (!this->getTopDevice()->allowImageFilter(paint->getImageFilter())) {
            if (justForImageFilter) {
                // early exit if the layer was just for the imageFilter
                return count;
            }
            SkPaint* p = lazyP.set(*paint);
            p->setImageFilter(NULL);
            paint = p;
        }
    }

    bool isOpaque = !SkToBool(flags & kHasAlphaLayer_SaveFlag);
    SkImageInfo info = SkImageInfo::MakeN32(ir.width(), ir.height(),
                        isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);

    SkBaseDevice* device;
    if (paint && paint->getImageFilter()) {
        device = createCompatibleDevice(this, info);
    } else {
        device = this->createLayerDevice(info);
    }
    if (NULL == device) {
        SkDebugf("Unable to create device for layer.");
        return count;
    }

    device->setOrigin(ir.fLeft, ir.fTop);
    DeviceCM* layer = SkNEW_ARGS(DeviceCM, (device, ir.fLeft, ir.fTop, paint, this));
    device->unref();

    layer->fNext = fMCRec->fTopLayer;
    fMCRec->fLayer = layer;
    fMCRec->fTopLayer = layer;    // this field is NOT an owner of layer

    fSaveLayerCount += 1;
    return count;
}

int SkCanvas::saveLayerAlpha(const SkRect* bounds, U8CPU alpha,
                             SaveFlags flags) {
    if (0xFF == alpha) {
        return this->saveLayer(bounds, NULL, flags);
    } else {
        SkPaint tmpPaint;
        tmpPaint.setAlpha(alpha);
        return this->saveLayer(bounds, &tmpPaint, flags);
    }
}

void SkCanvas::willRestore() {
    // Do nothing. Subclasses may do something.
}

void SkCanvas::restore() {
    // check for underflow
    if (fMCStack.count() > 1) {
        this->willRestore();
        this->internalRestore();
    }
}

void SkCanvas::internalRestore() {
    SkASSERT(fMCStack.count() != 0);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    if (SkCanvas::kClip_SaveFlag & fMCRec->fFlags) {
        fClipStack.restore();
    }

    // reserve our layer (if any)
    DeviceCM* layer = fMCRec->fLayer;   // may be null
    // now detach it from fMCRec so we can pop(). Gets freed after its drawn
    fMCRec->fLayer = NULL;

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*)fMCStack.back();

    /*  Time to draw the layer's offscreen. We can't call the public drawSprite,
        since if we're being recorded, we don't want to record this (the
        recorder will have already recorded the restore).
    */
    if (NULL != layer) {
        if (layer->fNext) {
            const SkIPoint& origin = layer->fDevice->getOrigin();
            this->internalDrawDevice(layer->fDevice, origin.x(), origin.y(),
                                     layer->fPaint);
            // reset this, since internalDrawDevice will have set it to true
            fDeviceCMDirty = true;

            SkASSERT(fSaveLayerCount > 0);
            fSaveLayerCount -= 1;
        }
        SkDELETE(layer);
    }
}

int SkCanvas::getSaveCount() const {
    return fMCStack.count();
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

bool SkCanvas::isDrawingToLayer() const {
    return fSaveLayerCount > 0;
}

SkSurface* SkCanvas::newSurface(const SkImageInfo& info) {
    return this->onNewSurface(info);
}

SkSurface* SkCanvas::onNewSurface(const SkImageInfo& info) {
    SkBaseDevice* dev = this->getDevice();
    return dev ? dev->newSurface(info) : NULL;
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
    return this->onPeekPixels(info, rowBytes);
}

const void* SkCanvas::onPeekPixels(SkImageInfo* info, size_t* rowBytes) {
    SkBaseDevice* dev = this->getDevice();
    return dev ? dev->peekPixels(info, rowBytes) : NULL;
}

void* SkCanvas::accessTopLayerPixels(SkImageInfo* info, size_t* rowBytes) {
    return this->onAccessTopLayerPixels(info, rowBytes);
}

void* SkCanvas::onAccessTopLayerPixels(SkImageInfo* info, size_t* rowBytes) {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessPixels(info, rowBytes) : NULL;
}

SkAutoROCanvasPixels::SkAutoROCanvasPixels(SkCanvas* canvas) {
    fAddr = canvas->peekPixels(&fInfo, &fRowBytes);
    if (NULL == fAddr) {
        fInfo = canvas->imageInfo();
        if (kUnknown_SkColorType == fInfo.colorType() ||
            !fBitmap.allocPixels(fInfo))
        {
            return; // failure, fAddr is NULL
        }
        fBitmap.lockPixels();
        if (!canvas->readPixels(&fBitmap, 0, 0)) {
            return; // failure, fAddr is NULL
        }
        fAddr = fBitmap.getPixels();
        fRowBytes = fBitmap.rowBytes();
    }
    SkASSERT(fAddr);    // success
}

bool SkAutoROCanvasPixels::asROBitmap(SkBitmap* bitmap) const {
    if (fAddr) {
        return bitmap->installPixels(fInfo, const_cast<void*>(fAddr), fRowBytes,
                                     NULL, NULL);
    } else {
        bitmap->reset();
        return false;
    }
}

void SkCanvas::onPushCull(const SkRect& cullRect) {
    // do nothing. Subclasses may do something
}

void SkCanvas::onPopCull() {
    // do nothing. Subclasses may do something
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::internalDrawBitmap(const SkBitmap& bitmap,
                                const SkMatrix& matrix, const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }

    SkLazyPaint lazy;
    if (NULL == paint) {
        paint = lazy.init();
    }

    SkDEBUGCODE(bitmap.validate();)
    CHECK_LOCKCOUNT_BALANCE(bitmap);

    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint && paint->canComputeFastBounds()) {
        bitmap.getBounds(&storage);
        matrix.mapRect(&storage);
        bounds = &paint->computeFastBounds(storage, &storage);
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawBitmap(iter, bitmap, matrix, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::internalDrawDevice(SkBaseDevice* srcDev, int x, int y,
                                  const SkPaint* paint) {
    SkPaint tmp;
    if (NULL == paint) {
        tmp.setDither(true);
        paint = &tmp;
    }

    LOOPER_BEGIN_DRAWDEVICE(*paint, SkDrawFilter::kBitmap_Type)
    while (iter.next()) {
        SkBaseDevice* dstDev = iter.fDevice;
        paint = &looper.paint();
        SkImageFilter* filter = paint->getImageFilter();
        SkIPoint pos = { x - iter.getX(), y - iter.getY() };
        if (filter && !dstDev->canHandleImageFilter(filter)) {
            SkDeviceImageFilterProxy proxy(dstDev);
            SkBitmap dst;
            SkIPoint offset = SkIPoint::Make(0, 0);
            const SkBitmap& src = srcDev->accessBitmap(false);
            SkMatrix matrix = *iter.fMatrix;
            matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
            SkIRect clipBounds = SkIRect::MakeWH(srcDev->width(), srcDev->height());
            SkImageFilter::Context ctx(matrix, clipBounds);
            if (filter->filterImage(&proxy, src, ctx, &dst, &offset)) {
                SkPaint tmpUnfiltered(*paint);
                tmpUnfiltered.setImageFilter(NULL);
                dstDev->drawSprite(iter, dst, pos.x() + offset.x(), pos.y() + offset.y(),
                                   tmpUnfiltered);
            }
        } else {
            dstDev->drawDevice(iter, srcDev, pos.x(), pos.y(), *paint);
        }
    }
    LOOPER_END
}

void SkCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                          const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }
    SkDEBUGCODE(bitmap.validate();)
    CHECK_LOCKCOUNT_BALANCE(bitmap);

    SkPaint tmp;
    if (NULL == paint) {
        paint = &tmp;
    }

    LOOPER_BEGIN_DRAWDEVICE(*paint, SkDrawFilter::kBitmap_Type)

    while (iter.next()) {
        paint = &looper.paint();
        SkImageFilter* filter = paint->getImageFilter();
        SkIPoint pos = { x - iter.getX(), y - iter.getY() };
        if (filter && !iter.fDevice->canHandleImageFilter(filter)) {
            SkDeviceImageFilterProxy proxy(iter.fDevice);
            SkBitmap dst;
            SkIPoint offset = SkIPoint::Make(0, 0);
            SkMatrix matrix = *iter.fMatrix;
            matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
            SkIRect clipBounds = SkIRect::MakeWH(bitmap.width(), bitmap.height());
            SkImageFilter::Context ctx(matrix, clipBounds);
            if (filter->filterImage(&proxy, bitmap, ctx, &dst, &offset)) {
                SkPaint tmpUnfiltered(*paint);
                tmpUnfiltered.setImageFilter(NULL);
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
void SkCanvas::didTranslate(SkScalar, SkScalar) {
    // Do nothing. Subclasses may do something.
}

bool SkCanvas::translate(SkScalar dx, SkScalar dy) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    bool res = fMCRec->fMatrix->preTranslate(dx, dy);

    this->didTranslate(dx, dy);
    return res;
}

void SkCanvas::didScale(SkScalar, SkScalar) {
    // Do nothing. Subclasses may do something.
}

bool SkCanvas::scale(SkScalar sx, SkScalar sy) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    bool res = fMCRec->fMatrix->preScale(sx, sy);

    this->didScale(sx, sy);
    return res;
}

void SkCanvas::didRotate(SkScalar) {
    // Do nothing. Subclasses may do something.
}

bool SkCanvas::rotate(SkScalar degrees) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    bool res = fMCRec->fMatrix->preRotate(degrees);

    this->didRotate(degrees);
    return res;
}

void SkCanvas::didSkew(SkScalar, SkScalar) {
    // Do nothing. Subclasses may do something.
}

bool SkCanvas::skew(SkScalar sx, SkScalar sy) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    bool res = fMCRec->fMatrix->preSkew(sx, sy);

    this->didSkew(sx, sy);
    return res;
}

void SkCanvas::didConcat(const SkMatrix&) {
    // Do nothing. Subclasses may do something.
}

bool SkCanvas::concat(const SkMatrix& matrix) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    bool res = fMCRec->fMatrix->preConcat(matrix);

    this->didConcat(matrix);
    return res;
}

void SkCanvas::didSetMatrix(const SkMatrix&) {
    // Do nothing. Subclasses may do something.
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    *fMCRec->fMatrix = matrix;
    this->didSetMatrix(matrix);
}

void SkCanvas::resetMatrix() {
    SkMatrix matrix;

    matrix.reset();
    this->setMatrix(matrix);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    this->onClipRect(rect, op, edgeStyle);
}

void SkCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
#ifdef SK_ENABLE_CLIP_QUICKREJECT
    if (SkRegion::kIntersect_Op == op) {
        if (fMCRec->fRasterClip->isEmpty()) {
            return false;
        }

        if (this->quickReject(rect)) {
            fDeviceCMDirty = true;
            fCachedLocalClipBoundsDirty = true;

            fClipStack.clipEmpty();
            return fMCRec->fRasterClip->setEmpty();
        }
    }
#endif

    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    if (!fAllowSoftClip) {
        edgeStyle = kHard_ClipEdgeStyle;
    }

    if (fMCRec->fMatrix->rectStaysRect()) {
        // for these simpler matrices, we can stay a rect even after applying
        // the matrix. This means we don't have to a) make a path, and b) tell
        // the region code to scan-convert the path, only to discover that it
        // is really just a rect.
        SkRect      r;

        fMCRec->fMatrix->mapRect(&r, rect);
        fClipStack.clipDevRect(r, op, kSoft_ClipEdgeStyle == edgeStyle);
        fMCRec->fRasterClip->op(r, op, kSoft_ClipEdgeStyle == edgeStyle);
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

static void clip_path_helper(const SkCanvas* canvas, SkRasterClip* currClip,
                             const SkPath& devPath, SkRegion::Op op, bool doAA) {
    // base is used to limit the size (and therefore memory allocation) of the
    // region that results from scan converting devPath.
    SkRegion base;

    if (SkRegion::kIntersect_Op == op) {
        // since we are intersect, we can do better (tighter) with currRgn's
        // bounds, than just using the device. However, if currRgn is complex,
        // our region blitter may hork, so we do that case in two steps.
        if (currClip->isRect()) {
            // FIXME: we should also be able to do this when currClip->isBW(),
            // but relaxing the test above triggers GM asserts in
            // SkRgnBuilder::blitH(). We need to investigate what's going on.
            currClip->setPath(devPath, currClip->bwRgn(), doAA);
        } else {
            base.setRect(currClip->getBounds());
            SkRasterClip clip;
            clip.setPath(devPath, base, doAA);
            currClip->op(clip, op);
        }
    } else {
        const SkBaseDevice* device = canvas->getDevice();
        if (!device) {
            currClip->setEmpty();
            return;
        }

        base.setRect(0, 0, device->width(), device->height());

        if (SkRegion::kReplace_Op == op) {
            currClip->setPath(devPath, base, doAA);
        } else {
            SkRasterClip clip;
            clip.setPath(devPath, base, doAA);
            currClip->op(clip, op);
        }
    }
}

void SkCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    if (rrect.isRect()) {
        this->onClipRect(rrect.getBounds(), op, edgeStyle);
    } else {
        this->onClipRRect(rrect, op, edgeStyle);
    }
}

void SkCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    SkRRect transformedRRect;
    if (rrect.transform(*fMCRec->fMatrix, &transformedRRect)) {
        AutoValidateClip avc(this);

        fDeviceCMDirty = true;
        fCachedLocalClipBoundsDirty = true;
        if (!fAllowSoftClip) {
            edgeStyle = kHard_ClipEdgeStyle;
        }

        fClipStack.clipDevRRect(transformedRRect, op, kSoft_ClipEdgeStyle == edgeStyle);

        SkPath devPath;
        devPath.addRRect(transformedRRect);

        clip_path_helper(this, fMCRec->fRasterClip, devPath, op, kSoft_ClipEdgeStyle == edgeStyle);
        return;
    }

    SkPath path;
    path.addRRect(rrect);
    // call the non-virtual version
    this->SkCanvas::onClipPath(path, op, edgeStyle);
}

void SkCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    SkRect r;
    if (!path.isInverseFillType() && path.isRect(&r)) {
        this->onClipRect(r, op, edgeStyle);
    } else {
        this->onClipPath(path, op, edgeStyle);
    }
}

void SkCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
#ifdef SK_ENABLE_CLIP_QUICKREJECT
    if (SkRegion::kIntersect_Op == op && !path.isInverseFillType()) {
        if (fMCRec->fRasterClip->isEmpty()) {
            return false;
        }

        if (this->quickReject(path.getBounds())) {
            fDeviceCMDirty = true;
            fCachedLocalClipBoundsDirty = true;

            fClipStack.clipEmpty();
            return fMCRec->fRasterClip->setEmpty();
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
    path.transform(*fMCRec->fMatrix, &devPath);

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
    fClipStack.clipDevPath(devPath, op, kSoft_ClipEdgeStyle == edgeStyle);

    if (fAllowSimplifyClip) {
        devPath.reset();
        devPath.setFillType(SkPath::kInverseEvenOdd_FillType);
        const SkClipStack* clipStack = getClipStack();
        SkClipStack::Iter iter(*clipStack, SkClipStack::Iter::kBottom_IterStart);
        const SkClipStack::Element* element;
        while ((element = iter.next())) {
            SkClipStack::Element::Type type = element->getType();
            if (type == SkClipStack::Element::kEmpty_Type) {
                continue;
            }
            SkPath operand;
            element->asPath(&operand);
            SkRegion::Op elementOp = element->getOp();
            if (elementOp == SkRegion::kReplace_Op) {
                devPath = operand;
            } else {
                Op(devPath, operand, (SkPathOp) elementOp, &devPath);
            }
            // if the prev and curr clips disagree about aa -vs- not, favor the aa request.
            // perhaps we need an API change to avoid this sort of mixed-signals about
            // clipping.
            if (element->isAA()) {
                edgeStyle = kSoft_ClipEdgeStyle;
            }
        }
        op = SkRegion::kReplace_Op;
    }

    clip_path_helper(this, fMCRec->fRasterClip, devPath, op, edgeStyle);
}

void SkCanvas::updateClipConservativelyUsingBounds(const SkRect& bounds, SkRegion::Op op,
                                                   bool inverseFilled) {
    // This is for updating the clip conservatively using only bounds
    // information.
    // Contract:
    //    The current clip must contain the true clip. The true
    //    clip is the clip that would have normally been computed
    //    by calls to clipPath and clipRRect
    // Objective:
    //    Keep the current clip as small as possible without
    //    breaking the contract, using only clip bounding rectangles
    //    (for performance).

    // N.B.: This *never* calls back through a virtual on canvas, so subclasses
    // don't have to worry about getting caught in a loop. Thus anywhere
    // we call a virtual method, we explicitly prefix it with
    // SkCanvas:: to be sure to call the base-class.

    if (inverseFilled) {
        switch (op) {
            case SkRegion::kIntersect_Op:
            case SkRegion::kDifference_Op:
                // These ops can only shrink the current clip. So leaving
                // the clip unchanged conservatively respects the contract.
                break;
            case SkRegion::kUnion_Op:
            case SkRegion::kReplace_Op:
            case SkRegion::kReverseDifference_Op:
            case SkRegion::kXOR_Op: {
                    // These ops can grow the current clip up to the extents of
                    // the input clip, which is inverse filled, so we just set
                    // the current clip to the device bounds.
                    SkRect deviceBounds;
                    SkIRect deviceIBounds;
                    this->getDevice()->getGlobalBounds(&deviceIBounds);
                    deviceBounds = SkRect::Make(deviceIBounds);
                    this->SkCanvas::save(SkCanvas::kMatrix_SaveFlag);
                    // set the clip in device space
                    this->SkCanvas::setMatrix(SkMatrix::I());
                    this->SkCanvas::onClipRect(deviceBounds, SkRegion::kReplace_Op,
                                               kHard_ClipEdgeStyle);
                    this->SkCanvas::restore(); //pop the matrix, but keep the clip
                    break;
            }
            default:
                SkASSERT(0); // unhandled op?
        }
    } else {
        // Not inverse filled
        switch (op) {
            case SkRegion::kIntersect_Op:
            case SkRegion::kUnion_Op:
            case SkRegion::kReplace_Op:
                this->SkCanvas::onClipRect(bounds, op, kHard_ClipEdgeStyle);
                break;
            case SkRegion::kDifference_Op:
                // Difference can only shrink the current clip.
                // Leaving clip unchanged conservatively fullfills the contract.
                break;
            case SkRegion::kReverseDifference_Op:
                // To reverse, we swap in the bounds with a replace op.
                // As with difference, leave it unchanged.
                this->SkCanvas::onClipRect(bounds, SkRegion::kReplace_Op, kHard_ClipEdgeStyle);
                break;
            case SkRegion::kXOR_Op:
                // Be conservative, based on (A XOR B) always included in (A union B),
                // which is always included in (bounds(A) union bounds(B))
                this->SkCanvas::onClipRect(bounds, SkRegion::kUnion_Op, kHard_ClipEdgeStyle);
                break;
            default:
                SkASSERT(0); // unhandled op?
        }
    }
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkRegion::Op op) {
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkRegion::Op op) {
    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    // todo: signal fClipStack that we have a region, and therefore (I guess)
    // we have to ignore it, and use the region directly?
    fClipStack.clipDevRect(rgn.getBounds(), op);

    fMCRec->fRasterClip->op(rgn, op);
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
    SkRasterClip tmpClip(ir);

    SkClipStack::B2TIter                iter(fClipStack);
    const SkClipStack::Element* element;
    while ((element = iter.next()) != NULL) {
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
                clip_path_helper(this, &tmpClip, path, element->getOp(), element->isAA());
                break;
            }
        }
    }
}
#endif

void SkCanvas::replayClips(ClipVisitor* visitor) const {
    SkClipStack::B2TIter                iter(fClipStack);
    const SkClipStack::Element*         element;

    static const SkRect kEmpty = { 0, 0, 0, 0 };
    while ((element = iter.next()) != NULL) {
        switch (element->getType()) {
            case SkClipStack::Element::kPath_Type:
                visitor->clipPath(element->getPath(), element->getOp(), element->isAA());
                break;
            case SkClipStack::Element::kRRect_Type:
                visitor->clipRRect(element->getRRect(), element->getOp(), element->isAA());
                break;
            case SkClipStack::Element::kRect_Type:
                visitor->clipRect(element->getRect(), element->getOp(), element->isAA());
                break;
            case SkClipStack::Element::kEmpty_Type:
                visitor->clipRect(kEmpty, SkRegion::kIntersect_Op, false);
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkCanvas::isClipEmpty() const {
    return fMCRec->fRasterClip->isEmpty();
}

bool SkCanvas::isClipRect() const {
    return fMCRec->fRasterClip->isRect();
}

bool SkCanvas::quickReject(const SkRect& rect) const {

    if (!rect.isFinite())
        return true;

    if (fMCRec->fRasterClip->isEmpty()) {
        return true;
    }

    if (fMCRec->fMatrix->hasPerspective()) {
        SkRect dst;
        fMCRec->fMatrix->mapRect(&dst, rect);
        SkIRect idst;
        dst.roundOut(&idst);
        return !SkIRect::Intersects(idst, fMCRec->fRasterClip->getBounds());
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
    if (!fMCRec->fMatrix->invert(&inverse)) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    if (NULL != bounds) {
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
    const SkRasterClip& clip = *fMCRec->fRasterClip;
    if (clip.isEmpty()) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    if (NULL != bounds) {
        *bounds = clip.getBounds();
    }
    return true;
}

const SkMatrix& SkCanvas::getTotalMatrix() const {
    return *fMCRec->fMatrix;
}

#ifdef SK_SUPPORT_LEGACY_GETCLIPTYPE
SkCanvas::ClipType SkCanvas::getClipType() const {
    if (fMCRec->fRasterClip->isEmpty()) {
        return kEmpty_ClipType;
    }
    if (fMCRec->fRasterClip->isRect()) {
        return kRect_ClipType;
    }
    return kComplex_ClipType;
}
#endif

#ifdef SK_SUPPORT_LEGACY_GETTOTALCLIP
const SkRegion& SkCanvas::getTotalClip() const {
    return fMCRec->fRasterClip->forceGetBW();
}
#endif

const SkRegion& SkCanvas::internal_private_getTotalClip() const {
    return fMCRec->fRasterClip->forceGetBW();
}

void SkCanvas::internal_private_getTotalClipAsPath(SkPath* path) const {
    path->reset();

    const SkRegion& rgn = fMCRec->fRasterClip->forceGetBW();
    if (rgn.isEmpty()) {
        return;
    }
    (void)rgn.getBoundaryPath(path);
}

GrRenderTarget* SkCanvas::internal_private_accessTopLayerRenderTarget() {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessRenderTarget() : NULL;
}

SkBaseDevice* SkCanvas::createLayerDevice(const SkImageInfo& info) {
    SkBaseDevice* device = this->getTopDevice();
    return device ? device->createCompatibleDeviceForSaveLayer(info) : NULL;
}

GrContext* SkCanvas::getGrContext() {
#if SK_SUPPORT_GPU
    SkBaseDevice* device = this->getTopDevice();
    if (NULL != device) {
        GrRenderTarget* renderTarget = device->accessRenderTarget();
        if (NULL != renderTarget) {
            return renderTarget->getContext();
        }
    }
#endif

    return NULL;

}

void SkCanvas::drawDRRect(const SkRRect& outer, const SkRRect& inner,
                          const SkPaint& paint) {
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

//////////////////////////////////////////////////////////////////////////////
//  These are the virtual drawing methods
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clear(SkColor color) {
    SkDrawIter  iter(this);
    this->predrawNotify();
    while (iter.next()) {
        iter.fDevice->clear(color);
    }
}

void SkCanvas::drawPaint(const SkPaint& paint) {
    this->internalDrawPaint(paint);
}

void SkCanvas::internalDrawPaint(const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    LOOPER_BEGIN(paint, SkDrawFilter::kPaint_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawPaint(iter, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                          const SkPaint& paint) {
    if ((long)count <= 0) {
        return;
    }

    CHECK_SHADER_NOSETCONTEXT(paint);

    SkRect r, storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        // special-case 2 points (common for drawing a single line)
        if (2 == count) {
            r.set(pts[0], pts[1]);
        } else {
            r.set(pts, SkToInt(count));
        }
        bounds = &paint.computeFastStrokeBounds(r, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    SkASSERT(pts != NULL);

    LOOPER_BEGIN(paint, SkDrawFilter::kPoint_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPoints(iter, mode, count, pts, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(r, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawRect(iter, r, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawOval(const SkRect& oval, const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(oval, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kOval_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawOval(iter, oval, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(rrect.getBounds(), &storage);
        if (this->quickReject(*bounds)) {
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

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawRRect(iter, rrect, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                            const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(outer.getBounds(), &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawDRRect(iter, outer, inner, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    if (!path.isFinite()) {
        return;
    }

    SkRect storage;
    const SkRect* bounds = NULL;
    if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
        const SkRect& pathBounds = path.getBounds();
        bounds = &paint.computeFastBounds(pathBounds, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    const SkRect& r = path.getBounds();
    if (r.width() <= 0 && r.height() <= 0) {
        if (path.isInverseFillType()) {
            this->internalDrawPaint(paint);
        }
        return;
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPath(iter, path, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                          const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)

    if (NULL == paint || paint->canComputeFastBounds()) {
        SkRect bounds = {
            x, y,
            x + SkIntToScalar(bitmap.width()),
            y + SkIntToScalar(bitmap.height())
        };
        if (paint) {
            (void)paint->computeFastBounds(bounds, &bounds);
        }
        if (this->quickReject(bounds)) {
            return;
        }
    }

    SkMatrix matrix;
    matrix.setTranslate(x, y);
    this->internalDrawBitmap(bitmap, matrix, paint);
}

// this one is non-virtual, so it can be called safely by other canvas apis
void SkCanvas::internalDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) {
    if (bitmap.drawsNothing() || dst.isEmpty()) {
        return;
    }

    CHECK_LOCKCOUNT_BALANCE(bitmap);

    SkRect storage;
    const SkRect* bounds = &dst;
    if (NULL == paint || paint->canComputeFastBounds()) {
        if (paint) {
            bounds = &paint->computeFastBounds(dst, &storage);
        }
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (NULL == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawBitmapRect(iter, bitmap, src, dst, looper.paint(), flags);
    }

    LOOPER_END
}

void SkCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                    const SkRect& dst, const SkPaint* paint,
                                    DrawBitmapRectFlags flags) {
    SkDEBUGCODE(bitmap.validate();)
    this->internalDrawBitmapRect(bitmap, src, dst, paint, flags);
}

void SkCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                                const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)
    this->internalDrawBitmap(bitmap, matrix, paint);
}

void SkCanvas::internalDrawBitmapNine(const SkBitmap& bitmap,
                                      const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }
    if (NULL == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        const SkRect* bounds = &dst;
        if (paint) {
            bounds = &paint->computeFastBounds(dst, &storage);
        }
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    const int32_t w = bitmap.width();
    const int32_t h = bitmap.height();

    SkIRect c = center;
    // pin center to the bounds of the bitmap
    c.fLeft = SkMax32(0, center.fLeft);
    c.fTop = SkMax32(0, center.fTop);
    c.fRight = SkPin32(center.fRight, c.fLeft, w);
    c.fBottom = SkPin32(center.fBottom, c.fTop, h);

    const SkScalar srcX[4] = {
        0, SkIntToScalar(c.fLeft), SkIntToScalar(c.fRight), SkIntToScalar(w)
    };
    const SkScalar srcY[4] = {
        0, SkIntToScalar(c.fTop), SkIntToScalar(c.fBottom), SkIntToScalar(h)
    };
    SkScalar dstX[4] = {
        dst.fLeft, dst.fLeft + SkIntToScalar(c.fLeft),
        dst.fRight - SkIntToScalar(w - c.fRight), dst.fRight
    };
    SkScalar dstY[4] = {
        dst.fTop, dst.fTop + SkIntToScalar(c.fTop),
        dst.fBottom - SkIntToScalar(h - c.fBottom), dst.fBottom
    };

    if (dstX[1] > dstX[2]) {
        dstX[1] = dstX[0] + (dstX[3] - dstX[0]) * c.fLeft / (w - c.width());
        dstX[2] = dstX[1];
    }

    if (dstY[1] > dstY[2]) {
        dstY[1] = dstY[0] + (dstY[3] - dstY[0]) * c.fTop / (h - c.height());
        dstY[2] = dstY[1];
    }

    for (int y = 0; y < 3; y++) {
        SkRect s, d;

        s.fTop = srcY[y];
        s.fBottom = srcY[y+1];
        d.fTop = dstY[y];
        d.fBottom = dstY[y+1];
        for (int x = 0; x < 3; x++) {
            s.fLeft = srcX[x];
            s.fRight = srcX[x+1];
            d.fLeft = dstX[x];
            d.fRight = dstX[x+1];
            this->internalDrawBitmapRect(bitmap, &s, d, paint,
                                         kNone_DrawBitmapRectFlag);
        }
    }
}

void SkCanvas::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                              const SkRect& dst, const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)

    // Need a device entry-point, so gpu can use a mesh
    this->internalDrawBitmapNine(bitmap, center, dst, paint);
}

class SkDeviceFilteredPaint {
public:
    SkDeviceFilteredPaint(SkBaseDevice* device, const SkPaint& paint) {
        SkBaseDevice::TextFlags flags;
        if (device->filterTextFlags(paint, &flags)) {
            SkPaint* newPaint = fLazy.set(paint);
            newPaint->setFlags(flags.fFlags);
            newPaint->setHinting(flags.fHinting);
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
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 ||
        draw.fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) {
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

void SkCanvas::drawText(const void* text, size_t byteLength,
                        SkScalar x, SkScalar y, const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawText(iter, text, byteLength, x, y, dfp.paint());
        DrawTextDecorations(iter, dfp.paint(),
                            static_cast<const char*>(text), byteLength, x, y);
    }

    LOOPER_END
}

void SkCanvas::drawPosText(const void* text, size_t byteLength,
                           const SkPoint pos[], const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(iter, text, byteLength, &pos->fX, 0, 2,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPosTextH(const void* text, size_t byteLength,
                            const SkScalar xpos[], SkScalar constY,
                            const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(iter, text, byteLength, xpos, constY, 1,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::drawTextOnPath(const void* text, size_t byteLength,
                              const SkPath& path, const SkMatrix* matrix,
                              const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawTextOnPath(iter, text, byteLength, path,
                                     matrix, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawVertices(VertexMode vmode, int vertexCount,
                            const SkPoint verts[], const SkPoint texs[],
                            const SkColor colors[], SkXfermode* xmode,
                            const uint16_t indices[], int indexCount,
                            const SkPaint& paint) {
    CHECK_SHADER_NOSETCONTEXT(paint);

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawVertices(iter, vmode, vertexCount, verts, texs,
                                   colors, xmode, indices, indexCount,
                                   looper.paint());
    }

    LOOPER_END
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b,
                        SkXfermode::Mode mode) {
    SkPaint paint;

    paint.setARGB(a, r, g, b);
    if (SkXfermode::kSrcOver_Mode != mode) {
        paint.setXfermodeMode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawColor(SkColor c, SkXfermode::Mode mode) {
    SkPaint paint;

    paint.setColor(c);
    if (SkXfermode::kSrcOver_Mode != mode) {
        paint.setXfermodeMode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, const SkPaint& paint) {
    SkPoint pt;

    pt.set(x, y);
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, SkColor color) {
    SkPoint pt;
    SkPaint paint;

    pt.set(x, y);
    paint.setColor(color);
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1,
                        const SkPaint& paint) {
    SkPoint pts[2];

    pts[0].set(x0, y0);
    pts[1].set(x1, y1);
    this->drawPoints(kLines_PointMode, 2, pts, paint);
}

void SkCanvas::drawRectCoords(SkScalar left, SkScalar top,
                              SkScalar right, SkScalar bottom,
                              const SkPaint& paint) {
    SkRect  r;

    r.set(left, top, right, bottom);
    this->drawRect(r, paint);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius,
                          const SkPaint& paint) {
    if (radius < 0) {
        radius = 0;
    }

    SkRect  r;
    r.set(cx - radius, cy - radius, cx + radius, cy + radius);
    this->drawOval(r, paint);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry,
                             const SkPaint& paint) {
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
    SkMatrix    matrix;

    matrix.setTranslate(hOffset, vOffset);
    this->drawTextOnPath(text, byteLength, path, &matrix, paint);
}

///////////////////////////////////////////////////////////////////////////////
void SkCanvas::EXPERIMENTAL_optimize(SkPicture* picture) {
    SkBaseDevice* device = this->getDevice();
    if (NULL != device) {
        device->EXPERIMENTAL_optimize(picture);
    }
}

void SkCanvas::drawPicture(SkPicture& picture) {
    SkBaseDevice* device = this->getTopDevice();
    if (NULL != device) {
        // Canvas has to first give the device the opportunity to render
        // the picture itself.
        if (device->EXPERIMENTAL_drawPicture(picture)) {
            return; // the device has rendered the entire picture
        }
    }

    picture.draw(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkCanvas::LayerIter::LayerIter(SkCanvas* canvas, bool skipEmptyClips) {
    SK_COMPILE_ASSERT(sizeof(fStorage) >= sizeof(SkDrawIter), fStorage_too_small);

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
    if (NULL == paint) {
        paint = &fDefaultPaint;
    }
    return *paint;
}

const SkRegion& SkCanvas::LayerIter::clip() const { return fImpl->getClip(); }
int SkCanvas::LayerIter::x() const { return fImpl->getX(); }
int SkCanvas::LayerIter::y() const { return fImpl->getY(); }

///////////////////////////////////////////////////////////////////////////////

SkCanvas::ClipVisitor::~ClipVisitor() { }

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
        case kPMColor_SkColorType:
            break;
        default:
            return false;
    }

    return true;
}

SkCanvas* SkCanvas::NewRaster(const SkImageInfo& info) {
    if (!supported_for_raster_canvas(info)) {
        return NULL;
    }

    SkBitmap bitmap;
    if (!bitmap.allocPixels(info)) {
        return NULL;
    }

    // should this functionality be moved into allocPixels()?
    if (!bitmap.info().isOpaque()) {
        bitmap.eraseColor(0);
    }
    return SkNEW_ARGS(SkCanvas, (bitmap));
}

SkCanvas* SkCanvas::NewRasterDirect(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    if (!supported_for_raster_canvas(info)) {
        return NULL;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return NULL;
    }

    // should this functionality be moved into allocPixels()?
    if (!bitmap.info().isOpaque()) {
        bitmap.eraseColor(0);
    }
    return SkNEW_ARGS(SkCanvas, (bitmap));
}

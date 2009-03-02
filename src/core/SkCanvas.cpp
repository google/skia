/*
 * Copyright (C) 2006-2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkCanvas.h"
#include "SkBounder.h"
#include "SkDevice.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawLooper.h"
#include "SkPicture.h"
#include "SkScalarCompare.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include <new>

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

///////////////////////////////////////////////////////////////////////////////
// Helpers for computing fast bounds for quickReject tests

static SkCanvas::EdgeType paint2EdgeType(const SkPaint* paint) {
    return paint != NULL && paint->isAntiAlias() ?
            SkCanvas::kAA_EdgeType : SkCanvas::kBW_EdgeType;
}

///////////////////////////////////////////////////////////////////////////////

/*  This is the record we keep for each SkDevice that the user installs.
    The clip/matrix/proc are fields that reflect the top of the save/restore
    stack. Whenever the canvas changes, it marks a dirty flag, and then before
    these are used (assuming we're not on a layer) we rebuild these cache
    values: they reflect the top of the save stack, but translated and clipped
    by the device's XY offset and bitmap-bounds.
*/
struct DeviceCM {
    DeviceCM*           fNext;
    SkDevice*           fDevice;
    SkRegion            fClip;
    const SkMatrix*     fMatrix;
	SkPaint*			fPaint;	// may be null (in the future)
    int16_t             fX, fY; // relative to base matrix/clip

	DeviceCM(SkDevice* device, int x, int y, const SkPaint* paint)
            : fNext(NULL) {
        if (NULL != device) {
            device->ref();
            device->lockPixels();
        }
        fDevice = device;        
        fX = SkToS16(x);
        fY = SkToS16(y);
        fPaint = paint ? SkNEW_ARGS(SkPaint, (*paint)) : NULL;
	}

	~DeviceCM() {
        if (NULL != fDevice) {
            fDevice->unlockPixels();
            fDevice->unref();
        }
		SkDELETE(fPaint);
	}
    
    void updateMC(const SkMatrix& totalMatrix, const SkRegion& totalClip,
                  SkRegion* updateClip) {
        int x = fX;
        int y = fY;
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

        fClip.op(0, 0, width, height, SkRegion::kIntersect_Op);

        // intersect clip, but don't translate it (yet)
        
        if (updateClip) {
            updateClip->op(x, y, x + width, y + height,
                           SkRegion::kDifference_Op);
        }
        
        fDevice->setMatrixClip(*fMatrix, fClip);

#ifdef SK_DEBUG
        if (!fClip.isEmpty()) {
            SkIRect deviceR;
            deviceR.set(0, 0, width, height);
            SkASSERT(deviceR.contains(fClip.getBounds()));
        }
#endif
    }
    
    void translateClip() {
        if (fX | fY) {
            fClip.translate(fX, fY);
        }
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
    MCRec*          fNext;
    SkMatrix*       fMatrix;    // points to either fMatrixStorage or prev MCRec
    SkRegion*       fRegion;    // points to either fRegionStorage or prev MCRec
    SkDrawFilter*   fFilter;    // the current filter (or null)
    
    DeviceCM*   fLayer;
    /*  If there are any layers in the stack, this points to the top-most
        one that is at or below this level in the stack (so we know what
        bitmap/device to draw into from this level. This value is NOT
        reference counted, since the real owner is either our fLayer field,
        or a previous one in a lower level.)
    */
    DeviceCM*	fTopLayer;

    MCRec(const MCRec* prev, int flags) {
        if (NULL != prev) {
            if (flags & SkCanvas::kMatrix_SaveFlag) {
                fMatrixStorage = *prev->fMatrix;
                fMatrix = &fMatrixStorage;
            } else {
                fMatrix = prev->fMatrix;
            }
            
            if (flags & SkCanvas::kClip_SaveFlag) {
                fRegionStorage = *prev->fRegion;
                fRegion = &fRegionStorage;
            } else {
                fRegion = prev->fRegion;
            }

            fFilter = prev->fFilter;
            fFilter->safeRef();

            fTopLayer = prev->fTopLayer;
        } else {   // no prev
            fMatrixStorage.reset();
            
            fMatrix     = &fMatrixStorage;
            fRegion     = &fRegionStorage;
            fFilter     = NULL;
            fTopLayer   = NULL;
        }
        fLayer = NULL;

        // don't bother initializing fNext
        inc_rec();
    }
    ~MCRec() {
        fFilter->safeUnref();
        SkDELETE(fLayer);
        dec_rec();
    }
	
private:
    SkMatrix    fMatrixStorage;
    SkRegion    fRegionStorage;
};

class SkDrawIter : public SkDraw {
public:
    SkDrawIter(SkCanvas* canvas, bool skipEmptyClips = true) {
        fCanvas = canvas;
        canvas->updateDeviceCMCache();

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

        if (NULL != fCurrLayer) {
            const DeviceCM* rec = fCurrLayer;

            fMatrix = rec->fMatrix;
            fClip   = &rec->fClip;
            fDevice = rec->fDevice;
            fBitmap = &fDevice->accessBitmap(true);
            fLayerX = rec->fX;
            fLayerY = rec->fY;
            fPaint  = rec->fPaint;
            SkDEBUGCODE(this->validate();)

            fCurrLayer = rec->fNext;
            if (fBounder) {
                fBounder->setClip(fClip);
            }

            // fCurrLayer may be NULL now
            
            fCanvas->prepareForDeviceDraw(fDevice);
            return true;
        }
        return false;
    }
    
    int getX() const { return fLayerX; }
    int getY() const { return fLayerY; }
    SkDevice* getDevice() const { return fDevice; }
    const SkMatrix& getMatrix() const { return *fMatrix; }
    const SkRegion& getClip() const { return *fClip; }
    const SkPaint* getPaint() const { return fPaint; }
private:
    SkCanvas*       fCanvas;
    const DeviceCM* fCurrLayer;
    const SkPaint*  fPaint;     // May be null.
    int             fLayerX;
    int             fLayerY;
    SkBool8         fSkipEmptyClips;

    typedef SkDraw INHERITED;
};

/////////////////////////////////////////////////////////////////////////////

class AutoDrawLooper {
public:
    AutoDrawLooper(SkCanvas* canvas, const SkPaint& paint, SkDrawFilter::Type t)
            : fCanvas(canvas), fPaint((SkPaint*)&paint), fType(t) {
        if ((fLooper = paint.getLooper()) != NULL) {
            fLooper->init(canvas, (SkPaint*)&paint);
        } else {
            fOnce = true;
        }
        fFilter = canvas->getDrawFilter();
        fNeedFilterRestore = false;
    }

    ~AutoDrawLooper() {
        if (fNeedFilterRestore) {
            SkASSERT(fFilter);
            fFilter->restore(fCanvas, fPaint, fType);
        }
        if (NULL != fLooper) {
            fLooper->restore();
        }
    }
    
    bool next() {
        SkDrawFilter* filter = fFilter;

        // if we drew earlier with a filter, then we need to restore first
        if (fNeedFilterRestore) {
            SkASSERT(filter);
            filter->restore(fCanvas, fPaint, fType);
            fNeedFilterRestore = false;
        }
            
        bool result;
        
        if (NULL != fLooper) {
            result = fLooper->next();
        } else {
            result = fOnce;
            fOnce = false;
        }

        // if we're gonna draw, give the filter a chance to do its work
        if (result && NULL != filter) {
            fNeedFilterRestore = result = filter->filter(fCanvas, fPaint,
                                                         fType);
        }
        return result;
    }
    
private:
    SkDrawLooper*   fLooper;
    SkDrawFilter*   fFilter;
    SkCanvas*       fCanvas;
    SkPaint*        fPaint;
    SkDrawFilter::Type  fType;
    bool            fOnce;
    bool            fNeedFilterRestore;
    
};

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

#include "SkColorPriv.h"

class AutoValidator {
public:
    AutoValidator(SkDevice* device) : fDevice(device) {}
    ~AutoValidator() {
#ifdef SK_DEBUG
        const SkBitmap& bm = fDevice->accessBitmap(false);
        if (bm.config() == SkBitmap::kARGB_4444_Config) {
            for (int y = 0; y < bm.height(); y++) {
                const SkPMColor16* p = bm.getAddr16(0, y);
                for (int x = 0; x < bm.width(); x++) {
                    SkPMColor16 c = p[x];
                    SkPMColor16Assert(c);
                }
            }
        }
#endif
    }
private:
    SkDevice* fDevice;
};

////////// macros to place around the internal draw calls //////////////////

#define ITER_BEGIN(paint, type)                                     \
/*    AutoValidator   validator(fMCRec->fTopLayer->fDevice); */     \
    AutoDrawLooper  looper(this, paint, type);                      \
    while (looper.next()) {                                         \
        SkAutoBounderCommit ac(fBounder);                           \
        SkDrawIter          iter(this);
    
#define ITER_END    }

////////////////////////////////////////////////////////////////////////////

SkDevice* SkCanvas::init(SkDevice* device) {
    fBounder = NULL;
    fLocalBoundsCompareTypeDirty = true;

    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec(NULL, 0);

    fMCRec->fLayer = SkNEW_ARGS(DeviceCM, (NULL, 0, 0, NULL));
    fMCRec->fTopLayer = fMCRec->fLayer;
    fMCRec->fNext = NULL;

    return this->setDevice(device);
}

SkCanvas::SkCanvas(SkDevice* device)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    inc_canvas();

    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    inc_canvas();

    this->init(SkNEW_ARGS(SkDevice, (bitmap)))->unref();
}

SkCanvas::~SkCanvas() {
    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last
    this->internalRestore();    // restore the last, since we're going away

    fBounder->safeUnref();
    
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

///////////////////////////////////////////////////////////////////////////////

SkDevice* SkCanvas::getDevice() const {
    // return root device
    SkDeque::Iter   iter(fMCStack);
    MCRec*          rec = (MCRec*)iter.next();    
    SkASSERT(rec && rec->fLayer);
    return rec->fLayer->fDevice;
}

SkDevice* SkCanvas::setDevice(SkDevice* device) {
    // return root device
    SkDeque::Iter   iter(fMCStack);
    MCRec*          rec = (MCRec*)iter.next();    
    SkASSERT(rec && rec->fLayer);
    SkDevice*       rootDevice = rec->fLayer->fDevice;

    if (rootDevice == device) {
        return device;
    }
    
    /* Notify the devices that they are going in/out of scope, so they can do
       things like lock/unlock their pixels, etc.
    */
    if (device) {
        device->lockPixels();
    }
    if (rootDevice) {
        rootDevice->unlockPixels();
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

    if (NULL == device) {
        rec->fRegion->setEmpty();
        while ((rec = (MCRec*)iter.next()) != NULL) {
            (void)rec->fRegion->setEmpty();
        }
    } else {
        // compute our total bounds for all devices
        SkIRect bounds;
        
        bounds.set(0, 0, device->width(), device->height());

        // now jam our 1st clip to be bounds, and intersect the rest with that
        rec->fRegion->setRect(bounds);
        while ((rec = (MCRec*)iter.next()) != NULL) {
            (void)rec->fRegion->op(bounds, SkRegion::kIntersect_Op);
        }
    }
    return device;
}

SkDevice* SkCanvas::setBitmapDevice(const SkBitmap& bitmap) {
    SkDevice* device = this->setDevice(SkNEW_ARGS(SkDevice, (bitmap)));
    device->unref();
    return device;
}

//////////////////////////////////////////////////////////////////////////////

bool SkCanvas::getViewport(SkIPoint* size) const {
    return false;
}

bool SkCanvas::setViewport(int width, int height) {
    return false;
}

void SkCanvas::updateDeviceCMCache() {
    if (fDeviceCMDirty) {
        const SkMatrix& totalMatrix = this->getTotalMatrix();
        const SkRegion& totalClip = this->getTotalClip();
        DeviceCM*       layer = fMCRec->fTopLayer;
        
        if (NULL == layer->fNext) {   // only one layer
            layer->updateMC(totalMatrix, totalClip, NULL);
        } else {
            SkRegion clip;
            clip = totalClip;  // make a copy
            do {
                layer->updateMC(totalMatrix, clip, &clip);
            } while ((layer = layer->fNext) != NULL);
        }
        fDeviceCMDirty = false;
    }
}

void SkCanvas::prepareForDeviceDraw(SkDevice* device) {
    SkASSERT(device);
    device->gainFocus(this);
}

///////////////////////////////////////////////////////////////////////////////

int SkCanvas::internalSave(SaveFlags flags) {
    int saveCount = this->getSaveCount(); // record this before the actual save
    
    MCRec* newTop = (MCRec*)fMCStack.push_back();
    new (newTop) MCRec(fMCRec, flags);    // balanced in restore()
    
    newTop->fNext = fMCRec;
    fMCRec = newTop;
    
    return saveCount;
}

int SkCanvas::save(SaveFlags flags) {
    // call shared impl
    return this->internalSave(flags);
}

#define C32MASK (1 << SkBitmap::kARGB_8888_Config)
#define C16MASK (1 << SkBitmap::kRGB_565_Config)
#define C8MASK  (1 << SkBitmap::kA8_Config)

static SkBitmap::Config resolve_config(SkCanvas* canvas,
                                       const SkIRect& bounds,
                                       SkCanvas::SaveFlags flags,
                                       bool* isOpaque) {
    *isOpaque = (flags & SkCanvas::kHasAlphaLayer_SaveFlag) == 0;

#if 0
    // loop through and union all the configs we may draw into
    uint32_t configMask = 0;
    for (int i = canvas->countLayerDevices() - 1; i >= 0; --i)
    {
        SkDevice* device = canvas->getLayerDevice(i);
        if (device->intersects(bounds))
            configMask |= 1 << device->config();
    }

    // if the caller wants alpha or fullcolor, we can't return 565
    if (flags & (SkCanvas::kFullColorLayer_SaveFlag |
                 SkCanvas::kHasAlphaLayer_SaveFlag))
        configMask &= ~C16MASK;

    switch (configMask) {
    case C8MASK:    // if we only have A8, return that
        return SkBitmap::kA8_Config;

    case C16MASK:   // if we only have 565, return that
        return SkBitmap::kRGB_565_Config;

    default:
        return SkBitmap::kARGB_8888_Config; // default answer
    }
#else
    return SkBitmap::kARGB_8888_Config; // default answer
#endif
}

static bool bounds_affects_clip(SkCanvas::SaveFlags flags) {
    return (flags & SkCanvas::kClipToLayer_SaveFlag) != 0;
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                        SaveFlags flags) {
    // do this before we create the layer. We don't call the public save() since
    // that would invoke a possibly overridden virtual
    int count = this->internalSave(flags);

    fDeviceCMDirty = true;

    SkIRect         ir;
    const SkIRect&  clipBounds = this->getTotalClip().getBounds();

    if (NULL != bounds) {
        SkRect r;
        
        this->getTotalMatrix().mapRect(&r, *bounds);
        r.roundOut(&ir);
        // early exit if the layer's bounds are clipped out
        if (!ir.intersect(clipBounds)) {
            if (bounds_affects_clip(flags))
                fMCRec->fRegion->setEmpty();
            return count;
        }
    } else {    // no user bounds, so just use the clip
        ir = clipBounds;
    }

    // early exit if the clip is now empty
    if (bounds_affects_clip(flags) &&
        !fMCRec->fRegion->op(ir, SkRegion::kIntersect_Op)) {
        return count;
    }

    bool isOpaque;
    SkBitmap::Config config = resolve_config(this, ir, flags, &isOpaque);

    SkDevice* device = this->createDevice(config, ir.width(), ir.height(),
                                          isOpaque, true);
    DeviceCM* layer = SkNEW_ARGS(DeviceCM, (device, ir.fLeft, ir.fTop, paint));
    device->unref();

    layer->fNext = fMCRec->fTopLayer;
    fMCRec->fLayer = layer;
    fMCRec->fTopLayer = layer;    // this field is NOT an owner of layer

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

void SkCanvas::restore() {
    // check for underflow
    if (fMCStack.count() > 1) {
        this->internalRestore();
    }
}

void SkCanvas::internalRestore() {
    SkASSERT(fMCStack.count() != 0);

    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;

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
            this->drawDevice(layer->fDevice, layer->fX, layer->fY,
                             layer->fPaint);
            // reset this, since drawDevice will have set it to true
            fDeviceCMDirty = true;
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
    while (fMCStack.count() > count) {
        this->restore();
    }
}

/////////////////////////////////////////////////////////////////////////////

// can't draw it if its empty, or its too big for a fixed-point width or height
static bool reject_bitmap(const SkBitmap& bitmap) {
    return  bitmap.width() <= 0 || bitmap.height() <= 0 ||
            bitmap.width() > 32767 || bitmap.height() > 32767;
}

void SkCanvas::internalDrawBitmap(const SkBitmap& bitmap,
                                const SkMatrix& matrix, const SkPaint* paint) {
    if (reject_bitmap(bitmap)) {
        return;
    }

    if (NULL == paint) {
        SkPaint tmpPaint;
        this->commonDrawBitmap(bitmap, matrix, tmpPaint);
    } else {
        this->commonDrawBitmap(bitmap, matrix, *paint);
    }
}

void SkCanvas::drawDevice(SkDevice* device, int x, int y,
                          const SkPaint* paint) {
    SkPaint tmp;
    if (NULL == paint) {
        tmp.setDither(true);
        paint = &tmp;
    }
    
    ITER_BEGIN(*paint, SkDrawFilter::kBitmap_Type)
    while (iter.next()) {
        iter.fDevice->drawDevice(iter, device, x - iter.getX(), y - iter.getY(),
                                 *paint);
    }
    ITER_END
}

/////////////////////////////////////////////////////////////////////////////

bool SkCanvas::translate(SkScalar dx, SkScalar dy) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;
    return fMCRec->fMatrix->preTranslate(dx, dy);
}

bool SkCanvas::scale(SkScalar sx, SkScalar sy) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;
    return fMCRec->fMatrix->preScale(sx, sy);
}

bool SkCanvas::rotate(SkScalar degrees) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;
    return fMCRec->fMatrix->preRotate(degrees);
}

bool SkCanvas::skew(SkScalar sx, SkScalar sy) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;
    return fMCRec->fMatrix->preSkew(sx, sy);
}

bool SkCanvas::concat(const SkMatrix& matrix) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;
    return fMCRec->fMatrix->preConcat(matrix);
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;
    *fMCRec->fMatrix = matrix;
}

// this is not virtual, so it must call a virtual method so that subclasses
// will see its action
void SkCanvas::resetMatrix() {
    SkMatrix matrix;
    
    matrix.reset();
    this->setMatrix(matrix);
}

//////////////////////////////////////////////////////////////////////////////

bool SkCanvas::clipRect(const SkRect& rect, SkRegion::Op op) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;

    if (fMCRec->fMatrix->rectStaysRect()) {
        // for these simpler matrices, we can stay a rect ever after applying
        // the matrix. This means we don't have to a) make a path, and b) tell
        // the region code to scan-convert the path, only to discover that it
        // is really just a rect.
        SkRect      r;
        SkIRect     ir;

        fMCRec->fMatrix->mapRect(&r, rect);
        r.round(&ir);
        return fMCRec->fRegion->op(ir, op);
    } else {
        // since we're rotate or some such thing, we convert the rect to a path
        // and clip against that, since it can handle any matrix. However, to
        // avoid recursion in the case where we are subclassed (e.g. Pictures)
        // we explicitly call "our" version of clipPath.
        SkPath  path;

        path.addRect(rect);
        return this->SkCanvas::clipPath(path, op);
    }
}

bool SkCanvas::clipPath(const SkPath& path, SkRegion::Op op) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;

    SkPath devPath;
    path.transform(*fMCRec->fMatrix, &devPath);

    if (SkRegion::kIntersect_Op == op) {
        return fMCRec->fRegion->setPath(devPath, *fMCRec->fRegion);
    } else {
        SkRegion base;
        const SkBitmap& bm = this->getDevice()->accessBitmap(false);
        base.setRect(0, 0, bm.width(), bm.height());
        
        if (SkRegion::kReplace_Op == op) {
            return fMCRec->fRegion->setPath(devPath, base);
        } else {
            SkRegion rgn;
            rgn.setPath(devPath, base);
            return fMCRec->fRegion->op(rgn, op);
        }
    }
}

bool SkCanvas::clipRegion(const SkRegion& rgn, SkRegion::Op op) {
    fDeviceCMDirty = true;
    fLocalBoundsCompareTypeDirty = true;

    return fMCRec->fRegion->op(rgn, op);
}

void SkCanvas::computeLocalClipBoundsCompareType() const {
    SkRect r;
    
    if (!this->getClipBounds(&r, kAA_EdgeType)) {
        fLocalBoundsCompareType.setEmpty();
    } else {
        fLocalBoundsCompareType.set(SkScalarToCompareType(r.fLeft),
                                    SkScalarToCompareType(r.fTop),
                                    SkScalarToCompareType(r.fRight),
                                    SkScalarToCompareType(r.fBottom));
    }
}

bool SkCanvas::quickReject(const SkRect& rect, EdgeType) const {
    /*  current impl ignores edgetype, and relies on
        getLocalClipBoundsCompareType(), which always returns a value assuming
        antialiasing (worst case)
     */

    if (fMCRec->fRegion->isEmpty()) {
        return true;
    }
    
    // check for empty user rect (horizontal)
    SkScalarCompareType userL = SkScalarToCompareType(rect.fLeft);
    SkScalarCompareType userR = SkScalarToCompareType(rect.fRight);
    if (userL >= userR) {
        return true;
    }

    // check for empty user rect (vertical)
    SkScalarCompareType userT = SkScalarToCompareType(rect.fTop);
    SkScalarCompareType userB = SkScalarToCompareType(rect.fBottom);
    if (userT >= userB) {
        return true;
    }
    
    // check if we are completely outside of the local clip bounds
    const SkRectCompareType& clipR = this->getLocalClipBoundsCompareType();
    return  userL >= clipR.fRight || userT >= clipR.fBottom ||
            userR <= clipR.fLeft  || userB <= clipR.fTop;
}

bool SkCanvas::quickReject(const SkPath& path, EdgeType et) const {
    if (fMCRec->fRegion->isEmpty() || path.isEmpty()) {
        return true;
    }

    if (fMCRec->fMatrix->rectStaysRect()) {
        SkRect  r;
        path.computeBounds(&r, SkPath::kFast_BoundsType);
        return this->quickReject(r, et);
    }

    SkPath      dstPath;
    SkRect      r;
    SkIRect     ir;

    path.transform(*fMCRec->fMatrix, &dstPath);
    dstPath.computeBounds(&r, SkPath::kFast_BoundsType);
    r.round(&ir);
    if (kAA_EdgeType == et) {
        ir.inset(-1, -1);
    }
    return fMCRec->fRegion->quickReject(ir);
}

bool SkCanvas::quickRejectY(SkScalar top, SkScalar bottom, EdgeType et) const {
    /*  current impl ignores edgetype, and relies on
        getLocalClipBoundsCompareType(), which always returns a value assuming
        antialiasing (worst case)
     */

    if (fMCRec->fRegion->isEmpty()) {
        return true;
    }
    
    SkScalarCompareType userT = SkScalarAs2sCompliment(top);
    SkScalarCompareType userB = SkScalarAs2sCompliment(bottom);
    
    // check for invalid user Y coordinates (i.e. empty)
    if (userT >= userB) {
        return true;
    }
    
    // check if we are above or below the local clip bounds
    const SkRectCompareType& clipR = this->getLocalClipBoundsCompareType();
    return userT >= clipR.fBottom || userB <= clipR.fTop;
}

bool SkCanvas::getClipBounds(SkRect* bounds, EdgeType et) const {
    const SkRegion& clip = *fMCRec->fRegion;
    if (clip.isEmpty()) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    SkMatrix inverse;
    // if we can't invert the CTM, we can't return local clip bounds
    if (!fMCRec->fMatrix->invert(&inverse)) {
        return false;
    }

    if (NULL != bounds) {
        SkRect   r;
        // get the clip's bounds
        const SkIRect& ibounds = clip.getBounds();
        // adjust it outwards if we are antialiasing
        int inset = (kAA_EdgeType == et);
        r.iset(ibounds.fLeft - inset,  ibounds.fTop - inset,
               ibounds.fRight + inset, ibounds.fBottom + inset);
        
        // invert into local coordinates
        inverse.mapRect(bounds, r);
    }
    return true;
}

const SkMatrix& SkCanvas::getTotalMatrix() const {
    return *fMCRec->fMatrix;
}

const SkRegion& SkCanvas::getTotalClip() const {
    return *fMCRec->fRegion;
}

///////////////////////////////////////////////////////////////////////////////

SkDevice* SkCanvas::createDevice(SkBitmap::Config config, int width,
                                 int height, bool isOpaque, bool isForLayer) {
    SkBitmap bitmap;
    
    bitmap.setConfig(config, width, height);
    bitmap.setIsOpaque(isOpaque);

    // should this happen in the device subclass?
    bitmap.allocPixels();   
    if (!bitmap.isOpaque()) {
        bitmap.eraseARGB(0, 0, 0, 0);
    }

    return SkNEW_ARGS(SkDevice, (bitmap));
}

//////////////////////////////////////////////////////////////////////////////
//  These are the virtual drawing methods
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawPaint(const SkPaint& paint) {
    ITER_BEGIN(paint, SkDrawFilter::kPaint_Type)

    while (iter.next()) {
        iter.fDevice->drawPaint(iter, paint);
    }

    ITER_END
}

void SkCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                          const SkPaint& paint) {
    if ((long)count <= 0) {
        return;
    }

    SkASSERT(pts != NULL);

    ITER_BEGIN(paint, SkDrawFilter::kPoint_Type)
    
    while (iter.next()) {
        iter.fDevice->drawPoints(iter, mode, count, pts, paint);
    }
    
    ITER_END
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(r, &storage),
                              paint2EdgeType(&paint))) {
            return;
        }
    }
        
    ITER_BEGIN(paint, SkDrawFilter::kRect_Type)

    while (iter.next()) {
        iter.fDevice->drawRect(iter, r, paint);
    }

    ITER_END
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    if (paint.canComputeFastBounds()) {
        SkRect r;
        path.computeBounds(&r, SkPath::kFast_BoundsType);
        if (this->quickReject(paint.computeFastBounds(r, &r),
                              paint2EdgeType(&paint))) {
            return;
        }
    }

    ITER_BEGIN(paint, SkDrawFilter::kPath_Type)

    while (iter.next()) {
        iter.fDevice->drawPath(iter, path, paint);
    }

    ITER_END
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                          const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)

    if (NULL == paint || (paint->getMaskFilter() == NULL)) {
        SkRect fastBounds;
        fastBounds.set(x, y,
                       x + SkIntToScalar(bitmap.width()),
                       y + SkIntToScalar(bitmap.height()));
        if (this->quickReject(fastBounds, paint2EdgeType(paint))) {
            return;
        }
    }
        
    SkMatrix matrix;
    matrix.setTranslate(x, y);
    this->internalDrawBitmap(bitmap, matrix, paint);
}

void SkCanvas::drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                              const SkRect& dst, const SkPaint* paint) {
    if (bitmap.width() == 0 || bitmap.height() == 0 || dst.isEmpty()) {
        return;
    }
    
    // do this now, to avoid the cost of calling extract for RLE bitmaps
    if (this->quickReject(dst, paint2EdgeType(paint))) {
        return;
    }
    
    SkBitmap        tmp;    // storage if we need a subset of bitmap
    const SkBitmap* bitmapPtr = &bitmap;

    if (NULL != src) {
        if (!bitmap.extractSubset(&tmp, *src)) {
            return;     // extraction failed
        }
        bitmapPtr = &tmp;
    }
    
    SkScalar width = SkIntToScalar(bitmapPtr->width());
    SkScalar height = SkIntToScalar(bitmapPtr->height());    
    SkMatrix matrix;

    if (dst.width() == width && dst.height() == height) {
        matrix.setTranslate(dst.fLeft, dst.fTop);
    } else {
        SkRect tmpSrc;
        tmpSrc.set(0, 0, width, height);
        matrix.setRectToRect(tmpSrc, dst, SkMatrix::kFill_ScaleToFit);
    }
    this->internalDrawBitmap(*bitmapPtr, matrix, paint);
}

void SkCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                                const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)
    this->internalDrawBitmap(bitmap, matrix, paint);
}

void SkCanvas::commonDrawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
                                const SkPaint& paint) {
    SkDEBUGCODE(bitmap.validate();)

    ITER_BEGIN(paint, SkDrawFilter::kBitmap_Type)

    while (iter.next()) {
        iter.fDevice->drawBitmap(iter, bitmap, matrix, paint);
    }

    ITER_END
}

void SkCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                          const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)
    
    if (reject_bitmap(bitmap)) {
        return;
    }
    
    SkPaint tmp;
    if (NULL == paint) {
        paint = &tmp;
    }
    
    ITER_BEGIN(*paint, SkDrawFilter::kBitmap_Type)
    
    while (iter.next()) {
        iter.fDevice->drawSprite(iter, bitmap, x - iter.getX(), y - iter.getY(),
                                 *paint);
    }
    ITER_END
}

void SkCanvas::drawText(const void* text, size_t byteLength,
                        SkScalar x, SkScalar y, const SkPaint& paint) {
    ITER_BEGIN(paint, SkDrawFilter::kText_Type)

    while (iter.next()) {
        iter.fDevice->drawText(iter, text, byteLength, x, y, paint);
    }

    ITER_END
}

void SkCanvas::drawPosText(const void* text, size_t byteLength,
                           const SkPoint pos[], const SkPaint& paint) {
    ITER_BEGIN(paint, SkDrawFilter::kText_Type)
    
    while (iter.next()) {
        iter.fDevice->drawPosText(iter, text, byteLength, &pos->fX, 0, 2,
                                  paint);
    }
    
    ITER_END
}

void SkCanvas::drawPosTextH(const void* text, size_t byteLength,
                            const SkScalar xpos[], SkScalar constY,
                            const SkPaint& paint) {
    ITER_BEGIN(paint, SkDrawFilter::kText_Type)
    
    while (iter.next()) {
        iter.fDevice->drawPosText(iter, text, byteLength, xpos, constY, 1,
                                  paint);
    }
    
    ITER_END
}

void SkCanvas::drawTextOnPath(const void* text, size_t byteLength,
                              const SkPath& path, const SkMatrix* matrix,
                              const SkPaint& paint) {
    ITER_BEGIN(paint, SkDrawFilter::kText_Type)

    while (iter.next()) {
        iter.fDevice->drawTextOnPath(iter, text, byteLength, path,
                                     matrix, paint);
    }

    ITER_END
}

void SkCanvas::drawVertices(VertexMode vmode, int vertexCount,
                            const SkPoint verts[], const SkPoint texs[],
                            const SkColor colors[], SkXfermode* xmode,
                            const uint16_t indices[], int indexCount,
                            const SkPaint& paint) {
    ITER_BEGIN(paint, SkDrawFilter::kPath_Type)
    
    while (iter.next()) {
        iter.fDevice->drawVertices(iter, vmode, vertexCount, verts, texs,
                                   colors, xmode, indices, indexCount, paint);
    }
    
    ITER_END
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b,
                        SkPorterDuff::Mode mode) {
    SkPaint paint;

    paint.setARGB(a, r, g, b);
    if (SkPorterDuff::kSrcOver_Mode != mode) {
        paint.setPorterDuffXfermode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawColor(SkColor c, SkPorterDuff::Mode mode) {
    SkPaint paint;

    paint.setColor(c);
    if (SkPorterDuff::kSrcOver_Mode != mode) {
        paint.setPorterDuffXfermode(mode);
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
    
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(r, &storage),
                              paint2EdgeType(&paint))) {
            return;
        }
    }
    
    SkPath  path;
    path.addOval(r);
    this->drawPath(path, paint);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry,
                             const SkPaint& paint) {
    if (rx > 0 && ry > 0) {
        if (paint.canComputeFastBounds()) {
            SkRect storage;
            if (this->quickReject(paint.computeFastBounds(r, &storage),
                                  paint2EdgeType(&paint))) {
                return;
            }
        }

        SkPath  path;
        path.addRoundRect(r, rx, ry, SkPath::kCW_Direction);
        this->drawPath(path, paint);
    } else {
        this->drawRect(r, paint);
    }
}

void SkCanvas::drawOval(const SkRect& oval, const SkPaint& paint) {
    if (paint.canComputeFastBounds()) {
        SkRect storage;
        if (this->quickReject(paint.computeFastBounds(oval, &storage),
                              paint2EdgeType(&paint))) {
            return;
        }
    }

    SkPath  path;
    path.addOval(oval);
    this->drawPath(path, paint);
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

void SkCanvas::drawPicture(SkPicture& picture) {
    int saveCount = save();
    picture.draw(this);
    restoreToCount(saveCount);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkCanvas::LayerIter::LayerIter(SkCanvas* canvas, bool skipEmptyClips) {
    // need COMPILE_TIME_ASSERT
    SkASSERT(sizeof(fStorage) >= sizeof(SkDrawIter));

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

SkDevice* SkCanvas::LayerIter::device() const {
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


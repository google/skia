
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkBitmapDevice.h"
#include "SkBitmapHeap.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDrawLooper.h"
#include "SkGPipe.h"
#include "SkGPipePriv.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkWriteBuffer.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkPictureFlat.h"
#include "SkRasterizer.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTypeface.h"
#include "SkWriter32.h"

enum {
    kSizeOfFlatRRect = sizeof(SkRect) + 4 * sizeof(SkVector)
};

static bool isCrossProcess(uint32_t flags) {
    return SkToBool(flags & SkGPipeWriter::kCrossProcess_Flag);
}

static SkFlattenable* get_paintflat(const SkPaint& paint, unsigned paintFlat) {
    SkASSERT(paintFlat < kCount_PaintFlats);
    switch (paintFlat) {
        case kColorFilter_PaintFlat:    return paint.getColorFilter();
        case kDrawLooper_PaintFlat:     return paint.getLooper();
        case kMaskFilter_PaintFlat:     return paint.getMaskFilter();
        case kPathEffect_PaintFlat:     return paint.getPathEffect();
        case kRasterizer_PaintFlat:     return paint.getRasterizer();
        case kShader_PaintFlat:         return paint.getShader();
        case kImageFilter_PaintFlat:    return paint.getImageFilter();
        case kXfermode_PaintFlat:       return paint.getXfermode();
    }
    SkDEBUGFAIL("never gets here");
    return NULL;
}

static size_t writeTypeface(SkWriter32* writer, SkTypeface* typeface) {
    SkASSERT(typeface);
    SkDynamicMemoryWStream stream;
    typeface->serialize(&stream);
    size_t size = stream.getOffset();
    if (writer) {
        writer->write32(size);
        SkAutoDataUnref data(stream.copyToData());
        writer->writePad(data->data(), size);
    }
    return 4 + SkAlign4(size);
}

///////////////////////////////////////////////////////////////////////////////

class FlattenableHeap : public SkFlatController {
public:
    FlattenableHeap(int numFlatsToKeep, SkNamedFactorySet* fset, bool isCrossProcess)
    : INHERITED(isCrossProcess ? SkWriteBuffer::kCrossProcess_Flag : 0)
    , fNumFlatsToKeep(numFlatsToKeep) {
        SkASSERT((isCrossProcess && fset != NULL) || (!isCrossProcess && NULL == fset));
        if (isCrossProcess) {
            this->setNamedFactorySet(fset);
        }
    }

    ~FlattenableHeap() {
        fPointers.freeAll();
    }

    virtual void* allocThrow(size_t bytes) SK_OVERRIDE;

    virtual void unalloc(void* ptr) SK_OVERRIDE;

    void setBitmapStorage(SkBitmapHeap* heap) {
        this->setBitmapHeap(heap);
    }

    const SkFlatData* flatToReplace() const;

    // Mark an SkFlatData as one that should not be returned by flatToReplace.
    // Takes the result of SkFlatData::index() as its parameter.
    void markFlatForKeeping(int index) {
        *fFlatsThatMustBeKept.append() = index;
    }

    void markAllFlatsSafeToDelete() {
        fFlatsThatMustBeKept.reset();
    }

private:
    // Keep track of the indices (i.e. the result of SkFlatData::index()) of
    // flats that must be kept, since they are on the current paint.
    SkTDArray<int>   fFlatsThatMustBeKept;
    SkTDArray<void*> fPointers;
    const int        fNumFlatsToKeep;

    typedef SkFlatController INHERITED;
};

void FlattenableHeap::unalloc(void* ptr) {
    int indexToRemove = fPointers.rfind(ptr);
    if (indexToRemove >= 0) {
        sk_free(ptr);
        fPointers.remove(indexToRemove);
    }
}

void* FlattenableHeap::allocThrow(size_t bytes) {
    void* ptr = sk_malloc_throw(bytes);
    *fPointers.append() = ptr;
    return ptr;
}

const SkFlatData* FlattenableHeap::flatToReplace() const {
    // First, determine whether we should replace one.
    if (fPointers.count() > fNumFlatsToKeep) {
        // Look through the flattenable heap.
        // TODO: Return the LRU flat.
        for (int i = 0; i < fPointers.count(); i++) {
            SkFlatData* potential = (SkFlatData*)fPointers[i];
            // Make sure that it is not one that must be kept.
            bool mustKeep = false;
            for (int j = 0; j < fFlatsThatMustBeKept.count(); j++) {
                if (potential->index() == fFlatsThatMustBeKept[j]) {
                    mustKeep = true;
                    break;
                }
            }
            if (!mustKeep) {
                return potential;
            }
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

struct SkFlattenableTraits {
    static void Flatten(SkWriteBuffer& buffer, const SkFlattenable& flattenable) {
        buffer.writeFlattenable(&flattenable);
    }
    // No need to define unflatten if we never call it.
};
typedef SkFlatDictionary<SkFlattenable, SkFlattenableTraits> FlatDictionary;

///////////////////////////////////////////////////////////////////////////////

/**
 * If SkBitmaps are to be flattened to send to the reader, this class is
 * provided to the SkBitmapHeap to tell the SkGPipeCanvas to do so.
 */
class BitmapShuttle : public SkBitmapHeap::ExternalStorage {
public:
    BitmapShuttle(SkGPipeCanvas*);

    ~BitmapShuttle();

    virtual bool insert(const SkBitmap& bitmap, int32_t slot) SK_OVERRIDE;

    /**
     *  Remove the SkGPipeCanvas used for insertion. After this, calls to
     *  insert will crash.
     */
    void removeCanvas();

private:
    SkGPipeCanvas*    fCanvas;
};

///////////////////////////////////////////////////////////////////////////////

class SkGPipeCanvas : public SkCanvas {
public:
    SkGPipeCanvas(SkGPipeController*, SkWriter32*, uint32_t flags,
                  uint32_t width, uint32_t height);
    virtual ~SkGPipeCanvas();

    /**
     *  Called when nothing else is to be written to the stream. Any repeated
     *  calls are ignored.
     *
     *  @param notifyReaders Whether to send a message to the reader(s) that
     *      the writer is through sending commands. Should generally be true,
     *      unless there is an error which prevents further messages from
     *      being sent.
     */
    void finish(bool notifyReaders) {
        if (fDone) {
            return;
        }
        if (notifyReaders && this->needOpBytes()) {
            this->writeOp(kDone_DrawOp);
            this->doNotify();
        }
        if (shouldFlattenBitmaps(fFlags)) {
            // The following circular references exist:
            // fFlattenableHeap -> fWriteBuffer -> fBitmapStorage -> fExternalStorage -> fCanvas
            // fBitmapHeap -> fExternalStorage -> fCanvas
            // fFlattenableHeap -> fBitmapStorage -> fExternalStorage -> fCanvas

            // Break them all by destroying the final link to this SkGPipeCanvas.
            fBitmapShuttle->removeCanvas();
        }
        fDone = true;
    }

    void flushRecording(bool detachCurrentBlock);
    size_t freeMemoryIfPossible(size_t bytesToFree);

    size_t storageAllocatedForRecording() {
        return (NULL == fBitmapHeap) ? 0 : fBitmapHeap->bytesAllocated();
    }

    // overrides from SkCanvas
    virtual bool isDrawingToLayer() const SK_OVERRIDE;
    virtual void clear(SkColor) SK_OVERRIDE;
    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode, size_t count, const SkPoint pts[],
                            const SkPaint&) SK_OVERRIDE;
    virtual void drawOval(const SkRect&, const SkPaint&) SK_OVERRIDE;
    virtual void drawRect(const SkRect& rect, const SkPaint&) SK_OVERRIDE;
    virtual void drawRRect(const SkRRect&, const SkPaint&) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint&) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapRectToRect(const SkBitmap&, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                  const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap&, int left, int top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint&) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                            const SkPath& path, const SkMatrix* matrix,
                                const SkPaint&) SK_OVERRIDE;
    virtual void drawPicture(SkPicture& picture) SK_OVERRIDE;
    virtual void drawVertices(VertexMode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode*,
                          const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawData(const void*, size_t) SK_OVERRIDE;
    virtual void beginCommentGroup(const char* description) SK_OVERRIDE;
    virtual void addComment(const char* kywd, const char* value) SK_OVERRIDE;
    virtual void endCommentGroup() SK_OVERRIDE;

    /**
     * Flatten an SkBitmap to send to the reader, where it will be referenced
     * according to slot.
     */
    bool shuttleBitmap(const SkBitmap&, int32_t slot);

protected:
    virtual void willSave(SaveFlags) SK_OVERRIDE;
    virtual SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) SK_OVERRIDE;
    virtual void willRestore() SK_OVERRIDE;

    virtual void didTranslate(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didScale(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didRotate(SkScalar) SK_OVERRIDE;
    virtual void didSkew(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didConcat(const SkMatrix&) SK_OVERRIDE;
    virtual void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    virtual void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

private:
    enum {
        kNoSaveLayer = -1,
    };
    SkNamedFactorySet* fFactorySet;
    int                fFirstSaveLayerStackLevel;
    SkBitmapHeap*      fBitmapHeap;
    SkGPipeController* fController;
    SkWriter32&        fWriter;
    size_t             fBlockSize; // amount allocated for writer
    size_t             fBytesNotified;
    bool               fDone;
    const uint32_t     fFlags;

    SkRefCntSet        fTypefaceSet;

    uint32_t getTypefaceID(SkTypeface*);

    inline void writeOp(DrawOps op, unsigned flags, unsigned data) {
        fWriter.write32(DrawOp_packOpFlagData(op, flags, data));
    }

    inline void writeOp(DrawOps op) {
        fWriter.write32(DrawOp_packOpFlagData(op, 0, 0));
    }

    bool needOpBytes(size_t size = 0);

    inline void doNotify() {
        if (!fDone) {
            size_t bytes = fWriter.bytesWritten() - fBytesNotified;
            if (bytes > 0) {
                fController->notifyWritten(bytes);
                fBytesNotified += bytes;
            }
        }
    }

    // Should be called after any calls to an SkFlatDictionary::findAndReplace
    // if a new SkFlatData was added when in cross process mode
    void flattenFactoryNames();

    FlattenableHeap             fFlattenableHeap;
    FlatDictionary              fFlatDictionary;
    SkAutoTUnref<BitmapShuttle> fBitmapShuttle;
    int                         fCurrFlatIndex[kCount_PaintFlats];

    int flattenToIndex(SkFlattenable* obj, PaintFlats);

    // Common code used by drawBitmap*. Behaves differently depending on the
    // type of SkBitmapHeap being used, which is determined by the flags used.
    bool commonDrawBitmap(const SkBitmap& bm, DrawOps op, unsigned flags,
                          size_t opBytesNeeded, const SkPaint* paint);

    SkPaint fPaint;
    void writePaint(const SkPaint&);

    class AutoPipeNotify {
    public:
        AutoPipeNotify(SkGPipeCanvas* canvas) : fCanvas(canvas) {}
        ~AutoPipeNotify() { fCanvas->doNotify(); }
    private:
        SkGPipeCanvas* fCanvas;
    };
    friend class AutoPipeNotify;

    typedef SkCanvas INHERITED;
};

void SkGPipeCanvas::flattenFactoryNames() {
    const char* name;
    while ((name = fFactorySet->getNextAddedFactoryName()) != NULL) {
        size_t len = strlen(name);
        if (this->needOpBytes(len)) {
            this->writeOp(kDef_Factory_DrawOp);
            fWriter.writeString(name, len);
        }
    }
}

bool SkGPipeCanvas::shuttleBitmap(const SkBitmap& bm, int32_t slot) {
    SkASSERT(shouldFlattenBitmaps(fFlags));
    SkWriteBuffer buffer;
    buffer.setNamedFactoryRecorder(fFactorySet);
    buffer.writeBitmap(bm);
    this->flattenFactoryNames();
    uint32_t size = buffer.bytesWritten();
    if (this->needOpBytes(size)) {
        this->writeOp(kDef_Bitmap_DrawOp, 0, slot);
        void* dst = static_cast<void*>(fWriter.reserve(size));
        buffer.writeToMemory(dst);
        return true;
    }
    return false;
}

// return 0 for NULL (or unflattenable obj), or index-base-1
// return ~(index-base-1) if an old flattenable was replaced
int SkGPipeCanvas::flattenToIndex(SkFlattenable* obj, PaintFlats paintflat) {
    SkASSERT(!fDone && fBitmapHeap != NULL);
    if (NULL == obj) {
        return 0;
    }

    fBitmapHeap->deferAddingOwners();
    bool added, replaced;
    const SkFlatData* flat = fFlatDictionary.findAndReplace(*obj, fFlattenableHeap.flatToReplace(),
                                                            &added, &replaced);
    fBitmapHeap->endAddingOwnersDeferral(added);
    int index = flat->index();
    if (added) {
        if (isCrossProcess(fFlags)) {
            this->flattenFactoryNames();
        }
        size_t flatSize = flat->flatSize();
        if (this->needOpBytes(flatSize)) {
            this->writeOp(kDef_Flattenable_DrawOp, paintflat, index);
            fWriter.write(flat->data(), flatSize);
        }
    }
    if (replaced) {
        index = ~index;
    }
    return index;
}

///////////////////////////////////////////////////////////////////////////////

#define MIN_BLOCK_SIZE  (16 * 1024)
#define BITMAPS_TO_KEEP 5
#define FLATTENABLES_TO_KEEP 10

SkGPipeCanvas::SkGPipeCanvas(SkGPipeController* controller,
                             SkWriter32* writer, uint32_t flags,
                             uint32_t width, uint32_t height)
    : SkCanvas(width, height)
    , fFactorySet(isCrossProcess(flags) ? SkNEW(SkNamedFactorySet) : NULL)
    , fWriter(*writer)
    , fFlags(flags)
    , fFlattenableHeap(FLATTENABLES_TO_KEEP, fFactorySet, isCrossProcess(flags))
    , fFlatDictionary(&fFlattenableHeap)
{
    fController = controller;
    fDone = false;
    fBlockSize = 0; // need first block from controller
    fBytesNotified = 0;
    fFirstSaveLayerStackLevel = kNoSaveLayer;
    sk_bzero(fCurrFlatIndex, sizeof(fCurrFlatIndex));

    // Tell the reader the appropriate flags to use.
    if (this->needOpBytes()) {
        this->writeOp(kReportFlags_DrawOp, fFlags, 0);
    }

    if (shouldFlattenBitmaps(flags)) {
        fBitmapShuttle.reset(SkNEW_ARGS(BitmapShuttle, (this)));
        fBitmapHeap = SkNEW_ARGS(SkBitmapHeap, (fBitmapShuttle.get(), BITMAPS_TO_KEEP));
    } else {
        fBitmapHeap = SkNEW_ARGS(SkBitmapHeap,
                                 (BITMAPS_TO_KEEP, controller->numberOfReaders()));
        if (this->needOpBytes(sizeof(void*))) {
            this->writeOp(kShareBitmapHeap_DrawOp);
            fWriter.writePtr(static_cast<void*>(fBitmapHeap));
        }
    }
    fFlattenableHeap.setBitmapStorage(fBitmapHeap);
    this->doNotify();
}

SkGPipeCanvas::~SkGPipeCanvas() {
    this->finish(true);
    SkSafeUnref(fFactorySet);
    SkSafeUnref(fBitmapHeap);
}

bool SkGPipeCanvas::needOpBytes(size_t needed) {
    if (fDone) {
        return false;
    }

    needed += 4;  // size of DrawOp atom
    if (fWriter.bytesWritten() + needed > fBlockSize) {
        // Before we wipe out any data that has already been written, read it
        // out.
        this->doNotify();
        size_t blockSize = SkMax32(MIN_BLOCK_SIZE, needed);
        void* block = fController->requestBlock(blockSize, &fBlockSize);
        if (NULL == block) {
            // Do not notify the readers, which would call this function again.
            this->finish(false);
            return false;
        }
        SkASSERT(SkIsAlign4(fBlockSize));
        fWriter.reset(block, fBlockSize);
        fBytesNotified = 0;
    }
    return true;
}

uint32_t SkGPipeCanvas::getTypefaceID(SkTypeface* face) {
    uint32_t id = 0; // 0 means default/null typeface
    if (face) {
        id = fTypefaceSet.find(face);
        if (0 == id) {
            id = fTypefaceSet.add(face);
            size_t size = writeTypeface(NULL, face);
            if (this->needOpBytes(size)) {
                this->writeOp(kDef_Typeface_DrawOp);
                writeTypeface(&fWriter, face);
            }
        }
    }
    return id;
}

///////////////////////////////////////////////////////////////////////////////

#define NOTIFY_SETUP(canvas)    \
    AutoPipeNotify apn(canvas)

void SkGPipeCanvas::willSave(SaveFlags flags) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kSave_DrawOp, 0, flags);
    }

    this->INHERITED::willSave(flags);
}

SkCanvas::SaveLayerStrategy SkGPipeCanvas::willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                                         SaveFlags saveFlags) {
    NOTIFY_SETUP(this);
    size_t size = 0;
    unsigned opFlags = 0;

    if (bounds) {
        opFlags |= kSaveLayer_HasBounds_DrawOpFlag;
        size += sizeof(SkRect);
    }
    if (paint) {
        opFlags |= kSaveLayer_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }

    if (this->needOpBytes(size)) {
        this->writeOp(kSaveLayer_DrawOp, opFlags, saveFlags);
        if (bounds) {
            fWriter.writeRect(*bounds);
        }
    }

    if (kNoSaveLayer == fFirstSaveLayerStackLevel){
        fFirstSaveLayerStackLevel = this->getSaveCount();
    }

    this->INHERITED::willSaveLayer(bounds, paint, saveFlags);
    // we don't create a layer
    return kNoLayer_SaveLayerStrategy;
}

void SkGPipeCanvas::willRestore() {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kRestore_DrawOp);
    }

    if (this->getSaveCount() - 1 == fFirstSaveLayerStackLevel){
        fFirstSaveLayerStackLevel = kNoSaveLayer;
    }

    this->INHERITED::willRestore();
}

bool SkGPipeCanvas::isDrawingToLayer() const {
    return kNoSaveLayer != fFirstSaveLayerStackLevel;
}

void SkGPipeCanvas::didTranslate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kTranslate_DrawOp);
            fWriter.writeScalar(dx);
            fWriter.writeScalar(dy);
        }
    }
    this->INHERITED::didTranslate(dx, dy);
}

void SkGPipeCanvas::didScale(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kScale_DrawOp);
            fWriter.writeScalar(sx);
            fWriter.writeScalar(sy);
        }
    }
    this->INHERITED::didScale(sx, sy);
}

void SkGPipeCanvas::didRotate(SkScalar degrees) {
    if (degrees) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(sizeof(SkScalar))) {
            this->writeOp(kRotate_DrawOp);
            fWriter.writeScalar(degrees);
        }
    }
    this->INHERITED::didRotate(degrees);
}

void SkGPipeCanvas::didSkew(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kSkew_DrawOp);
            fWriter.writeScalar(sx);
            fWriter.writeScalar(sy);
        }
    }
    this->INHERITED::didSkew(sx, sy);
}

void SkGPipeCanvas::didConcat(const SkMatrix& matrix) {
    if (!matrix.isIdentity()) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(matrix.writeToMemory(NULL))) {
            this->writeOp(kConcat_DrawOp);
            fWriter.writeMatrix(matrix);
        }
    }
    this->INHERITED::didConcat(matrix);
}

void SkGPipeCanvas::didSetMatrix(const SkMatrix& matrix) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(matrix.writeToMemory(NULL))) {
        this->writeOp(kSetMatrix_DrawOp);
        fWriter.writeMatrix(matrix);
    }
    this->INHERITED::didSetMatrix(matrix);
}

void SkGPipeCanvas::onClipRect(const SkRect& rect, SkRegion::Op rgnOp,
                               ClipEdgeStyle edgeStyle) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(sizeof(SkRect))) {
        unsigned flags = 0;
        if (kSoft_ClipEdgeStyle == edgeStyle) {
            flags = kClip_HasAntiAlias_DrawOpFlag;
        }
        this->writeOp(kClipRect_DrawOp, flags, rgnOp);
        fWriter.writeRect(rect);
    }
    this->INHERITED::onClipRect(rect, rgnOp, edgeStyle);
}

void SkGPipeCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op rgnOp,
                                ClipEdgeStyle edgeStyle) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(kSizeOfFlatRRect)) {
        unsigned flags = 0;
        if (kSoft_ClipEdgeStyle == edgeStyle) {
            flags = kClip_HasAntiAlias_DrawOpFlag;
        }
        this->writeOp(kClipRRect_DrawOp, flags, rgnOp);
        fWriter.writeRRect(rrect);
    }
    this->INHERITED::onClipRRect(rrect, rgnOp, edgeStyle);
}

void SkGPipeCanvas::onClipPath(const SkPath& path, SkRegion::Op rgnOp,
                               ClipEdgeStyle edgeStyle) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(path.writeToMemory(NULL))) {
        unsigned flags = 0;
        if (kSoft_ClipEdgeStyle == edgeStyle) {
            flags = kClip_HasAntiAlias_DrawOpFlag;
        }
        this->writeOp(kClipPath_DrawOp, flags, rgnOp);
        fWriter.writePath(path);
    }
    // we just pass on the bounds of the path
    this->INHERITED::onClipRect(path.getBounds(), rgnOp, edgeStyle);
}

void SkGPipeCanvas::onClipRegion(const SkRegion& region, SkRegion::Op rgnOp) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(region.writeToMemory(NULL))) {
        this->writeOp(kClipRegion_DrawOp, 0, rgnOp);
        fWriter.writeRegion(region);
    }
    this->INHERITED::onClipRegion(region, rgnOp);
}

///////////////////////////////////////////////////////////////////////////////

void SkGPipeCanvas::clear(SkColor color) {
    NOTIFY_SETUP(this);
    unsigned flags = 0;
    if (color) {
        flags |= kClear_HasColor_DrawOpFlag;
    }
    if (this->needOpBytes(sizeof(SkColor))) {
        this->writeOp(kDrawClear_DrawOp, flags, 0);
        if (color) {
            fWriter.write32(color);
        }
    }
}

void SkGPipeCanvas::drawPaint(const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes()) {
        this->writeOp(kDrawPaint_DrawOp);
    }
}

void SkGPipeCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    if (count) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPoints_DrawOp, mode, 0);
            fWriter.write32(count);
            fWriter.write(pts, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kDrawOval_DrawOp);
        fWriter.writeRect(rect);
    }
}

void SkGPipeCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kDrawRect_DrawOp);
        fWriter.writeRect(rect);
    }
}

void SkGPipeCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(kSizeOfFlatRRect)) {
        this->writeOp(kDrawRRect_DrawOp);
        fWriter.writeRRect(rrect);
    }
}

void SkGPipeCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                 const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(kSizeOfFlatRRect * 2)) {
        this->writeOp(kDrawDRRect_DrawOp);
        fWriter.writeRRect(outer);
        fWriter.writeRRect(inner);
    }
}

void SkGPipeCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(path.writeToMemory(NULL))) {
        this->writeOp(kDrawPath_DrawOp);
        fWriter.writePath(path);
    }
}

bool SkGPipeCanvas::commonDrawBitmap(const SkBitmap& bm, DrawOps op,
                                     unsigned flags,
                                     size_t opBytesNeeded,
                                     const SkPaint* paint) {
    if (paint != NULL) {
        flags |= kDrawBitmap_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }
    if (this->needOpBytes(opBytesNeeded)) {
        SkASSERT(fBitmapHeap != NULL);
        int32_t bitmapIndex = fBitmapHeap->insert(bm);
        if (SkBitmapHeap::INVALID_SLOT == bitmapIndex) {
            return false;
        }
        this->writeOp(op, flags, bitmapIndex);
        return true;
    }
    return false;
}

void SkGPipeCanvas::drawBitmap(const SkBitmap& bm, SkScalar left, SkScalar top,
                               const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(SkScalar) * 2;

    if (this->commonDrawBitmap(bm, kDrawBitmap_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.writeScalar(left);
        fWriter.writeScalar(top);
    }
}

void SkGPipeCanvas::drawBitmapRectToRect(const SkBitmap& bm, const SkRect* src,
                                         const SkRect& dst, const SkPaint* paint,
                                         DrawBitmapRectFlags dbmrFlags) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(SkRect);
    bool hasSrc = src != NULL;
    unsigned flags;
    if (hasSrc) {
        flags = kDrawBitmap_HasSrcRect_DrawOpFlag;
        opBytesNeeded += sizeof(int32_t) * 4;
    } else {
        flags = 0;
    }
    if (dbmrFlags & kBleed_DrawBitmapRectFlag) {
        flags |= kDrawBitmap_Bleed_DrawOpFlag;
    }

    if (this->commonDrawBitmap(bm, kDrawBitmapRectToRect_DrawOp, flags, opBytesNeeded, paint)) {
        if (hasSrc) {
            fWriter.writeRect(*src);
        }
        fWriter.writeRect(dst);
    }
}

void SkGPipeCanvas::drawBitmapMatrix(const SkBitmap& bm, const SkMatrix& matrix,
                                     const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = matrix.writeToMemory(NULL);

    if (this->commonDrawBitmap(bm, kDrawBitmapMatrix_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.writeMatrix(matrix);
    }
}

void SkGPipeCanvas::drawBitmapNine(const SkBitmap& bm, const SkIRect& center,
                                   const SkRect& dst, const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(int32_t) * 4 + sizeof(SkRect);

    if (this->commonDrawBitmap(bm, kDrawBitmapNine_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.write32(center.fLeft);
        fWriter.write32(center.fTop);
        fWriter.write32(center.fRight);
        fWriter.write32(center.fBottom);
        fWriter.writeRect(dst);
    }
}

void SkGPipeCanvas::drawSprite(const SkBitmap& bm, int left, int top,
                                   const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(int32_t) * 2;

    if (this->commonDrawBitmap(bm, kDrawSprite_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.write32(left);
        fWriter.write32(top);
    }
}

void SkGPipeCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                                 SkScalar y, const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 2 * sizeof(SkScalar))) {
            this->writeOp(kDrawText_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.writeScalar(x);
            fWriter.writeScalar(y);
        }
    }
}

void SkGPipeCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPosText_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(pos, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkScalar) + 4)) {
            this->writeOp(kDrawPosTextH_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(xpos, count * sizeof(SkScalar));
            fWriter.writeScalar(constY);
        }
    }
}

void SkGPipeCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        unsigned flags = 0;
        size_t size = 4 + SkAlign4(byteLength) + path.writeToMemory(NULL);
        if (matrix) {
            flags |= kDrawTextOnPath_HasMatrix_DrawOpFlag;
            size += matrix->writeToMemory(NULL);
        }
        this->writePaint(paint);
        if (this->needOpBytes(size)) {
            this->writeOp(kDrawTextOnPath_DrawOp, flags, 0);

            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);

            fWriter.writePath(path);
            if (matrix) {
                fWriter.writeMatrix(*matrix);
            }
        }
    }
}

void SkGPipeCanvas::drawPicture(SkPicture& picture) {
    // we want to playback the picture into individual draw calls
    this->INHERITED::drawPicture(picture);
}

void SkGPipeCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xfer,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    if (0 == vertexCount) {
        return;
    }

    NOTIFY_SETUP(this);
    size_t size = 4 + vertexCount * sizeof(SkPoint);
    this->writePaint(paint);
    unsigned flags = 0;
    if (texs) {
        flags |= kDrawVertices_HasTexs_DrawOpFlag;
        size += vertexCount * sizeof(SkPoint);
    }
    if (colors) {
        flags |= kDrawVertices_HasColors_DrawOpFlag;
        size += vertexCount * sizeof(SkColor);
    }
    if (indices && indexCount > 0) {
        flags |= kDrawVertices_HasIndices_DrawOpFlag;
        size += 4 + SkAlign4(indexCount * sizeof(uint16_t));
    }
    if (xfer && !SkXfermode::IsMode(xfer, SkXfermode::kModulate_Mode)) {
        flags |= kDrawVertices_HasXfermode_DrawOpFlag;
        size += sizeof(int32_t);    // mode enum
    }

    if (this->needOpBytes(size)) {
        this->writeOp(kDrawVertices_DrawOp, flags, 0);
        fWriter.write32(vmode);
        fWriter.write32(vertexCount);
        fWriter.write(vertices, vertexCount * sizeof(SkPoint));
        if (texs) {
            fWriter.write(texs, vertexCount * sizeof(SkPoint));
        }
        if (colors) {
            fWriter.write(colors, vertexCount * sizeof(SkColor));
        }
        if (flags & kDrawVertices_HasXfermode_DrawOpFlag) {
            SkXfermode::Mode mode = SkXfermode::kModulate_Mode;
            (void)xfer->asMode(&mode);
            fWriter.write32(mode);
        }
        if (indices && indexCount > 0) {
            fWriter.write32(indexCount);
            fWriter.writePad(indices, indexCount * sizeof(uint16_t));
        }
    }
}

void SkGPipeCanvas::drawData(const void* ptr, size_t size) {
    if (size && ptr) {
        NOTIFY_SETUP(this);
        unsigned data = 0;
        if (size < (1 << DRAWOPS_DATA_BITS)) {
            data = (unsigned)size;
        }
        if (this->needOpBytes(4 + SkAlign4(size))) {
            this->writeOp(kDrawData_DrawOp, 0, data);
            if (0 == data) {
                fWriter.write32(size);
            }
            fWriter.writePad(ptr, size);
        }
    }
}

void SkGPipeCanvas::beginCommentGroup(const char* description) {
    // ignore for now
}

void SkGPipeCanvas::addComment(const char* kywd, const char* value) {
    // ignore for now
}

void SkGPipeCanvas::endCommentGroup() {
    // ignore for now
}

void SkGPipeCanvas::flushRecording(bool detachCurrentBlock) {
    doNotify();
    if (detachCurrentBlock) {
        // force a new block to be requested for the next recorded command
        fBlockSize = 0;
    }
}

size_t SkGPipeCanvas::freeMemoryIfPossible(size_t bytesToFree) {
    return (NULL == fBitmapHeap) ? 0 : fBitmapHeap->freeMemoryIfPossible(bytesToFree);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> uint32_t castToU32(T value) {
    union {
        T           fSrc;
        uint32_t    fDst;
    } data;
    data.fSrc = value;
    return data.fDst;
}

void SkGPipeCanvas::writePaint(const SkPaint& paint) {
    if (fDone) {
        return;
    }
    SkPaint& base = fPaint;
    uint32_t storage[32];
    uint32_t* ptr = storage;

    if (base.getFlags() != paint.getFlags()) {
        *ptr++ = PaintOp_packOpData(kFlags_PaintOp, paint.getFlags());
        base.setFlags(paint.getFlags());
    }
    if (base.getColor() != paint.getColor()) {
        *ptr++ = PaintOp_packOp(kColor_PaintOp);
        *ptr++ = paint.getColor();
        base.setColor(paint.getColor());
    }
    if (base.getStyle() != paint.getStyle()) {
        *ptr++ = PaintOp_packOpData(kStyle_PaintOp, paint.getStyle());
        base.setStyle(paint.getStyle());
    }
    if (base.getStrokeJoin() != paint.getStrokeJoin()) {
        *ptr++ = PaintOp_packOpData(kJoin_PaintOp, paint.getStrokeJoin());
        base.setStrokeJoin(paint.getStrokeJoin());
    }
    if (base.getStrokeCap() != paint.getStrokeCap()) {
        *ptr++ = PaintOp_packOpData(kCap_PaintOp, paint.getStrokeCap());
        base.setStrokeCap(paint.getStrokeCap());
    }
    if (base.getStrokeWidth() != paint.getStrokeWidth()) {
        *ptr++ = PaintOp_packOp(kWidth_PaintOp);
        *ptr++ = castToU32(paint.getStrokeWidth());
        base.setStrokeWidth(paint.getStrokeWidth());
    }
    if (base.getStrokeMiter() != paint.getStrokeMiter()) {
        *ptr++ = PaintOp_packOp(kMiter_PaintOp);
        *ptr++ = castToU32(paint.getStrokeMiter());
        base.setStrokeMiter(paint.getStrokeMiter());
    }
    if (base.getTextEncoding() != paint.getTextEncoding()) {
        *ptr++ = PaintOp_packOpData(kEncoding_PaintOp, paint.getTextEncoding());
        base.setTextEncoding(paint.getTextEncoding());
    }
    if (base.getHinting() != paint.getHinting()) {
        *ptr++ = PaintOp_packOpData(kHinting_PaintOp, paint.getHinting());
        base.setHinting(paint.getHinting());
    }
    if (base.getTextAlign() != paint.getTextAlign()) {
        *ptr++ = PaintOp_packOpData(kAlign_PaintOp, paint.getTextAlign());
        base.setTextAlign(paint.getTextAlign());
    }
    if (base.getTextSize() != paint.getTextSize()) {
        *ptr++ = PaintOp_packOp(kTextSize_PaintOp);
        *ptr++ = castToU32(paint.getTextSize());
        base.setTextSize(paint.getTextSize());
    }
    if (base.getTextScaleX() != paint.getTextScaleX()) {
        *ptr++ = PaintOp_packOp(kTextScaleX_PaintOp);
        *ptr++ = castToU32(paint.getTextScaleX());
        base.setTextScaleX(paint.getTextScaleX());
    }
    if (base.getTextSkewX() != paint.getTextSkewX()) {
        *ptr++ = PaintOp_packOp(kTextSkewX_PaintOp);
        *ptr++ = castToU32(paint.getTextSkewX());
        base.setTextSkewX(paint.getTextSkewX());
    }

    if (!SkTypeface::Equal(base.getTypeface(), paint.getTypeface())) {
        if (isCrossProcess(fFlags)) {
            uint32_t id = this->getTypefaceID(paint.getTypeface());
            *ptr++ = PaintOp_packOpData(kTypeface_PaintOp, id);
        } else if (this->needOpBytes(sizeof(void*))) {
            // Add to the set for ref counting.
            fTypefaceSet.add(paint.getTypeface());
            // It is safe to write the typeface to the stream before the rest
            // of the paint unless we ever send a kReset_PaintOp, which we
            // currently never do.
            this->writeOp(kSetTypeface_DrawOp);
            fWriter.writePtr(paint.getTypeface());
        }
        base.setTypeface(paint.getTypeface());
    }

    // This is a new paint, so all old flats can be safely purged, if necessary.
    fFlattenableHeap.markAllFlatsSafeToDelete();
    for (int i = 0; i < kCount_PaintFlats; i++) {
        int index = this->flattenToIndex(get_paintflat(paint, i), (PaintFlats)i);
        bool replaced = index < 0;
        if (replaced) {
            index = ~index;
        }
        // Store the index of any flat that needs to be kept. 0 means no flat.
        if (index > 0) {
            fFlattenableHeap.markFlatForKeeping(index);
        }
        SkASSERT(index >= 0 && index <= fFlatDictionary.count());
        if (index != fCurrFlatIndex[i] || replaced) {
            *ptr++ = PaintOp_packOpFlagData(kFlatIndex_PaintOp, i, index);
            fCurrFlatIndex[i] = index;
        }
    }

    size_t size = (char*)ptr - (char*)storage;
    if (size && this->needOpBytes(size)) {
        this->writeOp(kPaintOp_DrawOp, 0, size);
        fWriter.write(storage, size);
        for (size_t i = 0; i < size/4; i++) {
//            SkDebugf("[%d] %08X\n", i, storage[i]);
        }
    }

    //
    //  Do these after we've written kPaintOp_DrawOp

    if (base.getAnnotation() != paint.getAnnotation()) {
        if (NULL == paint.getAnnotation()) {
            if (this->needOpBytes()) {
                this->writeOp(kSetAnnotation_DrawOp, 0, 0);
            }
        } else {
            SkWriteBuffer buffer;
            paint.getAnnotation()->writeToBuffer(buffer);
            const size_t size = buffer.bytesWritten();
            if (this->needOpBytes(size)) {
                this->writeOp(kSetAnnotation_DrawOp, 0, size);
                buffer.writeToMemory(fWriter.reserve(size));
            }
        }
        base.setAnnotation(paint.getAnnotation());
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

SkGPipeController::~SkGPipeController() {
    SkSafeUnref(fCanvas);
}

void SkGPipeController::setCanvas(SkGPipeCanvas* canvas) {
    SkRefCnt_SafeAssign(fCanvas, canvas);
}

///////////////////////////////////////////////////////////////////////////////

SkGPipeWriter::SkGPipeWriter()
: fWriter(0) {
    fCanvas = NULL;
}

SkGPipeWriter::~SkGPipeWriter() {
    this->endRecording();
}

SkCanvas* SkGPipeWriter::startRecording(SkGPipeController* controller, uint32_t flags,
                                        uint32_t width, uint32_t height) {
    if (NULL == fCanvas) {
        fWriter.reset(NULL, 0);
        fCanvas = SkNEW_ARGS(SkGPipeCanvas, (controller, &fWriter, flags, width, height));
    }
    controller->setCanvas(fCanvas);
    return fCanvas;
}

void SkGPipeWriter::endRecording() {
    if (fCanvas) {
        fCanvas->finish(true);
        fCanvas->unref();
        fCanvas = NULL;
    }
}

void SkGPipeWriter::flushRecording(bool detachCurrentBlock) {
    if (fCanvas) {
        fCanvas->flushRecording(detachCurrentBlock);
    }
}

size_t SkGPipeWriter::freeMemoryIfPossible(size_t bytesToFree) {
    if (fCanvas) {
        return fCanvas->freeMemoryIfPossible(bytesToFree);
    }
    return 0;
}

size_t SkGPipeWriter::storageAllocatedForRecording() const {
    return NULL == fCanvas ? 0 : fCanvas->storageAllocatedForRecording();
}

///////////////////////////////////////////////////////////////////////////////

BitmapShuttle::BitmapShuttle(SkGPipeCanvas* canvas) {
    SkASSERT(canvas != NULL);
    fCanvas = canvas;
    fCanvas->ref();
}

BitmapShuttle::~BitmapShuttle() {
    this->removeCanvas();
}

bool BitmapShuttle::insert(const SkBitmap& bitmap, int32_t slot) {
    SkASSERT(fCanvas != NULL);
    return fCanvas->shuttleBitmap(bitmap, slot);
}

void BitmapShuttle::removeCanvas() {
    if (NULL == fCanvas) {
        return;
    }
    fCanvas->unref();
    fCanvas = NULL;
}

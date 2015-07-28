
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkBitmapHeap.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDrawLooper.h"
#include "SkGPipe.h"
#include "SkGPipePriv.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkRSXform.h"
#include "SkWriteBuffer.h"
#include "SkPaint.h"
#include "SkPatchUtils.h"
#include "SkPathEffect.h"
#include "SkPictureFlat.h"
#include "SkPtrRecorder.h"
#include "SkRasterizer.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTSearch.h"
#include "SkTypeface.h"
#include "SkWriter32.h"

enum {
    kSizeOfFlatRRect = sizeof(SkRect) + 4 * sizeof(SkVector)
};

static bool is_cross_process(uint32_t flags) {
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
        writer->write32(SkToU32(size));
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

    void* allocThrow(size_t bytes) override;

    void unalloc(void* ptr) override;

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

    bool insert(const SkBitmap& bitmap, int32_t slot) override;

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
        size_t bytesAllocated = 0;
        if (NULL != fBitmapHeap) {
            bytesAllocated += fBitmapHeap->bytesAllocated();
        }
        if (NULL != fImageHeap) {
            bytesAllocated += fImageHeap->bytesInCache();
        }
        return bytesAllocated;
    }

    /**
     * Flatten an SkBitmap to send to the reader, where it will be referenced
     * according to slot.
     */
    bool shuttleBitmap(const SkBitmap&, int32_t slot);

    void resetImageHeap();

protected:
    void willSave() override;
    SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) override;
    void willRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint&) override;
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint&) override;
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint&) override;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode,
                     const SkPaint& paint) override;
    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onDrawSprite(const SkBitmap&, int left, int top, const SkPaint*) override;
    void onDrawVertices(VertexMode vmode, int vertexCount,
                        const SkPoint vertices[], const SkPoint texs[],
                        const SkColor colors[], SkXfermode* xmode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int count, SkXfermode::Mode, const SkRect* cull, const SkPaint*) override;
    void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkRegion::Op) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

private:
    void recordTranslate(const SkMatrix&);
    void recordScale(const SkMatrix&);
    void recordConcat(const SkMatrix&);

    SkNamedFactorySet* fFactorySet;
    SkBitmapHeap*      fBitmapHeap;
    SkImageHeap*       fImageHeap;
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

    typedef SkAutoSTMalloc<128, uint8_t> TypefaceBuffer;
    size_t getInProcessTypefaces(const SkRefCntSet& typefaceSet, TypefaceBuffer*);
    size_t getCrossProcessTypefaces(const SkRefCntSet& typefaceSet, TypefaceBuffer*);

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
    bool commonDrawBitmap(const SkBitmap&, DrawOps, unsigned flags, size_t bytes, const SkPaint*);
    bool commonDrawImage(const SkImage*, DrawOps, unsigned flags, size_t bytes, const SkPaint*);

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
        if (this->needOpBytes(SkWriter32::WriteStringSize(name, len))) {
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
    size_t size = buffer.bytesWritten();
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
        if (is_cross_process(fFlags)) {
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
    , fFactorySet(is_cross_process(flags) ? SkNEW(SkNamedFactorySet) : NULL)
    , fWriter(*writer)
    , fFlags(flags)
    , fFlattenableHeap(FLATTENABLES_TO_KEEP, fFactorySet, is_cross_process(flags))
    , fFlatDictionary(&fFlattenableHeap)
{
    fController = controller;
    fDone = false;
    fBlockSize = 0; // need first block from controller
    fBytesNotified = 0;
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

    fImageHeap = SkNEW(SkImageHeap);
    if (this->needOpBytes(sizeof(void*))) {
        this->writeOp(kShareImageHeap_DrawOp);
        fWriter.writePtr(static_cast<void*>(fImageHeap));
    }

    this->doNotify();
}

SkGPipeCanvas::~SkGPipeCanvas() {
    this->finish(true);
    SkSafeUnref(fFactorySet);
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fImageHeap);
}

bool SkGPipeCanvas::needOpBytes(size_t needed) {
    if (fDone) {
        return false;
    }

    needed += 4;  // size of DrawOp atom
    needed = SkAlign4(needed);
    if (fWriter.bytesWritten() + needed > fBlockSize) {
        // Before we wipe out any data that has already been written, read it out.
        this->doNotify();

        // If we're going to allocate a new block, allocate enough to make it worthwhile.
        needed = SkTMax<size_t>(MIN_BLOCK_SIZE, needed);

        void* block = fController->requestBlock(needed, &fBlockSize);
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

void SkGPipeCanvas::willSave() {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kSave_DrawOp);
    }

    this->INHERITED::willSave();
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

    this->INHERITED::willSaveLayer(bounds, paint, saveFlags);
    // we don't create a layer
    return kNoLayer_SaveLayerStrategy;
}

void SkGPipeCanvas::willRestore() {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kRestore_DrawOp);
    }

    this->INHERITED::willRestore();
}

void SkGPipeCanvas::recordTranslate(const SkMatrix& m) {
    if (this->needOpBytes(2 * sizeof(SkScalar))) {
        this->writeOp(kTranslate_DrawOp);
        fWriter.writeScalar(m.getTranslateX());
        fWriter.writeScalar(m.getTranslateY());
    }
}

void SkGPipeCanvas::recordScale(const SkMatrix& m) {
    if (this->needOpBytes(2 * sizeof(SkScalar))) {
        this->writeOp(kScale_DrawOp);
        fWriter.writeScalar(m.getScaleX());
        fWriter.writeScalar(m.getScaleY());
    }
}

void SkGPipeCanvas::recordConcat(const SkMatrix& m) {
    if (this->needOpBytes(m.writeToMemory(NULL))) {
        this->writeOp(kConcat_DrawOp);
        fWriter.writeMatrix(m);
    }
}

void SkGPipeCanvas::didConcat(const SkMatrix& matrix) {
    if (!matrix.isIdentity()) {
        NOTIFY_SETUP(this);
        switch (matrix.getType()) {
            case SkMatrix::kTranslate_Mask:
                this->recordTranslate(matrix);
                break;
            case SkMatrix::kScale_Mask:
                this->recordScale(matrix);
                break;
            default:
                this->recordConcat(matrix);
                break;
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

void SkGPipeCanvas::onDrawPaint(const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes()) {
        this->writeOp(kDrawPaint_DrawOp);
    }
}

void SkGPipeCanvas::onDrawPoints(PointMode mode, size_t count,
                                 const SkPoint pts[], const SkPaint& paint) {
    if (count) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPoints_DrawOp, mode, 0);
            fWriter.write32(SkToU32(count));
            fWriter.write(pts, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kDrawOval_DrawOp);
        fWriter.writeRect(rect);
    }
}

void SkGPipeCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kDrawRect_DrawOp);
        fWriter.writeRect(rect);
    }
}

void SkGPipeCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
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

void SkGPipeCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
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
    if (fDone) {
        return false;
    }

    if (paint != NULL) {
        flags |= kDrawBitmap_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }
    // This needs to run first so its calls to needOpBytes() and its writes
    // don't interlace with the needOpBytes() and write below.
    SkASSERT(fBitmapHeap != NULL);
    int32_t bitmapIndex = fBitmapHeap->insert(bm);
    if (SkBitmapHeap::INVALID_SLOT == bitmapIndex) {
        return false;
    }

    if (this->needOpBytes(opBytesNeeded)) {
        this->writeOp(op, flags, bitmapIndex);
        return true;
    }
    return false;
}

void SkGPipeCanvas::onDrawBitmap(const SkBitmap& bm, SkScalar left, SkScalar top,
                                 const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(SkScalar) * 2;

    if (this->commonDrawBitmap(bm, kDrawBitmap_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.writeScalar(left);
        fWriter.writeScalar(top);
    }
}

void SkGPipeCanvas::onDrawBitmapRect(const SkBitmap& bm, const SkRect* src, const SkRect& dst,
                                     const SkPaint* paint, SrcRectConstraint constraint) {
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
    if (kFast_SrcRectConstraint == constraint) {
        flags |= kDrawBitmap_Bleed_DrawOpFlag;
    }

    if (this->commonDrawBitmap(bm, kDrawBitmapRect_DrawOp, flags, opBytesNeeded, paint)) {
        if (hasSrc) {
            fWriter.writeRect(*src);
        }
        fWriter.writeRect(dst);
    }
}

void SkGPipeCanvas::onDrawBitmapNine(const SkBitmap& bm, const SkIRect& center,
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

void SkGPipeCanvas::onDrawSprite(const SkBitmap& bm, int left, int top, const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(int32_t) * 2;

    if (this->commonDrawBitmap(bm, kDrawSprite_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.write32(left);
        fWriter.write32(top);
    }
}

bool SkGPipeCanvas::commonDrawImage(const SkImage* image, DrawOps op, unsigned flags,
                                    size_t opBytesNeeded, const SkPaint* paint) {
    if (fDone) {
        return false;
    }
    
    if (paint != NULL) {
        flags |= kDrawBitmap_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }
    // This needs to run first so its calls to needOpBytes() and its writes
    // don't interlace with the needOpBytes() and write below.
    int32_t slot = fImageHeap->insert(image);
    SkASSERT(slot != 0);
    if (this->needOpBytes(opBytesNeeded)) {
        this->writeOp(op, flags, slot);
        return true;
    }
    return false;
}

void SkGPipeCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
    NOTIFY_SETUP(this);
    if (this->commonDrawImage(image, kDrawImage_DrawOp, 0, sizeof(SkScalar) * 2, paint)) {
        fWriter.writeScalar(x);
        fWriter.writeScalar(y);
    }
}

void SkGPipeCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    NOTIFY_SETUP(this);
    unsigned flags = 0;
    size_t opBytesNeeded = sizeof(SkRect);  // dst
    if (src) {
        flags |= kDrawBitmap_HasSrcRect_DrawOpFlag;
        opBytesNeeded += sizeof(SkRect);    // src
    }
    if (this->commonDrawImage(image, kDrawImageRect_DrawOp, flags, opBytesNeeded, paint)) {
        if (src) {
            fWriter.writeRect(*src);
        }
        fWriter.writeRect(dst);
        fWriter.writeInt(constraint);
    }
}

void SkGPipeCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                                    const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(SkIRect) + sizeof(SkRect);  // center + dst
    if (this->commonDrawImage(image, kDrawImageNine_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.writeIRect(center);
        fWriter.writeRect(dst);
    }
}

void SkGPipeCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                               const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 2 * sizeof(SkScalar))) {
            this->writeOp(kDrawText_DrawOp);
            fWriter.write32(SkToU32(byteLength));
            fWriter.writePad(text, byteLength);
            fWriter.writeScalar(x);
            fWriter.writeScalar(y);
        }
    }
}

void SkGPipeCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                  const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPosText_DrawOp);
            fWriter.write32(SkToU32(byteLength));
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(pos, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                   SkScalar constY, const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkScalar) + 4)) {
            this->writeOp(kDrawPosTextH_DrawOp);
            fWriter.write32(SkToU32(byteLength));
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(xpos, count * sizeof(SkScalar));
            fWriter.writeScalar(constY);
        }
    }
}

void SkGPipeCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                     const SkMatrix* matrix, const SkPaint& paint) {
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

            fWriter.write32(SkToU32(byteLength));
            fWriter.writePad(text, byteLength);

            fWriter.writePath(path);
            if (matrix) {
                fWriter.writeMatrix(*matrix);
            }
        }
    }
}

size_t SkGPipeCanvas::getInProcessTypefaces(const SkRefCntSet& typefaceSet,
                                            TypefaceBuffer* buffer) {
    // When in-process, we simply write out the typeface pointers.
    size_t size = typefaceSet.count() * sizeof(SkTypeface*);
    buffer->reset(size);
    typefaceSet.copyToArray(reinterpret_cast<SkRefCnt**>(buffer->get()));

    return size;
}

size_t SkGPipeCanvas::getCrossProcessTypefaces(const SkRefCntSet& typefaceSet,
                                               TypefaceBuffer* buffer) {
    // For cross-process we use typeface IDs.
    size_t size = typefaceSet.count() * sizeof(uint32_t);
    buffer->reset(size);

    uint32_t* idBuffer = reinterpret_cast<uint32_t*>(buffer->get());
    SkRefCntSet::Iter iter(typefaceSet);
    int i = 0;

    for (void* setPtr = iter.next(); setPtr; setPtr = iter.next()) {
        idBuffer[i++] = this->getTypefaceID(reinterpret_cast<SkTypeface*>(setPtr));
    }

    SkASSERT(i == typefaceSet.count());

    return size;
}

void SkGPipeCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                   const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);

    // FIXME: this is inefficient but avoids duplicating the blob serialization logic.
    SkRefCntSet typefaceSet;
    SkWriteBuffer blobBuffer;
    blobBuffer.setTypefaceRecorder(&typefaceSet);
    blob->flatten(blobBuffer);

    // Unlike most draw ops (which only use one paint/typeface), text blobs may reference
    // an arbitrary number of typefaces. Since the one-paint-per-op model is not applicable,
    // we need to serialize these explicitly.
    TypefaceBuffer typefaceBuffer;
    size_t typefaceSize = is_cross_process(fFlags)
        ? this->getCrossProcessTypefaces(typefaceSet, &typefaceBuffer)
        : this->getInProcessTypefaces(typefaceSet, &typefaceBuffer);

    // blob byte count + typeface count + x + y + blob data + an index (cross-process)
    // or pointer (in-process) for each typeface
    size_t size = 2 * sizeof(uint32_t)
                + 2 * sizeof(SkScalar)
                + blobBuffer.bytesWritten()
                + typefaceSize;

    if (this->needOpBytes(size)) {
        this->writeOp(kDrawTextBlob_DrawOp);
        SkDEBUGCODE(size_t initialOffset = fWriter.bytesWritten();)

        fWriter.writeScalar(x);
        fWriter.writeScalar(y);

        fWriter.write32(typefaceSet.count());
        fWriter.write(typefaceBuffer.get(), typefaceSize);

        fWriter.write32(SkToU32(blobBuffer.bytesWritten()));
        uint32_t* pad = fWriter.reservePad(blobBuffer.bytesWritten());
        blobBuffer.writeToMemory(pad);

        SkASSERT(initialOffset + size == fWriter.bytesWritten());
    }
}

void SkGPipeCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                  const SkPaint* paint) {
    // we want to playback the picture into individual draw calls
    //
    // todo: do we always have to unroll? If the pipe is not cross-process, seems like
    // we could just ref the picture and move on...? <reed, scroggo>
    //
    this->INHERITED::onDrawPicture(picture, matrix, paint);
}

void SkGPipeCanvas::onDrawVertices(VertexMode vmode, int vertexCount,
                                   const SkPoint vertices[], const SkPoint texs[],
                                   const SkColor colors[], SkXfermode* xfer,
                                   const uint16_t indices[], int indexCount,
                                   const SkPaint& paint) {
    if (0 == vertexCount) {
        return;
    }

    NOTIFY_SETUP(this);
    this->writePaint(paint);

    unsigned flags = 0;  // packs with the op, so needs no extra space

    size_t size = 0;
    size += 4;                              // vmode
    size += 4;                              // vertex count
    size += vertexCount * sizeof(SkPoint);  // vertices

    if (texs) {
        flags |= kDrawVertices_HasTexs_DrawOpFlag;
        size += vertexCount * sizeof(SkPoint);
    }
    if (colors) {
        flags |= kDrawVertices_HasColors_DrawOpFlag;
        size += vertexCount * sizeof(SkColor);
    }
    if (xfer && !SkXfermode::IsMode(xfer, SkXfermode::kModulate_Mode)) {
        flags |= kDrawVertices_HasXfermode_DrawOpFlag;
        size += sizeof(int32_t);    // SkXfermode::Mode
    }
    if (indices && indexCount > 0) {
        flags |= kDrawVertices_HasIndices_DrawOpFlag;
        size += 4;                                        // index count
        size += SkAlign4(indexCount * sizeof(uint16_t));  // indices
    }

    if (this->needOpBytes(size)) {
        this->writeOp(kDrawVertices_DrawOp, flags, 0);
        fWriter.write32(vmode);
        fWriter.write32(vertexCount);
        fWriter.write(vertices, vertexCount * sizeof(SkPoint));
        if (flags & kDrawVertices_HasTexs_DrawOpFlag) {
            fWriter.write(texs, vertexCount * sizeof(SkPoint));
        }
        if (flags & kDrawVertices_HasColors_DrawOpFlag) {
            fWriter.write(colors, vertexCount * sizeof(SkColor));
        }
        if (flags & kDrawVertices_HasXfermode_DrawOpFlag) {
            SkXfermode::Mode mode = SkXfermode::kModulate_Mode;
            SkAssertResult(xfer->asMode(&mode));
            fWriter.write32(mode);
        }
        if (flags & kDrawVertices_HasIndices_DrawOpFlag) {
            fWriter.write32(indexCount);
            fWriter.writePad(indices, indexCount * sizeof(uint16_t));
        }
    }
}

void SkGPipeCanvas::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                                const SkColor colors[], int count, SkXfermode::Mode mode,
                                const SkRect* cull, const SkPaint* paint) {
    NOTIFY_SETUP(this);
    unsigned flags = 0;  // packs with the op, so needs no extra space

    if (paint) {
        flags |= kDrawAtlas_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }

    size_t size = 4;                        // image-slot
    size += 4;                              // count
    size += 4;                              // mode
    size += count * sizeof(SkRSXform);      // xform
    size += count * sizeof(SkRect);         // tex
    if (colors) {
        flags |= kDrawAtlas_HasColors_DrawOpFlag;
        size += count * sizeof(SkColor);    // colors
    }
    if (cull) {
        flags |= kDrawAtlas_HasCull_DrawOpFlag;
        size += sizeof(SkRect);             // cull
    }
    
    if (this->needOpBytes(size)) {
        this->writeOp(kDrawAtlas_DrawOp, flags, 0);
        int32_t slot = fImageHeap->insert(atlas);
        fWriter.write32(slot);
        fWriter.write32(count);
        fWriter.write32(mode);
        fWriter.write(xform, count * sizeof(SkRSXform));
        fWriter.write(tex, count * sizeof(SkRect));
        if (colors) {
            fWriter.write(colors, count * sizeof(SkColor));
        }
        if (cull) {
            fWriter.writeRect(*cull);
        }
    }
}

void SkGPipeCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                const SkPoint texCoords[4], SkXfermode* xmode,
                                const SkPaint& paint) {
    NOTIFY_SETUP(this);
    
    size_t size = SkPatchUtils::kNumCtrlPts * sizeof(SkPoint);
    unsigned flags = 0;
    if (colors) {
        flags |= kDrawVertices_HasColors_DrawOpFlag;
        size += SkPatchUtils::kNumCorners * sizeof(SkColor);
    }
    if (texCoords) {
        flags |= kDrawVertices_HasTexs_DrawOpFlag;
        size += SkPatchUtils::kNumCorners * sizeof(SkPoint);
    }
    if (xmode) {
        SkXfermode::Mode mode;
        if (xmode->asMode(&mode) && SkXfermode::kModulate_Mode != mode) {
            flags |= kDrawVertices_HasXfermode_DrawOpFlag;
            size += sizeof(int32_t);
        }
    }
    
    this->writePaint(paint);
    if (this->needOpBytes(size)) {
        this->writeOp(kDrawPatch_DrawOp, flags, 0);
        
        fWriter.write(cubics, SkPatchUtils::kNumCtrlPts * sizeof(SkPoint));
        
        if (colors) {
            fWriter.write(colors, SkPatchUtils::kNumCorners * sizeof(SkColor));
        }
        
        if (texCoords) {
            fWriter.write(texCoords, SkPatchUtils::kNumCorners * sizeof(SkPoint));
        }
        
        if (flags & kDrawVertices_HasXfermode_DrawOpFlag) {
            SkXfermode::Mode mode = SkXfermode::kModulate_Mode;
            SkAssertResult(xmode->asMode(&mode));
            fWriter.write32(mode);
        }
    }
}

void SkGPipeCanvas::flushRecording(bool detachCurrentBlock) {
    this->doNotify();
    if (detachCurrentBlock) {
        // force a new block to be requested for the next recorded command
        fBlockSize = 0;
    }
}

void SkGPipeCanvas::resetImageHeap() {
    if (fImageHeap) {
        fImageHeap->reset();
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
    if (base.getFilterQuality() != paint.getFilterQuality()) {
        *ptr++ = PaintOp_packOpData(kFilterLevel_PaintOp, paint.getFilterQuality());
        base.setFilterQuality(paint.getFilterQuality());
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
        if (is_cross_process(fFlags)) {
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
        this->writeOp(kPaintOp_DrawOp, 0, SkToU32(size));
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
                this->writeOp(kSetAnnotation_DrawOp, 0, SkToU32(size));
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

void SkGPipeController::purgeCaches()
{
    fCanvas->resetImageHeap();
    // Other caches are self-purging with a small MRU pool
    // We could purge them as well, but it is not clear whether
    // that would be a win.
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

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImageHeap::SkImageHeap() : fBytesInCache (0) {}

SkImageHeap::~SkImageHeap() {
    fArray.unrefAll();
}

void SkImageHeap::reset() {
    fArray.unrefAll();
    fArray.rewind();
    fBytesInCache = 0;
}

const SkImage* SkImageHeap::get(int32_t slot) const {
    SkASSERT(slot > 0);
    return fArray[slot - 1];
}

int32_t SkImageHeap::find(const SkImage* img) const {
    int index = fArray.find(img);
    if (index >= 0) {
        return index + 1;   // found
    }
    return 0;   // not found
}

int32_t SkImageHeap::insert(const SkImage* img) {
    int32_t slot = this->find(img);
    if (slot) {
        return slot;
    }
    // TODO: SkImage does not expose bytes per pixel, 4 is just a best guess.
    fBytesInCache += img->width() * img->height() * 4;
    *fArray.append() = SkRef(img);
    return fArray.count();  // slot is always index+1
}


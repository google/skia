
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureFlat_DEFINED
#define SkPictureFlat_DEFINED


#include "SkBitmapHeap.h"
#include "SkChecksum.h"
#include "SkChunkAlloc.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPtrRecorder.h"
#include "SkTDynamicHash.h"

enum DrawType {
    UNUSED,
    CLIP_PATH,
    CLIP_REGION,
    CLIP_RECT,
    CLIP_RRECT,
    CONCAT,
    DRAW_BITMAP,
    DRAW_BITMAP_MATRIX, // deprecated, M41 was last Chromium version to write this to an .skp
    DRAW_BITMAP_NINE,
    DRAW_BITMAP_RECT,
    DRAW_CLEAR,
    DRAW_DATA,
    DRAW_OVAL,
    DRAW_PAINT,
    DRAW_PATH,
    DRAW_PICTURE,
    DRAW_POINTS,
    DRAW_POS_TEXT,
    DRAW_POS_TEXT_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT
    DRAW_POS_TEXT_H,
    DRAW_POS_TEXT_H_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT_H
    DRAW_RECT,
    DRAW_RRECT,
    DRAW_SPRITE,
    DRAW_TEXT,
    DRAW_TEXT_ON_PATH,
    DRAW_TEXT_TOP_BOTTOM,   // fast variant of DRAW_TEXT
    DRAW_VERTICES,
    RESTORE,
    ROTATE,
    SAVE,
    SAVE_LAYER,
    SCALE,
    SET_MATRIX,
    SKEW,
    TRANSLATE,
    NOOP,
    BEGIN_COMMENT_GROUP, // deprecated (M44)
    COMMENT,             // deprecated (M44)
    END_COMMENT_GROUP,   // deprecated (M44)

    // new ops -- feel free to re-alphabetize on next version bump
    DRAW_DRRECT,
    PUSH_CULL,  // deprecated, M41 was last Chromium version to write this to an .skp
    POP_CULL,   // deprecated, M41 was last Chromium version to write this to an .skp

    DRAW_PATCH, // could not add in aphabetical order
    DRAW_PICTURE_MATRIX_PAINT,
    DRAW_TEXT_BLOB,
    DRAW_IMAGE,
    DRAW_IMAGE_RECT_STRICT, // deprecated (M45)
    DRAW_ATLAS,
    DRAW_IMAGE_NINE,
    DRAW_IMAGE_RECT,

    LAST_DRAWTYPE_ENUM = DRAW_IMAGE_RECT
};

// In the 'match' method, this constant will match any flavor of DRAW_BITMAP*
static const int kDRAW_BITMAP_FLAVOR = LAST_DRAWTYPE_ENUM+1;

enum DrawVertexFlags {
    DRAW_VERTICES_HAS_TEXS    = 0x01,
    DRAW_VERTICES_HAS_COLORS  = 0x02,
    DRAW_VERTICES_HAS_INDICES = 0x04,
    DRAW_VERTICES_HAS_XFER    = 0x08,
};

enum DrawAtlasFlags {
    DRAW_ATLAS_HAS_COLORS   = 1 << 0,
    DRAW_ATLAS_HAS_CULL     = 1 << 1,
};

///////////////////////////////////////////////////////////////////////////////
// clipparams are packed in 5 bits
//  doAA:1 | regionOp:4

static inline uint32_t ClipParams_pack(SkRegion::Op op, bool doAA) {
    unsigned doAABit = doAA ? 1 : 0;
    return (doAABit << 4) | op;
}

static inline SkRegion::Op ClipParams_unpackRegionOp(uint32_t packed) {
    return (SkRegion::Op)(packed & 0xF);
}

static inline bool ClipParams_unpackDoAA(uint32_t packed) {
    return SkToBool((packed >> 4) & 1);
}

///////////////////////////////////////////////////////////////////////////////

class SkTypefacePlayback {
public:
    SkTypefacePlayback();
    virtual ~SkTypefacePlayback();

    int count() const { return fCount; }

    void reset(const SkRefCntSet*);

    void setCount(int count);
    SkRefCnt* set(int index, SkRefCnt*);

    void setupBuffer(SkReadBuffer& buffer) const {
        buffer.setTypefaceArray((SkTypeface**)fArray, fCount);
    }

protected:
    int fCount;
    SkRefCnt** fArray;
};

class SkFactoryPlayback {
public:
    SkFactoryPlayback(int count) : fCount(count) {
        fArray = SkNEW_ARRAY(SkFlattenable::Factory, count);
    }

    ~SkFactoryPlayback() {
        SkDELETE_ARRAY(fArray);
    }

    SkFlattenable::Factory* base() const { return fArray; }

    void setupBuffer(SkReadBuffer& buffer) const {
        buffer.setFactoryPlayback(fArray, fCount);
    }

private:
    int fCount;
    SkFlattenable::Factory* fArray;
};

///////////////////////////////////////////////////////////////////////////////
//
//
// The following templated classes provide an efficient way to store and compare
// objects that have been flattened (i.e. serialized in an ordered binary
// format).
//
// SkFlatData:       is a simple indexable container for the flattened data
//                   which is agnostic to the type of data is is indexing. It is
//                   also responsible for flattening/unflattening objects but
//                   details of that operation are hidden in the provided traits
// SkFlatDictionary: is an abstract templated dictionary that maintains a
//                   searchable set of SkFlatData objects of type T.
// SkFlatController: is an interface provided to SkFlatDictionary which handles
//                   allocation (and unallocation in some cases). It also holds
//                   ref count recorders and the like.
//
// NOTE: any class that wishes to be used in conjunction with SkFlatDictionary must subclass the
// dictionary and provide the necessary flattening traits.  SkFlatController must also be
// implemented, or SkChunkFlatController can be used to use an SkChunkAllocator and never do
// replacements.
//
//
///////////////////////////////////////////////////////////////////////////////

class SkFlatData;

class SkFlatController : public SkRefCnt {
public:
    

    SkFlatController(uint32_t writeBufferFlags = 0);
    virtual ~SkFlatController();
    /**
     * Return a new block of memory for the SkFlatDictionary to use.
     * This memory is owned by the controller and has the same lifetime unless you
     * call unalloc(), in which case it may be freed early.
     */
    virtual void* allocThrow(size_t bytes) = 0;

    /**
     * Hint that this block, which was allocated with allocThrow, is no longer needed.
     * The implementation may choose to free this memory any time beteween now and destruction.
     */
    virtual void unalloc(void* ptr) = 0;

    /**
     * Used during creation and unflattening of SkFlatData objects. If the
     * objects being flattened contain bitmaps they are stored in this heap
     * and the flattenable stores the index to the bitmap on the heap.
     * This should be set by the protected setBitmapHeap.
     */
    SkBitmapHeap* getBitmapHeap() { return fBitmapHeap; }

    /**
     * Used during creation of SkFlatData objects. If a typeface recorder is
     * required to flatten the objects being flattened (i.e. for SkPaints), this
     * should be set by the protected setTypefaceSet.
     */
    SkRefCntSet* getTypefaceSet() { return fTypefaceSet; }

    /**
     * Used during unflattening of the SkFlatData objects in the
     * SkFlatDictionary. Needs to be set by the protected setTypefacePlayback
     * and needs to be reset to the SkRefCntSet passed to setTypefaceSet.
     */
    SkTypefacePlayback* getTypefacePlayback() { return fTypefacePlayback; }

    /**
     * Optional factory recorder used during creation of SkFlatData objects. Set
     * using the protected method setNamedFactorySet.
     */
    SkNamedFactorySet* getNamedFactorySet() { return fFactorySet; }

    /**
     * Flags to use during creation of SkFlatData objects. Defaults to zero.
     */
    uint32_t getWriteBufferFlags() { return fWriteBufferFlags; }

protected:
    /**
     * Set an SkBitmapHeap to be used to store/read SkBitmaps. Ref counted.
     */
    void setBitmapHeap(SkBitmapHeap*);

    /**
     * Set an SkRefCntSet to be used to store SkTypefaces during flattening. Ref
     * counted.
     */
    void setTypefaceSet(SkRefCntSet*);

    /**
     * Set an SkTypefacePlayback to be used to find references to SkTypefaces
     * during unflattening. Should be reset to the set provided to
     * setTypefaceSet.
     */
    void setTypefacePlayback(SkTypefacePlayback*);

    /**
     * Set an SkNamedFactorySet to be used to store Factorys and their
     * corresponding names during flattening. Ref counted. Returns the same
     * set as a convenience.
     */
    SkNamedFactorySet* setNamedFactorySet(SkNamedFactorySet*);

private:
    SkBitmapHeap*       fBitmapHeap;
    SkRefCntSet*        fTypefaceSet;
    SkTypefacePlayback* fTypefacePlayback;
    SkNamedFactorySet*  fFactorySet;
    const uint32_t      fWriteBufferFlags;

    typedef SkRefCnt INHERITED;
};

class SkFlatData {
public:
    // Flatten obj into an SkFlatData with this index.  controller owns the SkFlatData*.
    template <typename Traits, typename T>
    static SkFlatData* Create(SkFlatController* controller, const T& obj, int index) {
        // A buffer of 256 bytes should fit most paints, regions, and matrices.
        uint32_t storage[64];
        SkWriteBuffer buffer(storage, sizeof(storage), controller->getWriteBufferFlags());

        buffer.setBitmapHeap(controller->getBitmapHeap());
        buffer.setTypefaceRecorder(controller->getTypefaceSet());
        buffer.setNamedFactoryRecorder(controller->getNamedFactorySet());

        Traits::Flatten(buffer, obj);
        size_t size = buffer.bytesWritten();
        SkASSERT(SkIsAlign4(size));

        // Allocate enough memory to hold SkFlatData struct and the flat data itself.
        size_t allocSize = sizeof(SkFlatData) + size;
        SkFlatData* result = (SkFlatData*) controller->allocThrow(allocSize);

        // Put the serialized contents into the data section of the new allocation.
        buffer.writeToMemory(result->data());
        // Stamp the index, size and checksum in the header.
        result->stampHeader(index, SkToS32(size));
        return result;
    }

    // Unflatten this into result, using bitmapHeap and facePlayback for bitmaps and fonts if given
    template <typename Traits, typename T>
    void unflatten(T* result,
                   SkBitmapHeap* bitmapHeap = NULL,
                   SkTypefacePlayback* facePlayback = NULL) const {
        SkReadBuffer buffer(this->data(), fFlatSize);

        if (bitmapHeap) {
            buffer.setBitmapStorage(bitmapHeap);
        }
        if (facePlayback) {
            facePlayback->setupBuffer(buffer);
        }

        Traits::Unflatten(buffer, result);
        SkASSERT(fFlatSize == (int32_t)buffer.offset());
    }

    // Do these contain the same data?  Ignores index() and topBot().
    bool operator==(const SkFlatData& that) const {
        if (this->checksum() != that.checksum() || this->flatSize() != that.flatSize()) {
            return false;
        }
        return memcmp(this->data(), that.data(), this->flatSize()) == 0;
    }

    int index() const { return fIndex; }
    const uint8_t* data() const { return (const uint8_t*)this + sizeof(*this); }
    size_t flatSize() const { return fFlatSize; }
    uint32_t checksum() const { return fChecksum; }

    // Returns true if fTopBot[] has been recorded.
    bool isTopBotWritten() const {
        return !SkScalarIsNaN(fTopBot[0]);
    }

    // Returns fTopBot array, so it can be passed to a routine to compute them.
    // For efficiency, we assert that fTopBot have not been recorded yet.
    SkScalar* writableTopBot() const {
        SkASSERT(!this->isTopBotWritten());
        return fTopBot;
    }

    // Return the topbot[] after it has been recorded.
    const SkScalar* topBot() const {
        SkASSERT(this->isTopBotWritten());
        return fTopBot;
    }

private:
    struct HashTraits {
        static const SkFlatData& GetKey(const SkFlatData& flat) { return flat; }
        static uint32_t Hash(const SkFlatData& flat) { return flat.checksum(); }
    };

    void setIndex(int index) { fIndex = index; }
    uint8_t* data() { return (uint8_t*)this + sizeof(*this); }

    // This assumes the payload flat data has already been written and does not modify it.
    void stampHeader(int index, int32_t size) {
        SkASSERT(SkIsAlign4(size));
        fIndex     = index;
        fFlatSize  = size;
        fTopBot[0] = SK_ScalarNaN;  // Mark as unwritten.
        fChecksum  = SkChecksum::Compute((uint32_t*)this->data(), size);
    }

    int fIndex;
    int32_t fFlatSize;
    uint32_t fChecksum;
    mutable SkScalar fTopBot[2];  // Cache of FontMetrics fTop, fBottom.  Starts as [NaN,?].
    // uint32_t flattenedData[] implicitly hangs off the end.

    template <typename T, typename Traits> friend class SkFlatDictionary;
};

template <typename T, typename Traits>
class SkFlatDictionary {
public:
    explicit SkFlatDictionary(SkFlatController* controller)
    : fController(SkRef(controller))
    , fScratch(controller->getWriteBufferFlags())
    , fReady(false) {
        this->reset();
    }

    /**
     * Clears the dictionary of all entries. However, it does NOT free the
     * memory that was allocated for each entry (that's owned by controller).
     */
    void reset() {
        fIndexedData.rewind();
    }

    int count() const {
        SkASSERT(fHash.count() == fIndexedData.count());
        return fHash.count();
    }

    // For testing only.  Index is zero-based.
    const SkFlatData* operator[](int index) {
        return fIndexedData[index];
    }

    /**
     * Given an element of type T return its 1-based index in the dictionary. If
     * the element wasn't previously in the dictionary it is automatically
     * added.
     *
     */
    int find(const T& element) {
        return this->findAndReturnFlat(element)->index();
    }

    /**
     * Similar to find. Allows the caller to specify an SkFlatData to replace in
     * the case of an add. Also tells the caller whether a new SkFlatData was
     * added and whether the old one was replaced. The parameters added and
     * replaced are required to be non-NULL. Rather than returning the index of
     * the entry in the dictionary, it returns the actual SkFlatData.
     */
    const SkFlatData* findAndReplace(const T& element,
                                     const SkFlatData* toReplace,
                                     bool* added,
                                     bool* replaced) {
        SkASSERT(added != NULL && replaced != NULL);

        const int oldCount = this->count();
        SkFlatData* flat = this->findAndReturnMutableFlat(element);
        *added = this->count() > oldCount;

        // If we don't want to replace anything, we're done.
        if (!*added || toReplace == NULL) {
            *replaced = false;
            return flat;
        }

        // If we don't have the thing to replace, we're done.
        const SkFlatData* found = fHash.find(*toReplace);
        if (found == NULL) {
            *replaced = false;
            return flat;
        }

        // findAndReturnMutableFlat put flat at the back.  Swap it into found->index() instead.
        // indices in SkFlatData are 1-based, while fIndexedData is 0-based.  Watch out!
        SkASSERT(flat->index() == this->count());
        flat->setIndex(found->index());
        fIndexedData.removeShuffle(found->index()-1);
        SkASSERT(flat == fIndexedData[found->index()-1]);

        // findAndReturnMutableFlat already called fHash.add(), so we just clean up the old entry.
        fHash.remove(*found);
        fController->unalloc((void*)found);
        SkASSERT(this->count() == oldCount);

        *replaced = true;
        return flat;
    }

    /**
     * Unflatten the specific object at the given index.
     * Caller takes ownership of the result.
     */
    T* unflatten(int index) const {
        // index is 1-based, while fIndexedData is 0-based.
        const SkFlatData* element = fIndexedData[index-1];
        SkASSERT(index == element->index());

        T* dst = new T;
        this->unflatten(dst, element);
        return dst;
    }

    /**
     * Find or insert a flattened version of element into the dictionary.
     * Caller does not take ownership of the result.  This will not return NULL.
     */
    const SkFlatData* findAndReturnFlat(const T& element) {
        return this->findAndReturnMutableFlat(element);
    }

private:
    // We have to delay fScratch's initialization until its first use; fController might not
    // be fully set up by the time we get it in the constructor.
    void lazyInit() {
        if (fReady) {
            return;
        }

        // Without a bitmap heap, we'll flatten bitmaps into paints.  That's never what you want.
        SkASSERT(fController->getBitmapHeap() != NULL);
        fScratch.setBitmapHeap(fController->getBitmapHeap());
        fScratch.setTypefaceRecorder(fController->getTypefaceSet());
        fScratch.setNamedFactoryRecorder(fController->getNamedFactorySet());
        fReady = true;
    }

    // As findAndReturnFlat, but returns a mutable pointer for internal use.
    SkFlatData* findAndReturnMutableFlat(const T& element) {
        // Only valid until the next call to resetScratch().
        const SkFlatData& scratch = this->resetScratch(element, this->count()+1);

        SkFlatData* candidate = fHash.find(scratch);
        if (candidate != NULL) {
            return candidate;
        }

        SkFlatData* detached = this->detachScratch();
        fHash.add(detached);
        *fIndexedData.append() = detached;
        SkASSERT(fIndexedData.top()->index() == this->count());
        return detached;
    }

    // This reference is valid only until the next call to resetScratch() or detachScratch().
    const SkFlatData& resetScratch(const T& element, int index) {
        this->lazyInit();

        // Layout of fScratch: [ SkFlatData header, 20 bytes ] [ data ..., 4-byte aligned ]
        fScratch.reset();
        fScratch.reserve(sizeof(SkFlatData));
        Traits::Flatten(fScratch, element);
        const size_t dataSize = fScratch.bytesWritten() - sizeof(SkFlatData);

        // Reinterpret data in fScratch as an SkFlatData.
        SkFlatData* scratch = (SkFlatData*)fScratch.getWriter32()->contiguousArray();
        SkASSERT(scratch != NULL);
        scratch->stampHeader(index, SkToS32(dataSize));
        return *scratch;
    }

    // This result is owned by fController and lives as long as it does (unless unalloc'd).
    SkFlatData* detachScratch() {
        // Allocate a new SkFlatData exactly big enough to hold our current scratch.
        // We use the controller for this allocation to extend the allocation's lifetime and allow
        // the controller to do whatever memory management it wants.
        SkFlatData* detached = (SkFlatData*)fController->allocThrow(fScratch.bytesWritten());

        // Copy scratch into the new SkFlatData.
        SkFlatData* scratch = (SkFlatData*)fScratch.getWriter32()->contiguousArray();
        SkASSERT(scratch != NULL);
        memcpy(detached, scratch, fScratch.bytesWritten());

        // We can now reuse fScratch, and detached will live until fController dies.
        return detached;
    }

    void unflatten(T* dst, const SkFlatData* element) const {
        element->unflatten<Traits>(dst,
                                   fController->getBitmapHeap(),
                                   fController->getTypefacePlayback());
    }

    // All SkFlatData* stored in fIndexedData and fHash are owned by the controller.
    SkAutoTUnref<SkFlatController> fController;
    SkWriteBuffer fScratch;
    bool fReady;

    // For index -> SkFlatData.  0-based, while all indices in the API are 1-based.  Careful!
    SkTDArray<const SkFlatData*> fIndexedData;

    // For SkFlatData -> cached SkFlatData, which has index().
    SkTDynamicHash<SkFlatData, SkFlatData, SkFlatData::HashTraits> fHash;
};

#endif

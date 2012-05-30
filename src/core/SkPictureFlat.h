
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureFlat_DEFINED
#define SkPictureFlat_DEFINED

#include "SkChunkAlloc.h"
#include "SkBitmap.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkPicture.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkTSearch.h"

enum DrawType {
    UNUSED,
    CLIP_PATH,
    CLIP_REGION,
    CLIP_RECT,
    CONCAT,
    DRAW_BITMAP,
    DRAW_BITMAP_MATRIX,
    DRAW_BITMAP_NINE,
    DRAW_BITMAP_RECT,
    DRAW_CLEAR,
    DRAW_DATA,
    DRAW_PAINT,
    DRAW_PATH,
    DRAW_PICTURE,
    DRAW_POINTS,
    DRAW_POS_TEXT,
    DRAW_POS_TEXT_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT
    DRAW_POS_TEXT_H,
    DRAW_POS_TEXT_H_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT_H
    DRAW_RECT,
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
    TRANSLATE
};

enum DrawVertexFlags {
    DRAW_VERTICES_HAS_TEXS    = 0x01,
    DRAW_VERTICES_HAS_COLORS  = 0x02,
    DRAW_VERTICES_HAS_INDICES = 0x04
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

class SkRefCntPlayback {
public:
    SkRefCntPlayback();
    virtual ~SkRefCntPlayback();
    
    int count() const { return fCount; }
    
    void reset(const SkRefCntSet*);

    void setCount(int count);
    SkRefCnt* set(int index, SkRefCnt*);

    virtual void setupBuffer(SkFlattenableReadBuffer& buffer) const {
        buffer.setRefCntArray(fArray, fCount);
    }
    
protected:
    int fCount;
    SkRefCnt** fArray;
};

class SkTypefacePlayback : public SkRefCntPlayback {
public:
    virtual void setupBuffer(SkFlattenableReadBuffer& buffer) const {
        buffer.setTypefaceArray((SkTypeface**)fArray, fCount);
    }
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

    void setupBuffer(SkFlattenableReadBuffer& buffer) const {
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
//                   details of that operation are hidden in the provided procs
// SkFlatDictionary: is a abstract templated dictionary that maintains a
//                   searchable set of SkFlataData objects of type T.
//
// NOTE: any class that wishes to be used in conjunction with SkFlatDictionary
// must subclass the dictionary and provide the necessary flattening procs.
// The end of this header contains dictionary subclasses for some common classes
// like SkBitmap, SkMatrix, SkPaint, and SkRegion.
//
//
///////////////////////////////////////////////////////////////////////////////

class SkFlatData {
public:

    static int Compare(const SkFlatData* a, const SkFlatData* b) {
        return memcmp(a->data(), b->data(), a->fAllocSize);
    }
    
    int index() const { return fIndex; }
    void* data() const { return (char*)this + sizeof(*this); }
    
#ifdef SK_DEBUG_SIZE
    size_t size() const { return sizeof(SkFlatData) + fAllocSize; }
#endif

    static SkFlatData* Create(SkChunkAlloc* heap, const void* obj, int index,
                              void (*flattenProc)(SkOrderedWriteBuffer&, const void*),
                              SkRefCntSet* refCntRecorder = NULL,
                              SkRefCntSet* faceRecorder = NULL);
    void unflatten(void* result,
                   void (*unflattenProc)(SkOrderedReadBuffer&, void*),
                   SkRefCntPlayback* refCntPlayback = NULL,
                   SkTypefacePlayback* facePlayback = NULL) const;

private:
    int fIndex;
    int32_t fAllocSize;
};

template <class T>
class SkFlatDictionary {
public:
    SkFlatDictionary(SkChunkAlloc* heap) {
        fFlattenProc = NULL;
        fUnflattenProc = NULL;
        fHeap = heap;
        // set to 1 since returning a zero from find() indicates failure
        fNextIndex = 1;
    }

    int count() const { return fData.count(); }

    const SkFlatData*  operator[](int index) const {
        SkASSERT(index >= 0 && index < fData.count());
        return fData[index];
    }

    /**
     * Clears the dictionary of all entries. However, it does NOT free the
     * memory that was allocated for each entry.
     */
    void reset() { fData.reset(); fNextIndex = 1; }

    /**
     * Given an element of type T it returns its index in the dictionary. If
     * the element wasn't previously in the dictionary it is automatically added
     */
    int find(const T* element, SkRefCntSet* refCntRecorder = NULL,
             SkRefCntSet* faceRecorder = NULL) {
        if (element == NULL)
            return 0;
        SkFlatData* flat = SkFlatData::Create(fHeap, element, fNextIndex,
                fFlattenProc, refCntRecorder, faceRecorder);
        int index = SkTSearch<SkFlatData>((const SkFlatData**) fData.begin(),
                fData.count(), flat, sizeof(flat), &SkFlatData::Compare);
        if (index >= 0) {
            (void)fHeap->unalloc(flat);
            return fData[index]->index();
        }
        index = ~index;
        *fData.insert(index) = flat;
        SkASSERT(fData.count() == fNextIndex);
        return fNextIndex++;
    }

    /**
     * Given a pointer to a array of type T we allocate the array and fill it
     * with the unflattened dictionary contents. The return value is the size of
     * the allocated array.
     */
    int unflattenDictionary(T*& array, SkRefCntPlayback* refCntPlayback = NULL,
            SkTypefacePlayback* facePlayback = NULL) const {
        int elementCount = fData.count();
        if (elementCount > 0) {
            array = SkNEW_ARRAY(T, elementCount);
            for (const SkFlatData** elementPtr = fData.begin();
                    elementPtr != fData.end(); elementPtr++) {
                const SkFlatData* element = *elementPtr;
                int index = element->index() - 1;
                element->unflatten(&array[index], fUnflattenProc,
                                   refCntPlayback, facePlayback);
            }
        }
        return elementCount;
    }

protected:
    void (*fFlattenProc)(SkOrderedWriteBuffer&, const void*);
    void (*fUnflattenProc)(SkOrderedReadBuffer&, void*);

private:
    SkChunkAlloc* fHeap;
    int fNextIndex;
    SkTDArray<const SkFlatData*> fData;
};

///////////////////////////////////////////////////////////////////////////////
// Some common dictionaries are defined here for both reference and convenience
///////////////////////////////////////////////////////////////////////////////

template <class T>
static void SkFlattenObjectProc(SkOrderedWriteBuffer& buffer, const void* obj) {
    ((T*)obj)->flatten(buffer);
}

template <class T>
static void SkUnflattenObjectProc(SkOrderedReadBuffer& buffer, void* obj) {
    ((T*)obj)->unflatten(buffer);
}

class SkBitmapDictionary : public SkFlatDictionary<SkBitmap> {
public:
    SkBitmapDictionary(SkChunkAlloc* heap) : SkFlatDictionary<SkBitmap>(heap) {
        fFlattenProc = &SkFlattenObjectProc<SkBitmap>;
        fUnflattenProc = &SkUnflattenObjectProc<SkBitmap>;
    }
};

class SkMatrixDictionary : public SkFlatDictionary<SkMatrix> {
 public:
    SkMatrixDictionary(SkChunkAlloc* heap) : SkFlatDictionary<SkMatrix>(heap) {
        fFlattenProc = &flattenMatrix;
        fUnflattenProc = &unflattenMatrix;
    }

    static void flattenMatrix(SkOrderedWriteBuffer& buffer, const void* obj) {
        buffer.getWriter32()->writeMatrix(*(SkMatrix*)obj);
    }

    static void unflattenMatrix(SkOrderedReadBuffer& buffer, void* obj) {
        buffer.getReader32()->readMatrix((SkMatrix*)obj);
    }
};

class SkPaintDictionary : public SkFlatDictionary<SkPaint> {
 public:
    SkPaintDictionary(SkChunkAlloc* heap) : SkFlatDictionary<SkPaint>(heap) {
        fFlattenProc = &SkFlattenObjectProc<SkPaint>;
        fUnflattenProc = &SkUnflattenObjectProc<SkPaint>;
    }
};

class SkRegionDictionary : public SkFlatDictionary<SkRegion> {
 public:
    SkRegionDictionary(SkChunkAlloc* heap) : SkFlatDictionary<SkRegion>(heap) {
        fFlattenProc = &flattenRegion;
        fUnflattenProc = &unflattenRegion;
    }

    static void flattenRegion(SkOrderedWriteBuffer& buffer, const void* obj) {
        buffer.getWriter32()->writeRegion(*(SkRegion*)obj);
    }

    static void unflattenRegion(SkOrderedReadBuffer& buffer, void* obj) {
        buffer.getReader32()->readRegion((SkRegion*)obj);
    }
};

#endif

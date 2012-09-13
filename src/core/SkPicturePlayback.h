
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "SkPicture.h"
#include "SkReader32.h"

#include "SkBitmap.h"
#include "SkData.h"
#include "SkMatrix.h"
#include "SkOrderedReadBuffer.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathHeap.h"
#include "SkRegion.h"
#include "SkPictureFlat.h"

#ifdef SK_BUILD_FOR_ANDROID
#include "SkThread.h"
#endif

class SkPictureRecord;
class SkStream;
class SkWStream;
class SkBBoxHierarchy;
class SkPictureStateTree;

struct SkPictInfo {
    enum Flags {
        kCrossProcess_Flag      = 1 << 0,
        kScalarIsFloat_Flag     = 1 << 1,
        kPtrIs64Bit_Flag        = 1 << 2,
    };

    uint32_t    fVersion;
    uint32_t    fWidth;
    uint32_t    fHeight;
    uint32_t    fFlags;
};

/**
 * Container for data that is needed to deep copy a SkPicture. The container
 * enables the data to be generated once and reused for subsequent copies.
 */
struct SkPictCopyInfo {
    SkPictCopyInfo() : initialized(false), controller(1024) {}

    bool initialized;
    SkChunkFlatController controller;
    SkTDArray<SkFlatData*> paintData;
};

class SkPicturePlayback {
public:
    SkPicturePlayback();
    SkPicturePlayback(const SkPicturePlayback& src, SkPictCopyInfo* deepCopyInfo = NULL);
    explicit SkPicturePlayback(const SkPictureRecord& record, bool deepCopy = false);
    SkPicturePlayback(SkStream*, const SkPictInfo&, bool* isValid);

    virtual ~SkPicturePlayback();

    void draw(SkCanvas& canvas);

    void serialize(SkWStream*) const;

    void dumpSize() const;

    // Can be called in the middle of playback (the draw() call). WIll abort the
    // drawing and return from draw() after the "current" op code is done
    void abort();

private:
    class TextContainer {
    public:
        size_t length() { return fByteLength; }
        const void* text() { return (const void*) fText; }
        size_t fByteLength;
        const char* fText;
    };

    const SkBitmap& getBitmap(SkReader32& reader) {
        int index = reader.readInt();
        return (*fBitmaps)[index];
    }

    const SkMatrix* getMatrix(SkReader32& reader) {
        int index = reader.readInt();
        if (index == 0) {
            return NULL;
        }
        return &(*fMatrices)[index - 1];
    }

    const SkPath& getPath(SkReader32& reader) {
        return (*fPathHeap)[reader.readInt() - 1];
    }

    SkPicture& getPicture(SkReader32& reader) {
        int index = reader.readInt();
        SkASSERT(index > 0 && index <= fPictureCount);
        return *fPictureRefs[index - 1];
    }

    const SkPaint* getPaint(SkReader32& reader) {
        int index = reader.readInt();
        if (index == 0) {
            return NULL;
        }
        return &(*fPaints)[index - 1];
    }

    const SkRect* getRectPtr(SkReader32& reader) {
        if (reader.readBool()) {
            return &reader.skipT<SkRect>();
        } else {
            return NULL;
        }
    }

    const SkIRect* getIRectPtr(SkReader32& reader) {
        if (reader.readBool()) {
            return &reader.skipT<SkIRect>();
        } else {
            return NULL;
        }
    }

    const SkRegion& getRegion(SkReader32& reader) {
        int index = reader.readInt();
        return (*fRegions)[index - 1];
    }

    void getText(SkReader32& reader, TextContainer* text) {
        size_t length = text->fByteLength = reader.readInt();
        text->fText = (const char*)reader.skip(length);
    }

    void init();

#ifdef SK_DEBUG_SIZE
public:
    int size(size_t* sizePtr);
    int bitmaps(size_t* size);
    int paints(size_t* size);
    int paths(size_t* size);
    int regions(size_t* size);
#endif

#ifdef SK_DEBUG_DUMP
private:
    void dumpBitmap(const SkBitmap& bitmap) const;
    void dumpMatrix(const SkMatrix& matrix) const;
    void dumpPaint(const SkPaint& paint) const;
    void dumpPath(const SkPath& path) const;
    void dumpPicture(const SkPicture& picture) const;
    void dumpRegion(const SkRegion& region) const;
    int dumpDrawType(char* bufferPtr, char* buffer, DrawType drawType);
    int dumpInt(char* bufferPtr, char* buffer, char* name);
    int dumpRect(char* bufferPtr, char* buffer, char* name);
    int dumpPoint(char* bufferPtr, char* buffer, char* name);
    void dumpPointArray(char** bufferPtrPtr, char* buffer, int count);
    int dumpPtr(char* bufferPtr, char* buffer, char* name, void* ptr);
    int dumpRectPtr(char* bufferPtr, char* buffer, char* name);
    int dumpScalar(char* bufferPtr, char* buffer, char* name);
    void dumpText(char** bufferPtrPtr, char* buffer);
    void dumpStream();

public:
    void dump() const;
#endif

private:    // these help us with reading/writing
    bool parseStreamTag(SkStream*, const SkPictInfo&, uint32_t tag, size_t size);
    bool parseBufferTag(SkOrderedReadBuffer&, uint32_t tag, size_t size);
    void flattenToBuffer(SkOrderedWriteBuffer&) const;

private:
    SkAutoTUnref<SkBitmapHeap> fBitmapHeap;
    SkAutoTUnref<SkPathHeap> fPathHeap;

    SkTRefArray<SkBitmap>* fBitmaps;
    SkTRefArray<SkMatrix>* fMatrices;
    SkTRefArray<SkPaint>* fPaints;
    SkTRefArray<SkRegion>* fRegions;

    SkData* fOpData;    // opcodes and parameters

    SkPicture** fPictureRefs;
    int fPictureCount;

    SkBBoxHierarchy* fBoundingHierarchy;
    SkPictureStateTree* fStateTree;

    SkTypefacePlayback fTFPlayback;
    SkFactoryPlayback* fFactoryPlayback;
#ifdef SK_BUILD_FOR_ANDROID
    SkMutex fDrawMutex;
#endif
};

#endif


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
#include "SkReadBuffer.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathHeap.h"
#include "SkRegion.h"
#include "SkRRect.h"
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

    char        fMagic[8];
    uint32_t    fVersion;
    uint32_t    fWidth;
    uint32_t    fHeight;
    uint32_t    fFlags;
};

#define SK_PICT_READER_TAG     SkSetFourByteTag('r', 'e', 'a', 'd')
#define SK_PICT_FACTORY_TAG    SkSetFourByteTag('f', 'a', 'c', 't')
#define SK_PICT_TYPEFACE_TAG   SkSetFourByteTag('t', 'p', 'f', 'c')
#define SK_PICT_PICTURE_TAG    SkSetFourByteTag('p', 'c', 't', 'r')

// This tag specifies the size of the ReadBuffer, needed for the following tags
#define SK_PICT_BUFFER_SIZE_TAG     SkSetFourByteTag('a', 'r', 'a', 'y')
// these are all inside the ARRAYS tag
#define SK_PICT_BITMAP_BUFFER_TAG  SkSetFourByteTag('b', 't', 'm', 'p')
#define SK_PICT_PAINT_BUFFER_TAG   SkSetFourByteTag('p', 'n', 't', ' ')
#define SK_PICT_PATH_BUFFER_TAG    SkSetFourByteTag('p', 't', 'h', ' ')

// Always write this guy last (with no length field afterwards)
#define SK_PICT_EOF_TAG     SkSetFourByteTag('e', 'o', 'f', ' ')

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
    SkPicturePlayback(const SkPicture* picture, const SkPicturePlayback& src, 
                      SkPictCopyInfo* deepCopyInfo = NULL);
    SkPicturePlayback(const SkPicture* picture, const SkPictureRecord& record, const SkPictInfo&, 
                      bool deepCopy = false);
    static SkPicturePlayback* CreateFromStream(SkPicture* picture,
                                               SkStream*,
                                               const SkPictInfo&,
                                               SkPicture::InstallPixelRefProc);
    static SkPicturePlayback* CreateFromBuffer(SkPicture* picture, 
                                               SkReadBuffer&, 
                                               const SkPictInfo&);

    virtual ~SkPicturePlayback();

    const SkPicture::OperationList& getActiveOps(const SkIRect& queryRect);

    void draw(SkCanvas& canvas, SkDrawPictureCallback*);

    void serialize(SkWStream*, SkPicture::EncodeBitmap) const;
    void flatten(SkWriteBuffer&) const;

    void dumpSize() const;

    bool containsBitmaps() const;

#ifdef SK_BUILD_FOR_ANDROID
    // Can be called in the middle of playback (the draw() call). WIll abort the
    // drawing and return from draw() after the "current" op code is done
    void abort() { fAbortCurrentPlayback = true; }
#endif

    size_t curOpID() const { return fCurOffset; }
    void resetOpID() { fCurOffset = 0; }

protected:
    explicit SkPicturePlayback(const SkPicture* picture, const SkPictInfo& info);

    bool parseStream(SkPicture* picture, SkStream*, SkPicture::InstallPixelRefProc);
    bool parseBuffer(SkPicture* picture, SkReadBuffer& buffer);
#ifdef SK_DEVELOPER
    virtual bool preDraw(int opIndex, int type);
    virtual void postDraw(int opIndex);
#endif

private:
    class TextContainer {
    public:
        size_t length() { return fByteLength; }
        const void* text() { return (const void*) fText; }
        size_t fByteLength;
        const char* fText;
    };

    const SkBitmap& getBitmap(SkReader32& reader) {
        const int index = reader.readInt();
        if (SkBitmapHeap::INVALID_SLOT == index) {
#ifdef SK_DEBUG
            SkDebugf("An invalid bitmap was recorded!\n");
#endif
            return fBadBitmap;
        }
        return (*fBitmaps)[index];
    }

    void getMatrix(SkReader32& reader, SkMatrix* matrix) {
        reader.readMatrix(matrix);
    }

    const SkPath& getPath(SkReader32& reader) {
        return fPicture->getPath(reader.readInt() - 1);
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

    void getRegion(SkReader32& reader, SkRegion* region) {
        reader.readRegion(region);
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
    bool parseStreamTag(SkPicture* picture, SkStream*, uint32_t tag, uint32_t size, 
                        SkPicture::InstallPixelRefProc);
    bool parseBufferTag(SkPicture* picture, SkReadBuffer&, uint32_t tag, uint32_t size);
    void flattenToBuffer(SkWriteBuffer&) const;

private:
    // The picture that owns this SkPicturePlayback object
    const SkPicture* fPicture;

    // Only used by getBitmap() if the passed in index is SkBitmapHeap::INVALID_SLOT. This empty
    // bitmap allows playback to draw nothing and move on.
    SkBitmap fBadBitmap;

    SkAutoTUnref<SkBitmapHeap> fBitmapHeap;

    SkTRefArray<SkBitmap>* fBitmaps;
    SkTRefArray<SkPaint>* fPaints;

    SkData* fOpData;    // opcodes and parameters

    SkPicture** fPictureRefs;
    int fPictureCount;

    SkBBoxHierarchy* fBoundingHierarchy;
    SkPictureStateTree* fStateTree;

    class CachedOperationList : public SkPicture::OperationList {
    public:
        CachedOperationList() {
            fCacheQueryRect.setEmpty();
        }

        virtual bool valid() const { return true; }
        virtual int numOps() const SK_OVERRIDE { return fOps.count(); }
        virtual uint32_t offset(int index) const SK_OVERRIDE;
        virtual const SkMatrix& matrix(int index) const SK_OVERRIDE;

        // The query rect for which the cached active ops are valid
        SkIRect          fCacheQueryRect;

        // The operations which are active within 'fCachedQueryRect'
        SkTDArray<void*> fOps;

    private:
        typedef SkPicture::OperationList INHERITED;
    };

    CachedOperationList* fCachedActiveOps;

    SkTypefacePlayback fTFPlayback;
    SkFactoryPlayback* fFactoryPlayback;

    // The offset of the current operation when within the draw method
    size_t fCurOffset;

    const SkPictInfo fInfo;

    static void WriteFactories(SkWStream* stream, const SkFactorySet& rec);
    static void WriteTypefaces(SkWStream* stream, const SkRefCntSet& rec);

#ifdef SK_BUILD_FOR_ANDROID
    SkMutex fDrawMutex;
    bool fAbortCurrentPlayback;
#endif
};

#endif

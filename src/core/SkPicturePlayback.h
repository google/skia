
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "SkBitmap.h"
#include "SkPathHeap.h"
#include "SkPicture.h"
#include "SkPictureFlat.h"

#ifdef SK_BUILD_FOR_ANDROID
#include "SkThread.h"
#endif

class SkData;
class SkPictureRecord;
class SkReader32;
class SkStream;
class SkWStream;
class SkBBoxHierarchy;
class SkMatrix;
class SkPaint;
class SkPath;
class SkPictureStateTree;
class SkReadBuffer;
class SkRegion;

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

// SkPictureContentInfo is not serialized! It is intended solely for use
// with suitableForGpuRasterization.
class SkPictureContentInfo {
public:
    SkPictureContentInfo() { this->reset(); }

    SkPictureContentInfo(const SkPictureContentInfo& src) { this->set(src); }

    void set(const SkPictureContentInfo& src) {
        fNumPaintWithPathEffectUses = src.fNumPaintWithPathEffectUses;
        fNumAAConcavePaths = src.fNumAAConcavePaths;
        fNumAAHairlineConcavePaths = src.fNumAAHairlineConcavePaths;
    }

    void reset() {
        fNumPaintWithPathEffectUses = 0;
        fNumAAConcavePaths = 0;
        fNumAAHairlineConcavePaths = 0;
    }

    void swap(SkPictureContentInfo* other) {
        SkTSwap(fNumPaintWithPathEffectUses, other->fNumPaintWithPathEffectUses);
        SkTSwap(fNumAAConcavePaths, other->fNumAAConcavePaths);
        SkTSwap(fNumAAHairlineConcavePaths, other->fNumAAHairlineConcavePaths);
    }

    void incPaintWithPathEffectUses() { ++fNumPaintWithPathEffectUses; }
    int numPaintWithPathEffectUses() const { return fNumPaintWithPathEffectUses; }

    void incAAConcavePaths() { ++fNumAAConcavePaths; }
    int numAAConcavePaths() const { return fNumAAConcavePaths; }

    void incAAHairlineConcavePaths() {
        ++fNumAAHairlineConcavePaths;
        SkASSERT(fNumAAHairlineConcavePaths <= fNumAAConcavePaths);
    }
    int numAAHairlineConcavePaths() const { return fNumAAHairlineConcavePaths; }

private:
    // This field is incremented every time a paint with a path effect is
    // used (i.e., it is not a de-duplicated count)
    int fNumPaintWithPathEffectUses;
    // This field is incremented every time an anti-aliased drawPath call is
    // issued with a concave path
    int fNumAAConcavePaths;
    // This field is incremented every time a drawPath call is
    // issued for a hairline stroked concave path.
    int fNumAAHairlineConcavePaths;
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
    SkPicturePlayback(const SkPicturePlayback& src,
                      SkPictCopyInfo* deepCopyInfo = NULL);
    SkPicturePlayback(const SkPictureRecord& record, const SkPictInfo&, bool deepCopyOps);
    static SkPicturePlayback* CreateFromStream(SkStream*,
                                               const SkPictInfo&,
                                               SkPicture::InstallPixelRefProc);
    static SkPicturePlayback* CreateFromBuffer(SkReadBuffer&,
                                               const SkPictInfo&);

    virtual ~SkPicturePlayback();

    const SkPicture::OperationList& getActiveOps(const SkIRect& queryRect);

    void setUseBBH(bool useBBH) { fUseBBH = useBBH; }

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
    explicit SkPicturePlayback(const SkPictInfo& info);

    bool parseStream(SkStream*, SkPicture::InstallPixelRefProc);
    bool parseBuffer(SkReadBuffer& buffer);
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
        int index = reader.readInt() - 1;
        return (*fPathHeap.get())[index];
    }

    const SkPicture* getPicture(SkReader32& reader) {
        int index = reader.readInt();
        SkASSERT(index > 0 && index <= fPictureCount);
        return fPictureRefs[index - 1];
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

#if SK_SUPPORT_GPU
    bool suitableForGpuRasterization(GrContext* context, const char **reason) const;
#endif

private:    // these help us with reading/writing
    bool parseStreamTag(SkStream*, uint32_t tag, uint32_t size, SkPicture::InstallPixelRefProc);
    bool parseBufferTag(SkReadBuffer&, uint32_t tag, uint32_t size);
    void flattenToBuffer(SkWriteBuffer&) const;

private:
    friend class SkPicture;
    friend class SkGpuDevice;   // for access to setDrawLimits & setReplacements

    // Only used by getBitmap() if the passed in index is SkBitmapHeap::INVALID_SLOT. This empty
    // bitmap allows playback to draw nothing and move on.
    SkBitmap fBadBitmap;

    SkAutoTUnref<SkBitmapHeap> fBitmapHeap;

    SkTRefArray<SkBitmap>* fBitmaps;
    SkTRefArray<SkPaint>* fPaints;

    SkData* fOpData;    // opcodes and parameters

    SkAutoTUnref<const SkPathHeap> fPathHeap;  // reference counted

    const SkPicture** fPictureRefs;
    int fPictureCount;

    SkBBoxHierarchy* fBoundingHierarchy;
    SkPictureStateTree* fStateTree;

    SkPictureContentInfo fContentInfo;

    // Limit the opcode playback to be between the offsets 'start' and 'stop'.
    // The opcode at 'start' should be a saveLayer while the opcode at
    // 'stop' should be a restore. Neither of those commands will be issued.
    // Set both start & stop to 0 to disable draw limiting
    // Draw limiting cannot be enabled at the same time as draw replacing
    void setDrawLimits(size_t start, size_t stop) {
        SkASSERT(NULL == fReplacements);
        fStart = start;
        fStop = stop;
    }

    // PlaybackReplacements collects op ranges that can be replaced with
    // a single drawBitmap call (using a precomputed bitmap).
    class PlaybackReplacements {
    public:
        // All the operations between fStart and fStop (inclusive) will be replaced with
        // a single drawBitmap call using fPos, fBM and fPaint.
        // fPaint will be NULL if the picture's paint wasn't copyable
        struct ReplacementInfo {
            size_t          fStart;
            size_t          fStop;
            SkIPoint        fPos;
            SkBitmap*       fBM;
            const SkPaint*  fPaint;  // Note: this object doesn't own the paint
        };

        ~PlaybackReplacements() { this->freeAll(); }

        // Add a new replacement range. The replacement ranges should be
        // sorted in increasing order and non-overlapping (esp. no nested
        // saveLayers).
        ReplacementInfo* push();

    private:
        friend class SkPicturePlayback; // for access to lookupByStart

        // look up a replacement range by its start offset
        ReplacementInfo* lookupByStart(size_t start);

        void freeAll();

#ifdef SK_DEBUG
        void validate() const;
#endif

        SkTDArray<ReplacementInfo> fReplacements;
    };

    // Replace all the draw ops in the replacement ranges in 'replacements' with
    // the associated drawBitmap call
    // Draw replacing cannot be enabled at the same time as draw limiting
    void setReplacements(PlaybackReplacements* replacements) {
        SkASSERT(fStart == 0 && fStop == 0);
        fReplacements = replacements;
    }

    bool   fUseBBH;
    size_t fStart;
    size_t fStop;
    PlaybackReplacements* fReplacements;

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

    void initForPlayback() const;

#ifdef SK_BUILD_FOR_ANDROID
    SkMutex fDrawMutex;
    bool fAbortCurrentPlayback;
#endif
};

#endif

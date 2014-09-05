/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureData_DEFINED
#define SkPictureData_DEFINED

#include "SkBitmap.h"
#include "SkPathHeap.h"
#include "SkPicture.h"
#include "SkPictureContentInfo.h"
#include "SkPictureFlat.h"
#include "SkPictureStateTree.h"

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
class SkTextBlob;

struct SkPictInfo {
    enum Flags {
        kCrossProcess_Flag      = 1 << 0,
        kScalarIsFloat_Flag     = 1 << 1,
        kPtrIs64Bit_Flag        = 1 << 2,
    };

    char        fMagic[8];
    uint32_t    fVersion;
    SkRect      fCullRect;
    uint32_t    fFlags;
};

#define SK_PICT_READER_TAG     SkSetFourByteTag('r', 'e', 'a', 'd')
#define SK_PICT_FACTORY_TAG    SkSetFourByteTag('f', 'a', 'c', 't')
#define SK_PICT_TYPEFACE_TAG   SkSetFourByteTag('t', 'p', 'f', 'c')
#define SK_PICT_PICTURE_TAG    SkSetFourByteTag('p', 'c', 't', 'r')

// This tag specifies the size of the ReadBuffer, needed for the following tags
#define SK_PICT_BUFFER_SIZE_TAG     SkSetFourByteTag('a', 'r', 'a', 'y')
// these are all inside the ARRAYS tag
#define SK_PICT_BITMAP_BUFFER_TAG   SkSetFourByteTag('b', 't', 'm', 'p')
#define SK_PICT_PAINT_BUFFER_TAG    SkSetFourByteTag('p', 'n', 't', ' ')
#define SK_PICT_PATH_BUFFER_TAG     SkSetFourByteTag('p', 't', 'h', ' ')
#define SK_PICT_TEXTBLOB_BUFFER_TAG SkSetFourByteTag('b', 'l', 'o', 'b')

// Always write this guy last (with no length field afterwards)
#define SK_PICT_EOF_TAG     SkSetFourByteTag('e', 'o', 'f', ' ')

#ifdef SK_SUPPORT_LEGACY_PICTURE_CLONE
/**
 * Container for data that is needed to deep copy a SkPicture. The container
 * enables the data to be generated once and reused for subsequent copies.
 */
struct SkPictCopyInfo {
    SkPictCopyInfo() : controller(1024) {}

    SkChunkFlatController controller;
    SkTDArray<SkFlatData*> paintData;
};
#endif

class SkPictureData {
public:
#ifdef SK_SUPPORT_LEGACY_PICTURE_CLONE
    SkPictureData(const SkPictureData& src, SkPictCopyInfo* deepCopyInfo = NULL);
#endif
    SkPictureData(const SkPictureRecord& record, const SkPictInfo&, bool deepCopyOps);
    static SkPictureData* CreateFromStream(SkStream*,
                                           const SkPictInfo&,
                                           SkPicture::InstallPixelRefProc);
    static SkPictureData* CreateFromBuffer(SkReadBuffer&, const SkPictInfo&);

    virtual ~SkPictureData();

    const SkPicture::OperationList* getActiveOps(const SkRect& queryRect) const;

    void serialize(SkWStream*, SkPicture::EncodeBitmap) const;
    void flatten(SkWriteBuffer&) const;

    bool containsBitmaps() const;

    bool hasText() const { return fContentInfo.hasText(); }

    int opCount() const { return fContentInfo.numOperations(); }

    const SkData* opData() const { return fOpData; }

protected:
    explicit SkPictureData(const SkPictInfo& info);

    bool parseStream(SkStream*, SkPicture::InstallPixelRefProc);
    bool parseBuffer(SkReadBuffer& buffer);

public:
    const SkBitmap& getBitmap(SkReader32* reader) const {
        const int index = reader->readInt();
        if (SkBitmapHeap::INVALID_SLOT == index) {
#ifdef SK_DEBUG
            SkDebugf("An invalid bitmap was recorded!\n");
#endif
            return fBadBitmap;
        }
        return (*fBitmaps)[index];
    }

    const SkPath& getPath(SkReader32* reader) const {
        int index = reader->readInt() - 1;
        return (*fPathHeap.get())[index];
    }

    const SkPicture* getPicture(SkReader32* reader) const {
        int index = reader->readInt();
        SkASSERT(index > 0 && index <= fPictureCount);
        return fPictureRefs[index - 1];
    }

    const SkPaint* getPaint(SkReader32* reader) const {
        int index = reader->readInt();
        if (index == 0) {
            return NULL;
        }
        return &(*fPaints)[index - 1];
    }

    const SkTextBlob* getTextBlob(SkReader32* reader) const {
        int index = reader->readInt();
        SkASSERT(index > 0 && index <= fTextBlobCount);
        return fTextBlobRefs[index - 1];
    }

    void initIterator(SkPictureStateTree::Iterator* iter,
                      const SkTDArray<void*>& draws,
                      SkCanvas* canvas) const {
        if (fStateTree) {
            fStateTree->initIterator(iter, draws, canvas);
        }
    }

#if SK_SUPPORT_GPU
    /**
     * sampleCount is the number of samples-per-pixel or zero if non-MSAA.
     * It is defaulted to be zero.
     */
    bool suitableForGpuRasterization(GrContext* context, const char **reason,
                                     int sampleCount = 0) const;

    /**
     * Calls getRecommendedSampleCount with GrPixelConfig and dpi to calculate sampleCount
     * and then calls the above version of suitableForGpuRasterization
     */
    bool suitableForGpuRasterization(GrContext* context, const char **reason,
                                     GrPixelConfig config, SkScalar dpi) const;

    bool suitableForLayerOptimization() const;
#endif

private:
    friend class SkPicture; // needed in SkPicture::clone (rm when it is removed)

    void init();

    // these help us with reading/writing
    bool parseStreamTag(SkStream*, uint32_t tag, uint32_t size, SkPicture::InstallPixelRefProc);
    bool parseBufferTag(SkReadBuffer&, uint32_t tag, uint32_t size);
    void flattenToBuffer(SkWriteBuffer&) const;

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
    const SkTextBlob** fTextBlobRefs;
    int fTextBlobCount;

    SkBBoxHierarchy* fBoundingHierarchy;
    SkPictureStateTree* fStateTree;

    SkPictureContentInfo fContentInfo;

    SkTypefacePlayback fTFPlayback;
    SkFactoryPlayback* fFactoryPlayback;

    const SkPictInfo fInfo;

    static void WriteFactories(SkWStream* stream, const SkFactorySet& rec);
    static void WriteTypefaces(SkWStream* stream, const SkRefCntSet& rec);

    void initForPlayback() const;
};

#endif

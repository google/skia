/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureData_DEFINED
#define SkPictureData_DEFINED

#include "SkBitmap.h"
#include "SkDrawable.h"
#include "SkPicture.h"
#include "SkPictureContentInfo.h"
#include "SkPictureFlat.h"

class SkData;
class SkPictureRecord;
class SkPixelSerializer;
class SkReader32;
class SkStream;
class SkWStream;
class SkBBoxHierarchy;
class SkMatrix;
class SkPaint;
class SkPath;
class SkReadBuffer;
class SkTextBlob;

struct SkPictInfo {
    enum Flags {
        kCrossProcess_Flag      = 1 << 0,
        kScalarIsFloat_Flag     = 1 << 1,
        kPtrIs64Bit_Flag        = 1 << 2,
    };

    SkPictInfo() : fVersion(~0U) {}

    uint32_t getVersion() const {
        SkASSERT(fVersion != ~0U);
        return fVersion;
    }

    void setVersion(uint32_t version) {
        SkASSERT(version != ~0U);
        fVersion = version;
    }

public:
    char        fMagic[8];
private:
    uint32_t    fVersion;
public:
    SkRect      fCullRect;
    uint32_t    fFlags;
};

#define SK_PICT_READER_TAG     SkSetFourByteTag('r', 'e', 'a', 'd')
#define SK_PICT_FACTORY_TAG    SkSetFourByteTag('f', 'a', 'c', 't')
#define SK_PICT_TYPEFACE_TAG   SkSetFourByteTag('t', 'p', 'f', 'c')
#define SK_PICT_PICTURE_TAG    SkSetFourByteTag('p', 'c', 't', 'r')
#define SK_PICT_DRAWABLE_TAG   SkSetFourByteTag('d', 'r', 'a', 'w')

// This tag specifies the size of the ReadBuffer, needed for the following tags
#define SK_PICT_BUFFER_SIZE_TAG     SkSetFourByteTag('a', 'r', 'a', 'y')
// these are all inside the ARRAYS tag
#define SK_PICT_BITMAP_BUFFER_TAG   SkSetFourByteTag('b', 't', 'm', 'p')
#define SK_PICT_PAINT_BUFFER_TAG    SkSetFourByteTag('p', 'n', 't', ' ')
#define SK_PICT_PATH_BUFFER_TAG     SkSetFourByteTag('p', 't', 'h', ' ')
#define SK_PICT_TEXTBLOB_BUFFER_TAG SkSetFourByteTag('b', 'l', 'o', 'b')
#define SK_PICT_VERTICES_BUFFER_TAG SkSetFourByteTag('v', 'e', 'r', 't')
#define SK_PICT_IMAGE_BUFFER_TAG    SkSetFourByteTag('i', 'm', 'a', 'g')

// Always write this guy last (with no length field afterwards)
#define SK_PICT_EOF_TAG     SkSetFourByteTag('e', 'o', 'f', ' ')

class SkPictureData {
public:
    SkPictureData(const SkPictureRecord& record, const SkPictInfo&);
    // Does not affect ownership of SkStream.
    static SkPictureData* CreateFromStream(SkStream*,
                                           const SkPictInfo&,
                                           SkImageDeserializer*,
                                           SkTypefacePlayback*);
    static SkPictureData* CreateFromBuffer(SkReadBuffer&, const SkPictInfo&);

    virtual ~SkPictureData();

    void serialize(SkWStream*, SkPixelSerializer*, SkRefCntSet*) const;
    void flatten(SkWriteBuffer&) const;

    bool containsBitmaps() const;

    bool hasText() const { return fContentInfo.hasText(); }

    int opCount() const { return fContentInfo.numOperations(); }

    const sk_sp<SkData>& opData() const { return fOpData; }

protected:
    explicit SkPictureData(const SkPictInfo& info);

    // Does not affect ownership of SkStream.
    bool parseStream(SkStream*, SkImageDeserializer*, SkTypefacePlayback*);
    bool parseBuffer(SkReadBuffer& buffer);

public:
    const SkImage* getBitmapAsImage(SkReadBuffer* reader) const {
        const int index = reader->readInt();
        return reader->validateIndex(index, fBitmapImageCount) ? fBitmapImageRefs[index] : nullptr;
    }

    const SkImage* getImage(SkReadBuffer* reader) const {
        const int index = reader->readInt();
        return reader->validateIndex(index, fImageCount) ? fImageRefs[index] : nullptr;
    }

    const SkPath& getPath(SkReadBuffer* reader) const {
        const int index = reader->readInt() - 1;
        return reader->validateIndex(index, fPaths.count()) ? fPaths[index] : fEmptyPath;
    }

    const SkPicture* getPicture(SkReadBuffer* reader) const {
        const int index = reader->readInt() - 1;
        return reader->validateIndex(index, fPictureCount) ? fPictureRefs[index] : nullptr;
    }

    SkDrawable* getDrawable(SkReadBuffer* reader) const {
        int index = reader->readInt();
        SkASSERT(index > 0 && index <= fDrawableCount);
        return fDrawableRefs[index - 1];
    }

    const SkPaint* getPaint(SkReadBuffer* reader) const {
        const int index = reader->readInt() - 1;
        if (index == -1) {  // recorder wrote a zero for no paint (likely drawimage)
            return nullptr;
        }
        return reader->validateIndex(index, fPaints.count()) ? &fPaints[index] : nullptr;
    }

    const SkTextBlob* getTextBlob(SkReadBuffer* reader) const {
        const int index = reader->readInt() - 1;
        return reader->validateIndex(index, fTextBlobCount) ? fTextBlobRefs[index] : nullptr;
    }

    const SkVertices* getVertices(SkReadBuffer* reader) const {
        const int index = reader->readInt() - 1;
        return reader->validateIndex(index, fVerticesCount) ? fVerticesRefs[index] : nullptr;
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
    void init();

    // these help us with reading/writing
    // Does not affect ownership of SkStream.
    bool parseStreamTag(SkStream*, uint32_t tag, uint32_t size,
                        SkImageDeserializer*, SkTypefacePlayback*);
    bool parseBufferTag(SkReadBuffer&, uint32_t tag, uint32_t size);
    void flattenToBuffer(SkWriteBuffer&) const;

    SkTArray<SkPaint>  fPaints;
    SkTArray<SkPath>   fPaths;

    sk_sp<SkData>   fOpData;    // opcodes and parameters

    const SkPath    fEmptyPath;
    const SkBitmap  fEmptyBitmap;

    const SkPicture** fPictureRefs;
    int fPictureCount;
    SkDrawable** fDrawableRefs;
    int fDrawableCount;
    const SkTextBlob** fTextBlobRefs;
    int fTextBlobCount;
    const SkVertices** fVerticesRefs;
    int fVerticesCount;
    const SkImage** fImageRefs;
    int fImageCount;
    const SkImage** fBitmapImageRefs;
    int fBitmapImageCount;

    SkPictureContentInfo fContentInfo;

    SkTypefacePlayback fTFPlayback;
    SkFactoryPlayback* fFactoryPlayback;

    const SkPictInfo fInfo;

    static void WriteFactories(SkWStream* stream, const SkFactorySet& rec);
    static void WriteTypefaces(SkWStream* stream, const SkRefCntSet& rec);

    void initForPlayback() const;
};

#endif

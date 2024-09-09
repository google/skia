/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureData_DEFINED
#define SkPictureData_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkTArray.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkPictureFlat.h"
#include "src/core/SkReadBuffer.h"

#include <cstdint>
#include <memory>

class SkFactorySet;
class SkPictureRecord;
class SkRefCntSet;
class SkStream;
class SkWStream;
class SkWriteBuffer;
struct SkDeserialProcs;
struct SkSerialProcs;

struct SkPictInfo {
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
};

#define SK_PICT_READER_TAG     SkSetFourByteTag('r', 'e', 'a', 'd')
#define SK_PICT_FACTORY_TAG    SkSetFourByteTag('f', 'a', 'c', 't')
#define SK_PICT_TYPEFACE_TAG   SkSetFourByteTag('t', 'p', 'f', 'c')
#define SK_PICT_PICTURE_TAG    SkSetFourByteTag('p', 'c', 't', 'r')
#define SK_PICT_DRAWABLE_TAG   SkSetFourByteTag('d', 'r', 'a', 'w')

// This tag specifies the size of the ReadBuffer, needed for the following tags
#define SK_PICT_BUFFER_SIZE_TAG     SkSetFourByteTag('a', 'r', 'a', 'y')
// these are all inside the ARRAYS tag
#define SK_PICT_PAINT_BUFFER_TAG    SkSetFourByteTag('p', 'n', 't', ' ')
#define SK_PICT_PATH_BUFFER_TAG     SkSetFourByteTag('p', 't', 'h', ' ')
#define SK_PICT_TEXTBLOB_BUFFER_TAG SkSetFourByteTag('b', 'l', 'o', 'b')
#define SK_PICT_SLUG_BUFFER_TAG SkSetFourByteTag('s', 'l', 'u', 'g')
#define SK_PICT_VERTICES_BUFFER_TAG SkSetFourByteTag('v', 'e', 'r', 't')
#define SK_PICT_IMAGE_BUFFER_TAG    SkSetFourByteTag('i', 'm', 'a', 'g')

// Always write this last (with no length field afterwards)
#define SK_PICT_EOF_TAG     SkSetFourByteTag('e', 'o', 'f', ' ')

template <typename T>
T* read_index_base_1_or_null(SkReadBuffer* reader,
                             const skia_private::TArray<sk_sp<T>>& array) {
    int index = reader->readInt();
    return reader->validate(index > 0 && index <= array.size()) ? array[index - 1].get() : nullptr;
}

class SkPictureData {
public:
    SkPictureData(const SkPictureRecord& record, const SkPictInfo&);
    // Does not affect ownership of SkStream.
    static SkPictureData* CreateFromStream(SkStream*,
                                           const SkPictInfo&,
                                           const SkDeserialProcs&,
                                           SkTypefacePlayback*,
                                           int recursionLimit);
    static SkPictureData* CreateFromBuffer(SkReadBuffer&, const SkPictInfo&);

    void serialize(SkWStream*, const SkSerialProcs&, SkRefCntSet*, bool textBlobsOnly=false) const;
    void flatten(SkWriteBuffer&) const;

    const SkPictInfo& info() const { return fInfo; }

    const sk_sp<SkData>& opData() const { return fOpData; }

protected:
    explicit SkPictureData(const SkPictInfo& info);

    // Does not affect ownership of SkStream.
    bool parseStream(SkStream*, const SkDeserialProcs&, SkTypefacePlayback*,
                     int recursionLimit);
    bool parseBuffer(SkReadBuffer& buffer);

public:
    const SkImage* getImage(SkReadBuffer* reader) const {
        // images are written base-0, unlike paths, pictures, drawables, etc.
        const int index = reader->readInt();
        return reader->validateIndex(index, fImages.size()) ? fImages[index].get() : nullptr;
    }

    const SkPath& getPath(SkReadBuffer* reader) const {
        int index = reader->readInt();
        return reader->validate(index > 0 && index <= fPaths.size()) ?
                fPaths[index - 1] : fEmptyPath;
    }

    const SkPicture* getPicture(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fPictures);
    }

    SkDrawable* getDrawable(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fDrawables);
    }

    // Return a paint if one was used for this op, or nullptr if none was used.
    const SkPaint* optionalPaint(SkReadBuffer* reader) const;

    // Return the paint used for this op, invalidating the SkReadBuffer if there appears to be none.
    // The returned paint is always safe to use.
    const SkPaint& requiredPaint(SkReadBuffer* reader) const;

    const SkTextBlob* getTextBlob(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fTextBlobs);
    }

    const sktext::gpu::Slug* getSlug(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fSlugs);
    }

    const SkVertices* getVertices(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fVertices);
    }

private:
    // these help us with reading/writing
    // Does not affect ownership of SkStream.
    bool parseStreamTag(SkStream*, uint32_t tag, uint32_t size,
                        const SkDeserialProcs&, SkTypefacePlayback*,
                        int recursionLimit);
    void parseBufferTag(SkReadBuffer&, uint32_t tag, uint32_t size);
    void flattenToBuffer(SkWriteBuffer&, bool textBlobsOnly) const;

    skia_private::TArray<SkPaint> fPaints;
    skia_private::TArray<SkPath>  fPaths;

    sk_sp<SkData>                 fOpData;    // opcodes and parameters

    const SkPath                  fEmptyPath;
    const SkBitmap                fEmptyBitmap;

    skia_private::TArray<sk_sp<const SkPicture>>   fPictures;
    skia_private::TArray<sk_sp<SkDrawable>>        fDrawables;
    skia_private::TArray<sk_sp<const SkTextBlob>>  fTextBlobs;
    skia_private::TArray<sk_sp<const SkVertices>>  fVertices;
    skia_private::TArray<sk_sp<const SkImage>>     fImages;
    skia_private::TArray<sk_sp<const sktext::gpu::Slug>> fSlugs;

    SkTypefacePlayback                 fTFPlayback;
    std::unique_ptr<SkFactoryPlayback> fFactoryPlayback;

    const SkPictInfo fInfo;

    static void WriteFactories(SkWStream* stream, const SkFactorySet& rec);
    static void WriteTypefaces(SkWStream* stream, const SkRefCntSet& rec, const SkSerialProcs&);

    void initForPlayback() const;
};

#endif

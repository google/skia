/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureData_DEFINED
#define SkPictureData_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPicture.h"
#include "include/private/SkTArray.h"
#include "src/core/SkPictureFlat.h"

#include <memory>

class SkData;
class SkPictureRecord;
class SkReader32;
struct SkSerialProcs;
class SkStream;
class SkWStream;
class SkBBoxHierarchy;
class SkMatrix;
class SkPaint;
class SkPath;
class SkReadBuffer;
class SkTextBlob;

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
#define SK_PICT_VERTICES_BUFFER_TAG SkSetFourByteTag('v', 'e', 'r', 't')
#define SK_PICT_IMAGE_BUFFER_TAG    SkSetFourByteTag('i', 'm', 'a', 'g')

// Always write this guy last (with no length field afterwards)
#define SK_PICT_EOF_TAG     SkSetFourByteTag('e', 'o', 'f', ' ')

template <typename T>
T* read_index_base_1_or_null(SkReadBuffer* reader, const SkTArray<sk_sp<T>>& array) {
    int index = reader->readInt();
    return reader->validate(index > 0 && index <= array.count()) ? array[index - 1].get() : nullptr;
}

class SkPictureData {
public:
    SkPictureData(const SkPictureRecord& record, const SkPictInfo&);
    // Does not affect ownership of SkStream.
    static SkPictureData* CreateFromStream(SkStream*,
                                           const SkPictInfo&,
                                           const SkDeserialProcs&,
                                           SkTypefacePlayback*);
    static SkPictureData* CreateFromBuffer(SkReadBuffer&, const SkPictInfo&);

    void serialize(SkWStream*, const SkSerialProcs&, SkRefCntSet*) const;
    void flatten(SkWriteBuffer&) const;

    const sk_sp<SkData>& opData() const { return fOpData; }

protected:
    explicit SkPictureData(const SkPictInfo& info);

    // Does not affect ownership of SkStream.
    bool parseStream(SkStream*, const SkDeserialProcs&, SkTypefacePlayback*);
    bool parseBuffer(SkReadBuffer& buffer);

public:
    const SkImage* getImage(SkReadBuffer* reader) const {
        // images are written base-0, unlike paths, pictures, drawables, etc.
        const int index = reader->readInt();
        return reader->validateIndex(index, fImages.count()) ? fImages[index].get() : nullptr;
    }

    const SkPath& getPath(SkReadBuffer* reader) const {
        int index = reader->readInt();
        return reader->validate(index > 0 && index <= fPaths.count()) ?
                fPaths[index - 1] : fEmptyPath;
    }

    const SkPicture* getPicture(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fPictures);
    }

    SkDrawable* getDrawable(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fDrawables);
    }

    const SkPaint* getPaint(SkReadBuffer* reader) const {
        int index = reader->readInt();
        if (index == 0) {
            return nullptr; // recorder wrote a zero for no paint (likely drawimage)
        }
        return reader->validate(index > 0 && index <= fPaints.count()) ?
                &fPaints[index - 1] : nullptr;
    }

    const SkTextBlob* getTextBlob(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fTextBlobs);
    }

    const SkVertices* getVertices(SkReadBuffer* reader) const {
        return read_index_base_1_or_null(reader, fVertices);
    }

private:
    // these help us with reading/writing
    // Does not affect ownership of SkStream.
    bool parseStreamTag(SkStream*, uint32_t tag, uint32_t size,
                        const SkDeserialProcs&, SkTypefacePlayback*);
    void parseBufferTag(SkReadBuffer&, uint32_t tag, uint32_t size);
    void flattenToBuffer(SkWriteBuffer&) const;

    SkTArray<SkPaint>  fPaints;
    SkTArray<SkPath>   fPaths;

    sk_sp<SkData>   fOpData;    // opcodes and parameters

    const SkPath    fEmptyPath;
    const SkBitmap  fEmptyBitmap;

    SkTArray<sk_sp<const SkPicture>>   fPictures;
    SkTArray<sk_sp<SkDrawable>>        fDrawables;
    SkTArray<sk_sp<const SkTextBlob>>  fTextBlobs;
    SkTArray<sk_sp<const SkVertices>>  fVertices;
    SkTArray<sk_sp<const SkImage>>     fImages;

    SkTypefacePlayback                 fTFPlayback;
    std::unique_ptr<SkFactoryPlayback> fFactoryPlayback;

    const SkPictInfo fInfo;

    static void WriteFactories(SkWStream* stream, const SkFactorySet& rec);
    static void WriteTypefaces(SkWStream* stream, const SkRefCntSet& rec, const SkSerialProcs&);

    void initForPlayback() const;
};

#endif

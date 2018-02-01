/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <new>

#include "SkAutoMalloc.h"
#include "SkImageGenerator.h"
#include "SkPictureData.h"
#include "SkPictureRecord.h"
#include "SkReadBuffer.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

template <typename T> int SafeCount(const T* obj) {
    return obj ? obj->count() : 0;
}

SkPictureData::SkPictureData(const SkPictInfo& info)
    : fInfo(info) {
    this->init();
}

void SkPictureData::initForPlayback() const {
    // ensure that the paths bounds are pre-computed
    for (int i = 0; i < fPaths.count(); i++) {
        fPaths[i].updateBoundsCache();
    }
}

SkPictureData::SkPictureData(const SkPictureRecord& record,
                             const SkPictInfo& info)
    : fInfo(info) {

    this->init();

    fOpData = record.opData();

    fPaints  = record.fPaints;

    fPaths.reset(record.fPaths.count());
    record.fPaths.foreach([this](const SkPath& path, int n) {
        // These indices are logically 1-based, but we need to serialize them
        // 0-based to keep the deserializing SkPictureData::getPath() working.
        fPaths[n-1] = path;
    });

    this->initForPlayback();

    const SkTDArray<const SkPicture* >& pictures = record.getPictureRefs();
    fPictureCount = pictures.count();
    if (fPictureCount > 0) {
        fPictureRefs = new const SkPicture* [fPictureCount];
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i] = pictures[i];
            fPictureRefs[i]->ref();
        }
    }

    const SkTDArray<SkDrawable* >& drawables = record.getDrawableRefs();
    fDrawableCount = drawables.count();
    if (fDrawableCount > 0) {
        fDrawableRefs = new SkDrawable* [fDrawableCount];
        for (int i = 0; i < fDrawableCount; i++) {
            fDrawableRefs[i] = drawables[i];
            fDrawableRefs[i]->ref();
        }
    }

    // templatize to consolidate with similar picture logic?
    const SkTDArray<const SkTextBlob*>& blobs = record.getTextBlobRefs();
    fTextBlobCount = blobs.count();
    if (fTextBlobCount > 0) {
        fTextBlobRefs = new const SkTextBlob* [fTextBlobCount];
        for (int i = 0; i < fTextBlobCount; ++i) {
            fTextBlobRefs[i] = SkRef(blobs[i]);
        }
    }

    const SkTDArray<const SkVertices*>& verts = record.getVerticesRefs();
    fVerticesCount = verts.count();
    if (fVerticesCount > 0) {
        fVerticesRefs = new const SkVertices* [fVerticesCount];
        for (int i = 0; i < fVerticesCount; ++i) {
            fVerticesRefs[i] = SkRef(verts[i]);
        }
    }

    const SkTDArray<const SkImage*>& imgs = record.getImageRefs();
    fImageCount = imgs.count();
    if (fImageCount > 0) {
        fImageRefs = new const SkImage* [fImageCount];
        for (int i = 0; i < fImageCount; ++i) {
            fImageRefs[i] = SkRef(imgs[i]);
        }
    }
}

void SkPictureData::init() {
    fPictureRefs = nullptr;
    fPictureCount = 0;
    fDrawableRefs = nullptr;
    fDrawableCount = 0;
    fTextBlobRefs = nullptr;
    fTextBlobCount = 0;
    fVerticesRefs = nullptr;
    fVerticesCount = 0;
    fImageRefs = nullptr;
    fImageCount = 0;
    fBitmapImageCount = 0;
    fBitmapImageRefs = nullptr;
    fFactoryPlayback = nullptr;
}

SkPictureData::~SkPictureData() {
    for (int i = 0; i < fPictureCount; i++) {
        fPictureRefs[i]->unref();
    }
    delete[] fPictureRefs;

    for (int i = 0; i < fDrawableCount; i++) {
        fDrawableRefs[i]->unref();
    }
    if (fDrawableCount > 0) {
        SkASSERT(fDrawableRefs);
        delete[] fDrawableRefs;
    }

    for (int i = 0; i < fTextBlobCount; i++) {
        fTextBlobRefs[i]->unref();
    }
    delete[] fTextBlobRefs;

    for (int i = 0; i < fVerticesCount; i++) {
        fVerticesRefs[i]->unref();
    }
    delete[] fVerticesRefs;

    for (int i = 0; i < fImageCount; i++) {
        fImageRefs[i]->unref();
    }
    delete[] fImageRefs;

    delete fFactoryPlayback;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static size_t compute_chunk_size(SkFlattenable::Factory* array, int count) {
    size_t size = 4;  // for 'count'

    for (int i = 0; i < count; i++) {
        const char* name = SkFlattenable::FactoryToName(array[i]);
        if (nullptr == name || 0 == *name) {
            size += SkWStream::SizeOfPackedUInt(0);
        } else {
            size_t len = strlen(name);
            size += SkWStream::SizeOfPackedUInt(len);
            size += len;
        }
    }

    return size;
}

static void write_tag_size(SkWriteBuffer& buffer, uint32_t tag, size_t size) {
    buffer.writeUInt(tag);
    buffer.writeUInt(SkToU32(size));
}

static void write_tag_size(SkWStream* stream, uint32_t tag, size_t size) {
    stream->write32(tag);
    stream->write32(SkToU32(size));
}

void SkPictureData::WriteFactories(SkWStream* stream, const SkFactorySet& rec) {
    int count = rec.count();

    SkAutoSTMalloc<16, SkFlattenable::Factory> storage(count);
    SkFlattenable::Factory* array = (SkFlattenable::Factory*)storage.get();
    rec.copyToArray(array);

    size_t size = compute_chunk_size(array, count);

    // TODO: write_tag_size should really take a size_t
    write_tag_size(stream, SK_PICT_FACTORY_TAG, (uint32_t) size);
    SkDEBUGCODE(size_t start = stream->bytesWritten());
    stream->write32(count);

    for (int i = 0; i < count; i++) {
        const char* name = SkFlattenable::FactoryToName(array[i]);
        if (nullptr == name || 0 == *name) {
            stream->writePackedUInt(0);
        } else {
            size_t len = strlen(name);
            stream->writePackedUInt(len);
            stream->write(name, len);
        }
    }

    SkASSERT(size == (stream->bytesWritten() - start));
}

void SkPictureData::WriteTypefaces(SkWStream* stream, const SkRefCntSet& rec) {
    int count = rec.count();

    write_tag_size(stream, SK_PICT_TYPEFACE_TAG, count);

    SkAutoSTMalloc<16, SkTypeface*> storage(count);
    SkTypeface** array = (SkTypeface**)storage.get();
    rec.copyToArray((SkRefCnt**)array);

    for (int i = 0; i < count; i++) {
        array[i]->serialize(stream);
    }
}

void SkPictureData::flattenToBuffer(SkWriteBuffer& buffer) const {
    int i, n;

    if ((n = fPaints.count()) > 0) {
        write_tag_size(buffer, SK_PICT_PAINT_BUFFER_TAG, n);
        for (i = 0; i < n; i++) {
            buffer.writePaint(fPaints[i]);
        }
    }

    if ((n = fPaths.count()) > 0) {
        write_tag_size(buffer, SK_PICT_PATH_BUFFER_TAG, n);
        buffer.writeInt(n);
        for (int i = 0; i < n; i++) {
            buffer.writePath(fPaths[i]);
        }
    }

    if (fTextBlobCount > 0) {
        write_tag_size(buffer, SK_PICT_TEXTBLOB_BUFFER_TAG, fTextBlobCount);
        for (i = 0; i  < fTextBlobCount; ++i) {
            fTextBlobRefs[i]->flatten(buffer);
        }
    }

    if (fVerticesCount > 0) {
        write_tag_size(buffer, SK_PICT_VERTICES_BUFFER_TAG, fVerticesCount);
        for (i = 0; i  < fVerticesCount; ++i) {
            buffer.writeDataAsByteArray(fVerticesRefs[i]->encode().get());
        }
    }

    if (fImageCount > 0) {
        write_tag_size(buffer, SK_PICT_IMAGE_BUFFER_TAG, fImageCount);
        for (i = 0; i  < fImageCount; ++i) {
            buffer.writeImage(fImageRefs[i]);
        }
    }
}

void SkPictureData::serialize(SkWStream* stream, const SkSerialProcs& procs,
                              SkRefCntSet* topLevelTypeFaceSet) const {
    // This can happen at pretty much any time, so might as well do it first.
    write_tag_size(stream, SK_PICT_READER_TAG, fOpData->size());
    stream->write(fOpData->bytes(), fOpData->size());

    // We serialize all typefaces into the typeface section of the top-level picture.
    SkRefCntSet localTypefaceSet;
    SkRefCntSet* typefaceSet = topLevelTypeFaceSet ? topLevelTypeFaceSet : &localTypefaceSet;

    // We delay serializing the bulk of our data until after we've serialized
    // factories and typefaces by first serializing to an in-memory write buffer.
    SkFactorySet factSet;  // buffer refs factSet, so factSet must come first.
    SkBinaryWriteBuffer buffer;
    buffer.setFactoryRecorder(&factSet);
    buffer.setSerialProcs(procs);
    buffer.setTypefaceRecorder(typefaceSet);
    this->flattenToBuffer(buffer);

    // Dummy serialize our sub-pictures for the side effect of filling
    // typefaceSet with typefaces from sub-pictures.
    struct DevNull: public SkWStream {
        DevNull() : fBytesWritten(0) {}
        size_t fBytesWritten;
        bool write(const void*, size_t size) override { fBytesWritten += size; return true; }
        size_t bytesWritten() const override { return fBytesWritten; }
    } devnull;
    for (int i = 0; i < fPictureCount; i++) {
        fPictureRefs[i]->serialize(&devnull, nullptr, typefaceSet);
    }

    // We need to write factories before we write the buffer.
    // We need to write typefaces before we write the buffer or any sub-picture.
    WriteFactories(stream, factSet);
    if (typefaceSet == &localTypefaceSet) {
        WriteTypefaces(stream, *typefaceSet);
    }

    // Write the buffer.
    write_tag_size(stream, SK_PICT_BUFFER_SIZE_TAG, buffer.bytesWritten());
    buffer.writeToStream(stream);

    // Write sub-pictures by calling serialize again.
    if (fPictureCount > 0) {
        write_tag_size(stream, SK_PICT_PICTURE_TAG, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i]->serialize(stream, &procs, typefaceSet);
        }
    }

    stream->write32(SK_PICT_EOF_TAG);
}

void SkPictureData::flatten(SkWriteBuffer& buffer) const {
    write_tag_size(buffer, SK_PICT_READER_TAG, fOpData->size());
    buffer.writeByteArray(fOpData->bytes(), fOpData->size());

    if (fPictureCount > 0) {
        write_tag_size(buffer, SK_PICT_PICTURE_TAG, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i]->flatten(buffer);
        }
    }

    if (fDrawableCount > 0) {
        write_tag_size(buffer, SK_PICT_DRAWABLE_TAG, fDrawableCount);
        for (int i = 0; i < fDrawableCount; i++) {
            buffer.writeFlattenable(fDrawableRefs[i]);
        }
    }

    // Write this picture playback's data into a writebuffer
    this->flattenToBuffer(buffer);
    buffer.write32(SK_PICT_EOF_TAG);
}

///////////////////////////////////////////////////////////////////////////////

bool SkPictureData::parseStreamTag(SkStream* stream,
                                   uint32_t tag,
                                   uint32_t size,
                                   const SkDeserialProcs& procs,
                                   SkTypefacePlayback* topLevelTFPlayback) {
    /*
     *  By the time we encounter BUFFER_SIZE_TAG, we need to have already seen
     *  its dependents: FACTORY_TAG and TYPEFACE_TAG. These two are not required
     *  but if they are present, they need to have been seen before the buffer.
     *
     *  We assert that if/when we see either of these, that we have not yet seen
     *  the buffer tag, because if we have, then its too-late to deal with the
     *  factories or typefaces.
     */
    SkDEBUGCODE(bool haveBuffer = false;)

    switch (tag) {
        case SK_PICT_READER_TAG:
            SkASSERT(nullptr == fOpData);
            fOpData = SkData::MakeFromStream(stream, size);
            if (!fOpData) {
                return false;
            }
            break;
        case SK_PICT_FACTORY_TAG: {
            SkASSERT(!haveBuffer);
            size = stream->readU32();
            fFactoryPlayback = new SkFactoryPlayback(size);
            for (size_t i = 0; i < size; i++) {
                SkString str;
                const size_t len = stream->readPackedUInt();
                str.resize(len);
                if (stream->read(str.writable_str(), len) != len) {
                    return false;
                }
                fFactoryPlayback->base()[i] = SkFlattenable::NameToFactory(str.c_str());
            }
        } break;
        case SK_PICT_TYPEFACE_TAG: {
            SkASSERT(!haveBuffer);
            const int count = SkToInt(size);
            fTFPlayback.setCount(count);
            for (int i = 0; i < count; i++) {
                sk_sp<SkTypeface> tf(SkTypeface::MakeDeserialize(stream));
                if (!tf.get()) {    // failed to deserialize
                    // fTFPlayback asserts it never has a null, so we plop in
                    // the default here.
                    tf = SkTypeface::MakeDefault();
                }
                fTFPlayback.set(i, tf.get());
            }
        } break;
        case SK_PICT_PICTURE_TAG: {
            fPictureCount = 0;
            fPictureRefs = new const SkPicture* [size];
            for (uint32_t i = 0; i < size; i++) {
                fPictureRefs[i] = SkPicture::MakeFromStream(stream, &procs, topLevelTFPlayback).release();
                if (!fPictureRefs[i]) {
                    return false;
                }
                fPictureCount++;
            }
        } break;
        case SK_PICT_BUFFER_SIZE_TAG: {
            SkAutoMalloc storage(size);
            if (stream->read(storage.get(), size) != size) {
                return false;
            }

            SkReadBuffer buffer(storage.get(), size);
            buffer.setVersion(fInfo.getVersion());

            if (!fFactoryPlayback) {
                return false;
            }
            fFactoryPlayback->setupBuffer(buffer);
            buffer.setDeserialProcs(procs);

            if (fTFPlayback.count() > 0) {
                // .skp files <= v43 have typefaces serialized with each sub picture.
                fTFPlayback.setupBuffer(buffer);
            } else {
                // Newer .skp files serialize all typefaces with the top picture.
                topLevelTFPlayback->setupBuffer(buffer);
            }

            while (!buffer.eof() && buffer.isValid()) {
                tag = buffer.readUInt();
                size = buffer.readUInt();
                this->parseBufferTag(buffer, tag, size);
            }
            if (!buffer.isValid()) {
                return false;
            }
            SkDEBUGCODE(haveBuffer = true;)
        } break;
    }
    return true;    // success
}

static sk_sp<SkImage> create_image_from_buffer(SkReadBuffer& buffer) {
    return buffer.readImage();
}
static sk_sp<SkVertices> create_vertices_from_buffer(SkReadBuffer& buffer) {
    auto data = buffer.readByteArrayAsData();
    return data ? SkVertices::Decode(data->data(), data->size()) : nullptr;
}

static sk_sp<SkDrawable> create_drawable_from_buffer(SkReadBuffer& buffer) {
    return sk_sp<SkDrawable>((SkDrawable*)buffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
}

template <typename T>
bool new_array_from_buffer(SkReadBuffer& buffer, uint32_t inCount,
                           const T*** array, int* outCount, sk_sp<T> (*factory)(SkReadBuffer&)) {
    if (!buffer.validate((0 == *outCount) && (nullptr == *array))) {
        return false;
    }
    if (0 == inCount) {
        return true;
    }
    if (!buffer.validate(SkTFitsIn<int>(inCount))) {
        return false;
    }

    *outCount = inCount;
    *array = new const T* [*outCount];
    bool success = true;
    int i = 0;
    for (; i < *outCount; i++) {
        (*array)[i] = factory(buffer).release();
        if (nullptr == (*array)[i]) {
            success = false;
            break;
        }
    }
    if (!success) {
        // Delete all of the blobs that were already created (up to but excluding i):
        for (int j = 0; j < i; j++) {
            (*array)[j]->unref();
        }
        // Delete the array
        delete[] * array;
        *array = nullptr;
        *outCount = 0;
        return buffer.validate(false);
    }
    return true;
}

void SkPictureData::parseBufferTag(SkReadBuffer& buffer, uint32_t tag, uint32_t size) {
    switch (tag) {
        case SK_PICT_PAINT_BUFFER_TAG: {
            if (!buffer.validate(SkTFitsIn<int>(size))) {
                return;
            }
            const int count = SkToInt(size);
            fPaints.reset(count);
            for (int i = 0; i < count; ++i) {
                if (!buffer.readPaint(&fPaints[i])) {
                    return;
                }
            }
        } break;
        case SK_PICT_PATH_BUFFER_TAG:
            if (size > 0) {
                const int count = buffer.readInt();
                if (!buffer.validate(count >= 0)) {
                    return;
                }
                fPaths.reset(count);
                for (int i = 0; i < count; i++) {
                    buffer.readPath(&fPaths[i]);
                }
            } break;
        case SK_PICT_TEXTBLOB_BUFFER_TAG:
            new_array_from_buffer(buffer, size, &fTextBlobRefs, &fTextBlobCount,
                                  SkTextBlob::MakeFromBuffer);
            break;
        case SK_PICT_VERTICES_BUFFER_TAG:
            new_array_from_buffer(buffer, size, &fVerticesRefs, &fVerticesCount,
                                  create_vertices_from_buffer);
            break;
        case SK_PICT_IMAGE_BUFFER_TAG:
            new_array_from_buffer(buffer, size, &fImageRefs, &fImageCount,
                                  create_image_from_buffer);
            break;
        case SK_PICT_READER_TAG: {
            auto data(SkData::MakeUninitialized(size));
            if (!buffer.readByteArray(data->writable_data(), size) ||
                !buffer.validate(nullptr == fOpData)) {
                return;
            }
            SkASSERT(nullptr == fOpData);
            fOpData = std::move(data);
        } break;
        case SK_PICT_PICTURE_TAG:
            new_array_from_buffer(buffer, size, &fPictureRefs, &fPictureCount,
                                  SkPicture::MakeFromBuffer);
            break;
        case SK_PICT_DRAWABLE_TAG:
            new_array_from_buffer(buffer, size, (const SkDrawable***)&fDrawableRefs,
                                  &fDrawableCount, create_drawable_from_buffer);
            break;
        default:
            buffer.validate(false); // The tag was invalid.
            break;
    }
}

SkPictureData* SkPictureData::CreateFromStream(SkStream* stream,
                                               const SkPictInfo& info,
                                               const SkDeserialProcs& procs,
                                               SkTypefacePlayback* topLevelTFPlayback) {
    std::unique_ptr<SkPictureData> data(new SkPictureData(info));
    if (!topLevelTFPlayback) {
        topLevelTFPlayback = &data->fTFPlayback;
    }

    if (!data->parseStream(stream, procs, topLevelTFPlayback)) {
        return nullptr;
    }
    return data.release();
}

SkPictureData* SkPictureData::CreateFromBuffer(SkReadBuffer& buffer,
                                               const SkPictInfo& info) {
    std::unique_ptr<SkPictureData> data(new SkPictureData(info));
    buffer.setVersion(info.getVersion());

    if (!data->parseBuffer(buffer)) {
        return nullptr;
    }
    return data.release();
}

bool SkPictureData::parseStream(SkStream* stream,
                                const SkDeserialProcs& procs,
                                SkTypefacePlayback* topLevelTFPlayback) {
    for (;;) {
        uint32_t tag = stream->readU32();
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t size = stream->readU32();
        if (!this->parseStreamTag(stream, tag, size, procs, topLevelTFPlayback)) {
            return false; // we're invalid
        }
    }
    return true;
}

bool SkPictureData::parseBuffer(SkReadBuffer& buffer) {
    while (buffer.isValid()) {
        uint32_t tag = buffer.readUInt();
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }
        this->parseBufferTag(buffer, tag, buffer.readUInt());
    }

    // Check that we encountered required tags
    if (!buffer.validate(this->opData())) {
        // If we didn't build any opData, we are invalid. Even an EmptyPicture allocates the
        // SkData for the ops (though its length may be zero).
        return false;
    }
    return true;
}

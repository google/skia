/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <new>
#include "SkBBoxHierarchy.h"
#include "SkDrawPictureCallback.h"
#include "SkPictureData.h"
#include "SkPictureRecord.h"
#include "SkReadBuffer.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkTSort.h"
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
    if (fPathHeap.get()) {
        for (int i = 0; i < fPathHeap->count(); i++) {
            (*fPathHeap.get())[i].updateBoundsCache();
        }
    }
}

SkPictureData::SkPictureData(const SkPictureRecord& record,
                             const SkPictInfo& info,
                             bool deepCopyOps)
    : fInfo(info) {

    this->init();

    fOpData = record.opData(deepCopyOps);

    fBoundingHierarchy = record.fBoundingHierarchy;
    fStateTree = record.fStateTree;

    SkSafeRef(fBoundingHierarchy);
    SkSafeRef(fStateTree);
    fContentInfo.set(record.fContentInfo);

    if (fBoundingHierarchy) {
        fBoundingHierarchy->flushDeferredInserts();
    }

    // copy over the refcnt dictionary to our reader
    record.fFlattenableHeap.setupPlaybacks();

    fBitmaps = record.fBitmapHeap->extractBitmaps();
    fPaints = record.fPaints.unflattenToArray();

    fBitmapHeap.reset(SkSafeRef(record.fBitmapHeap));
    fPathHeap.reset(SkSafeRef(record.pathHeap()));

    this->initForPlayback();

    const SkTDArray<const SkPicture* >& pictures = record.getPictureRefs();
    fPictureCount = pictures.count();
    if (fPictureCount > 0) {
        fPictureRefs = SkNEW_ARRAY(const SkPicture*, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i] = pictures[i];
            fPictureRefs[i]->ref();
        }
    }

    // templatize to consolidate with similar picture logic?
    const SkTDArray<const SkTextBlob*>& blobs = record.getTextBlobRefs();
    fTextBlobCount = blobs.count();
    if (fTextBlobCount > 0) {
        fTextBlobRefs = SkNEW_ARRAY(const SkTextBlob*, fTextBlobCount);
        for (int i = 0; i < fTextBlobCount; ++i) {
            fTextBlobRefs[i] = SkRef(blobs[i]);
        }
    }
}

#ifdef SK_SUPPORT_LEGACY_PICTURE_CLONE
SkPictureData::SkPictureData(const SkPictureData& src, SkPictCopyInfo* deepCopyInfo)
    : fInfo(src.fInfo) {
    this->init();

    fBitmapHeap.reset(SkSafeRef(src.fBitmapHeap.get()));
    fPathHeap.reset(SkSafeRef(src.fPathHeap.get()));

    fOpData = SkSafeRef(src.fOpData);

    fBoundingHierarchy = src.fBoundingHierarchy;
    fStateTree = src.fStateTree;
    fContentInfo.set(src.fContentInfo);

    SkSafeRef(fBoundingHierarchy);
    SkSafeRef(fStateTree);

    if (deepCopyInfo) {
        int paintCount = SafeCount(src.fPaints);

        if (src.fBitmaps) {
            fBitmaps = SkTRefArray<SkBitmap>::Create(src.fBitmaps->begin(), src.fBitmaps->count());
        }

        fPaints = SkTRefArray<SkPaint>::Create(paintCount);
        SkASSERT(deepCopyInfo->paintData.count() == paintCount);
        SkBitmapHeap* bmHeap = deepCopyInfo->controller.getBitmapHeap();
        SkTypefacePlayback* tfPlayback = deepCopyInfo->controller.getTypefacePlayback();
        for (int i = 0; i < paintCount; i++) {
            if (deepCopyInfo->paintData[i]) {
                deepCopyInfo->paintData[i]->unflatten<SkPaint::FlatteningTraits>(
                    &fPaints->writableAt(i), bmHeap, tfPlayback);
            } else {
                // needs_deep_copy was false, so just need to assign
                fPaints->writableAt(i) = src.fPaints->at(i);
            }
        }

    } else {
        fBitmaps = SkSafeRef(src.fBitmaps);
        fPaints = SkSafeRef(src.fPaints);
    }

    fPictureCount = src.fPictureCount;
    fPictureRefs = SkNEW_ARRAY(const SkPicture*, fPictureCount);
    for (int i = 0; i < fPictureCount; i++) {
        if (deepCopyInfo) {
            fPictureRefs[i] = src.fPictureRefs[i]->clone();
        } else {
            fPictureRefs[i] = src.fPictureRefs[i];
            fPictureRefs[i]->ref();
        }
    }
}
#endif//SK_SUPPORT_LEGACY_PICTURE_CLONE

void SkPictureData::init() {
    fBitmaps = NULL;
    fPaints = NULL;
    fPictureRefs = NULL;
    fPictureCount = 0;
    fTextBlobRefs = NULL;
    fTextBlobCount = 0;
    fOpData = NULL;
    fFactoryPlayback = NULL;
    fBoundingHierarchy = NULL;
    fStateTree = NULL;
}

SkPictureData::~SkPictureData() {
    SkSafeUnref(fOpData);

    SkSafeUnref(fBitmaps);
    SkSafeUnref(fPaints);
    SkSafeUnref(fBoundingHierarchy);
    SkSafeUnref(fStateTree);

    for (int i = 0; i < fPictureCount; i++) {
        fPictureRefs[i]->unref();
    }
    SkDELETE_ARRAY(fPictureRefs);

    for (int i = 0; i < fTextBlobCount; i++) {
        fTextBlobRefs[i]->unref();
    }
    SkDELETE_ARRAY(fTextBlobRefs);

    SkDELETE(fFactoryPlayback);
}

bool SkPictureData::containsBitmaps() const {
    if (fBitmaps && fBitmaps->count() > 0) {
        return true;
    }
    for (int i = 0; i < fPictureCount; ++i) {
        if (fPictureRefs[i]->willPlayBackBitmaps()) {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static size_t compute_chunk_size(SkFlattenable::Factory* array, int count) {
    size_t size = 4;  // for 'count'

    for (int i = 0; i < count; i++) {
        const char* name = SkFlattenable::FactoryToName(array[i]);
        if (NULL == name || 0 == *name) {
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
        if (NULL == name || 0 == *name) {
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

    if ((n = SafeCount(fBitmaps)) > 0) {
        write_tag_size(buffer, SK_PICT_BITMAP_BUFFER_TAG, n);
        for (i = 0; i < n; i++) {
            buffer.writeBitmap((*fBitmaps)[i]);
        }
    }

    if ((n = SafeCount(fPaints)) > 0) {
        write_tag_size(buffer, SK_PICT_PAINT_BUFFER_TAG, n);
        for (i = 0; i < n; i++) {
            buffer.writePaint((*fPaints)[i]);
        }
    }

    if ((n = SafeCount(fPathHeap.get())) > 0) {
        write_tag_size(buffer, SK_PICT_PATH_BUFFER_TAG, n);
        fPathHeap->flatten(buffer);
    }

    if (fTextBlobCount > 0) {
        write_tag_size(buffer, SK_PICT_TEXTBLOB_BUFFER_TAG, fTextBlobCount);
        for (i = 0; i  < fTextBlobCount; ++i) {
            fTextBlobRefs[i]->flatten(buffer);
        }
    }
}

void SkPictureData::serialize(SkWStream* stream,
                                  SkPicture::EncodeBitmap encoder) const {
    write_tag_size(stream, SK_PICT_READER_TAG, fOpData->size());
    stream->write(fOpData->bytes(), fOpData->size());

    if (fPictureCount > 0) {
        write_tag_size(stream, SK_PICT_PICTURE_TAG, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i]->serialize(stream, encoder);
        }
    }

    // Write some of our data into a writebuffer, and then serialize that
    // into our stream
    {
        SkRefCntSet  typefaceSet;
        SkFactorySet factSet;

        SkWriteBuffer buffer(SkWriteBuffer::kCrossProcess_Flag);
        buffer.setTypefaceRecorder(&typefaceSet);
        buffer.setFactoryRecorder(&factSet);
        buffer.setBitmapEncoder(encoder);

        this->flattenToBuffer(buffer);

        // We have to write these two sets into the stream *before* we write
        // the buffer, since parsing that buffer will require that we already
        // have these sets available to use.
        WriteFactories(stream, factSet);
        WriteTypefaces(stream, typefaceSet);

        write_tag_size(stream, SK_PICT_BUFFER_SIZE_TAG, buffer.bytesWritten());
        buffer.writeToStream(stream);
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

    // Write this picture playback's data into a writebuffer
    this->flattenToBuffer(buffer);
    buffer.write32(SK_PICT_EOF_TAG);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return the corresponding SkReadBuffer flags, given a set of
 *  SkPictInfo flags.
 */
static uint32_t pictInfoFlagsToReadBufferFlags(uint32_t pictInfoFlags) {
    static const struct {
        uint32_t    fSrc;
        uint32_t    fDst;
    } gSD[] = {
        { SkPictInfo::kCrossProcess_Flag,   SkReadBuffer::kCrossProcess_Flag },
        { SkPictInfo::kScalarIsFloat_Flag,  SkReadBuffer::kScalarIsFloat_Flag },
        { SkPictInfo::kPtrIs64Bit_Flag,     SkReadBuffer::kPtrIs64Bit_Flag },
    };

    uint32_t rbMask = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gSD); ++i) {
        if (pictInfoFlags & gSD[i].fSrc) {
            rbMask |= gSD[i].fDst;
        }
    }
    return rbMask;
}

bool SkPictureData::parseStreamTag(SkStream* stream,
                                   uint32_t tag,
                                   uint32_t size,
                                   SkPicture::InstallPixelRefProc proc) {
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
            SkASSERT(NULL == fOpData);
            fOpData = SkData::NewFromStream(stream, size);
            if (!fOpData) {
                return false;
            }
            break;
        case SK_PICT_FACTORY_TAG: {
            SkASSERT(!haveBuffer);
        // Remove this code when v21 and below are no longer supported. At the
        // same time add a new 'count' variable and use it rather then reusing 'size'.
#ifndef DISABLE_V21_COMPATIBILITY_CODE
            if (fInfo.fVersion >= 22) {
                // in v22 this tag's size represents the size of the chunk in bytes
                // and the number of factory strings is written out separately
#endif
                size = stream->readU32();
#ifndef DISABLE_V21_COMPATIBILITY_CODE
            }
#endif
            fFactoryPlayback = SkNEW_ARGS(SkFactoryPlayback, (size));
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
                SkAutoTUnref<SkTypeface> tf(SkTypeface::Deserialize(stream));
                if (!tf.get()) {    // failed to deserialize
                    // fTFPlayback asserts it never has a null, so we plop in
                    // the default here.
                    tf.reset(SkTypeface::RefDefault());
                }
                fTFPlayback.set(i, tf);
            }
        } break;
        case SK_PICT_PICTURE_TAG: {
            fPictureCount = size;
            fPictureRefs = SkNEW_ARRAY(const SkPicture*, fPictureCount);
            bool success = true;
            int i = 0;
            for ( ; i < fPictureCount; i++) {
                fPictureRefs[i] = SkPicture::CreateFromStream(stream, proc);
                if (NULL == fPictureRefs[i]) {
                    success = false;
                    break;
                }
            }
            if (!success) {
                // Delete all of the pictures that were already created (up to but excluding i):
                for (int j = 0; j < i; j++) {
                    fPictureRefs[j]->unref();
                }
                // Delete the array
                SkDELETE_ARRAY(fPictureRefs);
                fPictureCount = 0;
                return false;
            }
        } break;
        case SK_PICT_BUFFER_SIZE_TAG: {
            SkAutoMalloc storage(size);
            if (stream->read(storage.get(), size) != size) {
                return false;
            }

            SkReadBuffer buffer(storage.get(), size);
            buffer.setFlags(pictInfoFlagsToReadBufferFlags(fInfo.fFlags));
            buffer.setVersion(fInfo.fVersion);

            fFactoryPlayback->setupBuffer(buffer);
            fTFPlayback.setupBuffer(buffer);
            buffer.setBitmapDecoder(proc);

            while (!buffer.eof()) {
                tag = buffer.readUInt();
                size = buffer.readUInt();
                if (!this->parseBufferTag(buffer, tag, size)) {
                    return false;
                }
            }
            SkDEBUGCODE(haveBuffer = true;)
        } break;
    }
    return true;    // success
}

bool SkPictureData::parseBufferTag(SkReadBuffer& buffer,
                                   uint32_t tag, uint32_t size) {
    switch (tag) {
        case SK_PICT_BITMAP_BUFFER_TAG: {
            const int count = SkToInt(size);
            fBitmaps = SkTRefArray<SkBitmap>::Create(size);
            for (int i = 0; i < count; ++i) {
                SkBitmap* bm = &fBitmaps->writableAt(i);
                buffer.readBitmap(bm);
                bm->setImmutable();
            }
        } break;
        case SK_PICT_PAINT_BUFFER_TAG: {
            const int count = SkToInt(size);
            fPaints = SkTRefArray<SkPaint>::Create(size);
            for (int i = 0; i < count; ++i) {
                buffer.readPaint(&fPaints->writableAt(i));
            }
        } break;
        case SK_PICT_PATH_BUFFER_TAG:
            if (size > 0) {
                fPathHeap.reset(SkNEW_ARGS(SkPathHeap, (buffer)));
            }
            break;
        case SK_PICT_TEXTBLOB_BUFFER_TAG: {
            if (!buffer.validate((0 == fTextBlobCount) && (NULL == fTextBlobRefs))) {
                return false;
            }
            fTextBlobCount = size;
            fTextBlobRefs = SkNEW_ARRAY(const SkTextBlob*, fTextBlobCount);
            bool success = true;
            int i = 0;
            for ( ; i < fTextBlobCount; i++) {
                fTextBlobRefs[i] = SkTextBlob::CreateFromBuffer(buffer);
                if (NULL == fTextBlobRefs[i]) {
                    success = false;
                    break;
                }
            }
            if (!success) {
                // Delete all of the blobs that were already created (up to but excluding i):
                for (int j = 0; j < i; j++) {
                    fTextBlobRefs[j]->unref();
                }
                // Delete the array
                SkDELETE_ARRAY(fTextBlobRefs);
                fTextBlobRefs = NULL;
                fTextBlobCount = 0;
                return false;
            }
        } break;
        case SK_PICT_READER_TAG: {
            SkAutoDataUnref data(SkData::NewUninitialized(size));
            if (!buffer.readByteArray(data->writable_data(), size) ||
                !buffer.validate(NULL == fOpData)) {
                return false;
            }
            SkASSERT(NULL == fOpData);
            fOpData = data.detach();
        } break;
        case SK_PICT_PICTURE_TAG: {
            if (!buffer.validate((0 == fPictureCount) && (NULL == fPictureRefs))) {
                return false;
            }
            fPictureCount = size;
            fPictureRefs = SkNEW_ARRAY(const SkPicture*, fPictureCount);
            bool success = true;
            int i = 0;
            for ( ; i < fPictureCount; i++) {
                fPictureRefs[i] = SkPicture::CreateFromBuffer(buffer);
                if (NULL == fPictureRefs[i]) {
                    success = false;
                    break;
                }
            }
            if (!success) {
                // Delete all of the pictures that were already created (up to but excluding i):
                for (int j = 0; j < i; j++) {
                    fPictureRefs[j]->unref();
                }
                // Delete the array
                SkDELETE_ARRAY(fPictureRefs);
                fPictureCount = 0;
                return false;
            }
        } break;
        default:
            // The tag was invalid.
            return false;
    }
    return true;    // success
}

SkPictureData* SkPictureData::CreateFromStream(SkStream* stream,
                                               const SkPictInfo& info,
                                               SkPicture::InstallPixelRefProc proc) {
    SkAutoTDelete<SkPictureData> data(SkNEW_ARGS(SkPictureData, (info)));

    if (!data->parseStream(stream, proc)) {
        return NULL;
    }
    return data.detach();
}

SkPictureData* SkPictureData::CreateFromBuffer(SkReadBuffer& buffer,
                                               const SkPictInfo& info) {
    SkAutoTDelete<SkPictureData> data(SkNEW_ARGS(SkPictureData, (info)));
    buffer.setVersion(info.fVersion);

    if (!data->parseBuffer(buffer)) {
        return NULL;
    }
    return data.detach();
}

bool SkPictureData::parseStream(SkStream* stream,
                                SkPicture::InstallPixelRefProc proc) {
    for (;;) {
        uint32_t tag = stream->readU32();
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t size = stream->readU32();
        if (!this->parseStreamTag(stream, tag, size, proc)) {
            return false; // we're invalid
        }
    }
    return true;
}

bool SkPictureData::parseBuffer(SkReadBuffer& buffer) {
    for (;;) {
        uint32_t tag = buffer.readUInt();
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t size = buffer.readUInt();
        if (!this->parseBufferTag(buffer, tag, size)) {
            return false; // we're invalid
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const SkPicture::OperationList* SkPictureData::getActiveOps(const SkRect& query) const {
    if (NULL == fStateTree || NULL == fBoundingHierarchy) {
        return NULL;
    }

    SkPicture::OperationList* activeOps = SkNEW(SkPicture::OperationList);
    fBoundingHierarchy->search(query, &(activeOps->fOps));
    return activeOps;
}

#if SK_SUPPORT_GPU
bool SkPictureData::suitableForGpuRasterization(GrContext* context, const char **reason,
                                                int sampleCount) const {
    return fContentInfo.suitableForGpuRasterization(context, reason, sampleCount);
}

bool SkPictureData::suitableForGpuRasterization(GrContext* context, const char **reason,
                                                GrPixelConfig config, SkScalar dpi) const {

    if (context != NULL) {
        return this->suitableForGpuRasterization(context, reason,
                                                 context->getRecommendedSampleCount(config, dpi));
    } else {
        return this->suitableForGpuRasterization(NULL, reason);
    }
}

bool SkPictureData::suitableForLayerOptimization() const {
    return fContentInfo.numLayers() > 0;
}
#endif
///////////////////////////////////////////////////////////////////////////////



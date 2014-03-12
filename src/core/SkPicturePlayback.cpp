
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <new>
#include "SkBBoxHierarchy.h"
#include "SkOffsetTable.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureStateTree.h"
#include "SkReadBuffer.h"
#include "SkTypeface.h"
#include "SkTSort.h"
#include "SkWriteBuffer.h"

template <typename T> int SafeCount(const T* obj) {
    return obj ? obj->count() : 0;
}

/*  Define this to spew out a debug statement whenever we skip the remainder of
    a save/restore block because a clip... command returned false (empty).
 */
#define SPEW_CLIP_SKIPPINGx

SkPicturePlayback::SkPicturePlayback() {
    this->init();
}

SkPicturePlayback::SkPicturePlayback(const SkPictureRecord& record, bool deepCopy) {
#ifdef SK_DEBUG_SIZE
    size_t overallBytes, bitmapBytes, matricesBytes,
    paintBytes, pathBytes, pictureBytes, regionBytes;
    int bitmaps = record.bitmaps(&bitmapBytes);
    int matrices = record.matrices(&matricesBytes);
    int paints = record.paints(&paintBytes);
    int paths = record.paths(&pathBytes);
    int pictures = record.pictures(&pictureBytes);
    int regions = record.regions(&regionBytes);
    SkDebugf("picture record mem used %zd (stream %zd) ", record.size(),
             record.streamlen());
    if (bitmaps != 0)
        SkDebugf("bitmaps size %zd (bitmaps:%d) ", bitmapBytes, bitmaps);
    if (matrices != 0)
        SkDebugf("matrices size %zd (matrices:%d) ", matricesBytes, matrices);
    if (paints != 0)
        SkDebugf("paints size %zd (paints:%d) ", paintBytes, paints);
    if (paths != 0)
        SkDebugf("paths size %zd (paths:%d) ", pathBytes, paths);
    if (pictures != 0)
        SkDebugf("pictures size %zd (pictures:%d) ", pictureBytes, pictures);
    if (regions != 0)
        SkDebugf("regions size %zd (regions:%d) ", regionBytes, regions);
    if (record.fPointWrites != 0)
        SkDebugf("points size %zd (points:%d) ", record.fPointBytes, record.fPointWrites);
    if (record.fRectWrites != 0)
        SkDebugf("rects size %zd (rects:%d) ", record.fRectBytes, record.fRectWrites);
    if (record.fTextWrites != 0)
        SkDebugf("text size %zd (text strings:%d) ", record.fTextBytes, record.fTextWrites);

    SkDebugf("\n");
#endif
#ifdef SK_DEBUG_DUMP
    record.dumpMatrices();
    record.dumpPaints();
#endif

    record.validate(record.writeStream().bytesWritten(), 0);
    const SkWriter32& writer = record.writeStream();
    init();
    SkASSERT(!fOpData);
    if (writer.bytesWritten() == 0) {
        fOpData = SkData::NewEmpty();
        return;
    }
    fOpData = writer.snapshotAsData();

    fBoundingHierarchy = record.fBoundingHierarchy;
    fStateTree = record.fStateTree;

    SkSafeRef(fBoundingHierarchy);
    SkSafeRef(fStateTree);

    if (NULL != fBoundingHierarchy) {
        fBoundingHierarchy->flushDeferredInserts();
    }

    // copy over the refcnt dictionary to our reader
    record.fFlattenableHeap.setupPlaybacks();

    fBitmaps = record.fBitmapHeap->extractBitmaps();
    fPaints = record.fPaints.unflattenToArray();

    fBitmapHeap.reset(SkSafeRef(record.fBitmapHeap));
    fPathHeap.reset(SkSafeRef(record.fPathHeap));

    fBitmapUseOffsets.reset(SkSafeRef(record.fBitmapUseOffsets.get()));

    // ensure that the paths bounds are pre-computed
    if (fPathHeap.get()) {
        for (int i = 0; i < fPathHeap->count(); i++) {
            (*fPathHeap)[i].updateBoundsCache();
        }
    }

    const SkTDArray<SkPicture* >& pictures = record.getPictureRefs();
    fPictureCount = pictures.count();
    if (fPictureCount > 0) {
        fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            if (deepCopy) {
                fPictureRefs[i] = pictures[i]->clone();
            } else {
                fPictureRefs[i] = pictures[i];
                fPictureRefs[i]->ref();
            }
        }
    }

#ifdef SK_DEBUG_SIZE
    int overall = fPlayback->size(&overallBytes);
    bitmaps = fPlayback->bitmaps(&bitmapBytes);
    paints = fPlayback->paints(&paintBytes);
    paths = fPlayback->paths(&pathBytes);
    pictures = fPlayback->pictures(&pictureBytes);
    regions = fPlayback->regions(&regionBytes);
    SkDebugf("playback size %zd (objects:%d) ", overallBytes, overall);
    if (bitmaps != 0)
        SkDebugf("bitmaps size %zd (bitmaps:%d) ", bitmapBytes, bitmaps);
    if (paints != 0)
        SkDebugf("paints size %zd (paints:%d) ", paintBytes, paints);
    if (paths != 0)
        SkDebugf("paths size %zd (paths:%d) ", pathBytes, paths);
    if (pictures != 0)
        SkDebugf("pictures size %zd (pictures:%d) ", pictureBytes, pictures);
    if (regions != 0)
        SkDebugf("regions size %zd (regions:%d) ", regionBytes, regions);
    SkDebugf("\n");
#endif
}

static bool needs_deep_copy(const SkPaint& paint) {
    /*
     *  These fields are known to be immutable, and so can be shallow-copied
     *
     *  getTypeface()
     *  getAnnotation()
     *  paint.getColorFilter()
     *  getXfermode()
     */

    return paint.getPathEffect() ||
           paint.getShader() ||
           paint.getMaskFilter() ||
           paint.getRasterizer() ||
           paint.getLooper() ||
           paint.getImageFilter();
}

SkPicturePlayback::SkPicturePlayback(const SkPicturePlayback& src, SkPictCopyInfo* deepCopyInfo) {
    this->init();

    fBitmapHeap.reset(SkSafeRef(src.fBitmapHeap.get()));
    fPathHeap.reset(SkSafeRef(src.fPathHeap.get()));

    fOpData = SkSafeRef(src.fOpData);

    fBoundingHierarchy = src.fBoundingHierarchy;
    fStateTree = src.fStateTree;

    SkSafeRef(fBoundingHierarchy);
    SkSafeRef(fStateTree);

    if (deepCopyInfo) {
        int paintCount = SafeCount(src.fPaints);

        if (src.fBitmaps) {
            fBitmaps = SkTRefArray<SkBitmap>::Create(src.fBitmaps->begin(), src.fBitmaps->count());
        }

        if (!deepCopyInfo->initialized) {
            /* The alternative to doing this is to have a clone method on the paint and have it make
             * the deep copy of its internal structures as needed. The holdup to doing that is at
             * this point we would need to pass the SkBitmapHeap so that we don't unnecessarily
             * flatten the pixels in a bitmap shader.
             */
            deepCopyInfo->paintData.setCount(paintCount);

            /* Use an SkBitmapHeap to avoid flattening bitmaps in shaders. If there already is one,
             * use it. If this SkPicturePlayback was created from a stream, fBitmapHeap will be
             * NULL, so create a new one.
             */
            if (fBitmapHeap.get() == NULL) {
                // FIXME: Put this on the stack inside SkPicture::clone. Further, is it possible to
                // do the rest of this initialization in SkPicture::clone as well?
                SkBitmapHeap* heap = SkNEW(SkBitmapHeap);
                deepCopyInfo->controller.setBitmapStorage(heap);
                heap->unref();
            } else {
                deepCopyInfo->controller.setBitmapStorage(fBitmapHeap);
            }

            SkDEBUGCODE(int heapSize = SafeCount(fBitmapHeap.get());)
            for (int i = 0; i < paintCount; i++) {
                if (needs_deep_copy(src.fPaints->at(i))) {
                    deepCopyInfo->paintData[i] =
                        SkFlatData::Create<SkPaint::FlatteningTraits>(&deepCopyInfo->controller,
                                                          src.fPaints->at(i), 0);

                } else {
                    // this is our sentinel, which we use in the unflatten loop
                    deepCopyInfo->paintData[i] = NULL;
                }
            }
            SkASSERT(SafeCount(fBitmapHeap.get()) == heapSize);

            // needed to create typeface playback
            deepCopyInfo->controller.setupPlaybacks();
            deepCopyInfo->initialized = true;
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
    fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
    for (int i = 0; i < fPictureCount; i++) {
        if (deepCopyInfo) {
            fPictureRefs[i] = src.fPictureRefs[i]->clone();
        } else {
            fPictureRefs[i] = src.fPictureRefs[i];
            fPictureRefs[i]->ref();
        }
    }
}

void SkPicturePlayback::init() {
    fBitmaps = NULL;
    fPaints = NULL;
    fPictureRefs = NULL;
    fPictureCount = 0;
    fOpData = NULL;
    fFactoryPlayback = NULL;
    fBoundingHierarchy = NULL;
    fStateTree = NULL;
}

SkPicturePlayback::~SkPicturePlayback() {
    SkSafeUnref(fOpData);

    SkSafeUnref(fBitmaps);
    SkSafeUnref(fPaints);
    SkSafeUnref(fBoundingHierarchy);
    SkSafeUnref(fStateTree);

    for (int i = 0; i < fPictureCount; i++) {
        fPictureRefs[i]->unref();
    }
    SkDELETE_ARRAY(fPictureRefs);

    SkDELETE(fFactoryPlayback);
}

void SkPicturePlayback::dumpSize() const {
    SkDebugf("--- picture size: ops=%d bitmaps=%d [%d] paints=%d [%d] paths=%d\n",
             fOpData->size(),
             SafeCount(fBitmaps), SafeCount(fBitmaps) * sizeof(SkBitmap),
             SafeCount(fPaints), SafeCount(fPaints) * sizeof(SkPaint),
             SafeCount(fPathHeap.get()));
}

bool SkPicturePlayback::containsBitmaps() const {
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

static void write_tag_size(SkWriteBuffer& buffer, uint32_t tag, uint32_t size) {
    buffer.writeUInt(tag);
    buffer.writeUInt(size);
}

static void write_tag_size(SkWStream* stream, uint32_t tag,  uint32_t size) {
    stream->write32(tag);
    stream->write32(size);
}

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

static void write_factories(SkWStream* stream, const SkFactorySet& rec) {
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
//        SkDebugf("---- write factories [%d] %p <%s>\n", i, array[i], name);
        if (NULL == name || 0 == *name) {
            stream->writePackedUInt(0);
        } else {
            uint32_t len = strlen(name);
            stream->writePackedUInt(len);
            stream->write(name, len);
        }
    }

    SkASSERT(size == (stream->bytesWritten() - start));
}

static void write_typefaces(SkWStream* stream, const SkRefCntSet& rec) {
    int count = rec.count();

    write_tag_size(stream, SK_PICT_TYPEFACE_TAG, count);

    SkAutoSTMalloc<16, SkTypeface*> storage(count);
    SkTypeface** array = (SkTypeface**)storage.get();
    rec.copyToArray((SkRefCnt**)array);

    for (int i = 0; i < count; i++) {
        array[i]->serialize(stream);
    }
}

void SkPicturePlayback::flattenToBuffer(SkWriteBuffer& buffer) const {
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
}

void SkPicturePlayback::serialize(SkWStream* stream,
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
        write_factories(stream, factSet);
        write_typefaces(stream, typefaceSet);

        write_tag_size(stream, SK_PICT_BUFFER_SIZE_TAG, buffer.bytesWritten());
        buffer.writeToStream(stream);
    }

    stream->write32(SK_PICT_EOF_TAG);
}

void SkPicturePlayback::flatten(SkWriteBuffer& buffer) const {
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

bool SkPicturePlayback::parseStreamTag(SkStream* stream, const SkPictInfo& info, uint32_t tag,
                                       size_t size, SkPicture::InstallPixelRefProc proc) {
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
        case SK_PICT_READER_TAG: {
            SkAutoMalloc storage(size);
            if (stream->read(storage.get(), size) != size) {
                return false;
            }
            SkASSERT(NULL == fOpData);
            fOpData = SkData::NewFromMalloc(storage.detach(), size);
        } break;
        case SK_PICT_FACTORY_TAG: {
            SkASSERT(!haveBuffer);
        // Remove this code when v21 and below are no longer supported. At the
        // same time add a new 'count' variable and use it rather then reusing 'size'.
#ifndef DISABLE_V21_COMPATIBILITY_CODE
            if (info.fVersion >= 22) {
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
            fTFPlayback.setCount(size);
            for (size_t i = 0; i < size; i++) {
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
            fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
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
            buffer.setFlags(pictInfoFlagsToReadBufferFlags(info.fFlags));

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

bool SkPicturePlayback::parseBufferTag(SkReadBuffer& buffer,
                                       uint32_t tag, size_t size) {
    switch (tag) {
        case SK_PICT_BITMAP_BUFFER_TAG: {
            fBitmaps = SkTRefArray<SkBitmap>::Create(size);
            for (size_t i = 0; i < size; ++i) {
                SkBitmap* bm = &fBitmaps->writableAt(i);
                buffer.readBitmap(bm);
                bm->setImmutable();
            }
        } break;
        case SK_PICT_PAINT_BUFFER_TAG: {
            fPaints = SkTRefArray<SkPaint>::Create(size);
            for (size_t i = 0; i < size; ++i) {
                buffer.readPaint(&fPaints->writableAt(i));
            }
        } break;
        case SK_PICT_PATH_BUFFER_TAG:
            if (size > 0) {
                fPathHeap.reset(SkNEW_ARGS(SkPathHeap, (buffer)));
            }
            break;
        case SK_PICT_READER_TAG: {
            SkAutoMalloc storage(size);
            if (!buffer.readByteArray(storage.get(), size) ||
                !buffer.validate(NULL == fOpData)) {
                return false;
            }
            SkASSERT(NULL == fOpData);
            fOpData = SkData::NewFromMalloc(storage.detach(), size);
        } break;
        case SK_PICT_PICTURE_TAG: {
            if (!buffer.validate((0 == fPictureCount) && (NULL == fPictureRefs))) {
                return false;
            }
            fPictureCount = size;
            fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
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

SkPicturePlayback* SkPicturePlayback::CreateFromStream(SkStream* stream,
                                                       const SkPictInfo& info,
                                                       SkPicture::InstallPixelRefProc proc) {
    SkAutoTDelete<SkPicturePlayback> playback(SkNEW(SkPicturePlayback));

    if (!playback->parseStream(stream, info, proc)) {
        return NULL;
    }
    return playback.detach();
}

SkPicturePlayback* SkPicturePlayback::CreateFromBuffer(SkReadBuffer& buffer) {
    SkAutoTDelete<SkPicturePlayback> playback(SkNEW(SkPicturePlayback));

    if (!playback->parseBuffer(buffer)) {
        return NULL;
    }
    return playback.detach();
}

bool SkPicturePlayback::parseStream(SkStream* stream, const SkPictInfo& info,
                                    SkPicture::InstallPixelRefProc proc) {
    for (;;) {
        uint32_t tag = stream->readU32();
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t size = stream->readU32();
        if (!this->parseStreamTag(stream, info, tag, size, proc)) {
            return false; // we're invalid
        }
    }
    return true;
}

bool SkPicturePlayback::parseBuffer(SkReadBuffer& buffer) {
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

#ifdef SPEW_CLIP_SKIPPING
struct SkipClipRec {
    int     fCount;
    size_t  fSize;

    SkipClipRec() {
        fCount = 0;
        fSize = 0;
    }

    void recordSkip(size_t bytes) {
        fCount += 1;
        fSize += bytes;
    }
};
#endif

#ifdef SK_DEVELOPER
bool SkPicturePlayback::preDraw(int opIndex, int type) {
    return false;
}

void SkPicturePlayback::postDraw(int opIndex) {
}
#endif

/*
 * Read the next op code and chunk size from 'reader'. The returned size
 * is the entire size of the chunk (including the opcode). Thus, the
 * offset just prior to calling read_op_and_size + 'size' is the offset
 * to the next chunk's op code. This also means that the size of a chunk
 * with no arguments (just an opcode) will be 4.
 */
static DrawType read_op_and_size(SkReader32* reader, uint32_t* size) {
    uint32_t temp = reader->readInt();
    uint32_t op;
    if (((uint8_t) temp) == temp) {
        // old skp file - no size information
        op = temp;
        *size = 0;
    } else {
        UNPACK_8_24(temp, op, *size);
        if (MASK_24 == *size) {
            *size = reader->readInt();
        }
    }
    return (DrawType) op;
}

// The activeOps parameter is actually "const SkTDArray<SkPictureStateTree::Draw*>&".
// It represents the operations about to be drawn, as generated by some spatial
// subdivision helper class. It should already be in 'fOffset' sorted order.
void SkPicturePlayback::preLoadBitmaps(const SkTDArray<void*>& activeOps) {
    if (0 == activeOps.count() || NULL == fBitmapUseOffsets) {
        return;
    }

    SkTDArray<int> active;

    SkAutoTDeleteArray<bool> needToCheck(new bool[fBitmapUseOffsets->numIDs()]);
    for (int i = 0; i < fBitmapUseOffsets->numIDs(); ++i) {
        needToCheck.get()[i] = true;
    }

    uint32_t max = ((SkPictureStateTree::Draw*)activeOps[activeOps.count()-1])->fOffset;

    for (int i = 0; i < activeOps.count(); ++i) {
        SkPictureStateTree::Draw* draw = (SkPictureStateTree::Draw*) activeOps[i];

        for (int j = 0; j < fBitmapUseOffsets->numIDs(); ++j) {
            if (!needToCheck.get()[j]) {
                continue;
            }

            if (!fBitmapUseOffsets->overlap(j, draw->fOffset, max)) {
                needToCheck.get()[j] = false;
                continue;
            }

            if (!fBitmapUseOffsets->includes(j, draw->fOffset)) {
                continue;
            }

            *active.append() = j;
            needToCheck.get()[j] = false;
        }
    }

    for (int i = 0; i < active.count(); ++i) {
        SkDebugf("preload texture %d\n", active[i]);
    }
}

void SkPicturePlayback::draw(SkCanvas& canvas, SkDrawPictureCallback* callback) {
#ifdef ENABLE_TIME_DRAW
    SkAutoTime  at("SkPicture::draw", 50);
#endif

#ifdef SPEW_CLIP_SKIPPING
    SkipClipRec skipRect, skipRRect, skipRegion, skipPath, skipCull;
    int opCount = 0;
#endif

#ifdef SK_BUILD_FOR_ANDROID
    SkAutoMutexAcquire autoMutex(fDrawMutex);
#endif

    // kDrawComplete will be the signal that we have reached the end of
    // the command stream
    static const uint32_t kDrawComplete = SK_MaxU32;

    SkReader32 reader(fOpData->bytes(), fOpData->size());
    TextContainer text;
    SkTDArray<void*> activeOps;

    if (NULL != fStateTree && NULL != fBoundingHierarchy) {
        SkRect clipBounds;
        if (canvas.getClipBounds(&clipBounds)) {
            SkIRect query;
            clipBounds.roundOut(&query);
            fBoundingHierarchy->search(query, &activeOps);
            if (activeOps.count() == 0) {
                return;
            }
            SkTQSort<SkPictureStateTree::Draw>(
                reinterpret_cast<SkPictureStateTree::Draw**>(activeOps.begin()),
                reinterpret_cast<SkPictureStateTree::Draw**>(activeOps.end()-1));
        }
    }

    SkPictureStateTree::Iterator it = (NULL == fStateTree) ?
        SkPictureStateTree::Iterator() :
        fStateTree->getIterator(activeOps, &canvas);

    if (it.isValid()) {
        uint32_t skipTo = it.draw();
        if (kDrawComplete == skipTo) {
            return;
        }
        reader.setOffset(skipTo);
    }

    this->preLoadBitmaps(activeOps);

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkMatrix initialMatrix = canvas.getTotalMatrix();
    int originalSaveCount = canvas.getSaveCount();

#ifdef SK_BUILD_FOR_ANDROID
    fAbortCurrentPlayback = false;
#endif

#ifdef SK_DEVELOPER
    int opIndex = -1;
#endif

    while (!reader.eof()) {
        if (callback && callback->abortDrawing()) {
            canvas.restoreToCount(originalSaveCount);
            return;
        }
#ifdef SK_BUILD_FOR_ANDROID
        if (fAbortCurrentPlayback) {
            return;
        }
#endif

#ifdef SPEW_CLIP_SKIPPING
        opCount++;
#endif

        size_t curOffset = reader.offset();
        uint32_t size;
        DrawType op = read_op_and_size(&reader, &size);
        size_t skipTo = 0;
        if (NOOP == op) {
            // NOOPs are to be ignored - do not propagate them any further
            skipTo = curOffset + size;
#ifdef SK_DEVELOPER
        } else {
            opIndex++;
            if (this->preDraw(opIndex, op)) {
                skipTo = curOffset + size;
            }
#endif
        }

        if (0 != skipTo) {
            if (it.isValid()) {
                // If using a bounding box hierarchy, advance the state tree
                // iterator until at or after skipTo
                uint32_t adjustedSkipTo;
                do {
                    adjustedSkipTo = it.draw();
                } while (adjustedSkipTo < skipTo);
                skipTo = adjustedSkipTo;
            }
            if (kDrawComplete == skipTo) {
                break;
            }
            reader.setOffset(skipTo);
            continue;
        }

        switch (op) {
            case CLIP_PATH: {
                const SkPath& path = getPath(reader);
                uint32_t packed = reader.readInt();
                SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
                bool doAA = ClipParams_unpackDoAA(packed);
                size_t offsetToRestore = reader.readInt();
                SkASSERT(!offsetToRestore || \
                    offsetToRestore >= reader.offset());
                canvas.clipPath(path, regionOp, doAA);
                if (canvas.isClipEmpty() && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipPath.recordSkip(offsetToRestore - reader.offset());
#endif
                    reader.setOffset(offsetToRestore);
                }
            } break;
            case CLIP_REGION: {
                SkRegion region;
                this->getRegion(reader, &region);
                uint32_t packed = reader.readInt();
                SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
                size_t offsetToRestore = reader.readInt();
                SkASSERT(!offsetToRestore || \
                    offsetToRestore >= reader.offset());
                canvas.clipRegion(region, regionOp);
                if (canvas.isClipEmpty() && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipRegion.recordSkip(offsetToRestore - reader.offset());
#endif
                    reader.setOffset(offsetToRestore);
                }
            } break;
            case CLIP_RECT: {
                const SkRect& rect = reader.skipT<SkRect>();
                uint32_t packed = reader.readInt();
                SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
                bool doAA = ClipParams_unpackDoAA(packed);
                size_t offsetToRestore = reader.readInt();
                SkASSERT(!offsetToRestore || \
                         offsetToRestore >= reader.offset());
                canvas.clipRect(rect, regionOp, doAA);
                if (canvas.isClipEmpty() && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipRect.recordSkip(offsetToRestore - reader.offset());
#endif
                    reader.setOffset(offsetToRestore);
                }
            } break;
            case CLIP_RRECT: {
                SkRRect rrect;
                reader.readRRect(&rrect);
                uint32_t packed = reader.readInt();
                SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
                bool doAA = ClipParams_unpackDoAA(packed);
                size_t offsetToRestore = reader.readInt();
                SkASSERT(!offsetToRestore || \
                         offsetToRestore >= reader.offset());
                canvas.clipRRect(rrect, regionOp, doAA);
                if (canvas.isClipEmpty() && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipRRect.recordSkip(offsetToRestore - reader.offset());
#endif
                    reader.setOffset(offsetToRestore);
                }
            } break;
            case PUSH_CULL: {
                const SkRect& cullRect = reader.skipT<SkRect>();
                size_t offsetToRestore = reader.readInt();
                if (offsetToRestore && canvas.quickReject(cullRect)) {
#ifdef SPEW_CLIP_SKIPPING
                    skipCull.recordSkip(offsetToRestore - reader.offset());
#endif
                    reader.setOffset(offsetToRestore);
                } else {
                    canvas.pushCull(cullRect);
                }
            } break;
            case POP_CULL:
                canvas.popCull();
                break;
            case CONCAT: {
                SkMatrix matrix;
                this->getMatrix(reader, &matrix);
                canvas.concat(matrix);
                break;
            }
            case DRAW_BITMAP: {
                const SkPaint* paint = this->getPaint(reader);
                const SkBitmap& bitmap = this->getBitmap(reader);
                const SkPoint& loc = reader.skipT<SkPoint>();
                canvas.drawBitmap(bitmap, loc.fX, loc.fY, paint);
            } break;
            case DRAW_BITMAP_RECT_TO_RECT: {
                const SkPaint* paint = this->getPaint(reader);
                const SkBitmap& bitmap = this->getBitmap(reader);
                const SkRect* src = this->getRectPtr(reader);   // may be null
                const SkRect& dst = reader.skipT<SkRect>();     // required
                SkCanvas::DrawBitmapRectFlags flags;
                flags = (SkCanvas::DrawBitmapRectFlags) reader.readInt();
                canvas.drawBitmapRectToRect(bitmap, src, dst, paint, flags);
            } break;
            case DRAW_BITMAP_MATRIX: {
                const SkPaint* paint = this->getPaint(reader);
                const SkBitmap& bitmap = this->getBitmap(reader);
                SkMatrix matrix;
                this->getMatrix(reader, &matrix);
                canvas.drawBitmapMatrix(bitmap, matrix, paint);
            } break;
            case DRAW_BITMAP_NINE: {
                const SkPaint* paint = this->getPaint(reader);
                const SkBitmap& bitmap = this->getBitmap(reader);
                const SkIRect& src = reader.skipT<SkIRect>();
                const SkRect& dst = reader.skipT<SkRect>();
                canvas.drawBitmapNine(bitmap, src, dst, paint);
            } break;
            case DRAW_CLEAR:
                canvas.clear(reader.readInt());
                break;
            case DRAW_DATA: {
                size_t length = reader.readInt();
                canvas.drawData(reader.skip(length), length);
                // skip handles padding the read out to a multiple of 4
            } break;
            case DRAW_DRRECT: {
                const SkPaint& paint = *this->getPaint(reader);
                SkRRect outer, inner;
                reader.readRRect(&outer);
                reader.readRRect(&inner);
                canvas.drawDRRect(outer, inner, paint);
            } break;
            case BEGIN_COMMENT_GROUP: {
                const char* desc = reader.readString();
                canvas.beginCommentGroup(desc);
            } break;
            case COMMENT: {
                const char* kywd = reader.readString();
                const char* value = reader.readString();
                canvas.addComment(kywd, value);
            } break;
            case END_COMMENT_GROUP: {
                canvas.endCommentGroup();
            } break;
            case DRAW_OVAL: {
                const SkPaint& paint = *this->getPaint(reader);
                canvas.drawOval(reader.skipT<SkRect>(), paint);
            } break;
            case DRAW_PAINT:
                canvas.drawPaint(*this->getPaint(reader));
                break;
            case DRAW_PATH: {
                const SkPaint& paint = *this->getPaint(reader);
                canvas.drawPath(getPath(reader), paint);
            } break;
            case DRAW_PICTURE:
                canvas.drawPicture(this->getPicture(reader));
                break;
            case DRAW_POINTS: {
                const SkPaint& paint = *this->getPaint(reader);
                SkCanvas::PointMode mode = (SkCanvas::PointMode)reader.readInt();
                size_t count = reader.readInt();
                const SkPoint* pts = (const SkPoint*)reader.skip(sizeof(SkPoint) * count);
                canvas.drawPoints(mode, count, pts, paint);
            } break;
            case DRAW_POS_TEXT: {
                const SkPaint& paint = *this->getPaint(reader);
                getText(reader, &text);
                size_t points = reader.readInt();
                const SkPoint* pos = (const SkPoint*)reader.skip(points * sizeof(SkPoint));
                canvas.drawPosText(text.text(), text.length(), pos, paint);
            } break;
            case DRAW_POS_TEXT_TOP_BOTTOM: {
                const SkPaint& paint = *this->getPaint(reader);
                getText(reader, &text);
                size_t points = reader.readInt();
                const SkPoint* pos = (const SkPoint*)reader.skip(points * sizeof(SkPoint));
                const SkScalar top = reader.readScalar();
                const SkScalar bottom = reader.readScalar();
                if (!canvas.quickRejectY(top, bottom)) {
                    canvas.drawPosText(text.text(), text.length(), pos, paint);
                }
            } break;
            case DRAW_POS_TEXT_H: {
                const SkPaint& paint = *this->getPaint(reader);
                getText(reader, &text);
                size_t xCount = reader.readInt();
                const SkScalar constY = reader.readScalar();
                const SkScalar* xpos = (const SkScalar*)reader.skip(xCount * sizeof(SkScalar));
                canvas.drawPosTextH(text.text(), text.length(), xpos, constY,
                                    paint);
            } break;
            case DRAW_POS_TEXT_H_TOP_BOTTOM: {
                const SkPaint& paint = *this->getPaint(reader);
                getText(reader, &text);
                size_t xCount = reader.readInt();
                const SkScalar* xpos = (const SkScalar*)reader.skip((3 + xCount) * sizeof(SkScalar));
                const SkScalar top = *xpos++;
                const SkScalar bottom = *xpos++;
                const SkScalar constY = *xpos++;
                if (!canvas.quickRejectY(top, bottom)) {
                    canvas.drawPosTextH(text.text(), text.length(), xpos,
                                        constY, paint);
                }
            } break;
            case DRAW_RECT: {
                const SkPaint& paint = *this->getPaint(reader);
                canvas.drawRect(reader.skipT<SkRect>(), paint);
            } break;
            case DRAW_RRECT: {
                const SkPaint& paint = *this->getPaint(reader);
                SkRRect rrect;
                reader.readRRect(&rrect);
                canvas.drawRRect(rrect, paint);
            } break;
            case DRAW_SPRITE: {
                const SkPaint* paint = this->getPaint(reader);
                const SkBitmap& bitmap = this->getBitmap(reader);
                int left = reader.readInt();
                int top = reader.readInt();
                canvas.drawSprite(bitmap, left, top, paint);
            } break;
            case DRAW_TEXT: {
                const SkPaint& paint = *this->getPaint(reader);
                this->getText(reader, &text);
                SkScalar x = reader.readScalar();
                SkScalar y = reader.readScalar();
                canvas.drawText(text.text(), text.length(), x, y, paint);
            } break;
            case DRAW_TEXT_TOP_BOTTOM: {
                const SkPaint& paint = *this->getPaint(reader);
                this->getText(reader, &text);
                const SkScalar* ptr = (const SkScalar*)reader.skip(4 * sizeof(SkScalar));
                // ptr[0] == x
                // ptr[1] == y
                // ptr[2] == top
                // ptr[3] == bottom
                if (!canvas.quickRejectY(ptr[2], ptr[3])) {
                    canvas.drawText(text.text(), text.length(), ptr[0], ptr[1],
                                    paint);
                }
            } break;
            case DRAW_TEXT_ON_PATH: {
                const SkPaint& paint = *this->getPaint(reader);
                getText(reader, &text);
                const SkPath& path = this->getPath(reader);
                SkMatrix matrix;
                this->getMatrix(reader, &matrix);
                canvas.drawTextOnPath(text.text(), text.length(), path, &matrix, paint);
            } break;
            case DRAW_VERTICES: {
                SkAutoTUnref<SkXfermode> xfer;
                const SkPaint& paint = *this->getPaint(reader);
                DrawVertexFlags flags = (DrawVertexFlags)reader.readInt();
                SkCanvas::VertexMode vmode = (SkCanvas::VertexMode)reader.readInt();
                int vCount = reader.readInt();
                const SkPoint* verts = (const SkPoint*)reader.skip(
                                                    vCount * sizeof(SkPoint));
                const SkPoint* texs = NULL;
                const SkColor* colors = NULL;
                const uint16_t* indices = NULL;
                int iCount = 0;
                if (flags & DRAW_VERTICES_HAS_TEXS) {
                    texs = (const SkPoint*)reader.skip(
                                                    vCount * sizeof(SkPoint));
                }
                if (flags & DRAW_VERTICES_HAS_COLORS) {
                    colors = (const SkColor*)reader.skip(
                                                    vCount * sizeof(SkColor));
                }
                if (flags & DRAW_VERTICES_HAS_INDICES) {
                    iCount = reader.readInt();
                    indices = (const uint16_t*)reader.skip(
                                                    iCount * sizeof(uint16_t));
                }
                if (flags & DRAW_VERTICES_HAS_XFER) {
                    int mode = reader.readInt();
                    if (mode < 0 || mode > SkXfermode::kLastMode) {
                        mode = SkXfermode::kModulate_Mode;
                    }
                    xfer.reset(SkXfermode::Create((SkXfermode::Mode)mode));
                }
                canvas.drawVertices(vmode, vCount, verts, texs, colors, xfer,
                                    indices, iCount, paint);
            } break;
            case RESTORE:
                canvas.restore();
                break;
            case ROTATE:
                canvas.rotate(reader.readScalar());
                break;
            case SAVE:
                canvas.save((SkCanvas::SaveFlags) reader.readInt());
                break;
            case SAVE_LAYER: {
                const SkRect* boundsPtr = this->getRectPtr(reader);
                const SkPaint* paint = this->getPaint(reader);
                canvas.saveLayer(boundsPtr, paint, (SkCanvas::SaveFlags) reader.readInt());
                } break;
            case SCALE: {
                SkScalar sx = reader.readScalar();
                SkScalar sy = reader.readScalar();
                canvas.scale(sx, sy);
            } break;
            case SET_MATRIX: {
                SkMatrix matrix;
                this->getMatrix(reader, &matrix);
                matrix.postConcat(initialMatrix);
                canvas.setMatrix(matrix);
            } break;
            case SKEW: {
                SkScalar sx = reader.readScalar();
                SkScalar sy = reader.readScalar();
                canvas.skew(sx, sy);
            } break;
            case TRANSLATE: {
                SkScalar dx = reader.readScalar();
                SkScalar dy = reader.readScalar();
                canvas.translate(dx, dy);
            } break;
            default:
                SkASSERT(0);
        }

#ifdef SK_DEVELOPER
        this->postDraw(opIndex);
#endif

        if (it.isValid()) {
            uint32_t skipTo = it.draw();
            if (kDrawComplete == skipTo) {
                break;
            }
            reader.setOffset(skipTo);
        }
    }

#ifdef SPEW_CLIP_SKIPPING
    {
        size_t size =  skipRect.fSize + skipRRect.fSize + skipPath.fSize + skipRegion.fSize +
                skipCull.fSize;
        SkDebugf("--- Clip skips %d%% rect:%d rrect:%d path:%d rgn:%d cull:%d\n",
             size * 100 / reader.offset(), skipRect.fCount, skipRRect.fCount,
                 skipPath.fCount, skipRegion.fCount, skipCull.fCount);
        SkDebugf("--- Total ops: %d\n", opCount);
    }
#endif
//    this->dumpSize();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG_SIZE
int SkPicturePlayback::size(size_t* sizePtr) {
    int objects = bitmaps(sizePtr);
    objects += paints(sizePtr);
    objects += paths(sizePtr);
    objects += pictures(sizePtr);
    objects += regions(sizePtr);
    *sizePtr = fOpData.size();
    return objects;
}

int SkPicturePlayback::bitmaps(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fBitmapCount; index++) {
     //   const SkBitmap& bitmap = fBitmaps[index];
        result += sizeof(SkBitmap); // bitmap->size();
    }
    *size = result;
    return fBitmapCount;
}

int SkPicturePlayback::paints(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fPaintCount; index++) {
    //    const SkPaint& paint = fPaints[index];
        result += sizeof(SkPaint); // paint->size();
    }
    *size = result;
    return fPaintCount;
}

int SkPicturePlayback::paths(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fPathCount; index++) {
        const SkPath& path = fPaths[index];
        result += path.flatten(NULL);
    }
    *size = result;
    return fPathCount;
}
#endif

#ifdef SK_DEBUG_DUMP
void SkPicturePlayback::dumpBitmap(const SkBitmap& bitmap) const {
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "BitmapData bitmap%p = {", &bitmap);
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "{kWidth, %d}, ", bitmap.width());
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "{kHeight, %d}, ", bitmap.height());
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "{kRowBytes, %d}, ", bitmap.rowBytes());
//        start here;
    SkDebugf("%s{0}};\n", pBuffer);
}

void dumpMatrix(const SkMatrix& matrix) const {
    SkMatrix defaultMatrix;
    defaultMatrix.reset();
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "MatrixData matrix%p = {", &matrix);
    SkScalar scaleX = matrix.getScaleX();
    if (scaleX != defaultMatrix.getScaleX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kScaleX, %g}, ", SkScalarToFloat(scaleX));
    SkScalar scaleY = matrix.getScaleY();
    if (scaleY != defaultMatrix.getScaleY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kScaleY, %g}, ", SkScalarToFloat(scaleY));
    SkScalar skewX = matrix.getSkewX();
    if (skewX != defaultMatrix.getSkewX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kSkewX, %g}, ", SkScalarToFloat(skewX));
    SkScalar skewY = matrix.getSkewY();
    if (skewY != defaultMatrix.getSkewY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kSkewY, %g}, ", SkScalarToFloat(skewY));
    SkScalar translateX = matrix.getTranslateX();
    if (translateX != defaultMatrix.getTranslateX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTranslateX, %g}, ", SkScalarToFloat(translateX));
    SkScalar translateY = matrix.getTranslateY();
    if (translateY != defaultMatrix.getTranslateY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTranslateY, %g}, ", SkScalarToFloat(translateY));
    SkScalar perspX = matrix.getPerspX();
    if (perspX != defaultMatrix.getPerspX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kPerspX, %g}, ", perspX);
    SkScalar perspY = matrix.getPerspY();
    if (perspY != defaultMatrix.getPerspY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kPerspY, %g}, ", perspY);
    SkDebugf("%s{0}};\n", pBuffer);
}

void dumpPaint(const SkPaint& paint) const {
    SkPaint defaultPaint;
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "PaintPointers paintPtrs%p = {", &paint);
    const SkTypeface* typeface = paint.getTypeface();
    if (typeface != defaultPaint.getTypeface())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTypeface, %p}, ", typeface);
    const SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != defaultPaint.getPathEffect())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kPathEffect, %p}, ", pathEffect);
    const SkShader* shader = paint.getShader();
    if (shader != defaultPaint.getShader())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kShader, %p}, ", shader);
    const SkXfermode* xfermode = paint.getXfermode();
    if (xfermode != defaultPaint.getXfermode())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kXfermode, %p}, ", xfermode);
    const SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != defaultPaint.getMaskFilter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kMaskFilter, %p}, ", maskFilter);
    const SkColorFilter* colorFilter = paint.getColorFilter();
    if (colorFilter != defaultPaint.getColorFilter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kColorFilter, %p}, ", colorFilter);
    const SkRasterizer* rasterizer = paint.getRasterizer();
    if (rasterizer != defaultPaint.getRasterizer())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kRasterizer, %p}, ", rasterizer);
    const SkDrawLooper* drawLooper = paint.getLooper();
    if (drawLooper != defaultPaint.getLooper())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kDrawLooper, %p}, ", drawLooper);
    SkDebugf("%s{0}};\n", pBuffer);
    bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "PaintScalars paintScalars%p = {", &paint);
    SkScalar textSize = paint.getTextSize();
    if (textSize != defaultPaint.getTextSize())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextSize, %g}, ", SkScalarToFloat(textSize));
    SkScalar textScaleX = paint.getTextScaleX();
    if (textScaleX != defaultPaint.getTextScaleX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextScaleX, %g}, ", SkScalarToFloat(textScaleX));
    SkScalar textSkewX = paint.getTextSkewX();
    if (textSkewX != defaultPaint.getTextSkewX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextSkewX, %g}, ", SkScalarToFloat(textSkewX));
    SkScalar strokeWidth = paint.getStrokeWidth();
    if (strokeWidth != defaultPaint.getStrokeWidth())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStrokeWidth, %g}, ", SkScalarToFloat(strokeWidth));
    SkScalar strokeMiter = paint.getStrokeMiter();
    if (strokeMiter != defaultPaint.getStrokeMiter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStrokeMiter, %g}, ", SkScalarToFloat(strokeMiter));
    SkDebugf("%s{0}};\n", pBuffer);
    bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "PaintInts = paintInts%p = {", &paint);
    unsigned color = paint.getColor();
    if (color != defaultPaint.getColor())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kColor, 0x%x}, ", color);
    unsigned flags = paint.getFlags();
    if (flags != defaultPaint.getFlags())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kFlags, 0x%x}, ", flags);
    int align = paint.getTextAlign();
    if (align != defaultPaint.getTextAlign())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kAlign, 0x%x}, ", align);
    int strokeCap = paint.getStrokeCap();
    if (strokeCap != defaultPaint.getStrokeCap())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStrokeCap, 0x%x}, ", strokeCap);
    int strokeJoin = paint.getStrokeJoin();
    if (strokeJoin != defaultPaint.getStrokeJoin())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kAlign, 0x%x}, ", strokeJoin);
    int style = paint.getStyle();
    if (style != defaultPaint.getStyle())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStyle, 0x%x}, ", style);
    int textEncoding = paint.getTextEncoding();
    if (textEncoding != defaultPaint.getTextEncoding())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextEncoding, 0x%x}, ", textEncoding);
    SkDebugf("%s{0}};\n", pBuffer);

    SkDebugf("PaintData paint%p = {paintPtrs%p, paintScalars%p, paintInts%p};\n",
        &paint, &paint, &paint, &paint);
}

void SkPicturePlayback::dumpPath(const SkPath& path) const {
    SkDebugf("path dump unimplemented\n");
}

void SkPicturePlayback::dumpPicture(const SkPicture& picture) const {
    SkDebugf("picture dump unimplemented\n");
}

void SkPicturePlayback::dumpRegion(const SkRegion& region) const {
    SkDebugf("region dump unimplemented\n");
}

int SkPicturePlayback::dumpDrawType(char* bufferPtr, char* buffer, DrawType drawType) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "k%s, ", DrawTypeToString(drawType));
}

int SkPicturePlayback::dumpInt(char* bufferPtr, char* buffer, char* name) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:%d, ", name, getInt());
}

int SkPicturePlayback::dumpRect(char* bufferPtr, char* buffer, char* name) {
    const SkRect* rect = fReader.skipRect();
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:{l:%g t:%g r:%g b:%g}, ", name, SkScalarToFloat(rect.fLeft),
        SkScalarToFloat(rect.fTop),
        SkScalarToFloat(rect.fRight), SkScalarToFloat(rect.fBottom));
}

int SkPicturePlayback::dumpPoint(char* bufferPtr, char* buffer, char* name) {
    SkPoint pt;
    getPoint(&pt);
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:{x:%g y:%g}, ", name, SkScalarToFloat(pt.fX),
        SkScalarToFloat(pt.fY));
}

void SkPicturePlayback::dumpPointArray(char** bufferPtrPtr, char* buffer, int count) {
    char* bufferPtr = *bufferPtrPtr;
    const SkPoint* pts = (const SkPoint*)fReadStream.getAtPos();
    fReadStream.skip(sizeof(SkPoint) * count);
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "count:%d {", count);
    for (int index = 0; index < count; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "{x:%g y:%g}, ", SkScalarToFloat(pts[index].fX),
        SkScalarToFloat(pts[index].fY));
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "} ");
    *bufferPtrPtr = bufferPtr;
}

int SkPicturePlayback::dumpPtr(char* bufferPtr, char* buffer, char* name, void* ptr) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:%p, ", name, ptr);
}

int SkPicturePlayback::dumpRectPtr(char* bufferPtr, char* buffer, char* name) {
    char result;
    fReadStream.read(&result, sizeof(result));
    if (result)
        return dumpRect(bufferPtr, buffer, name);
    else
        return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
            "%s:NULL, ", name);
}

int SkPicturePlayback::dumpScalar(char* bufferPtr, char* buffer, char* name) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:%d, ", name, getScalar());
}

void SkPicturePlayback::dumpText(char** bufferPtrPtr, char* buffer) {
    char* bufferPtr = *bufferPtrPtr;
    int length = getInt();
    bufferPtr += dumpDrawType(bufferPtr, buffer);
    fReadStream.skipToAlign4();
    char* text = (char*) fReadStream.getAtPos();
    fReadStream.skip(length);
    bufferPtr += dumpInt(bufferPtr, buffer, "length");
    int limit = DUMP_BUFFER_SIZE - (bufferPtr - buffer) - 2;
    length >>= 1;
    if (limit > length)
        limit = length;
    if (limit > 0) {
        *bufferPtr++ = '"';
        for (int index = 0; index < limit; index++) {
            *bufferPtr++ = *(unsigned short*) text;
            text += sizeof(unsigned short);
        }
        *bufferPtr++ = '"';
    }
    *bufferPtrPtr = bufferPtr;
}

#define DUMP_DRAWTYPE(drawType) \
    bufferPtr += dumpDrawType(bufferPtr, buffer, drawType)

#define DUMP_INT(name) \
    bufferPtr += dumpInt(bufferPtr, buffer, #name)

#define DUMP_RECT_PTR(name) \
    bufferPtr += dumpRectPtr(bufferPtr, buffer, #name)

#define DUMP_POINT(name) \
    bufferPtr += dumpRect(bufferPtr, buffer, #name)

#define DUMP_RECT(name) \
    bufferPtr += dumpRect(bufferPtr, buffer, #name)

#define DUMP_POINT_ARRAY(count) \
    dumpPointArray(&bufferPtr, buffer, count)

#define DUMP_PTR(name, ptr) \
    bufferPtr += dumpPtr(bufferPtr, buffer, #name, (void*) ptr)

#define DUMP_SCALAR(name) \
    bufferPtr += dumpScalar(bufferPtr, buffer, #name)

#define DUMP_TEXT() \
    dumpText(&bufferPtr, buffer)

void SkPicturePlayback::dumpStream() {
    SkDebugf("RecordStream stream = {\n");
    DrawType drawType;
    TextContainer text;
    fReadStream.rewind();
    char buffer[DUMP_BUFFER_SIZE], * bufferPtr;
    while (fReadStream.read(&drawType, sizeof(drawType))) {
        bufferPtr = buffer;
        DUMP_DRAWTYPE(drawType);
        switch (drawType) {
            case CLIP_PATH: {
                DUMP_PTR(SkPath, &getPath());
                DUMP_INT(SkRegion::Op);
                DUMP_INT(offsetToRestore);
                } break;
            case CLIP_REGION: {
                DUMP_INT(SkRegion::Op);
                DUMP_INT(offsetToRestore);
            } break;
            case CLIP_RECT: {
                DUMP_RECT(rect);
                DUMP_INT(SkRegion::Op);
                DUMP_INT(offsetToRestore);
                } break;
            case CONCAT:
                break;
            case DRAW_BITMAP: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_PTR(SkBitmap, &getBitmap());
                DUMP_SCALAR(left);
                DUMP_SCALAR(top);
                } break;
            case DRAW_PAINT:
                DUMP_PTR(SkPaint, getPaint());
                break;
            case DRAW_PATH: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_PTR(SkPath, &getPath());
                } break;
            case DRAW_PICTURE: {
                DUMP_PTR(SkPicture, &getPicture());
                } break;
            case DRAW_POINTS: {
                DUMP_PTR(SkPaint, getPaint());
                (void)getInt(); // PointMode
                size_t count = getInt();
                fReadStream.skipToAlign4();
                DUMP_POINT_ARRAY(count);
                } break;
            case DRAW_POS_TEXT: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                size_t points = getInt();
                fReadStream.skipToAlign4();
                DUMP_POINT_ARRAY(points);
                } break;
            case DRAW_POS_TEXT_H: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                size_t points = getInt();
                fReadStream.skipToAlign4();
                DUMP_SCALAR(top);
                DUMP_SCALAR(bottom);
                DUMP_SCALAR(constY);
                DUMP_POINT_ARRAY(points);
                } break;
            case DRAW_RECT: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_RECT(rect);
                } break;
            case DRAW_SPRITE: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_PTR(SkBitmap, &getBitmap());
                DUMP_SCALAR(left);
                DUMP_SCALAR(top);
                } break;
            case DRAW_TEXT: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                DUMP_SCALAR(x);
                DUMP_SCALAR(y);
                } break;
            case DRAW_TEXT_ON_PATH: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                DUMP_PTR(SkPath, &getPath());
                } break;
            case RESTORE:
                break;
            case ROTATE:
                DUMP_SCALAR(rotate);
                break;
            case SAVE:
                DUMP_INT(SkCanvas::SaveFlags);
                break;
            case SAVE_LAYER: {
                DUMP_RECT_PTR(layer);
                DUMP_PTR(SkPaint, getPaint());
                DUMP_INT(SkCanvas::SaveFlags);
                } break;
            case SCALE: {
                DUMP_SCALAR(sx);
                DUMP_SCALAR(sy);
                } break;
            case SKEW: {
                DUMP_SCALAR(sx);
                DUMP_SCALAR(sy);
                } break;
            case TRANSLATE: {
                DUMP_SCALAR(dx);
                DUMP_SCALAR(dy);
                } break;
            default:
                SkASSERT(0);
        }
        SkDebugf("%s\n", buffer);
    }
}

void SkPicturePlayback::dump() const {
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    int index;
    if (fBitmapCount > 0)
        SkDebugf("// bitmaps (%d)\n", fBitmapCount);
    for (index = 0; index < fBitmapCount; index++) {
        const SkBitmap& bitmap = fBitmaps[index];
        dumpBitmap(bitmap);
    }
    if (fBitmapCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Bitmaps bitmaps = {");
    for (index = 0; index < fBitmapCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "bitmap%p, ", &fBitmaps[index]);
    if (fBitmapCount > 0)
        SkDebugf("%s0};\n", pBuffer);


    if (fPaintCount > 0)
        SkDebugf("// paints (%d)\n", fPaintCount);
    for (index = 0; index < fPaintCount; index++) {
        const SkPaint& paint = fPaints[index];
        dumpPaint(paint);
    }
    bufferPtr = pBuffer;
    if (fPaintCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Paints paints = {");
    for (index = 0; index < fPaintCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "paint%p, ", &fPaints[index]);
    if (fPaintCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    for (index = 0; index < fPathCount; index++) {
        const SkPath& path = fPaths[index];
        dumpPath(path);
    }
    bufferPtr = pBuffer;
    if (fPathCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Paths paths = {");
    for (index = 0; index < fPathCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "path%p, ", &fPaths[index]);
    if (fPathCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    for (index = 0; index < fPictureCount; index++) {
        dumpPicture(*fPictureRefs[index]);
    }
    bufferPtr = pBuffer;
    if (fPictureCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Pictures pictures = {");
    for (index = 0; index < fPictureCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "picture%p, ", fPictureRefs[index]);
    if (fPictureCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    const_cast<SkPicturePlayback*>(this)->dumpStream();
}

#endif

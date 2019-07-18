/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCollatingCanvas.h"

#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkPictureRecorder.h"
#include "include/private/SkTo.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkTSearch.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/image/SkImage_Base.h"
#include "src/utils/SkPatchUtils.h"

#define HEAP_BLOCK_SIZE 4096

// A value that save or getSaveCount would never return, used to init fInitialSaveCount
static int const kNoInitialSave = -1;
// A lot of basic types get stored as a uint32_t: bools, ints, paint indices, etc.
static int const kUInt32Size = 4;

SkCollatingCanvas::SkCollatingCanvas(const SkISize& dimensions, SkCollatingCanvas* top)
    : INHERITED(dimensions.width(), dimensions.height())
    , fInitialSaveCount(kNoInitialSave)
    , topLevelPic(top) {
}

void SkCollatingCanvas::beginRecording() {
    // Do not allow recording on an SkCollatingCanvas that has already been recorded to, or one
    // that was created with CreateFromStream
    SkASSERT(!fOpData);
    // we have to call this *after* our constructor, to ensure that it gets
    // recorded. This is balanced by restoreToCount() call from endRecording,
    // which in-turn calls our overridden restore(), so those get recorded too.
    fInitialSaveCount = this->save();
}

void SkCollatingCanvas::endRecording() {
    SkASSERT(kNoInitialSave != fInitialSaveCount);
    this->restoreToCount(fInitialSaveCount);
    fOpData = fOpRecorder.snapshotAsData();
}

namespace {
// serialization helper
static void write_tag_size(SkWriteBuffer& buffer, uint32_t tag, size_t size) {
    buffer.writeUInt(tag);
    buffer.writeUInt(SkToU32(size));
}

static void write_tag_size(SkWStream* stream, uint32_t tag, size_t size) {
    stream->write32(tag);
    stream->write32(SkToU32(size));
}

// Reads file header. May return an error
SkString readSKPHeader(SkStream* stream, SkPictInfo* pInfo) {
    if (!stream) {
        return SkString("No Stream");
    }

    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (stream->read(&info.fMagic, sizeof(kMagic)) != sizeof(kMagic)) {
        return SkString("Could not read magic word");
    }
    if (0 != memcmp(info.fMagic, kMagic, sizeof(kMagic))) {
        return SkStringPrintf("Found a \"%s\" in magic word slot, expected %s", info.fMagic, kMagic);
    }

    uint32_t version;
    if (!stream->readU32(&version)) {
        return SkString("Could not read file version number");
    }
    info.setVersion(version);
    if (version < MIN_PICTURE_VERSION || info.getVersion() > CURRENT_PICTURE_VERSION) {
        return SkString(
            "Found SKP version %d. This build of SkCollatingCanvas can only read [%d,%d]",
            version, MIN_PICTURE_VERSION, CURRENT_PICTURE_VERSION);
    }
    if (!stream->readScalar(&info.fCullRect.fLeft  )
        || !stream->readScalar(&info.fCullRect.fTop)
        || !stream->readScalar(&info.fCullRect.fRight )
        || !stream->readScalar(&info.fCullRect.fBottom)) {
        return SkString("Could not read pic bounds");
    }
    if (info.getVersion() < SkReadBuffer::kRemoveHeaderFlags_Version) {
        if (!stream->readU32(nullptr)) {
            return SkStringPrintf(
                "This SKP's pictbuffer version is %d. This build of SkReadBuffer can only read %d or newer",
                info.getVersion(), SkReadBuffer::kRemoveHeaderFlags_Version);
        }
    }

    if (pInfo) { *pInfo = info; }
    return SkString(); // no error.
}

} // namespace

// basically SkPictureData::serialize
void SkCollatingCanvas::serializeToStream(SkWStream* stream, const SkSerialProcs& procs) const {
    // Only allow serialization after recording is finished.
    SkASSERT(fOpData);
    write_tag_size(stream, SK_PICT_READER_TAG, fOpData->size());
    stream->write(fOpData->bytes(), fOpData->size());

    // write buffer.
    // TODO: seems like a rather arbitrary subset of picture serialization is handled in
    // SkBinaryWriteBuffer, why? can this better share code? can it be better named?
    // note that most (all?) flattenables are ref counted, and recorded in the factset,
    // which dedupes them (not between subpictures though?)
    SkFactorySet factSet;
    SkRefCntSet* typefaceSet;

    SkBinaryWriteBuffer buffer;
    buffer.setFactoryRecorder(sk_ref_sp(&factSet));
    buffer.setSerialProcs(procs);
    buffer.setTypefaceRecorder(sk_ref_sp(typefaceSet));
    this->flattenToBuffer(buffer); // this was an SkPictureData method

    WriteFactories(stream, factSet);
    WriteTypefaces(stream, *typefaceSet, procs);

    write_tag_size(stream, SK_PICT_BUFFER_SIZE_TAG, buffer.bytesWritten());
    buffer.writeToStream(stream);

    // Write sub-pictures
    if (!fPictures.empty()) {
        write_tag_size(stream, SK_PICT_PICTURE_TAG, fPictures.count());
        for (const auto& pic : fPictures) {
            pic->serializeToStream(stream, procs);
        }
    }

    stream->write32(SK_PICT_EOF_TAG);
}

// basically SkPictureData::parseStream
SkCollatingCanvas::ResultOrError SkCollatingCanvas::CreateFromStream(
    SkStream* stream, const SkDeserialProcs& procs) {
    SkCollatingCanvas::ResultOrError r;
    SkString error;

    // read header, save in fPictInfo
    SkPictInfo info;
    error = readSKPHeader(stream, &info);
    if (!error.isEmpty()) {
        r.error = SkStringPrintf("%s | While reading header ", error.c_str());
        return r;
    }

    std::unique_ptr<SkCollatingCanvas> data(new SkCollatingCanvas(info));
    // The outer loop of deserialization. The file is divded into sections with tags.
    // Each tag is expected once. One of the tags is the op data, one is all the images, etc.
    for (;;) {
        uint32_t tag;
        if (!stream->readU32(&tag)) {
            r.error = SkString("Failed to read a tag (uint32).");
            return r;
        }
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t size;
        if (!stream->readU32(&size)) {
            r.error = SkString("Failed to read the size of section (uint32).");
            return r;
        }
        error = data->parseStreamTag(stream, tag, size, procs);
        if (!error.isEmpty()) {
            r.error = SkStringPrintf("%s | while processing tag %d", error.c_str(), tag);
            return r;
        }
    }

    r.cc = std::move(data);
    return r;
}

sk_sp<SkPicture> SkCollatingCanvas::asSkPicture() {
    if (!fPictInfo) {
        return nullptr; // Shoud I explicitly prevent you from using this as a recorder?
    }
    SkPictureRecorder r;
    SkCanvas* canvas = r.beginRecording(fPictInfo.fCullRect);

    // basically SkPicturePlayback::draw

    return r.finishRecordingAsPicture();
}

///// Private serialization and deserialization methods /////
/////////////////////////////////////////////////////////////

void SkCollatingCanvas::flattenToBuffer(SkWriteBuffer& buffer) const {
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

    if (!fTextBlobs.empty()) {
        write_tag_size(buffer, SK_PICT_TEXTBLOB_BUFFER_TAG, fTextBlobs.count());
        for (const auto& blob : fTextBlobs) {
            SkTextBlobPriv::Flatten(*blob, buffer);
        }
    }

    if (!fVertices.empty()) {
        write_tag_size(buffer, SK_PICT_VERTICES_BUFFER_TAG, fVertices.count());
        for (const auto& vert : fVertices) {
            buffer.writeDataAsByteArray(vert->encode().get());
        }
    }

    if (!fImages.empty()) {
        write_tag_size(buffer, SK_PICT_IMAGE_BUFFER_TAG, fImages.count());
        for (const auto& img : fImages) {
            buffer.writeImage(img.get());
        }
    }
}

SkString SkCollatingCanvas::parseStreamTag(SkStream* stream,
                                       uint32_t tag,
                                       uint32_t size,
                                       const SkDeserialProcs& procs) {
    // The meaning of `size` varies depending on the tag. It's usually the number of items.
    switch (tag) {
        case SK_PICT_READER_TAG:
            SkASSERT(nullptr == fOpData);
            fOpData = SkData::MakeFromStream(stream, size);
            if (!fOpData) {
                return SkString("Failed to read Op Data");
            }
            break;
        case SK_PICT_FACTORY_TAG: {
            // Why read size again if it's provided? / why are we re-using that variable?
            if (!stream->readU32(&size)) {
                return SkString("Failed to read size (uint32) of factory tag");
            }
            factoryPlayback = skstd::make_unique<SkFactoryPlayback>(size);
            for (size_t i = 0; i < size; i++) {
                SkString str;
                size_t len;
                if (!stream->readPackedUInt(&len)) {
                    return SkStringPrintf("Failed to read length (size_t) of factory %d", i);
                }
                str.resize(len);
                if (stream->read(str.writable_str(), len) != len) {
                    return SkStringPrintf("Failed to read %d bytes of factory %d", len, i);
                }
                factoryPlayback->base()[i] = SkFlattenable::NameToFactory(str.c_str());
            }
            // TODO save fFactoryPlayback somewhere
        } break;
        case SK_PICT_TYPEFACE_TAG: {
            fTFPlayback.setCount(size);
            for (uint32_t i = 0; i < size; ++i) {
                sk_sp<SkTypeface> tf(SkTypeface::MakeDeserialize(stream));
                if (!tf.get()) {    // failed to deserialize
                    // fTFPlayback asserts it never has a null, so we plop in
                    // the default here.
                    tf = SkTypeface::MakeDefault();
                }
                fTFPlayback[i] = std::move(tf);
            }
        } break;
        case SK_PICT_PICTURE_TAG: {
            SkASSERT(fPictures.empty());
            fPictures.reserve(SkToInt(size));

            for (uint32_t i = 0; i < size; i++) {
                // deserialize a SkCollatingCanvas here. This should return a result or error
                auto pic = SkPicture::MakeFromStream(stream, &procs);
                if (!pic) {
                    return false;
                }
                fPictures.push_back(std::move(pic));
            }
        } break;
        case SK_PICT_BUFFER_SIZE_TAG: {
            SkAutoMalloc storage(size);
            if (stream->read(storage.get(), size) != size) {
                return SkStringPrintf("Failed to read %d bytes of pict buffer tag", size);
            }

            SkReadBuffer buffer(storage.get(), size);
            buffer.setVersion(fInfo.getVersion());

            if (!fFactoryPlayback) {
                return SkString("Cannot read pict buffer tag before factories.", size);
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
                // TODO make pict buffer produce better errors.
                return SkString("Pict buffer became invalid while parsing", size);
            }
        } break;
    }
    return SkString(""); // success / no error
}


///// Canvas Implementation ////////////////
////////////////////////////////////////////

size_t SkCollatingCanvas::addDraw(DrawType drawType, size_t* size) {
    size_t offset = fOpRecorder.bytesWritten();

    //this->predrawNotify();

    SkASSERT(0 != *size);
    SkASSERT(((uint8_t) drawType) == drawType);

    if (0 != (*size & ~MASK_24) || *size == MASK_24) {
        fOpRecorder.writeInt(PACK_8_24(drawType, MASK_24));
        *size += 1;
        fOpRecorder.writeInt(SkToU32(*size));
    } else {
        fOpRecorder.writeInt(PACK_8_24(drawType, SkToU32(*size)));
    }

    return offset;
}

void SkCollatingCanvas::onFlush() {
    size_t size = sizeof(kUInt32Size);
    size_t initialOffset = this->addDraw(FLUSH, &size);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::willSave() {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push_back(-(int32_t)fOpRecorder.bytesWritten());
    this->recordSave();

    this->INHERITED::willSave();
}

void SkCollatingCanvas::recordSave() {
    // op only
    size_t size = sizeof(kUInt32Size);
    size_t initialOffset = this->addDraw(SAVE, &size);

    this->validate(initialOffset, size);
}

SkCanvas::SaveLayerStrategy SkCollatingCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push_back(-(int32_t)fOpRecorder.bytesWritten());
    this->recordSaveLayer(rec);

    (void)this->INHERITED::getSaveLayerStrategy(rec);
    /*  No need for a (potentially very big) layer which we don't actually need
        at this time (and may not be able to afford since during record our
        clip starts out the size of the picture, which is often much larger
        than the size of the actual device we'll use during playback).
     */
    return kNoLayer_SaveLayerStrategy;
}

bool SkCollatingCanvas::onDoSaveBehind(const SkRect* subset) {
    fRestoreOffsetStack.push_back(-(int32_t)fOpRecorder.bytesWritten());

    size_t size = sizeof(kUInt32Size) + sizeof(uint32_t); // op + flags
    uint32_t flags = 0;
    if (subset) {
        flags |= SAVEBEHIND_HAS_SUBSET;
        size += sizeof(*subset);
    }

    size_t initialOffset = this->addDraw(SAVE_BEHIND, &size);
    this->addInt(flags);
    if (subset) {
        this->addRect(*subset);
    }

    this->validate(initialOffset, size);
    return false;
}

void SkCollatingCanvas::recordSaveLayer(const SaveLayerRec& rec) {
    // op + flatflags
    size_t size = 2 * kUInt32Size;
    uint32_t flatFlags = 0;

    if (rec.fBounds) {
        flatFlags |= SAVELAYERREC_HAS_BOUNDS;
        size += sizeof(*rec.fBounds);
    }
    if (rec.fPaint) {
        flatFlags |= SAVELAYERREC_HAS_PAINT;
        size += sizeof(uint32_t); // index
    }
    if (rec.fBackdrop) {
        flatFlags |= SAVELAYERREC_HAS_BACKDROP;
        size += sizeof(uint32_t); // (paint) index
    }
    if (rec.fSaveLayerFlags) {
        flatFlags |= SAVELAYERREC_HAS_FLAGS;
        size += sizeof(uint32_t);
    }
    if (rec.fClipMask) {
        flatFlags |= SAVELAYERREC_HAS_CLIPMASK;
        size += sizeof(uint32_t); // clip image index
    }
    if (rec.fClipMatrix) {
        flatFlags |= SAVELAYERREC_HAS_CLIPMATRIX;
        size += SkMatrixPriv::WriteToMemory(*rec.fClipMatrix, nullptr);
    }

    const size_t initialOffset = this->addDraw(SAVE_LAYER_SAVELAYERREC, &size);
    this->addInt(flatFlags);
    if (flatFlags & SAVELAYERREC_HAS_BOUNDS) {
        this->addRect(*rec.fBounds);
    }
    if (flatFlags & SAVELAYERREC_HAS_PAINT) {
        this->addPaintPtr(rec.fPaint);
    }
    if (flatFlags & SAVELAYERREC_HAS_BACKDROP) {
        // overkill, but we didn't already track single flattenables, so using a paint for that
        SkPaint paint;
        paint.setImageFilter(sk_ref_sp(const_cast<SkImageFilter*>(rec.fBackdrop)));
        this->addPaint(paint);
    }
    if (flatFlags & SAVELAYERREC_HAS_FLAGS) {
        this->addInt(rec.fSaveLayerFlags);
    }
    if (flatFlags & SAVELAYERREC_HAS_CLIPMASK) {
        this->addImage(rec.fClipMask);
    }
    if (flatFlags & SAVELAYERREC_HAS_CLIPMATRIX) {
        this->addMatrix(*rec.fClipMatrix);
    }
    this->validate(initialOffset, size);
}

#ifdef SK_DEBUG
namespace {
    // Read the op code from 'offset' in 'writer' and extract the size too.
    static DrawType peek_op_and_size(SkWriter32* writer, size_t offset, uint32_t* size) {
        uint32_t peek = writer->readTAt<uint32_t>(offset);

        uint32_t op;
        UNPACK_8_24(peek, op, *size);
        if (MASK_24 == *size) {
            // size required its own slot right after the op code
            *size = writer->readTAt<uint32_t>(offset + kUInt32Size);
        }
        return (DrawType) op;
    }
}
#endif//SK_DEBUG

void SkCollatingCanvas::willRestore() {
    // check for underflow
    if (fRestoreOffsetStack.count() == 0) { return; }
    this->recordRestore();
    fRestoreOffsetStack.pop();
    this->INHERITED::willRestore();
}

void SkCollatingCanvas::recordRestore(bool fillInSkips) {
    if (fillInSkips) {
        this->fillRestoreOffsetPlaceholdersForCurrentStackLevel((uint32_t)fOpRecorder.bytesWritten());
    }
    size_t size = 1 * kUInt32Size; // RESTORE consists solely of 1 op code
    size_t initialOffset = this->addDraw(RESTORE, &size);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::recordTranslate(const SkMatrix& m) {
    SkASSERT(SkMatrix::kTranslate_Mask == m.getType());

    // op + dx + dy
    size_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(TRANSLATE, &size);
    this->addScalar(m.getTranslateX());
    this->addScalar(m.getTranslateY());
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::recordScale(const SkMatrix& m) {
    SkASSERT(SkMatrix::kScale_Mask == m.getType());

    // op + sx + sy
    size_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(SCALE, &size);
    this->addScalar(m.getScaleX());
    this->addScalar(m.getScaleY());
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::didConcat(const SkMatrix& matrix) {
    switch (matrix.getType()) {
        case SkMatrix::kTranslate_Mask:
            this->recordTranslate(matrix);
            break;
        case SkMatrix::kScale_Mask:
            this->recordScale(matrix);
            break;
        default:
            this->recordConcat(matrix);
            break;
    }
    this->INHERITED::didConcat(matrix);
}

void SkCollatingCanvas::recordConcat(const SkMatrix& matrix) {
    this->validate(fOpRecorder.bytesWritten(), 0);
    // op + matrix
    size_t size = kUInt32Size + SkMatrixPriv::WriteToMemory(matrix, nullptr);
    size_t initialOffset = this->addDraw(CONCAT, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->validate(fOpRecorder.bytesWritten(), 0);
    // op + matrix
    size_t size = kUInt32Size + SkMatrixPriv::WriteToMemory(matrix, nullptr);
    size_t initialOffset = this->addDraw(SET_MATRIX, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
    this->INHERITED::didSetMatrix(matrix);
}

static bool clipOpExpands(SkClipOp op) {
    switch (op) {
        case kUnion_SkClipOp:
        case kXOR_SkClipOp:
        case kReverseDifference_SkClipOp:
        case kReplace_SkClipOp:
            return true;
        case kIntersect_SkClipOp:
        case kDifference_SkClipOp:
            return false;
        default:
            SkDEBUGFAIL("unknown clipop");
            return false;
    }
}

void SkCollatingCanvas::fillRestoreOffsetPlaceholdersForCurrentStackLevel(uint32_t restoreOffset) {
    int32_t offset = fRestoreOffsetStack.top();
    while (offset > 0) {
        uint32_t peek = fOpRecorder.readTAt<uint32_t>(offset);
        fOpRecorder.overwriteTAt(offset, restoreOffset);
        offset = peek;
    }

#ifdef SK_DEBUG
    // offset of 0 has been disabled, so we skip it
    if (offset > 0) {
        // assert that the final offset value points to a save verb
        uint32_t opSize;
        DrawType drawOp = peek_op_and_size(&fOpRecorder, -offset, &opSize);
        SkASSERT(SAVE == drawOp || SAVE_LAYER_SAVELAYERREC == drawOp);
    }
#endif
}

size_t SkCollatingCanvas::recordRestoreOffsetPlaceholder(SkClipOp op) {
    if (fRestoreOffsetStack.isEmpty()) {
        return -1;
    }

    // The RestoreOffset field is initially filled with a placeholder
    // value that points to the offset of the previous RestoreOffset
    // in the current stack level, thus forming a linked list so that
    // the restore offsets can be filled in when the corresponding
    // restore command is recorded.
    int32_t prevOffset = fRestoreOffsetStack.top();

    if (clipOpExpands(op)) {
        // Run back through any previous clip ops, and mark their offset to
        // be 0, disabling their ability to trigger a jump-to-restore, otherwise
        // they could hide this clips ability to expand the clip (i.e. go from
        // empty to non-empty).
        this->fillRestoreOffsetPlaceholdersForCurrentStackLevel(0);

        // Reset the pointer back to the previous clip so that subsequent
        // restores don't overwrite the offsets we just cleared.
        prevOffset = 0;
    }

    size_t offset = fOpRecorder.bytesWritten();
    this->addInt(prevOffset);
    fRestoreOffsetStack.top() = SkToU32(offset);
    return offset;
}

void SkCollatingCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->recordClipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

size_t SkCollatingCanvas::recordClipRect(const SkRect& rect, SkClipOp op, bool doAA) {
    // id + rect + clip params
    size_t size = 1 * kUInt32Size + sizeof(rect) + 1 * kUInt32Size;
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
    size_t initialOffset = this->addDraw(CLIP_RECT, &size);
    this->addRect(rect);
    this->addInt(ClipParams_pack(op, doAA));
    size_t offset = this->recordRestoreOffsetPlaceholder(op);

    this->validate(initialOffset, size);
    return offset;
}

void SkCollatingCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->recordClipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

size_t SkCollatingCanvas::recordClipRRect(const SkRRect& rrect, SkClipOp op, bool doAA) {
    // op + rrect + clip params
    size_t size = 1 * kUInt32Size + SkRRect::kSizeInMemory + 1 * kUInt32Size;
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
    size_t initialOffset = this->addDraw(CLIP_RRECT, &size);
    this->addRRect(rrect);
    this->addInt(ClipParams_pack(op, doAA));
    size_t offset = recordRestoreOffsetPlaceholder(op);
    this->validate(initialOffset, size);
    return offset;
}

void SkCollatingCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    int pathID = this->addPathToHeap(path);
    this->recordClipPath(pathID, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

size_t SkCollatingCanvas::recordClipPath(int pathID, SkClipOp op, bool doAA) {
    // op + path index + clip params
    size_t size = 3 * kUInt32Size;
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
    size_t initialOffset = this->addDraw(CLIP_PATH, &size);
    this->addInt(pathID);
    this->addInt(ClipParams_pack(op, doAA));
    size_t offset = recordRestoreOffsetPlaceholder(op);
    this->validate(initialOffset, size);
    return offset;
}

void SkCollatingCanvas::onClipRegion(const SkRegion& region, SkClipOp op) {
    this->recordClipRegion(region, op);
    this->INHERITED::onClipRegion(region, op);
}

size_t SkCollatingCanvas::recordClipRegion(const SkRegion& region, SkClipOp op) {
    // op + clip params + region
    size_t size = 2 * kUInt32Size + region.writeToMemory(nullptr);
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
    size_t initialOffset = this->addDraw(CLIP_REGION, &size);
    this->addRegion(region);
    this->addInt(ClipParams_pack(op, false));
    size_t offset = this->recordRestoreOffsetPlaceholder(op);

    this->validate(initialOffset, size);
    return offset;
}

void SkCollatingCanvas::onDrawPaint(const SkPaint& paint) {
    // op + paint index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PAINT, &size);
    this->addPaint(paint);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawBehind(const SkPaint& paint) {
    // logically the same as drawPaint, but with a diff enum
    // op + paint index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_BEHIND_PAINT, &size);
    this->addPaint(paint);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                   const SkPaint& paint) {
    // op + paint index + mode + count + point data
    size_t size = 4 * kUInt32Size + count * sizeof(SkPoint);
    size_t initialOffset = this->addDraw(DRAW_POINTS, &size);
    this->addPaint(paint);

    this->addInt(mode);
    this->addInt(SkToInt(count));
    fOpRecorder.writeMul4(pts, count * sizeof(SkPoint));
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(oval);
    size_t initialOffset = this->addDraw(DRAW_OVAL, &size);
    this->addPaint(paint);
    this->addRect(oval);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                                bool useCenter, const SkPaint& paint) {
    // op + paint index + rect + start + sweep + bool (as int)
    size_t size = 2 * kUInt32Size + sizeof(oval) + sizeof(startAngle) + sizeof(sweepAngle) +
                  sizeof(int);
    size_t initialOffset = this->addDraw(DRAW_ARC, &size);
    this->addPaint(paint);
    this->addRect(oval);
    this->addScalar(startAngle);
    this->addScalar(sweepAngle);
    this->addInt(useCenter);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(rect);
    size_t initialOffset = this->addDraw(DRAW_RECT, &size);
    this->addPaint(paint);
    this->addRect(rect);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    // op + paint index + region
    size_t regionBytes = region.writeToMemory(nullptr);
    size_t size = 2 * kUInt32Size + regionBytes;
    size_t initialOffset = this->addDraw(DRAW_REGION, &size);
    this->addPaint(paint);
    fOpRecorder.writeRegion(region);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    // op + paint index + rrect
    size_t size = 2 * kUInt32Size + SkRRect::kSizeInMemory;
    size_t initialOffset = this->addDraw(DRAW_RRECT, &size);
    this->addPaint(paint);
    this->addRRect(rrect);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                   const SkPaint& paint) {
    // op + paint index + rrects
    size_t size = 2 * kUInt32Size + SkRRect::kSizeInMemory * 2;
    size_t initialOffset = this->addDraw(DRAW_DRRECT, &size);
    this->addPaint(paint);
    this->addRRect(outer);
    this->addRRect(inner);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    // op + paint index + path index
    size_t size = 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PATH, &size);
    this->addPaint(paint);
    this->addPath(path);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y,
                                  const SkPaint* paint) {
    // op + paint_index + image_index + x + y
    size_t size = 3 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(DRAW_IMAGE, &size);
    this->addPaintPtr(paint);
    this->addImage(image);
    this->addScalar(x);
    this->addScalar(y);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                      const SkPaint* paint, SrcRectConstraint constraint) {
    // id + paint_index + image_index + bool_for_src + constraint
    size_t size = 5 * kUInt32Size;
    if (src) {
        size += sizeof(*src);   // + rect
    }
    size += sizeof(dst);        // + rect

    size_t initialOffset = this->addDraw(DRAW_IMAGE_RECT, &size);
    this->addPaintPtr(paint);
    this->addImage(image);
    this->addRectPtr(src);  // may be null
    this->addRect(dst);
    this->addInt(constraint);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawImageNine(const SkImage* img, const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    // id + paint_index + image_index + center + dst
    size_t size = 3 * kUInt32Size + sizeof(SkIRect) + sizeof(SkRect);

    size_t initialOffset = this->addDraw(DRAW_IMAGE_NINE, &size);
    this->addPaintPtr(paint);
    this->addImage(img);
    this->addIRect(center);
    this->addRect(dst);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                         const SkRect& dst, const SkPaint* paint) {
    size_t latticeSize = SkCanvasPriv::WriteLattice(nullptr, lattice);
    // op + paint index + image index + lattice + dst rect
    size_t size = 3 * kUInt32Size + latticeSize + sizeof(dst);
    size_t initialOffset = this->addDraw(DRAW_IMAGE_LATTICE, &size);
    this->addPaintPtr(paint);
    this->addImage(image);
    (void)SkCanvasPriv::WriteLattice(fOpRecorder.reservePad(latticeSize), lattice);
    this->addRect(dst);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                     const SkPaint& paint) {

    // op + paint index + blob index + x/y
    size_t size = 3 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(DRAW_TEXT_BLOB, &size);

    this->addPaint(paint);
    this->addTextBlob(blob);
    this->addScalar(x);
    this->addScalar(y);

    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                    const SkPaint* paint) {
    // op + picture index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset;

    if (nullptr == matrix && nullptr == paint) {
        initialOffset = this->addDraw(DRAW_PICTURE, &size);
        this->addPicture(picture);
    } else {
        const SkMatrix& m = matrix ? *matrix : SkMatrix::I();
        size += SkMatrixPriv::WriteToMemory(m, nullptr) + kUInt32Size;    // matrix + paint
        initialOffset = this->addDraw(DRAW_PICTURE_MATRIX_PAINT, &size);
        this->addPaintPtr(paint);
        this->addMatrix(m);
        this->addPicture(picture);
    }
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    // op + drawable index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset;

    if (nullptr == matrix) {
        initialOffset = this->addDraw(DRAW_DRAWABLE, &size);
        this->addDrawable(drawable);
    } else {
        size += SkMatrixPriv::WriteToMemory(*matrix, nullptr);    // matrix
        initialOffset = this->addDraw(DRAW_DRAWABLE_MATRIX, &size);
        this->addMatrix(*matrix);
        this->addDrawable(drawable);
    }
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawVerticesObject(const SkVertices* vertices,
                                           const SkVertices::Bone bones[], int boneCount,
                                           SkBlendMode mode, const SkPaint& paint) {
    // op + paint index + vertices index + number of bones + bone matrices + mode
    size_t size = 5 * kUInt32Size + boneCount * sizeof(SkVertices::Bone);
    size_t initialOffset = this->addDraw(DRAW_VERTICES_OBJECT, &size);

    this->addPaint(paint);
    this->addVertices(vertices);
    this->addInt(boneCount);
    fOpRecorder.write(bones, boneCount * sizeof(SkVertices::Bone));
    this->addInt(static_cast<uint32_t>(mode));

    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                  const SkPoint texCoords[4], SkBlendMode bmode,
                                  const SkPaint& paint) {
    // op + paint index + patch 12 control points + flag + patch 4 colors + 4 texture coordinates
    size_t size = 2 * kUInt32Size + SkPatchUtils::kNumCtrlPts * sizeof(SkPoint) + kUInt32Size;
    uint32_t flag = 0;
    if (colors) {
        flag |= DRAW_VERTICES_HAS_COLORS;
        size += SkPatchUtils::kNumCorners * sizeof(SkColor);
    }
    if (texCoords) {
        flag |= DRAW_VERTICES_HAS_TEXS;
        size += SkPatchUtils::kNumCorners * sizeof(SkPoint);
    }
    if (SkBlendMode::kModulate != bmode) {
        flag |= DRAW_VERTICES_HAS_XFER;
        size += kUInt32Size;
    }

    size_t initialOffset = this->addDraw(DRAW_PATCH, &size);
    this->addPaint(paint);
    this->addPatch(cubics);
    this->addInt(flag);

    // write optional parameters
    if (colors) {
        fOpRecorder.write(colors, SkPatchUtils::kNumCorners * sizeof(SkColor));
    }
    if (texCoords) {
        fOpRecorder.write(texCoords, SkPatchUtils::kNumCorners * sizeof(SkPoint));
    }
    if (flag & DRAW_VERTICES_HAS_XFER) {
        this->addInt((int)bmode);
    }
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                                  const SkColor colors[], int count, SkBlendMode mode,
                                  const SkRect* cull, const SkPaint* paint) {
    // [op + paint-index + atlas-index + flags + count] + [xform] + [tex] + [*colors + mode] + cull
    size_t size = 5 * kUInt32Size + count * sizeof(SkRSXform) + count * sizeof(SkRect);
    uint32_t flags = 0;
    if (colors) {
        flags |= DRAW_ATLAS_HAS_COLORS;
        size += count * sizeof(SkColor);
        size += sizeof(uint32_t);   // xfermode::mode
    }
    if (cull) {
        flags |= DRAW_ATLAS_HAS_CULL;
        size += sizeof(SkRect);
    }

    size_t initialOffset = this->addDraw(DRAW_ATLAS, &size);
    this->addPaintPtr(paint);
    this->addImage(atlas);
    this->addInt(flags);
    this->addInt(count);
    fOpRecorder.write(xform, count * sizeof(SkRSXform));
    fOpRecorder.write(tex, count * sizeof(SkRect));

    // write optional parameters
    if (colors) {
        fOpRecorder.write(colors, count * sizeof(SkColor));
        this->addInt((int)mode);
    }
    if (cull) {
        fOpRecorder.write(cull, sizeof(SkRect));
    }
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    // op + path index + zParams + lightPos + lightRadius + spot/ambient alphas + color + flags
    size_t size = 2 * kUInt32Size + 2 * sizeof(SkPoint3) + 1 * sizeof(SkScalar) + 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_SHADOW_REC, &size);

    this->addPath(path);

    fOpRecorder.writePoint3(rec.fZPlaneParams);
    fOpRecorder.writePoint3(rec.fLightPos);
    fOpRecorder.writeScalar(rec.fLightRadius);
    fOpRecorder.write32(rec.fAmbientColor);
    fOpRecorder.write32(rec.fSpotColor);
    fOpRecorder.write32(rec.fFlags);

    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    size_t keyLen = fOpRecorder.WriteStringSize(key);
    size_t valueLen = fOpRecorder.WriteDataSize(value);
    size_t size = 4 + sizeof(SkRect) + keyLen + valueLen;

    size_t initialOffset = this->addDraw(DRAW_ANNOTATION, &size);
    this->addRect(rect);
    fOpRecorder.writeString(key);
    fOpRecorder.writeData(value);
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                       SkCanvas::QuadAAFlags aa, SkColor color, SkBlendMode mode) {

    // op + rect + aa flags + color + mode + hasClip(as int) + clipCount*points
    size_t size = 5 * kUInt32Size + sizeof(rect) + (clip ? 4 : 0) * sizeof(SkPoint);
    size_t initialOffset = this->addDraw(DRAW_EDGEAA_QUAD, &size);
    this->addRect(rect);
    this->addInt((int) aa);
    this->addInt((int) color);
    this->addInt((int) mode);
    this->addInt(clip != nullptr);
    if (clip) {
        this->addPoints(clip, 4);
    }
    this->validate(initialOffset, size);
}

void SkCollatingCanvas::onDrawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                           const SkPoint dstClips[],
                                           const SkMatrix preViewMatrices[],
                                           const SkPaint* paint,
                                           SkCanvas::SrcRectConstraint constraint) {
    static constexpr size_t kMatrixSize = 9 * sizeof(SkScalar); // *not* sizeof(SkMatrix)
    // op + count + paint + constraint + (image index, src rect, dst rect, alpha, aa flags,
    // hasClip(int), matrixIndex) * cnt + totalClipCount + dstClips + totalMatrixCount + matrices
    int totalDstClipCount, totalMatrixCount;
    SkCanvasPriv::GetDstClipAndMatrixCounts(set, count, &totalDstClipCount, &totalMatrixCount);

    size_t size = 6 * kUInt32Size + sizeof(SkPoint) * totalDstClipCount +
                  kMatrixSize * totalMatrixCount +
                  (4 * kUInt32Size + 2 * sizeof(SkRect) + sizeof(SkScalar)) * count;
    size_t initialOffset = this->addDraw(DRAW_EDGEAA_IMAGE_SET, &size);
    this->addInt(count);
    this->addPaintPtr(paint);
    this->addInt((int) constraint);
    for (int i = 0; i < count; ++i) {
        this->addImage(set[i].fImage.get());
        this->addRect(set[i].fSrcRect);
        this->addRect(set[i].fDstRect);
        this->addInt(set[i].fMatrixIndex);
        this->addScalar(set[i].fAlpha);
        this->addInt((int)set[i].fAAFlags);
        this->addInt(set[i].fHasClip);
    }
    this->addInt(totalDstClipCount);
    this->addPoints(dstClips, totalDstClipCount);
    this->addInt(totalMatrixCount);
    for (int i = 0; i < totalMatrixCount; ++i) {
        this->addMatrix(preViewMatrices[i]);
    }
    this->validate(initialOffset, size);
}

////////// Op Data recording methods /////////////////
//////////////////////////////////////////////////////

// De-duping helper.
namespace {
template <typename T>
static bool equals(T* a, T* b) { return a->uniqueID() == b->uniqueID(); }

template <>
bool equals(SkDrawable* a, SkDrawable* b) {
    // SkDrawable's generationID is not a stable unique identifier.
    return a == b;
}

template <typename T>
static int find_or_append(SkTArray<sk_sp<T>>& array, T* obj) {
    for (int i = 0; i < array.count(); i++) {
        if (equals(array[i].get(), obj)) {
            return i;
        }
    }

    array.push_back(sk_ref_sp(obj));

    return array.count() - 1;
}
} // namespace

sk_sp<SkSurface> SkCollatingCanvas::onNewSurface(const SkImageInfo& info, const SkSurfaceProps&) {
    return nullptr;
}

void SkCollatingCanvas::addImage(const SkImage* image) {
    // convention for images is 0-based index
    this->addInt(find_or_append(fImages, image));
}

void SkCollatingCanvas::addMatrix(const SkMatrix& matrix) {
    fOpRecorder.writeMatrix(matrix);
}

void SkCollatingCanvas::addPaintPtr(const SkPaint* paint) {
    // Used for adding an optional paint.
    if (paint) {
        fPaints.push_back(*paint);
        this->addInt(fPaints.count());
    } else {
        // Zero means no paint. (this is why the list is 1-indexed)
        this->addInt(0);
    }
}

int SkCollatingCanvas::addPathToHeap(const SkPath& path) {
    // Why not find_or_append?
    if (int* n = fPaths.find(path)) {
        return *n;
    }
    int n = fPaths.count() + 1;  // 0 is reserved for null / error.
    fPaths.set(path, n);
    return n;
}

void SkCollatingCanvas::addPath(const SkPath& path) {
    this->addInt(this->addPathToHeap(path));
}

void SkCollatingCanvas::addPatch(const SkPoint cubics[12]) {
    fOpRecorder.write(cubics, SkPatchUtils::kNumCtrlPts * sizeof(SkPoint));
}

void SkCollatingCanvas::addPicture(const SkPicture* picture) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fPictures, picture) + 1);
}

void SkCollatingCanvas::addDrawable(SkDrawable* drawable) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fDrawables, drawable) + 1);
}

void SkCollatingCanvas::addPoint(const SkPoint& point) {
    fOpRecorder.writePoint(point);
}

void SkCollatingCanvas::addPoints(const SkPoint pts[], int count) {
    fOpRecorder.writeMul4(pts, count * sizeof(SkPoint));
}

void SkCollatingCanvas::addNoOp() {
    size_t size = kUInt32Size; // op
    this->addDraw(NOOP, &size);
}

void SkCollatingCanvas::addRect(const SkRect& rect) {
    fOpRecorder.writeRect(rect);
}

void SkCollatingCanvas::addRectPtr(const SkRect* rect) {
    if (fOpRecorder.writeBool(rect != nullptr)) {
        fOpRecorder.writeRect(*rect);
    }
}

void SkCollatingCanvas::addIRect(const SkIRect& rect) {
    fOpRecorder.write(&rect, sizeof(rect));
}

void SkCollatingCanvas::addIRectPtr(const SkIRect* rect) {
    if (fOpRecorder.writeBool(rect != nullptr)) {
        *(SkIRect*)fOpRecorder.reserve(sizeof(SkIRect)) = *rect;
    }
}

void SkCollatingCanvas::addRRect(const SkRRect& rrect) {
    fOpRecorder.writeRRect(rrect);
}

void SkCollatingCanvas::addRegion(const SkRegion& region) {
    fOpRecorder.writeRegion(region);
}

void SkCollatingCanvas::addText(const void* text, size_t byteLength) {
    addInt(SkToInt(byteLength));
    fOpRecorder.writePad(text, byteLength);
}

void SkCollatingCanvas::addTextBlob(const SkTextBlob* blob) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fTextBlobs, blob) + 1);
}

void SkCollatingCanvas::addVertices(const SkVertices* vertices) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fVertices, vertices) + 1);
}




/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureRecord.h"
#include "SkImage_Base.h"
#include "SkPatchUtils.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkTextBlob.h"
#include "SkTSearch.h"

#define HEAP_BLOCK_SIZE 4096

enum {
    // just need a value that save or getSaveCount would never return
    kNoInitialSave = -1,
};

// A lot of basic types get stored as a uint32_t: bools, ints, paint indices, etc.
static int const kUInt32Size = 4;

SkPictureRecord::SkPictureRecord(const SkISize& dimensions, uint32_t flags)
    : INHERITED(dimensions.width(), dimensions.height())
    , fRecordFlags(flags)
    , fInitialSaveCount(kNoInitialSave) {
}

SkPictureRecord::~SkPictureRecord() {
    fImageRefs.unrefAll();
    fPictureRefs.unrefAll();
    fDrawableRefs.unrefAll();
    fTextBlobRefs.unrefAll();
}

///////////////////////////////////////////////////////////////////////////////

void SkPictureRecord::willSave() {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.bytesWritten());
    this->recordSave();

    this->INHERITED::willSave();
}

void SkPictureRecord::recordSave() {
    fContentInfo.onSave();

    // op only
    size_t size = sizeof(kUInt32Size);
    size_t initialOffset = this->addDraw(SAVE, &size);

    this->validate(initialOffset, size);
}

SkCanvas::SaveLayerStrategy SkPictureRecord::getSaveLayerStrategy(const SaveLayerRec& rec) {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.bytesWritten());
    this->recordSaveLayer(rec);

    (void)this->INHERITED::getSaveLayerStrategy(rec);
    /*  No need for a (potentially very big) layer which we don't actually need
        at this time (and may not be able to afford since during record our
        clip starts out the size of the picture, which is often much larger
        than the size of the actual device we'll use during playback).
     */
    return kNoLayer_SaveLayerStrategy;
}

void SkPictureRecord::recordSaveLayer(const SaveLayerRec& rec) {
    fContentInfo.onSaveLayer();

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
        paint.setImageFilter(const_cast<SkImageFilter*>(rec.fBackdrop));
        this->addPaint(paint);
    }
    if (flatFlags & SAVELAYERREC_HAS_FLAGS) {
        this->addInt(rec.fSaveLayerFlags);
    }
    this->validate(initialOffset, size);
}

#ifdef SK_DEBUG
/*
 * Read the op code from 'offset' in 'writer' and extract the size too.
 */
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
#endif//SK_DEBUG

void SkPictureRecord::willRestore() {
#if 0
    SkASSERT(fRestoreOffsetStack.count() > 1);
#endif

    // check for underflow
    if (fRestoreOffsetStack.count() == 0) {
        return;
    }

    this->recordRestore();

    fRestoreOffsetStack.pop();

    this->INHERITED::willRestore();
}

void SkPictureRecord::recordRestore(bool fillInSkips) {
    fContentInfo.onRestore();

    if (fillInSkips) {
        this->fillRestoreOffsetPlaceholdersForCurrentStackLevel((uint32_t)fWriter.bytesWritten());
    }
    size_t size = 1 * kUInt32Size; // RESTORE consists solely of 1 op code
    size_t initialOffset = this->addDraw(RESTORE, &size);
    this->validate(initialOffset, size);
}

void SkPictureRecord::recordTranslate(const SkMatrix& m) {
    SkASSERT(SkMatrix::kTranslate_Mask == m.getType());

    // op + dx + dy
    size_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(TRANSLATE, &size);
    this->addScalar(m.getTranslateX());
    this->addScalar(m.getTranslateY());
    this->validate(initialOffset, size);
}

void SkPictureRecord::recordScale(const SkMatrix& m) {
    SkASSERT(SkMatrix::kScale_Mask == m.getType());

    // op + sx + sy
    size_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(SCALE, &size);
    this->addScalar(m.getScaleX());
    this->addScalar(m.getScaleY());
    this->validate(initialOffset, size);
}

void SkPictureRecord::didConcat(const SkMatrix& matrix) {
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

void SkPictureRecord::recordConcat(const SkMatrix& matrix) {
    this->validate(fWriter.bytesWritten(), 0);
    // op + matrix
    size_t size = kUInt32Size + matrix.writeToMemory(nullptr);
    size_t initialOffset = this->addDraw(CONCAT, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
}

void SkPictureRecord::didSetMatrix(const SkMatrix& matrix) {
    this->validate(fWriter.bytesWritten(), 0);
    // op + matrix
    size_t size = kUInt32Size + matrix.writeToMemory(nullptr);
    size_t initialOffset = this->addDraw(SET_MATRIX, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
    this->INHERITED::didSetMatrix(matrix);
}

static bool regionOpExpands(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kUnion_Op:
        case SkRegion::kXOR_Op:
        case SkRegion::kReverseDifference_Op:
        case SkRegion::kReplace_Op:
            return true;
        case SkRegion::kIntersect_Op:
        case SkRegion::kDifference_Op:
            return false;
        default:
            SkDEBUGFAIL("unknown region op");
            return false;
    }
}

void SkPictureRecord::fillRestoreOffsetPlaceholdersForCurrentStackLevel(uint32_t restoreOffset) {
    int32_t offset = fRestoreOffsetStack.top();
    while (offset > 0) {
        uint32_t peek = fWriter.readTAt<uint32_t>(offset);
        fWriter.overwriteTAt(offset, restoreOffset);
        offset = peek;
    }

#ifdef SK_DEBUG
    // offset of 0 has been disabled, so we skip it
    if (offset > 0) {
        // assert that the final offset value points to a save verb
        uint32_t opSize;
        DrawType drawOp = peek_op_and_size(&fWriter, -offset, &opSize);
        SkASSERT(SAVE_LAYER_SAVEFLAGS_DEPRECATED != drawOp);
        SkASSERT(SAVE_LAYER_SAVELAYERFLAGS_DEPRECATED_JAN_2016 != drawOp);
        SkASSERT(SAVE == drawOp || SAVE_LAYER_SAVELAYERREC == drawOp);
    }
#endif
}

void SkPictureRecord::beginRecording() {
    // we have to call this *after* our constructor, to ensure that it gets
    // recorded. This is balanced by restoreToCount() call from endRecording,
    // which in-turn calls our overridden restore(), so those get recorded too.
    fInitialSaveCount = this->save();
}

void SkPictureRecord::endRecording() {
    SkASSERT(kNoInitialSave != fInitialSaveCount);
    this->restoreToCount(fInitialSaveCount);
}

size_t SkPictureRecord::recordRestoreOffsetPlaceholder(SkRegion::Op op) {
    if (fRestoreOffsetStack.isEmpty()) {
        return -1;
    }

    // The RestoreOffset field is initially filled with a placeholder
    // value that points to the offset of the previous RestoreOffset
    // in the current stack level, thus forming a linked list so that
    // the restore offsets can be filled in when the corresponding
    // restore command is recorded.
    int32_t prevOffset = fRestoreOffsetStack.top();

    if (regionOpExpands(op)) {
        // Run back through any previous clip ops, and mark their offset to
        // be 0, disabling their ability to trigger a jump-to-restore, otherwise
        // they could hide this clips ability to expand the clip (i.e. go from
        // empty to non-empty).
        this->fillRestoreOffsetPlaceholdersForCurrentStackLevel(0);

        // Reset the pointer back to the previous clip so that subsequent
        // restores don't overwrite the offsets we just cleared.
        prevOffset = 0;
    }

    size_t offset = fWriter.bytesWritten();
    this->addInt(prevOffset);
    fRestoreOffsetStack.top() = SkToU32(offset);
    return offset;
}

void SkPictureRecord::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->recordClipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

size_t SkPictureRecord::recordClipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
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

void SkPictureRecord::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->recordClipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

size_t SkPictureRecord::recordClipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
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

void SkPictureRecord::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    int pathID = this->addPathToHeap(path);
    this->recordClipPath(pathID, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

size_t SkPictureRecord::recordClipPath(int pathID, SkRegion::Op op, bool doAA) {
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

void SkPictureRecord::onClipRegion(const SkRegion& region, SkRegion::Op op) {
    this->recordClipRegion(region, op);
    this->INHERITED::onClipRegion(region, op);
}

size_t SkPictureRecord::recordClipRegion(const SkRegion& region, SkRegion::Op op) {
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

void SkPictureRecord::onDrawPaint(const SkPaint& paint) {
    // op + paint index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PAINT, &size);
    this->addPaint(paint);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                   const SkPaint& paint) {
    fContentInfo.onDrawPoints(count, paint);

    // op + paint index + mode + count + point data
    size_t size = 4 * kUInt32Size + count * sizeof(SkPoint);
    size_t initialOffset = this->addDraw(DRAW_POINTS, &size);
    this->addPaint(paint);

    this->addInt(mode);
    this->addInt(SkToInt(count));
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(oval);
    size_t initialOffset = this->addDraw(DRAW_OVAL, &size);
    this->addPaint(paint);
    this->addRect(oval);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(rect);
    size_t initialOffset = this->addDraw(DRAW_RECT, &size);
    this->addPaint(paint);
    this->addRect(rect);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    // op + paint index + rrect
    size_t size = 2 * kUInt32Size + SkRRect::kSizeInMemory;
    size_t initialOffset = this->addDraw(DRAW_RRECT, &size);
    this->addPaint(paint);
    this->addRRect(rrect);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                   const SkPaint& paint) {
    // op + paint index + rrects
    size_t size = 2 * kUInt32Size + SkRRect::kSizeInMemory * 2;
    size_t initialOffset = this->addDraw(DRAW_DRRECT, &size);
    this->addPaint(paint);
    this->addRRect(outer);
    this->addRRect(inner);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPath(const SkPath& path, const SkPaint& paint) {
    fContentInfo.onDrawPath(path, paint);

    // op + paint index + path index
    size_t size = 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PATH, &size);
    this->addPaint(paint);
    this->addPath(path);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                   const SkPaint* paint) {
    // op + paint index + bitmap index + left + top
    size_t size = 3 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(DRAW_BITMAP, &size);
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addScalar(left);
    this->addScalar(top);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                       const SkPaint* paint, SrcRectConstraint constraint) {
    // id + paint index + bitmap index + bool for 'src' + flags
    size_t size = 5 * kUInt32Size;
    if (src) {
        size += sizeof(*src);   // + rect
    }
    size += sizeof(dst);        // + rect

    size_t initialOffset = this->addDraw(DRAW_BITMAP_RECT, &size);
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addRectPtr(src);  // may be null
    this->addRect(dst);
    this->addInt(constraint);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawImage(const SkImage* image, SkScalar x, SkScalar y,
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

void SkPictureRecord::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
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

void SkPictureRecord::onDrawImageNine(const SkImage* img, const SkIRect& center, const SkRect& dst,
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

void SkPictureRecord::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                       const SkRect& dst, const SkPaint* paint) {
    // op + paint index + bitmap id + center + dst rect
    size_t size = 3 * kUInt32Size + sizeof(center) + sizeof(dst);
    size_t initialOffset = this->addDraw(DRAW_BITMAP_NINE, &size);
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addIRect(center);
    this->addRect(dst);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                 const SkPaint& paint) {
    // op + paint index + length + 'length' worth of chars + x + y
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 2 * sizeof(SkScalar);

    DrawType op = DRAW_TEXT;
    size_t initialOffset = this->addDraw(op, &size);
    this->addPaint(paint);
    this->addText(text, byteLength);
    this->addScalar(x);
    this->addScalar(y);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                    const SkPaint& paint) {
    int points = paint.countText(text, byteLength);

    // op + paint index + length + 'length' worth of data + num points + x&y point data
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + kUInt32Size + points * sizeof(SkPoint);

    DrawType op = DRAW_POS_TEXT;

    size_t initialOffset = this->addDraw(op, &size);
    this->addPaint(paint);
    this->addText(text, byteLength);
    this->addInt(points);
    fWriter.writeMul4(pos, points * sizeof(SkPoint));
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                     SkScalar constY, const SkPaint& paint) {
    int points = paint.countText(text, byteLength);

    // op + paint index + length + 'length' worth of data + num points
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
    // + y + the actual points
    size += 1 * kUInt32Size + points * sizeof(SkScalar);

    size_t initialOffset = this->addDraw(DRAW_POS_TEXT_H, &size);
    this->addPaint(paint);
    this->addText(text, byteLength);
    this->addInt(points);
    this->addScalar(constY);
    fWriter.writeMul4(xpos, points * sizeof(SkScalar));
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                       const SkMatrix* matrix, const SkPaint& paint) {
    // op + paint index + length + 'length' worth of data + path index + matrix
    const SkMatrix& m = matrix ? *matrix : SkMatrix::I();
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + kUInt32Size + m.writeToMemory(nullptr);
    size_t initialOffset = this->addDraw(DRAW_TEXT_ON_PATH, &size);
    this->addPaint(paint);
    this->addText(text, byteLength);
    this->addPath(path);
    this->addMatrix(m);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
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

void SkPictureRecord::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                    const SkPaint* paint) {
    // op + picture index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset;

    if (nullptr == matrix && nullptr == paint) {
        initialOffset = this->addDraw(DRAW_PICTURE, &size);
        this->addPicture(picture);
    } else {
        const SkMatrix& m = matrix ? *matrix : SkMatrix::I();
        size += m.writeToMemory(nullptr) + kUInt32Size;    // matrix + paint
        initialOffset = this->addDraw(DRAW_PICTURE_MATRIX_PAINT, &size);
        this->addPaintPtr(paint);
        this->addMatrix(m);
        this->addPicture(picture);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    // op + drawable index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset;

    if (nullptr == matrix) {
        initialOffset = this->addDraw(DRAW_DRAWABLE, &size);
        this->addDrawable(drawable);
    } else {
        size += matrix->writeToMemory(nullptr);    // matrix
        initialOffset = this->addDraw(DRAW_DRAWABLE_MATRIX, &size);
        this->addMatrix(*matrix);
        this->addDrawable(drawable);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawVertices(VertexMode vmode, int vertexCount,
                                     const SkPoint vertices[], const SkPoint texs[],
                                     const SkColor colors[], SkXfermode* xfer,
                                     const uint16_t indices[], int indexCount,
                                     const SkPaint& paint) {
    uint32_t flags = 0;
    if (texs) {
        flags |= DRAW_VERTICES_HAS_TEXS;
    }
    if (colors) {
        flags |= DRAW_VERTICES_HAS_COLORS;
    }
    if (indexCount > 0) {
        flags |= DRAW_VERTICES_HAS_INDICES;
    }
    if (xfer) {
        SkXfermode::Mode mode;
        if (xfer->asMode(&mode) && SkXfermode::kModulate_Mode != mode) {
            flags |= DRAW_VERTICES_HAS_XFER;
        }
    }

    // op + paint index + flags + vmode + vCount + vertices
    size_t size = 5 * kUInt32Size + vertexCount * sizeof(SkPoint);
    if (flags & DRAW_VERTICES_HAS_TEXS) {
        size += vertexCount * sizeof(SkPoint);  // + uvs
    }
    if (flags & DRAW_VERTICES_HAS_COLORS) {
        size += vertexCount * sizeof(SkColor);  // + vert colors
    }
    if (flags & DRAW_VERTICES_HAS_INDICES) {
        // + num indices + indices
        size += 1 * kUInt32Size + SkAlign4(indexCount * sizeof(uint16_t));
    }
    if (flags & DRAW_VERTICES_HAS_XFER) {
        size += kUInt32Size;    // mode enum
    }

    size_t initialOffset = this->addDraw(DRAW_VERTICES, &size);
    this->addPaint(paint);
    this->addInt(flags);
    this->addInt(vmode);
    this->addInt(vertexCount);
    this->addPoints(vertices, vertexCount);
    if (flags & DRAW_VERTICES_HAS_TEXS) {
        this->addPoints(texs, vertexCount);
    }
    if (flags & DRAW_VERTICES_HAS_COLORS) {
        fWriter.writeMul4(colors, vertexCount * sizeof(SkColor));
    }
    if (flags & DRAW_VERTICES_HAS_INDICES) {
        this->addInt(indexCount);
        fWriter.writePad(indices, indexCount * sizeof(uint16_t));
    }
    if (flags & DRAW_VERTICES_HAS_XFER) {
        SkXfermode::Mode mode = SkXfermode::kModulate_Mode;
        (void)xfer->asMode(&mode);
        this->addInt(mode);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                  const SkPoint texCoords[4], SkXfermode* xmode,
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
    if (xmode) {
        SkXfermode::Mode mode;
        if (xmode->asMode(&mode) && SkXfermode::kModulate_Mode != mode) {
            flag |= DRAW_VERTICES_HAS_XFER;
            size += kUInt32Size;
        }
    }

    size_t initialOffset = this->addDraw(DRAW_PATCH, &size);
    this->addPaint(paint);
    this->addPatch(cubics);
    this->addInt(flag);

    // write optional parameters
    if (colors) {
        fWriter.write(colors, SkPatchUtils::kNumCorners * sizeof(SkColor));
    }
    if (texCoords) {
        fWriter.write(texCoords, SkPatchUtils::kNumCorners * sizeof(SkPoint));
    }
    if (flag & DRAW_VERTICES_HAS_XFER) {
        SkXfermode::Mode mode = SkXfermode::kModulate_Mode;
        xmode->asMode(&mode);
        this->addInt(mode);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                                  const SkColor colors[], int count, SkXfermode::Mode mode,
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
    fWriter.write(xform, count * sizeof(SkRSXform));
    fWriter.write(tex, count * sizeof(SkRect));

    // write optional parameters
    if (colors) {
        fWriter.write(colors, count * sizeof(SkColor));
        this->addInt(mode);
    }
    if (cull) {
        fWriter.write(cull, sizeof(SkRect));
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    size_t keyLen = fWriter.WriteStringSize(key);
    size_t valueLen = fWriter.WriteDataSize(value);
    size_t size = 4 + sizeof(SkRect) + keyLen + valueLen;

    size_t initialOffset = this->addDraw(DRAW_ANNOTATION, &size);
    this->addRect(rect);
    fWriter.writeString(key);
    fWriter.writeData(value);
    this->validate(initialOffset, size);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkPictureRecord::onNewSurface(const SkImageInfo& info, const SkSurfaceProps&) {
    return nullptr;
}

// If we already have a stored, can we reuse it instead of also storing b?
static bool equivalent(const SkBitmap& a, const SkBitmap& b) {
    if (a.info() != b.info() || a.pixelRefOrigin() != b.pixelRefOrigin()) {
        // Requiring a.info() == b.info() may be overkill in some cases (alphatype mismatch),
        // but it sure makes things easier to reason about below.
        return false;
    }
    if (a.pixelRef() == b.pixelRef()) {
        return true;  // Same shape and same pixels -> same bitmap.
    }

    // From here down we're going to have to look at the bitmap data, so we require pixelRefs().
    if (!a.pixelRef() || !b.pixelRef()) {
        return false;
    }

    // If the bitmaps have encoded data, check first before locking pixels so they don't decode.
    SkAutoTUnref<SkData> encA(a.pixelRef()->refEncodedData()),
                         encB(b.pixelRef()->refEncodedData());
    if (encA && encB) {
        return encA->equals(encB);
    } else if (encA || encB) {
        return false;   // One has encoded data but the other does not.
    }

    // As a last resort, we have to look at the pixels.  This will read back textures.
    SkAutoLockPixels al(a), bl(b);
    const char* ap = (const char*)a.getPixels();
    const char* bp = (const char*)b.getPixels();
    if (ap && bp) {
        // We check row by row; row bytes might differ.
        SkASSERT(a.info() == b.info());          // We checked this above.
        SkASSERT(a.info().bytesPerPixel() > 0);  // If we have pixelRefs, this better be true.
        const SkImageInfo info = a.info();
        const size_t bytesToCompare = info.width() * info.bytesPerPixel();
        for (int row = 0; row < info.height(); row++) {
            if (0 != memcmp(ap, bp, bytesToCompare)) {
                return false;
            }
            ap += a.rowBytes();
            bp += b.rowBytes();
        }
        return true;
    }
    return false;  // Couldn't get pixels for both bitmaps.
}

void SkPictureRecord::addBitmap(const SkBitmap& bitmap) {
    // First see if we already have this bitmap.  This deduplication should really
    // only be important for our tests, where bitmaps tend not to be tagged immutable.
    // In Chrome (and hopefully Android?) they're typically immutable.
    for (int i = 0; i < fBitmaps.count(); i++) {
        if (equivalent(fBitmaps[i], bitmap)) {
            this->addInt(i);  // Unlike the rest, bitmap indices are 0-based.
            return;
        }
    }
    // Don't have it.  We'll add it to our list, making sure it's tagged as immutable.
    if (bitmap.isImmutable()) {
        // Shallow copies of bitmaps are cheap, so immutable == fast.
        fBitmaps.push_back(bitmap);
    } else {
        // If you see this block on a memory profile, it's a good opportunity to reduce RAM usage.
        SkBitmap copy;
        bitmap.copyTo(&copy);
        copy.setImmutable();
        fBitmaps.push_back(copy);
    }
    this->addInt(fBitmaps.count()-1);  // Remember, 0-based.
}

void SkPictureRecord::addImage(const SkImage* image) {
    int index = fImageRefs.find(image);
    if (index >= 0) {
        this->addInt(index);
    } else {
        *fImageRefs.append() = SkRef(image);
        this->addInt(fImageRefs.count()-1);
    }
}

void SkPictureRecord::addMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

void SkPictureRecord::addPaintPtr(const SkPaint* paint) {
    fContentInfo.onAddPaintPtr(paint);

    if (paint) {
        fPaints.push_back(*paint);
        this->addInt(fPaints.count());
    } else {
        this->addInt(0);
    }
}

int SkPictureRecord::addPathToHeap(const SkPath& path) {
    if (int* n = fPaths.find(path)) {
        return *n;
    }
    int n = fPaths.count() + 1;  // 0 is reserved for null / error.
    fPaths.set(path, n);
    return n;
}

void SkPictureRecord::addPath(const SkPath& path) {
    this->addInt(this->addPathToHeap(path));
}

void SkPictureRecord::addPatch(const SkPoint cubics[12]) {
    fWriter.write(cubics, SkPatchUtils::kNumCtrlPts * sizeof(SkPoint));
}

void SkPictureRecord::addPicture(const SkPicture* picture) {
    int index = fPictureRefs.find(picture);
    if (index < 0) {    // not found
        index = fPictureRefs.count();
        *fPictureRefs.append() = picture;
        picture->ref();
    }
    // follow the convention of recording a 1-based index
    this->addInt(index + 1);
}

void SkPictureRecord::addDrawable(SkDrawable* drawable) {
    int index = fDrawableRefs.find(drawable);
    if (index < 0) {    // not found
        index = fDrawableRefs.count();
        *fDrawableRefs.append() = drawable;
        drawable->ref();
    }
    // follow the convention of recording a 1-based index
    this->addInt(index + 1);
}

void SkPictureRecord::addPoint(const SkPoint& point) {
    fWriter.writePoint(point);
}

void SkPictureRecord::addPoints(const SkPoint pts[], int count) {
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
}

void SkPictureRecord::addNoOp() {
    size_t size = kUInt32Size; // op
    this->addDraw(NOOP, &size);
}

void SkPictureRecord::addRect(const SkRect& rect) {
    fWriter.writeRect(rect);
}

void SkPictureRecord::addRectPtr(const SkRect* rect) {
    if (fWriter.writeBool(rect != nullptr)) {
        fWriter.writeRect(*rect);
    }
}

void SkPictureRecord::addIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(rect));
}

void SkPictureRecord::addIRectPtr(const SkIRect* rect) {
    if (fWriter.writeBool(rect != nullptr)) {
        *(SkIRect*)fWriter.reserve(sizeof(SkIRect)) = *rect;
    }
}

void SkPictureRecord::addRRect(const SkRRect& rrect) {
    fWriter.writeRRect(rrect);
}

void SkPictureRecord::addRegion(const SkRegion& region) {
    fWriter.writeRegion(region);
}

void SkPictureRecord::addText(const void* text, size_t byteLength) {
    fContentInfo.onDrawText();
    addInt(SkToInt(byteLength));
    fWriter.writePad(text, byteLength);
}

void SkPictureRecord::addTextBlob(const SkTextBlob *blob) {
    int index = fTextBlobRefs.count();
    *fTextBlobRefs.append() = blob;
    blob->ref();
    // follow the convention of recording a 1-based index
    this->addInt(index + 1);
}

///////////////////////////////////////////////////////////////////////////////

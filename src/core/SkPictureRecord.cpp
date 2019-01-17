/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureRecord.h"

#include "SkCanvasPriv.h"
#include "SkClipOpPriv.h"
#include "SkDrawShadowInfo.h"
#include "SkImage_Base.h"
#include "SkMatrixPriv.h"
#include "SkPatchUtils.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkTSearch.h"
#include "SkTextBlob.h"
#include "SkTo.h"

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

///////////////////////////////////////////////////////////////////////////////

void SkPictureRecord::onFlush() {
    size_t size = sizeof(kUInt32Size);
    size_t initialOffset = this->addDraw(FLUSH, &size);
    this->validate(initialOffset, size);
}

void SkPictureRecord::willSave() {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push_back(-(int32_t)fWriter.bytesWritten());
    this->recordSave();

    this->INHERITED::willSave();
}

void SkPictureRecord::recordSave() {
    // op only
    size_t size = sizeof(kUInt32Size);
    size_t initialOffset = this->addDraw(SAVE, &size);

    this->validate(initialOffset, size);
}

SkCanvas::SaveLayerStrategy SkPictureRecord::getSaveLayerStrategy(const SaveLayerRec& rec) {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push_back(-(int32_t)fWriter.bytesWritten());
    this->recordSaveLayer(rec);

    (void)this->INHERITED::getSaveLayerStrategy(rec);
    /*  No need for a (potentially very big) layer which we don't actually need
        at this time (and may not be able to afford since during record our
        clip starts out the size of the picture, which is often much larger
        than the size of the actual device we'll use during playback).
     */
    return kNoLayer_SaveLayerStrategy;
}

bool SkPictureRecord::onDoSaveBehind(const SkRect* subset) {
    fRestoreOffsetStack.push_back(-(int32_t)fWriter.bytesWritten());

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

void SkPictureRecord::recordSaveLayer(const SaveLayerRec& rec) {
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
    size_t size = kUInt32Size + SkMatrixPriv::WriteToMemory(matrix, nullptr);
    size_t initialOffset = this->addDraw(CONCAT, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
}

void SkPictureRecord::didSetMatrix(const SkMatrix& matrix) {
    this->validate(fWriter.bytesWritten(), 0);
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

size_t SkPictureRecord::recordRestoreOffsetPlaceholder(SkClipOp op) {
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

    size_t offset = fWriter.bytesWritten();
    this->addInt(prevOffset);
    fRestoreOffsetStack.top() = SkToU32(offset);
    return offset;
}

void SkPictureRecord::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->recordClipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

size_t SkPictureRecord::recordClipRect(const SkRect& rect, SkClipOp op, bool doAA) {
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

void SkPictureRecord::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->recordClipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

size_t SkPictureRecord::recordClipRRect(const SkRRect& rrect, SkClipOp op, bool doAA) {
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

void SkPictureRecord::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    int pathID = this->addPathToHeap(path);
    this->recordClipPath(pathID, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

size_t SkPictureRecord::recordClipPath(int pathID, SkClipOp op, bool doAA) {
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

void SkPictureRecord::onClipRegion(const SkRegion& region, SkClipOp op) {
    this->recordClipRegion(region, op);
    this->INHERITED::onClipRegion(region, op);
}

size_t SkPictureRecord::recordClipRegion(const SkRegion& region, SkClipOp op) {
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

void SkPictureRecord::onDrawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
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

void SkPictureRecord::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(rect);
    size_t initialOffset = this->addDraw(DRAW_RECT, &size);
    this->addPaint(paint);
    this->addRect(rect);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawEdgeAARect(const SkRect& rect, SkCanvas::QuadAAFlags aa,
                                       SkColor color, SkBlendMode mode) {
    // op + rect + aa flags + color + mode
    size_t size = 4 * kUInt32Size + sizeof(rect);
    size_t initialOffset = this->addDraw(DRAW_EDGEAA_RECT, &size);
    this->addRect(rect);
    this->addInt((int) aa);
    this->addInt((int) color);
    this->addInt((int) mode);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    // op + paint index + region
    size_t regionBytes = region.writeToMemory(nullptr);
    size_t size = 2 * kUInt32Size + regionBytes;
    size_t initialOffset = this->addDraw(DRAW_REGION, &size);
    this->addPaint(paint);
    fWriter.writeRegion(region);
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
    // op + paint index + path index
    size_t size = 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PATH, &size);
    this->addPaint(paint);
    this->addPath(path);
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

void SkPictureRecord::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                         const SkRect& dst, const SkPaint* paint) {
    size_t latticeSize = SkCanvasPriv::WriteLattice(nullptr, lattice);
    // op + paint index + image index + lattice + dst rect
    size_t size = 3 * kUInt32Size + latticeSize + sizeof(dst);
    size_t initialOffset = this->addDraw(DRAW_IMAGE_LATTICE, &size);
    this->addPaintPtr(paint);
    this->addImage(image);
    (void)SkCanvasPriv::WriteLattice(fWriter.reservePad(latticeSize), lattice);
    this->addRect(dst);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                     SkFilterQuality filterQuality, SkBlendMode mode) {
    // op + count + alpha + fq + mode + (image index, src rect, dst rect, alpha, aa flags) * cnt
    size_t size =
            4 * kUInt32Size + (2 * kUInt32Size + 2 * sizeof(SkRect) + sizeof(SkScalar)) * count;
    size_t initialOffset = this->addDraw(DRAW_IMAGE_SET, &size);
    this->addInt(count);
    this->addInt((int)filterQuality);
    this->addInt((int)mode);
    for (int i = 0; i < count; ++i) {
        this->addImage(set[i].fImage.get());
        this->addRect(set[i].fSrcRect);
        this->addRect(set[i].fDstRect);
        this->addScalar(set[i].fAlpha);
        this->addInt((int)set[i].fAAFlags);
    }
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
        size += SkMatrixPriv::WriteToMemory(m, nullptr) + kUInt32Size;    // matrix + paint
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
        size += SkMatrixPriv::WriteToMemory(*matrix, nullptr);    // matrix
        initialOffset = this->addDraw(DRAW_DRAWABLE_MATRIX, &size);
        this->addMatrix(*matrix);
        this->addDrawable(drawable);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawVerticesObject(const SkVertices* vertices,
                                           const SkVertices::Bone bones[], int boneCount,
                                           SkBlendMode mode, const SkPaint& paint) {
    // op + paint index + vertices index + number of bones + bone matrices + mode
    size_t size = 5 * kUInt32Size + boneCount * sizeof(SkVertices::Bone);
    size_t initialOffset = this->addDraw(DRAW_VERTICES_OBJECT, &size);

    this->addPaint(paint);
    this->addVertices(vertices);
    this->addInt(boneCount);
    fWriter.write(bones, boneCount * sizeof(SkVertices::Bone));
    this->addInt(static_cast<uint32_t>(mode));

    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
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
        fWriter.write(colors, SkPatchUtils::kNumCorners * sizeof(SkColor));
    }
    if (texCoords) {
        fWriter.write(texCoords, SkPatchUtils::kNumCorners * sizeof(SkPoint));
    }
    if (flag & DRAW_VERTICES_HAS_XFER) {
        this->addInt((int)bmode);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
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
    fWriter.write(xform, count * sizeof(SkRSXform));
    fWriter.write(tex, count * sizeof(SkRect));

    // write optional parameters
    if (colors) {
        fWriter.write(colors, count * sizeof(SkColor));
        this->addInt((int)mode);
    }
    if (cull) {
        fWriter.write(cull, sizeof(SkRect));
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    // op + path index + zParams + lightPos + lightRadius + spot/ambient alphas + color + flags
    size_t size = 2 * kUInt32Size + 2 * sizeof(SkPoint3) + 1 * sizeof(SkScalar) + 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_SHADOW_REC, &size);

    this->addPath(path);

    fWriter.writePoint3(rec.fZPlaneParams);
    fWriter.writePoint3(rec.fLightPos);
    fWriter.writeScalar(rec.fLightRadius);
    fWriter.write32(rec.fAmbientColor);
    fWriter.write32(rec.fSpotColor);
    fWriter.write32(rec.fFlags);

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

// De-duping helper.

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

sk_sp<SkSurface> SkPictureRecord::onNewSurface(const SkImageInfo& info, const SkSurfaceProps&) {
    return nullptr;
}

void SkPictureRecord::addImage(const SkImage* image) {
    // convention for images is 0-based index
    this->addInt(find_or_append(fImages, image));
}

void SkPictureRecord::addMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

void SkPictureRecord::addPaintPtr(const SkPaint* paint) {
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
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fPictures, picture) + 1);
}

void SkPictureRecord::addDrawable(SkDrawable* drawable) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fDrawables, drawable) + 1);
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
    addInt(SkToInt(byteLength));
    fWriter.writePad(text, byteLength);
}

void SkPictureRecord::addTextBlob(const SkTextBlob* blob) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fTextBlobs, blob) + 1);
}

void SkPictureRecord::addVertices(const SkVertices* vertices) {
    // follow the convention of recording a 1-based index
    this->addInt(find_or_append(fVertices, vertices) + 1);
}

///////////////////////////////////////////////////////////////////////////////

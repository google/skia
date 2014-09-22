/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureRecord.h"
#include "SkBBoxHierarchy.h"
#include "SkDevice.h"
#include "SkPatchUtils.h"
#include "SkPictureStateTree.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkTextBlob.h"
#include "SkTSearch.h"

#define HEAP_BLOCK_SIZE 4096

// If SK_RECORD_LITERAL_PICTURES is defined, record our inputs as literally as possible.
// Otherwise, we can be clever and record faster equivalents.  kBeClever is normally true.
static const bool kBeClever =
#ifdef SK_RECORD_LITERAL_PICTURES
    false;
#else
    true;
#endif

enum {
    // just need a value that save or getSaveCount would never return
    kNoInitialSave = -1,
};

// A lot of basic types get stored as a uint32_t: bools, ints, paint indices, etc.
static int const kUInt32Size = 4;

static const uint32_t kSaveSize = kUInt32Size;
static const uint32_t kSaveLayerNoBoundsSize = 4 * kUInt32Size;
static const uint32_t kSaveLayerWithBoundsSize = 4 * kUInt32Size + sizeof(SkRect);

SkPictureRecord::SkPictureRecord(const SkISize& dimensions, uint32_t flags)
    : INHERITED(dimensions.width(), dimensions.height())
    , fBoundingHierarchy(NULL)
    , fStateTree(NULL)
    , fFlattenableHeap(HEAP_BLOCK_SIZE)
    , fPaints(&fFlattenableHeap)
    , fRecordFlags(flags)
    , fOptsEnabled(kBeClever) {

    fBitmapHeap = SkNEW(SkBitmapHeap);
    fFlattenableHeap.setBitmapStorage(fBitmapHeap);

    fFirstSavedLayerIndex = kNoSavedLayerIndex;
    fInitialSaveCount = kNoInitialSave;
}

SkPictureRecord::~SkPictureRecord() {
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fBoundingHierarchy);
    SkSafeUnref(fStateTree);
    fFlattenableHeap.setBitmapStorage(NULL);
    fPictureRefs.unrefAll();
    fTextBlobRefs.unrefAll();
}

///////////////////////////////////////////////////////////////////////////////

// Return the offset of the paint inside a given op's byte stream. A zero
// return value means there is no paint (and you really shouldn't be calling
// this method)
static inline size_t getPaintOffset(DrawType op, size_t opSize) {
    // These offsets are where the paint would be if the op size doesn't overflow
    static const uint8_t gPaintOffsets[] = {
        0,  // UNUSED - no paint
        0,  // CLIP_PATH - no paint
        0,  // CLIP_REGION - no paint
        0,  // CLIP_RECT - no paint
        0,  // CLIP_RRECT - no paint
        0,  // CONCAT - no paint
        1,  // DRAW_BITMAP - right after op code
        1,  // DRAW_BITMAP_MATRIX - right after op code
        1,  // DRAW_BITMAP_NINE - right after op code
        1,  // DRAW_BITMAP_RECT_TO_RECT - right after op code
        0,  // DRAW_CLEAR - no paint
        0,  // DRAW_DATA - no paint
        1,  // DRAW_OVAL - right after op code
        1,  // DRAW_PAINT - right after op code
        1,  // DRAW_PATH - right after op code
        0,  // DRAW_PICTURE - no paint
        1,  // DRAW_POINTS - right after op code
        1,  // DRAW_POS_TEXT - right after op code
        1,  // DRAW_POS_TEXT_TOP_BOTTOM - right after op code
        1,  // DRAW_POS_TEXT_H - right after op code
        1,  // DRAW_POS_TEXT_H_TOP_BOTTOM - right after op code
        1,  // DRAW_RECT - right after op code
        1,  // DRAW_RRECT - right after op code
        1,  // DRAW_SPRITE - right after op code
        1,  // DRAW_TEXT - right after op code
        1,  // DRAW_TEXT_ON_PATH - right after op code
        1,  // DRAW_TEXT_TOP_BOTTOM - right after op code
        1,  // DRAW_VERTICES - right after op code
        0,  // RESTORE - no paint
        0,  // ROTATE - no paint
        0,  // SAVE - no paint
        0,  // SAVE_LAYER - see below - this paint's location varies
        0,  // SCALE - no paint
        0,  // SET_MATRIX - no paint
        0,  // SKEW - no paint
        0,  // TRANSLATE - no paint
        0,  // NOOP - no paint
        0,  // BEGIN_GROUP - no paint
        0,  // COMMENT - no paint
        0,  // END_GROUP - no paint
        1,  // DRAWDRRECT - right after op code
        0,  // PUSH_CULL - no paint
        0,  // POP_CULL - no paint
        1,  // DRAW_PATCH - right after op code
        1,  // DRAW_PICTURE_MATRIX_PAINT - right after op code
        1,  // DRAW_TEXT_BLOB- right after op code
    };

    SK_COMPILE_ASSERT(sizeof(gPaintOffsets) == LAST_DRAWTYPE_ENUM + 1,
                      need_to_be_in_sync);
    SkASSERT((unsigned)op <= (unsigned)LAST_DRAWTYPE_ENUM);

    int overflow = 0;
    if (0 != (opSize & ~MASK_24) || opSize == MASK_24) {
        // This op's size overflows so an extra uint32_t will be written
        // after the op code
        overflow = sizeof(uint32_t);
    }

    if (SAVE_LAYER == op) {
        static const uint32_t kSaveLayerNoBoundsPaintOffset = 2 * kUInt32Size;
        static const uint32_t kSaveLayerWithBoundsPaintOffset = 2 * kUInt32Size + sizeof(SkRect);

        if (kSaveLayerNoBoundsSize == opSize) {
            return kSaveLayerNoBoundsPaintOffset + overflow;
        } else {
            SkASSERT(kSaveLayerWithBoundsSize == opSize);
            return kSaveLayerWithBoundsPaintOffset + overflow;
        }
    }

    SkASSERT(0 != gPaintOffsets[op]);   // really shouldn't be calling this method
    return gPaintOffsets[op] * sizeof(uint32_t) + overflow;
}

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
    size_t size = kSaveSize;
    size_t initialOffset = this->addDraw(SAVE, &size);

    this->validate(initialOffset, size);
}

SkCanvas::SaveLayerStrategy SkPictureRecord::willSaveLayer(const SkRect* bounds,
                                                           const SkPaint* paint, SaveFlags flags) {
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.bytesWritten());
    this->recordSaveLayer(bounds, paint, flags);
    if (kNoSavedLayerIndex == fFirstSavedLayerIndex) {
        fFirstSavedLayerIndex = fRestoreOffsetStack.count();
    }

    this->INHERITED::willSaveLayer(bounds, paint, flags);
    /*  No need for a (potentially very big) layer which we don't actually need
        at this time (and may not be able to afford since during record our
        clip starts out the size of the picture, which is often much larger
        than the size of the actual device we'll use during playback).
     */
    return kNoLayer_SaveLayerStrategy;
}

void SkPictureRecord::recordSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                      SaveFlags flags) {
    fContentInfo.onSaveLayer();

    // op + bool for 'bounds'
    size_t size = 2 * kUInt32Size;
    if (bounds) {
        size += sizeof(*bounds); // + rect
    }
    // + paint index + flags
    size += 2 * kUInt32Size;

    SkASSERT(kSaveLayerNoBoundsSize == size || kSaveLayerWithBoundsSize == size);

    size_t initialOffset = this->addDraw(SAVE_LAYER, &size);
    this->addRectPtr(bounds);
    SkASSERT(initialOffset+getPaintOffset(SAVE_LAYER, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    this->addInt(flags);

    this->validate(initialOffset, size);
}

bool SkPictureRecord::isDrawingToLayer() const {
    return fFirstSavedLayerIndex != kNoSavedLayerIndex;
}

/*
 * Read the op code from 'offset' in 'writer'.
 */
#ifdef SK_DEBUG
static DrawType peek_op(SkWriter32* writer, size_t offset) {
    return (DrawType)(writer->readTAt<uint32_t>(offset) >> 24);
}
#endif

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

// Is the supplied paint simply a color?
static bool is_simple(const SkPaint& p) {
    intptr_t orAccum = (intptr_t)p.getPathEffect()  |
                       (intptr_t)p.getShader()      |
                       (intptr_t)p.getXfermode()    |
                       (intptr_t)p.getMaskFilter()  |
                       (intptr_t)p.getColorFilter() |
                       (intptr_t)p.getRasterizer()  |
                       (intptr_t)p.getLooper()      |
                       (intptr_t)p.getImageFilter();
    return 0 == orAccum;
}

// CommandInfos are fed to the 'match' method and filled in with command
// information.
struct CommandInfo {
    DrawType fActualOp;
    uint32_t fOffset;
    uint32_t fSize;
};

/*
 * Attempt to match the provided pattern of commands starting at 'offset'
 * in the byte stream and stopping at the end of the stream. Upon success,
 * return true with all the pattern information filled out in the result
 * array (i.e., actual ops, offsets and sizes).
 * Note this method skips any NOOPs seen in the stream
 */
static bool match(SkWriter32* writer, uint32_t offset,
                  int* pattern, CommandInfo* result, int numCommands) {
    SkASSERT(offset < writer->bytesWritten());

    uint32_t curOffset = offset;
    uint32_t curSize = 0;
    int numMatched;
    for (numMatched = 0; numMatched < numCommands && curOffset < writer->bytesWritten(); ++numMatched) {
        DrawType op = peek_op_and_size(writer, curOffset, &curSize);
        while (NOOP == op) {
            curOffset += curSize;
            if (curOffset >= writer->bytesWritten()) {
                return false;
            }
            op = peek_op_and_size(writer, curOffset, &curSize);
        }

        if (kDRAW_BITMAP_FLAVOR == pattern[numMatched]) {
            if (DRAW_BITMAP != op && DRAW_BITMAP_MATRIX != op &&
                DRAW_BITMAP_NINE != op && DRAW_BITMAP_RECT_TO_RECT != op) {
                return false;
            }
        } else if (op != pattern[numMatched]) {
            return false;
        }

        result[numMatched].fActualOp = op;
        result[numMatched].fOffset = curOffset;
        result[numMatched].fSize = curSize;

        curOffset += curSize;
    }

    if (numMatched != numCommands) {
        return false;
    }

    if (curOffset < writer->bytesWritten()) {
        // Something else between the last command and the end of the stream
        return false;
    }

    return true;
}

// temporarily here to make code review easier
static bool merge_savelayer_paint_into_drawbitmp(SkWriter32* writer,
                                                 SkPaintDictionary* paintDict,
                                                 const CommandInfo& saveLayerInfo,
                                                 const CommandInfo& dbmInfo);

/*
 * Restore has just been called (but not recorded), look back at the
 * matching save* and see if we are in the configuration:
 *   SAVE_LAYER
 *       DRAW_BITMAP|DRAW_BITMAP_MATRIX|DRAW_BITMAP_NINE|DRAW_BITMAP_RECT_TO_RECT
 *   RESTORE
 * where the saveLayer's color can be moved into the drawBitmap*'s paint
 */
static bool remove_save_layer1(SkWriter32* writer, int32_t offset,
                               SkPaintDictionary* paintDict) {
    // back up to the save block
    // TODO: add a stack to track save*/restore offsets rather than searching backwards
    while (offset > 0) {
        offset = writer->readTAt<uint32_t>(offset);
    }

    int pattern[] = { SAVE_LAYER, kDRAW_BITMAP_FLAVOR, /* RESTORE */ };
    CommandInfo result[SK_ARRAY_COUNT(pattern)];

    if (!match(writer, -offset, pattern, result, SK_ARRAY_COUNT(pattern))) {
        return false;
    }

    if (kSaveLayerWithBoundsSize == result[0].fSize) {
        // The saveLayer's bound can offset where the dbm is drawn
        return false;
    }

    return merge_savelayer_paint_into_drawbitmp(writer, paintDict,
                                                result[0], result[1]);
}

/*
 * Convert the command code located at 'offset' to a NOOP. Leave the size
 * field alone so the NOOP can be skipped later.
 */
static void convert_command_to_noop(SkWriter32* writer, uint32_t offset) {
    uint32_t command = writer->readTAt<uint32_t>(offset);
    writer->overwriteTAt(offset, (command & MASK_24) | (NOOP << 24));
}

/*
 * Attempt to merge the saveLayer's paint into the drawBitmap*'s paint.
 * Return true on success; false otherwise.
 */
static bool merge_savelayer_paint_into_drawbitmp(SkWriter32* writer,
                                                 SkPaintDictionary* paintDict,
                                                 const CommandInfo& saveLayerInfo,
                                                 const CommandInfo& dbmInfo) {
    SkASSERT(SAVE_LAYER == saveLayerInfo.fActualOp);
    SkASSERT(DRAW_BITMAP == dbmInfo.fActualOp ||
             DRAW_BITMAP_MATRIX == dbmInfo.fActualOp ||
             DRAW_BITMAP_NINE == dbmInfo.fActualOp ||
             DRAW_BITMAP_RECT_TO_RECT == dbmInfo.fActualOp);

    size_t dbmPaintOffset = getPaintOffset(dbmInfo.fActualOp, dbmInfo.fSize);
    size_t slPaintOffset = getPaintOffset(SAVE_LAYER, saveLayerInfo.fSize);

    // we have a match, now we need to get the paints involved
    uint32_t dbmPaintId = writer->readTAt<uint32_t>(dbmInfo.fOffset + dbmPaintOffset);
    uint32_t saveLayerPaintId = writer->readTAt<uint32_t>(saveLayerInfo.fOffset + slPaintOffset);

    if (0 == saveLayerPaintId) {
        // In this case the saveLayer/restore isn't needed at all - just kill the saveLayer
        // and signal the caller (by returning true) to not add the RESTORE op
        convert_command_to_noop(writer, saveLayerInfo.fOffset);
        return true;
    }

    if (0 == dbmPaintId) {
        // In this case just make the DBM* use the saveLayer's paint, kill the saveLayer
        // and signal the caller (by returning true) to not add the RESTORE op
        convert_command_to_noop(writer, saveLayerInfo.fOffset);
        writer->overwriteTAt(dbmInfo.fOffset + dbmPaintOffset, saveLayerPaintId);
        return true;
    }

    SkAutoTDelete<SkPaint> saveLayerPaint(paintDict->unflatten(saveLayerPaintId));
    if (NULL == saveLayerPaint.get() || !is_simple(*saveLayerPaint)) {
        return false;
    }

    // For this optimization we only fold the saveLayer and drawBitmapRect
    // together if the saveLayer's draw is simple (i.e., no fancy effects) and
    // and the only difference in the colors is that the saveLayer's can have
    // an alpha while the drawBitmapRect's is opaque.
    // TODO: it should be possible to fold them together even if they both
    // have different non-255 alphas
    SkColor layerColor = saveLayerPaint->getColor() | 0xFF000000; // force opaque

    SkAutoTDelete<SkPaint> dbmPaint(paintDict->unflatten(dbmPaintId));
    if (NULL == dbmPaint.get() || dbmPaint->getColor() != layerColor || !is_simple(*dbmPaint)) {
        return false;
    }

    SkColor newColor = SkColorSetA(dbmPaint->getColor(),
                                   SkColorGetA(saveLayerPaint->getColor()));
    dbmPaint->setColor(newColor);

    const SkFlatData* data = paintDict->findAndReturnFlat(*dbmPaint);
    if (NULL == data) {
        return false;
    }

    // kill the saveLayer and alter the DBMR2R's paint to be the modified one
    convert_command_to_noop(writer, saveLayerInfo.fOffset);
    writer->overwriteTAt(dbmInfo.fOffset + dbmPaintOffset, data->index());
    return true;
}

/*
 * Restore has just been called (but not recorded), look back at the
 * matching save* and see if we are in the configuration:
 *   SAVE_LAYER (with NULL == bounds)
 *      SAVE
 *         CLIP_RECT
 *         DRAW_BITMAP|DRAW_BITMAP_MATRIX|DRAW_BITMAP_NINE|DRAW_BITMAP_RECT_TO_RECT
 *      RESTORE
 *   RESTORE
 * where the saveLayer's color can be moved into the drawBitmap*'s paint
 */
static bool remove_save_layer2(SkWriter32* writer, int32_t offset,
                               SkPaintDictionary* paintDict) {
    // back up to the save block
    // TODO: add a stack to track save*/restore offsets rather than searching backwards
    while (offset > 0) {
        offset = writer->readTAt<uint32_t>(offset);
    }

    int pattern[] = { SAVE_LAYER, SAVE, CLIP_RECT, kDRAW_BITMAP_FLAVOR, RESTORE, /* RESTORE */ };
    CommandInfo result[SK_ARRAY_COUNT(pattern)];

    if (!match(writer, -offset, pattern, result, SK_ARRAY_COUNT(pattern))) {
        return false;
    }

    if (kSaveLayerWithBoundsSize == result[0].fSize) {
        // The saveLayer's bound can offset where the dbm is drawn
        return false;
    }

    return merge_savelayer_paint_into_drawbitmp(writer, paintDict,
                                                result[0], result[3]);
}

static bool is_drawing_op(DrawType op) {

    // FIXME: yuck. convert to a lookup table?
    return (op > CONCAT && op < ROTATE)
            || DRAW_DRRECT == op
            || DRAW_PATCH == op
            || DRAW_PICTURE_MATRIX_PAINT == op
            || DRAW_TEXT_BLOB == op;
}

/*
 *  Restore has just been called (but not recorded), so look back at the
 *  matching save(), and see if we can eliminate the pair of them, due to no
 *  intervening matrix/clip calls.
 *
 *  If so, update the writer and return true, in which case we won't even record
 *  the restore() call. If we still need the restore(), return false.
 */
static bool collapse_save_clip_restore(SkWriter32* writer, int32_t offset,
                                       SkPaintDictionary* paintDict) {
    int32_t restoreOffset = (int32_t)writer->bytesWritten();

    // back up to the save block
    while (offset > 0) {
        offset = writer->readTAt<uint32_t>(offset);
    }

    // now offset points to a save
    offset = -offset;
    uint32_t opSize;
    DrawType op = peek_op_and_size(writer, offset, &opSize);
    if (SAVE_LAYER == op) {
        // not ready to cull these out yet (mrr)
        return false;
    }
    SkASSERT(SAVE == op);
    SkASSERT(kSaveSize == opSize);

    // Walk forward until we get back to either a draw-verb (abort) or we hit
    // our restore (success).
    int32_t saveOffset = offset;

    offset += opSize;
    while (offset < restoreOffset) {
        op = peek_op_and_size(writer, offset, &opSize);
        if (is_drawing_op(op) || (SAVE_LAYER == op)) {
            // drawing verb, abort
            return false;
        }
        offset += opSize;
    }

    writer->rewindToOffset(saveOffset);
    return true;
}

typedef bool (*PictureRecordOptProc)(SkWriter32* writer, int32_t offset,
                                     SkPaintDictionary* paintDict);
enum PictureRecordOptType {
    kRewind_OptType,  // Optimization rewinds the command stream
    kCollapseSaveLayer_OptType,  // Optimization eliminates a save/restore pair
};

enum PictureRecordOptFlags {
    kSkipIfBBoxHierarchy_Flag  = 0x1,  // Optimization should be skipped if the
                                       // SkPicture has a bounding box hierarchy.
    kRescindLastSave_Flag      = 0x2,
    kRescindLastSaveLayer_Flag = 0x4,
};

struct PictureRecordOpt {
    PictureRecordOptProc fProc;
    PictureRecordOptType fType;
    unsigned fFlags;
};
/*
 * A list of the optimizations that are tried upon seeing a restore
 * TODO: add a real API for such optimizations
 *       Add the ability to fire optimizations on any op (not just RESTORE)
 */
static const PictureRecordOpt gPictureRecordOpts[] = {
    // 'collapse_save_clip_restore' is skipped if there is a BBoxHierarchy
    // because it is redundant with the state traversal optimization in
    // SkPictureStateTree, and applying the optimization introduces significant
    // record time overhead because it requires rewinding contents that were
    // recorded into the BBoxHierarchy.
    { collapse_save_clip_restore, kRewind_OptType, 
                                                kSkipIfBBoxHierarchy_Flag|kRescindLastSave_Flag },
    { remove_save_layer1,         kCollapseSaveLayer_OptType, kRescindLastSaveLayer_Flag },
    { remove_save_layer2,         kCollapseSaveLayer_OptType, kRescindLastSaveLayer_Flag }
};

// This is called after an optimization has been applied to the command stream
// in order to adjust the contents and state of the bounding box hierarchy and
// state tree to reflect the optimization.
static void apply_optimization_to_bbh(PictureRecordOptType opt, SkPictureStateTree* stateTree,
                                      SkBBoxHierarchy* boundingHierarchy) {
    switch (opt) {
    case kCollapseSaveLayer_OptType:
        if (stateTree) {
            stateTree->saveCollapsed();
        }
        break;
    case kRewind_OptType:
        if (boundingHierarchy) {
            boundingHierarchy->rewindInserts();
        }
        // Note: No need to touch the state tree for this to work correctly.
        // Unused branches do not burden the playback, and pruning the tree
        // would be O(N^2), so it is best to leave it alone.
        break;
    default:
        SkASSERT(0);
    }
}

void SkPictureRecord::willRestore() {
    // FIXME: SkDeferredCanvas needs to be refactored to respect
    // save/restore balancing so that the following test can be
    // turned on permanently.
#if 0
    SkASSERT(fRestoreOffsetStack.count() > 1);
#endif

    // check for underflow
    if (fRestoreOffsetStack.count() == 0) {
        return;
    }

    if (fRestoreOffsetStack.count() == fFirstSavedLayerIndex) {
        fFirstSavedLayerIndex = kNoSavedLayerIndex;
    }

    size_t opt = 0;
    if (fOptsEnabled) {
        for (opt = 0; opt < SK_ARRAY_COUNT(gPictureRecordOpts); ++opt) {
            if (0 != (gPictureRecordOpts[opt].fFlags & kSkipIfBBoxHierarchy_Flag)
                && fBoundingHierarchy) {
                continue;
            }
            if ((*gPictureRecordOpts[opt].fProc)(&fWriter, fRestoreOffsetStack.top(), &fPaints)) {
                // Some optimization fired so don't add the RESTORE
                apply_optimization_to_bbh(gPictureRecordOpts[opt].fType,
                                          fStateTree, fBoundingHierarchy);
                if (gPictureRecordOpts[opt].fFlags & kRescindLastSave_Flag) {
                    fContentInfo.rescindLastSave();
                } else if (gPictureRecordOpts[opt].fFlags & kRescindLastSaveLayer_Flag) {
                    fContentInfo.rescindLastSaveLayer();
                } 
                break;
            }
        }
    }

    if (!fOptsEnabled || SK_ARRAY_COUNT(gPictureRecordOpts) == opt) {
        // No optimization fired so add the RESTORE
        this->recordRestore();
    }

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
    size_t size = kUInt32Size + matrix.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(CONCAT, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
}

void SkPictureRecord::didSetMatrix(const SkMatrix& matrix) {
    this->validate(fWriter.bytesWritten(), 0);
    // op + matrix
    size_t size = kUInt32Size + matrix.writeToMemory(NULL);
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
    // assert that the final offset value points to a save verb
    uint32_t opSize;
    DrawType drawOp = peek_op_and_size(&fWriter, -offset, &opSize);
    SkASSERT(SAVE == drawOp || SAVE_LAYER == drawOp);
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
    size_t size = 2 * kUInt32Size + region.writeToMemory(NULL);
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

void SkPictureRecord::clear(SkColor color) {
    // op + color
    size_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_CLEAR, &size);
    this->addInt(color);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPaint(const SkPaint& paint) {
    // op + paint index
    size_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PAINT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_PAINT, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                 const SkPaint& paint) {
    fContentInfo.onDrawPoints(count, paint);

    // op + paint index + mode + count + point data
    size_t size = 4 * kUInt32Size + count * sizeof(SkPoint);
    size_t initialOffset = this->addDraw(DRAW_POINTS, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_POINTS, size) == fWriter.bytesWritten());
    this->addPaint(paint);

    this->addInt(mode);
    this->addInt(SkToInt(count));
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawOval(const SkRect& oval, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(oval);
    size_t initialOffset = this->addDraw(DRAW_OVAL, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_OVAL, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addRect(oval);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawRect(const SkRect& rect, const SkPaint& paint) {
    // op + paint index + rect
    size_t size = 2 * kUInt32Size + sizeof(rect);
    size_t initialOffset = this->addDraw(DRAW_RECT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_RECT, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addRect(rect);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    if (rrect.isRect() && kBeClever) {
        this->SkPictureRecord::drawRect(rrect.getBounds(), paint);
    } else if (rrect.isOval() && kBeClever) {
        this->SkPictureRecord::drawOval(rrect.getBounds(), paint);
    } else {
        // op + paint index + rrect
        size_t size = 2 * kUInt32Size + SkRRect::kSizeInMemory;
        size_t initialOffset = this->addDraw(DRAW_RRECT, &size);
        SkASSERT(initialOffset+getPaintOffset(DRAW_RRECT, size) == fWriter.bytesWritten());
        this->addPaint(paint);
        this->addRRect(rrect);
        this->validate(initialOffset, size);
    }
}

void SkPictureRecord::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                   const SkPaint& paint) {
    // op + paint index + rrects
    size_t size = 2 * kUInt32Size + SkRRect::kSizeInMemory * 2;
    size_t initialOffset = this->addDraw(DRAW_DRRECT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_DRRECT, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addRRect(outer);
    this->addRRect(inner);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPath(const SkPath& path, const SkPaint& paint) {
    fContentInfo.onDrawPath(path, paint);

    // op + paint index + path index
    size_t size = 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PATH, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_PATH, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addPath(path);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                 const SkPaint* paint = NULL) {
    if (bitmap.drawsNothing() && kBeClever) {
        return;
    }

    // op + paint index + bitmap index + left + top
    size_t size = 3 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(DRAW_BITMAP, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addScalar(left);
    this->addScalar(top);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                           const SkRect& dst, const SkPaint* paint,
                                           DrawBitmapRectFlags flags) {
    if (bitmap.drawsNothing() && kBeClever) {
        return;
    }

    // id + paint index + bitmap index + bool for 'src' + flags
    size_t size = 5 * kUInt32Size;
    if (src) {
        size += sizeof(*src);   // + rect
    }
    size += sizeof(dst);        // + rect

    size_t initialOffset = this->addDraw(DRAW_BITMAP_RECT_TO_RECT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP_RECT_TO_RECT, size)
             == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addRectPtr(src);  // may be null
    this->addRect(dst);
    this->addInt(flags);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                                       const SkPaint* paint) {
    if (bitmap.drawsNothing() && kBeClever) {
        return;
    }

    // id + paint index + bitmap index + matrix
    size_t size = 3 * kUInt32Size + matrix.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(DRAW_BITMAP_MATRIX, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP_MATRIX, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                     const SkRect& dst, const SkPaint* paint) {
    if (bitmap.drawsNothing() && kBeClever) {
        return;
    }

    // op + paint index + bitmap id + center + dst rect
    size_t size = 3 * kUInt32Size + sizeof(center) + sizeof(dst);
    size_t initialOffset = this->addDraw(DRAW_BITMAP_NINE, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP_NINE, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addIRect(center);
    this->addRect(dst);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawSprite(const SkBitmap& bitmap, int left, int top,
                                 const SkPaint* paint = NULL) {
    if (bitmap.drawsNothing() && kBeClever) {
        return;
    }

    // op + paint index + bitmap index + left + top
    size_t size = 5 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_SPRITE, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_SPRITE, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    this->addBitmap(bitmap);
    this->addInt(left);
    this->addInt(top);
    this->validate(initialOffset, size);
}

void SkPictureRecord::ComputeFontMetricsTopBottom(const SkPaint& paint, SkScalar topbot[2]) {
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);
    SkRect bounds;
    // construct a rect so we can see any adjustments from the paint.
    // we use 0,1 for left,right, just so the rect isn't empty
    bounds.set(0, metrics.fTop, SK_Scalar1, metrics.fBottom);
    (void)paint.computeFastBounds(bounds, &bounds);
    topbot[0] = bounds.fTop;
    topbot[1] = bounds.fBottom;
}

void SkPictureRecord::addFontMetricsTopBottom(const SkPaint& paint, const SkFlatData& flat,
                                              SkScalar minY, SkScalar maxY) {
    WriteTopBot(paint, flat);
    this->addScalar(flat.topBot()[0] + minY);
    this->addScalar(flat.topBot()[1] + maxY);
}

void SkPictureRecord::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                 const SkPaint& paint) {
    bool fast = !paint.isVerticalText() && paint.canComputeFastBounds() && kBeClever;

    // op + paint index + length + 'length' worth of chars + x + y
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 2 * sizeof(SkScalar);
    if (fast) {
        size += 2 * sizeof(SkScalar); // + top & bottom
    }

    DrawType op = fast ? DRAW_TEXT_TOP_BOTTOM : DRAW_TEXT;
    size_t initialOffset = this->addDraw(op, &size);
    SkASSERT(initialOffset+getPaintOffset(op, size) == fWriter.bytesWritten());
    const SkFlatData* flatPaintData = addPaint(paint);
    SkASSERT(flatPaintData);
    this->addText(text, byteLength);
    this->addScalar(x);
    this->addScalar(y);
    if (fast) {
        this->addFontMetricsTopBottom(paint, *flatPaintData, y, y);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                    const SkPaint& paint) {
    int points = paint.countText(text, byteLength);
    if (0 == points)
        return;

    bool canUseDrawH = true;
    SkScalar minY = pos[0].fY;
    SkScalar maxY = pos[0].fY;
    // check if the caller really should have used drawPosTextH()
    {
        const SkScalar firstY = pos[0].fY;
        for (int index = 1; index < points; index++) {
            if (pos[index].fY != firstY) {
                canUseDrawH = false;
                if (pos[index].fY < minY) {
                    minY = pos[index].fY;
                } else if (pos[index].fY > maxY) {
                    maxY = pos[index].fY;
                }
            }
        }
    }

    bool fastBounds = !paint.isVerticalText() && paint.canComputeFastBounds() && kBeClever;
    bool fast = canUseDrawH && fastBounds && kBeClever;

    // op + paint index + length + 'length' worth of data + num points
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
    if (canUseDrawH) {
        if (fast) {
            size += 2 * sizeof(SkScalar); // + top & bottom
        }
        // + y-pos + actual x-point data
        size += sizeof(SkScalar) + points * sizeof(SkScalar);
    } else {
        // + x&y point data
        size += points * sizeof(SkPoint);
        if (fastBounds) {
            size += 2 * sizeof(SkScalar); // + top & bottom
        }
    }

    DrawType op;
    if (fast) {
        op = DRAW_POS_TEXT_H_TOP_BOTTOM;
    } else if (canUseDrawH) {
        op = DRAW_POS_TEXT_H;
    } else if (fastBounds) {
        op = DRAW_POS_TEXT_TOP_BOTTOM;
    } else {
        op = DRAW_POS_TEXT;
    }
    size_t initialOffset = this->addDraw(op, &size);
    SkASSERT(initialOffset+getPaintOffset(op, size) == fWriter.bytesWritten());
    const SkFlatData* flatPaintData = this->addPaint(paint);
    SkASSERT(flatPaintData);
    this->addText(text, byteLength);
    this->addInt(points);

    if (canUseDrawH) {
        if (fast) {
            this->addFontMetricsTopBottom(paint, *flatPaintData, pos[0].fY, pos[0].fY);
        }
        this->addScalar(pos[0].fY);
        SkScalar* xptr = (SkScalar*)fWriter.reserve(points * sizeof(SkScalar));
        for (int index = 0; index < points; index++)
            *xptr++ = pos[index].fX;
    } else {
        fWriter.writeMul4(pos, points * sizeof(SkPoint));
        if (fastBounds) {
            this->addFontMetricsTopBottom(paint, *flatPaintData, minY, maxY);
        }
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                     SkScalar constY, const SkPaint& paint) {
    const SkFlatData* flatPaintData = this->getFlatPaintData(paint);
    this->drawPosTextHImpl(text, byteLength, xpos, constY, paint, flatPaintData);
}

void SkPictureRecord::drawPosTextHImpl(const void* text, size_t byteLength,
                          const SkScalar xpos[], SkScalar constY,
                          const SkPaint& paint, const SkFlatData* flatPaintData) {
    int points = paint.countText(text, byteLength);
    if (0 == points && kBeClever) {
        return;
    }

    bool fast = !paint.isVerticalText() && paint.canComputeFastBounds() && kBeClever;

    // op + paint index + length + 'length' worth of data + num points
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
    if (fast) {
        size += 2 * sizeof(SkScalar); // + top & bottom
    }
    // + y + the actual points
    size += 1 * kUInt32Size + points * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(fast ? DRAW_POS_TEXT_H_TOP_BOTTOM : DRAW_POS_TEXT_H,
                                         &size);
    SkASSERT(flatPaintData);
    this->addFlatPaint(flatPaintData);

    this->addText(text, byteLength);
    this->addInt(points);

    if (fast) {
        this->addFontMetricsTopBottom(paint, *flatPaintData, constY, constY);
    }
    this->addScalar(constY);
    fWriter.writeMul4(xpos, points * sizeof(SkScalar));
    this->validate(initialOffset, size);
}

void SkPictureRecord::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                       const SkMatrix* matrix, const SkPaint& paint) {
    // op + paint index + length + 'length' worth of data + path index + matrix
    const SkMatrix& m = matrix ? *matrix : SkMatrix::I();
    size_t size = 3 * kUInt32Size + SkAlign4(byteLength) + kUInt32Size + m.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(DRAW_TEXT_ON_PATH, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_TEXT_ON_PATH, size) == fWriter.bytesWritten());
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
    SkASSERT(initialOffset + getPaintOffset(DRAW_TEXT_BLOB, size) == fWriter.bytesWritten());

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

    if (NULL == matrix && NULL == paint) {
        initialOffset = this->addDraw(DRAW_PICTURE, &size);
        this->addPicture(picture);
    } else {
        const SkMatrix& m = matrix ? *matrix : SkMatrix::I();
        size += m.writeToMemory(NULL) + kUInt32Size;    // matrix + paint
        initialOffset = this->addDraw(DRAW_PICTURE_MATRIX_PAINT, &size);
        SkASSERT(initialOffset + getPaintOffset(DRAW_PICTURE_MATRIX_PAINT, size)
                 == fWriter.bytesWritten());
        this->addPaintPtr(paint);
        this->addMatrix(m);
        this->addPicture(picture);
    }
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawVertices(VertexMode vmode, int vertexCount,
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
    SkASSERT(initialOffset+getPaintOffset(DRAW_VERTICES, size) == fWriter.bytesWritten());
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
    SkASSERT(initialOffset+getPaintOffset(DRAW_PATCH, size) == fWriter.bytesWritten());
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

void SkPictureRecord::drawData(const void* data, size_t length) {
    // op + length + 'length' worth of data
    size_t size = 2 * kUInt32Size + SkAlign4(length);
    size_t initialOffset = this->addDraw(DRAW_DATA, &size);
    this->addInt(SkToInt(length));
    fWriter.writePad(data, length);
    this->validate(initialOffset, size);
}

void SkPictureRecord::beginCommentGroup(const char* description) {
    // op/size + length of string + \0 terminated chars
    size_t length = strlen(description);
    size_t size = 2 * kUInt32Size + SkAlign4(length + 1);
    size_t initialOffset = this->addDraw(BEGIN_COMMENT_GROUP, &size);
    fWriter.writeString(description, length);
    this->validate(initialOffset, size);
}

void SkPictureRecord::addComment(const char* kywd, const char* value) {
    // op/size + 2x length of string + 2x \0 terminated chars
    size_t kywdLen = strlen(kywd);
    size_t valueLen = strlen(value);
    size_t size = 3 * kUInt32Size + SkAlign4(kywdLen + 1) + SkAlign4(valueLen + 1);
    size_t initialOffset = this->addDraw(COMMENT, &size);
    fWriter.writeString(kywd, kywdLen);
    fWriter.writeString(value, valueLen);
    this->validate(initialOffset, size);
}

void SkPictureRecord::endCommentGroup() {
    // op/size
    size_t size = 1 * kUInt32Size;
    size_t initialOffset = this->addDraw(END_COMMENT_GROUP, &size);
    this->validate(initialOffset, size);
}

// [op/size] [rect] [skip offset]
static const uint32_t kPushCullOpSize = 2 * kUInt32Size + sizeof(SkRect);
void SkPictureRecord::onPushCull(const SkRect& cullRect) {
    size_t size = kPushCullOpSize;
    size_t initialOffset = this->addDraw(PUSH_CULL, &size);
    // PUSH_CULL's size should stay constant (used to rewind).
    SkASSERT(size == kPushCullOpSize);

    this->addRect(cullRect);
    fCullOffsetStack.push(SkToU32(fWriter.bytesWritten()));
    this->addInt(0);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onPopCull() {
    SkASSERT(!fCullOffsetStack.isEmpty());

    uint32_t cullSkipOffset = fCullOffsetStack.top();
    fCullOffsetStack.pop();

    // Collapse empty push/pop pairs.
    if ((size_t)(cullSkipOffset + kUInt32Size) == fWriter.bytesWritten() && kBeClever) {
        SkASSERT(fWriter.bytesWritten() >= kPushCullOpSize);
        SkASSERT(PUSH_CULL == peek_op(&fWriter, fWriter.bytesWritten() - kPushCullOpSize));
        fWriter.rewindToOffset(fWriter.bytesWritten() - kPushCullOpSize);
        return;
    }

    // op only
    size_t size = kUInt32Size;
    size_t initialOffset = this->addDraw(POP_CULL, &size);

    // update the cull skip offset to point past this op.
    fWriter.overwriteTAt<uint32_t>(cullSkipOffset, SkToU32(fWriter.bytesWritten()));

    this->validate(initialOffset, size);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkPictureRecord::onNewSurface(const SkImageInfo& info, const SkSurfaceProps&) {
    return NULL;
}

int SkPictureRecord::addBitmap(const SkBitmap& bitmap) {
    const int index = fBitmapHeap->insert(bitmap);
    // In debug builds, a bad return value from insert() will crash, allowing for debugging. In
    // release builds, the invalid value will be recorded so that the reader will know that there
    // was a problem.
    SkASSERT(index != SkBitmapHeap::INVALID_SLOT);
    this->addInt(index);
    return index;
}

void SkPictureRecord::addMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

const SkFlatData* SkPictureRecord::getFlatPaintData(const SkPaint& paint) {
    return fPaints.findAndReturnFlat(paint);
}

const SkFlatData* SkPictureRecord::addPaintPtr(const SkPaint* paint) {
    fContentInfo.onAddPaintPtr(paint);

    const SkFlatData* data = paint ? getFlatPaintData(*paint) : NULL;
    this->addFlatPaint(data);
    return data;
}

void SkPictureRecord::addFlatPaint(const SkFlatData* flatPaint) {
    int index = flatPaint ? flatPaint->index() : 0;
    this->addInt(index);
}

int SkPictureRecord::addPathToHeap(const SkPath& path) {
    if (NULL == fPathHeap) {
        fPathHeap.reset(SkNEW(SkPathHeap));
    }
#ifdef SK_DEDUP_PICTURE_PATHS
    return fPathHeap->insert(path);
#else
    return fPathHeap->append(path);
#endif
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
    if (fWriter.writeBool(rect != NULL)) {
        fWriter.writeRect(*rect);
    }
}

void SkPictureRecord::addIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(rect));
}

void SkPictureRecord::addIRectPtr(const SkIRect* rect) {
    if (fWriter.writeBool(rect != NULL)) {
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


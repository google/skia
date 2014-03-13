/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureRecord.h"
#include "SkTSearch.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkBBoxHierarchy.h"
#include "SkDevice.h"
#include "SkOffsetTable.h"
#include "SkPictureStateTree.h"
#include "SkSurface.h"

#define HEAP_BLOCK_SIZE 4096

enum {
    // just need a value that save or getSaveCount would never return
    kNoInitialSave = -1,
};

// A lot of basic types get stored as a uint32_t: bools, ints, paint indices, etc.
static int const kUInt32Size = 4;

static const uint32_t kSaveSize = 2 * kUInt32Size;
static const uint32_t kSaveLayerNoBoundsSize = 4 * kUInt32Size;
static const uint32_t kSaveLayerWithBoundsSize = 4 * kUInt32Size + sizeof(SkRect);

SkPictureRecord::SkPictureRecord(const SkISize& dimensions, uint32_t flags)
    : INHERITED(dimensions.width(), dimensions.height())
    , fBoundingHierarchy(NULL)
    , fStateTree(NULL)
    , fFlattenableHeap(HEAP_BLOCK_SIZE)
    , fPaints(&fFlattenableHeap)
    , fRecordFlags(flags)
    , fOptsEnabled(true) {
#ifdef SK_DEBUG_SIZE
    fPointBytes = fRectBytes = fTextBytes = 0;
    fPointWrites = fRectWrites = fTextWrites = 0;
#endif

    fBitmapHeap = SkNEW(SkBitmapHeap);
    fFlattenableHeap.setBitmapStorage(fBitmapHeap);
    fPathHeap = NULL;   // lazy allocate

#ifndef SK_COLLAPSE_MATRIX_CLIP_STATE
    fFirstSavedLayerIndex = kNoSavedLayerIndex;
#endif

    fInitialSaveCount = kNoInitialSave;

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.init(this);
#endif
}

SkPictureRecord::~SkPictureRecord() {
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fPathHeap);
    SkSafeUnref(fBoundingHierarchy);
    SkSafeUnref(fStateTree);
    fFlattenableHeap.setBitmapStorage(NULL);
    fPictureRefs.unrefAll();
}

///////////////////////////////////////////////////////////////////////////////

// Return the offset of the paint inside a given op's byte stream. A zero
// return value means there is no paint (and you really shouldn't be calling
// this method)
static inline uint32_t getPaintOffset(DrawType op, uint32_t opSize) {
    // These offsets are where the paint would be if the op size doesn't overflow
    static const uint8_t gPaintOffsets[LAST_DRAWTYPE_ENUM + 1] = {
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

void SkPictureRecord::willSave(SaveFlags flags) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.save(flags);
#else
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.bytesWritten());
    this->recordSave(flags);
#endif

    this->INHERITED::willSave(flags);
}

void SkPictureRecord::recordSave(SaveFlags flags) {
    // op + flags
    uint32_t size = kSaveSize;
    size_t initialOffset = this->addDraw(SAVE, &size);
    this->addInt(flags);

    this->validate(initialOffset, size);
}

SkCanvas::SaveLayerStrategy SkPictureRecord::willSaveLayer(const SkRect* bounds,
                                                           const SkPaint* paint, SaveFlags flags) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.saveLayer(bounds, paint, flags);
#else
    // record the offset to us, making it non-positive to distinguish a save
    // from a clip entry.
    fRestoreOffsetStack.push(-(int32_t)fWriter.bytesWritten());
    this->recordSaveLayer(bounds, paint, flags);
    if (kNoSavedLayerIndex == fFirstSavedLayerIndex) {
        fFirstSavedLayerIndex = fRestoreOffsetStack.count();
    }
#endif

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
    // op + bool for 'bounds'
    uint32_t size = 2 * kUInt32Size;
    if (NULL != bounds) {
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
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    return fMCMgr.isDrawingToLayer();
#else
    return fFirstSavedLayerIndex != kNoSavedLayerIndex;
#endif
}

/*
 * Read the op code from 'offset' in 'writer'.
 */
#ifdef SK_DEBUG
static DrawType peek_op(SkWriter32* writer, int32_t offset) {
    return (DrawType)(writer->readTAt<uint32_t>(offset) >> 24);
}
#endif

/*
 * Read the op code from 'offset' in 'writer' and extract the size too.
 */
static DrawType peek_op_and_size(SkWriter32* writer, int32_t offset, uint32_t* size) {
    uint32_t peek = writer->readTAt<uint32_t>(offset);

    uint32_t op;
    UNPACK_8_24(peek, op, *size);
    if (MASK_24 == *size) {
        // size required its own slot right after the op code
        *size = writer->readTAt<uint32_t>(offset + kUInt32Size);
    }
    return (DrawType) op;
}

#ifdef TRACK_COLLAPSE_STATS
    static int gCollapseCount, gCollapseCalls;
#endif

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
        while (NOOP == op && curOffset < writer->bytesWritten()) {
            curOffset += curSize;
            op = peek_op_and_size(writer, curOffset, &curSize);
        }

        if (curOffset >= writer->bytesWritten()) {
            return false; // ran out of byte stream
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

    curOffset += curSize;
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

    uint32_t dbmPaintOffset = getPaintOffset(dbmInfo.fActualOp, dbmInfo.fSize);
    uint32_t slPaintOffset = getPaintOffset(SAVE_LAYER, saveLayerInfo.fSize);

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
    if (NULL == dbmPaint.get() || dbmPaint->getColor() != layerColor) {
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
    return (op > CONCAT && op < ROTATE) || DRAW_DRRECT == op;
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
#ifdef TRACK_COLLAPSE_STATS
    gCollapseCalls += 1;
#endif

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

    // get the save flag (last 4-bytes of the space allocated for the opSize)
    SkCanvas::SaveFlags saveFlags = (SkCanvas::SaveFlags) writer->readTAt<uint32_t>(offset + 4);
    if (SkCanvas::kMatrixClip_SaveFlag != saveFlags) {
        // This function's optimization is only correct for kMatrixClip style saves.
        // TODO: set checkMatrix & checkClip booleans here and then check for the
        // offending operations in the following loop.
        return false;
    }

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

#ifdef TRACK_COLLAPSE_STATS
    gCollapseCount += 1;
    SkDebugf("Collapse [%d out of %d] %g%spn", gCollapseCount, gCollapseCalls,
             (double)gCollapseCount / gCollapseCalls, "%");
#endif

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
    kSkipIfBBoxHierarchy_Flag = 0x1,  // Optimization should be skipped if the
                                      // SkPicture has a bounding box hierarchy.
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
    { collapse_save_clip_restore, kRewind_OptType, kSkipIfBBoxHierarchy_Flag },
    { remove_save_layer1,         kCollapseSaveLayer_OptType, 0 },
    { remove_save_layer2,         kCollapseSaveLayer_OptType, 0 }
};

// This is called after an optimization has been applied to the command stream
// in order to adjust the contents and state of the bounding box hierarchy and
// state tree to reflect the optimization.
static void apply_optimization_to_bbh(PictureRecordOptType opt, SkPictureStateTree* stateTree,
                                      SkBBoxHierarchy* boundingHierarchy) {
    switch (opt) {
    case kCollapseSaveLayer_OptType:
        if (NULL != stateTree) {
            stateTree->saveCollapsed();
        }
        break;
    case kRewind_OptType:
        if (NULL != boundingHierarchy) {
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

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    if (fMCMgr.getSaveCount() == 1) {
        return;
    }

    fMCMgr.restore();
#else
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
                && NULL != fBoundingHierarchy) {
                continue;
            }
            if ((*gPictureRecordOpts[opt].fProc)(&fWriter, fRestoreOffsetStack.top(), &fPaints)) {
                // Some optimization fired so don't add the RESTORE
                apply_optimization_to_bbh(gPictureRecordOpts[opt].fType,
                                          fStateTree, fBoundingHierarchy);
                break;
            }
        }
    }

    if (!fOptsEnabled || SK_ARRAY_COUNT(gPictureRecordOpts) == opt) {
        // No optimization fired so add the RESTORE
        this->recordRestore();
    }

    fRestoreOffsetStack.pop();
#endif

    this->INHERITED::willRestore();
}

void SkPictureRecord::recordRestore(bool fillInSkips) {
    uint32_t initialOffset, size;
    if (fillInSkips) {
        this->fillRestoreOffsetPlaceholdersForCurrentStackLevel((uint32_t)fWriter.bytesWritten());
    }
    size = 1 * kUInt32Size; // RESTORE consists solely of 1 op code
    initialOffset = this->addDraw(RESTORE, &size);
    this->validate(initialOffset, size);
}

void SkPictureRecord::didTranslate(SkScalar dx, SkScalar dy) {
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.translate(dx, dy);
#else
    // op + dx + dy
    uint32_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(TRANSLATE, &size);
    this->addScalar(dx);
    this->addScalar(dy);
    this->validate(initialOffset, size);
#endif
    this->INHERITED::didTranslate(dx, dy);
}

void SkPictureRecord::didScale(SkScalar sx, SkScalar sy) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.scale(sx, sy);
#else
    // op + sx + sy
    uint32_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(SCALE, &size);
    this->addScalar(sx);
    this->addScalar(sy);
    this->validate(initialOffset, size);
#endif
    this->INHERITED::didScale(sx, sy);
}

void SkPictureRecord::didRotate(SkScalar degrees) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.rotate(degrees);
#else
    // op + degrees
    uint32_t size = 1 * kUInt32Size + sizeof(SkScalar);
    size_t initialOffset = this->addDraw(ROTATE, &size);
    this->addScalar(degrees);
    this->validate(initialOffset, size);
#endif
    this->INHERITED::didRotate(degrees);
}

void SkPictureRecord::didSkew(SkScalar sx, SkScalar sy) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.skew(sx, sy);
#else
    // op + sx + sy
    uint32_t size = 1 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(SKEW, &size);
    this->addScalar(sx);
    this->addScalar(sy);
    this->validate(initialOffset, size);
#endif
    this->INHERITED::didSkew(sx, sy);
}

void SkPictureRecord::didConcat(const SkMatrix& matrix) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.concat(matrix);
#else
    this->recordConcat(matrix);
#endif
    this->INHERITED::didConcat(matrix);
}

void SkPictureRecord::recordConcat(const SkMatrix& matrix) {
    this->validate(fWriter.bytesWritten(), 0);
    // op + matrix
    uint32_t size = kUInt32Size + matrix.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(CONCAT, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
}

void SkPictureRecord::didSetMatrix(const SkMatrix& matrix) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.setMatrix(matrix);
#else
    this->validate(fWriter.bytesWritten(), 0);
    // op + matrix
    uint32_t size = kUInt32Size + matrix.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(SET_MATRIX, &size);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
#endif
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

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
void SkPictureRecord::fillRestoreOffsetPlaceholdersForCurrentStackLevel(uint32_t restoreOffset) {
    fMCMgr.fillInSkips(&fWriter, restoreOffset);
}
#else
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
#endif

void SkPictureRecord::beginRecording() {
    // we have to call this *after* our constructor, to ensure that it gets
    // recorded. This is balanced by restoreToCount() call from endRecording,
    // which in-turn calls our overridden restore(), so those get recorded too.
    fInitialSaveCount = this->save(kMatrixClip_SaveFlag);
}

void SkPictureRecord::endRecording() {
    SkASSERT(kNoInitialSave != fInitialSaveCount);
    this->restoreToCount(fInitialSaveCount);
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.finish();
#endif
}

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
int SkPictureRecord::recordRestoreOffsetPlaceholder(SkRegion::Op op) {
    size_t offset = fWriter.bytesWritten();
    this->addInt(-1);
    return offset;
}
#else
int SkPictureRecord::recordRestoreOffsetPlaceholder(SkRegion::Op op) {
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
    fRestoreOffsetStack.top() = offset;
    return offset;
}
#endif

void SkPictureRecord::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.clipRect(rect, op, doAA);
#else
    this->recordClipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
#endif
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

int SkPictureRecord::recordClipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    // id + rect + clip params
    uint32_t size = 1 * kUInt32Size + sizeof(rect) + 1 * kUInt32Size;
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    size += kUInt32Size;    // + restore offset
#else
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
#endif
    size_t initialOffset = this->addDraw(CLIP_RECT, &size);
    this->addRect(rect);
    this->addInt(ClipParams_pack(op, doAA));
    int offset = this->recordRestoreOffsetPlaceholder(op);

    this->validate(initialOffset, size);
    return offset;
}

void SkPictureRecord::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.clipRRect(rrect, op, doAA);
#else
    this->recordClipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
#endif
    if (fRecordFlags & SkPicture::kUsePathBoundsForClip_RecordingFlag) {
        this->updateClipConservativelyUsingBounds(rrect.getBounds(), op, false);
    } else {
        this->INHERITED::onClipRRect(rrect, op, edgeStyle);
    }
}

int SkPictureRecord::recordClipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    // op + rrect + clip params
    uint32_t size = 1 * kUInt32Size + SkRRect::kSizeInMemory + 1 * kUInt32Size;
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    size += kUInt32Size;    // + restore offset
#else
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
#endif
    size_t initialOffset = this->addDraw(CLIP_RRECT, &size);
    this->addRRect(rrect);
    this->addInt(ClipParams_pack(op, doAA));
    int offset = recordRestoreOffsetPlaceholder(op);
    this->validate(initialOffset, size);
    return offset;
}

void SkPictureRecord::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.clipPath(path, op, doAA);
#else
    int pathID = this->addPathToHeap(path);
    this->recordClipPath(pathID, op, kSoft_ClipEdgeStyle == edgeStyle);
#endif

    if (fRecordFlags & SkPicture::kUsePathBoundsForClip_RecordingFlag) {
        this->updateClipConservativelyUsingBounds(path.getBounds(), op,
                                                  path.isInverseFillType());
    } else {
        this->INHERITED::onClipPath(path, op, edgeStyle);
    }
}

int SkPictureRecord::recordClipPath(int pathID, SkRegion::Op op, bool doAA) {
    // op + path index + clip params
    uint32_t size = 3 * kUInt32Size;
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    size += kUInt32Size;    // + restore offset
#else
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
#endif
    size_t initialOffset = this->addDraw(CLIP_PATH, &size);
    this->addInt(pathID);
    this->addInt(ClipParams_pack(op, doAA));
    int offset = recordRestoreOffsetPlaceholder(op);
    this->validate(initialOffset, size);
    return offset;
}

void SkPictureRecord::onClipRegion(const SkRegion& region, SkRegion::Op op) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.clipRegion(region, op);
#else
    this->recordClipRegion(region, op);
#endif
    this->INHERITED::onClipRegion(region, op);
}

int SkPictureRecord::recordClipRegion(const SkRegion& region, SkRegion::Op op) {
    // op + clip params + region
    uint32_t size = 2 * kUInt32Size + region.writeToMemory(NULL);
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    size += kUInt32Size;    // + restore offset
#else
    // recordRestoreOffsetPlaceholder doesn't always write an offset
    if (!fRestoreOffsetStack.isEmpty()) {
        // + restore offset
        size += kUInt32Size;
    }
#endif
    size_t initialOffset = this->addDraw(CLIP_REGION, &size);
    this->addRegion(region);
    this->addInt(ClipParams_pack(op, false));
    int offset = this->recordRestoreOffsetPlaceholder(op);

    this->validate(initialOffset, size);
    return offset;
}

void SkPictureRecord::clear(SkColor color) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + color
    uint32_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_CLEAR, &size);
    this->addInt(color);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPaint(const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index
    uint32_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PAINT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_PAINT, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                 const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + mode + count + point data
    uint32_t size = 4 * kUInt32Size + count * sizeof(SkPoint);
    size_t initialOffset = this->addDraw(DRAW_POINTS, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_POINTS, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addInt(mode);
    this->addInt(count);
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawOval(const SkRect& oval, const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + rect
    uint32_t size = 2 * kUInt32Size + sizeof(oval);
    size_t initialOffset = this->addDraw(DRAW_OVAL, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_OVAL, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addRect(oval);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawRect(const SkRect& rect, const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + rect
    uint32_t size = 2 * kUInt32Size + sizeof(rect);
    size_t initialOffset = this->addDraw(DRAW_RECT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_RECT, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addRect(rect);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawRRect(const SkRRect& rrect, const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    if (rrect.isRect()) {
        this->SkPictureRecord::drawRect(rrect.getBounds(), paint);
    } else if (rrect.isOval()) {
        this->SkPictureRecord::drawOval(rrect.getBounds(), paint);
    } else {
        // op + paint index + rrect
        uint32_t initialOffset, size;
        size = 2 * kUInt32Size + SkRRect::kSizeInMemory;
        initialOffset = this->addDraw(DRAW_RRECT, &size);
        SkASSERT(initialOffset+getPaintOffset(DRAW_RRECT, size) == fWriter.bytesWritten());
        this->addPaint(paint);
        this->addRRect(rrect);
        this->validate(initialOffset, size);
    }
}

void SkPictureRecord::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                   const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + rrects
    uint32_t initialOffset, size;
    size = 2 * kUInt32Size + SkRRect::kSizeInMemory * 2;
    initialOffset = this->addDraw(DRAW_DRRECT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_DRRECT, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addRRect(outer);
    this->addRRect(inner);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPath(const SkPath& path, const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + path index
    uint32_t size = 3 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PATH, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_PATH, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addPath(path);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                 const SkPaint* paint = NULL) {
    if (bitmap.drawsNothing()) {
        return;
    }

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + bitmap index + left + top
    uint32_t size = 3 * kUInt32Size + 2 * sizeof(SkScalar);
    size_t initialOffset = this->addDraw(DRAW_BITMAP, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    int bitmapID = this->addBitmap(bitmap);
    this->addScalar(left);
    this->addScalar(top);
    this->validate(initialOffset, size);
    this->trackBitmapUse(bitmapID, initialOffset);
}

void SkPictureRecord::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                           const SkRect& dst, const SkPaint* paint,
                                           DrawBitmapRectFlags flags) {
    if (bitmap.drawsNothing()) {
        return;
    }

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif
    // id + paint index + bitmap index + bool for 'src' + flags
    uint32_t size = 5 * kUInt32Size;
    if (NULL != src) {
        size += sizeof(*src);   // + rect
    }
    size += sizeof(dst);        // + rect

    size_t initialOffset = this->addDraw(DRAW_BITMAP_RECT_TO_RECT, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP_RECT_TO_RECT, size)
             == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    int bitmapID = this->addBitmap(bitmap);
    this->addRectPtr(src);  // may be null
    this->addRect(dst);
    this->addInt(flags);
    this->validate(initialOffset, size);
    this->trackBitmapUse(bitmapID, initialOffset);
}

void SkPictureRecord::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                                       const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // id + paint index + bitmap index + matrix
    uint32_t size = 3 * kUInt32Size + matrix.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(DRAW_BITMAP_MATRIX, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP_MATRIX, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    int bitmapID = this->addBitmap(bitmap);
    this->addMatrix(matrix);
    this->validate(initialOffset, size);
    this->trackBitmapUse(bitmapID, initialOffset);
}

void SkPictureRecord::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                     const SkRect& dst, const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + bitmap id + center + dst rect
    uint32_t size = 3 * kUInt32Size + sizeof(center) + sizeof(dst);
    size_t initialOffset = this->addDraw(DRAW_BITMAP_NINE, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_BITMAP_NINE, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    int bitmapID = this->addBitmap(bitmap);
    this->addIRect(center);
    this->addRect(dst);
    this->validate(initialOffset, size);
    this->trackBitmapUse(bitmapID, initialOffset);
}

void SkPictureRecord::drawSprite(const SkBitmap& bitmap, int left, int top,
                                 const SkPaint* paint = NULL) {
    if (bitmap.drawsNothing()) {
        return;
    }

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + bitmap index + left + top
    uint32_t size = 5 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_SPRITE, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_SPRITE, size) == fWriter.bytesWritten());
    this->addPaintPtr(paint);
    int bitmapID = this->addBitmap(bitmap);
    this->addInt(left);
    this->addInt(top);
    this->validate(initialOffset, size);
    this->trackBitmapUse(bitmapID, initialOffset);
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

void SkPictureRecord::drawText(const void* text, size_t byteLength, SkScalar x,
                      SkScalar y, const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    bool fast = !paint.isVerticalText() && paint.canComputeFastBounds();

    // op + paint index + length + 'length' worth of chars + x + y
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 2 * sizeof(SkScalar);
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

void SkPictureRecord::drawPosText(const void* text, size_t byteLength,
                         const SkPoint pos[], const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    size_t points = paint.countText(text, byteLength);
    if (0 == points)
        return;

    bool canUseDrawH = true;
    SkScalar minY = pos[0].fY;
    SkScalar maxY = pos[0].fY;
    // check if the caller really should have used drawPosTextH()
    {
        const SkScalar firstY = pos[0].fY;
        for (size_t index = 1; index < points; index++) {
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

    bool fastBounds = !paint.isVerticalText() && paint.canComputeFastBounds();
    bool fast = canUseDrawH && fastBounds;

    // op + paint index + length + 'length' worth of data + num points
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
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

#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.bytesWritten();
#endif
    if (canUseDrawH) {
        if (fast) {
            this->addFontMetricsTopBottom(paint, *flatPaintData, pos[0].fY, pos[0].fY);
        }
        this->addScalar(pos[0].fY);
        SkScalar* xptr = (SkScalar*)fWriter.reserve(points * sizeof(SkScalar));
        for (size_t index = 0; index < points; index++)
            *xptr++ = pos[index].fX;
    } else {
        fWriter.writeMul4(pos, points * sizeof(SkPoint));
        if (fastBounds) {
            this->addFontMetricsTopBottom(paint, *flatPaintData, minY, maxY);
        }
    }
#ifdef SK_DEBUG_SIZE
    fPointBytes += fWriter.bytesWritten() - start;
    fPointWrites += points;
#endif
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPosTextH(const void* text, size_t byteLength,
                          const SkScalar xpos[], SkScalar constY,
                          const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    const SkFlatData* flatPaintData = this->getFlatPaintData(paint);
    this->drawPosTextHImpl(text, byteLength, xpos, constY, paint, flatPaintData);
}

void SkPictureRecord::drawPosTextHImpl(const void* text, size_t byteLength,
                          const SkScalar xpos[], SkScalar constY,
                          const SkPaint& paint, const SkFlatData* flatPaintData) {
    size_t points = paint.countText(text, byteLength);
    if (0 == points)
        return;

    bool fast = !paint.isVerticalText() && paint.canComputeFastBounds();

    // op + paint index + length + 'length' worth of data + num points
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + 1 * kUInt32Size;
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

#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.bytesWritten();
#endif
    if (fast) {
        this->addFontMetricsTopBottom(paint, *flatPaintData, constY, constY);
    }
    this->addScalar(constY);
    fWriter.writeMul4(xpos, points * sizeof(SkScalar));
#ifdef SK_DEBUG_SIZE
    fPointBytes += fWriter.bytesWritten() - start;
    fPointWrites += points;
#endif
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawTextOnPath(const void* text, size_t byteLength,
                            const SkPath& path, const SkMatrix* matrix,
                            const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + paint index + length + 'length' worth of data + path index + matrix
    const SkMatrix& m = matrix ? *matrix : SkMatrix::I();
    uint32_t size = 3 * kUInt32Size + SkAlign4(byteLength) + kUInt32Size + m.writeToMemory(NULL);
    size_t initialOffset = this->addDraw(DRAW_TEXT_ON_PATH, &size);
    SkASSERT(initialOffset+getPaintOffset(DRAW_TEXT_ON_PATH, size) == fWriter.bytesWritten());
    this->addPaint(paint);
    this->addText(text, byteLength);
    this->addPath(path);
    this->addMatrix(m);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawPicture(SkPicture& picture) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + picture index
    uint32_t size = 2 * kUInt32Size;
    size_t initialOffset = this->addDraw(DRAW_PICTURE, &size);
    this->addPicture(picture);
    this->validate(initialOffset, size);
}

void SkPictureRecord::drawVertices(VertexMode vmode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode* xfer,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

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
    if (NULL != xfer) {
        SkXfermode::Mode mode;
        if (xfer->asMode(&mode) && SkXfermode::kModulate_Mode != mode) {
            flags |= DRAW_VERTICES_HAS_XFER;
        }
    }

    // op + paint index + flags + vmode + vCount + vertices
    uint32_t size = 5 * kUInt32Size + vertexCount * sizeof(SkPoint);
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

void SkPictureRecord::drawData(const void* data, size_t length) {

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    fMCMgr.call(SkMatrixClipStateMgr::kOther_CallType);
#endif

    // op + length + 'length' worth of data
    uint32_t size = 2 * kUInt32Size + SkAlign4(length);
    size_t initialOffset = this->addDraw(DRAW_DATA, &size);
    this->addInt(length);
    fWriter.writePad(data, length);
    this->validate(initialOffset, size);
}

void SkPictureRecord::beginCommentGroup(const char* description) {
    // op/size + length of string + \0 terminated chars
    int length = strlen(description);
    uint32_t size = 2 * kUInt32Size + SkAlign4(length + 1);
    size_t initialOffset = this->addDraw(BEGIN_COMMENT_GROUP, &size);
    fWriter.writeString(description, length);
    this->validate(initialOffset, size);
}

void SkPictureRecord::addComment(const char* kywd, const char* value) {
    // op/size + 2x length of string + 2x \0 terminated chars
    int kywdLen = strlen(kywd);
    int valueLen = strlen(value);
    uint32_t size = 3 * kUInt32Size + SkAlign4(kywdLen + 1) + SkAlign4(valueLen + 1);
    size_t initialOffset = this->addDraw(COMMENT, &size);
    fWriter.writeString(kywd, kywdLen);
    fWriter.writeString(value, valueLen);
    this->validate(initialOffset, size);
}

void SkPictureRecord::endCommentGroup() {
    // op/size
    uint32_t size = 1 * kUInt32Size;
    size_t initialOffset = this->addDraw(END_COMMENT_GROUP, &size);
    this->validate(initialOffset, size);
}

// [op/size] [rect] [skip offset]
static const uint32_t kPushCullOpSize = 2 * kUInt32Size + sizeof(SkRect);
void SkPictureRecord::onPushCull(const SkRect& cullRect) {
    // Skip identical cull rects.
    if (!fCullOffsetStack.isEmpty()) {
        const SkRect& prevCull = fWriter.readTAt<SkRect>(fCullOffsetStack.top() - sizeof(SkRect));
        if (prevCull == cullRect) {
            // Skipped culls are tracked on the stack, but they point to the previous offset.
            fCullOffsetStack.push(fCullOffsetStack.top());
            return;
        }

        SkASSERT(prevCull.contains(cullRect));
    }

    uint32_t size = kPushCullOpSize;
    size_t initialOffset = this->addDraw(PUSH_CULL, &size);
    // PUSH_CULL's size should stay constant (used to rewind).
    SkASSERT(size == kPushCullOpSize);

    this->addRect(cullRect);
    fCullOffsetStack.push(fWriter.bytesWritten());
    this->addInt(0);
    this->validate(initialOffset, size);
}

void SkPictureRecord::onPopCull() {
    SkASSERT(!fCullOffsetStack.isEmpty());

    uint32_t cullSkipOffset = fCullOffsetStack.top();
    fCullOffsetStack.pop();

    // Skipped push, do the same for pop.
    if (!fCullOffsetStack.isEmpty() && cullSkipOffset == fCullOffsetStack.top()) {
        return;
    }

    // Collapse empty push/pop pairs.
    if ((size_t)(cullSkipOffset + kUInt32Size) == fWriter.bytesWritten()) {
        SkASSERT(fWriter.bytesWritten() >= kPushCullOpSize);
        SkASSERT(PUSH_CULL == peek_op(&fWriter, fWriter.bytesWritten() - kPushCullOpSize));
        fWriter.rewindToOffset(fWriter.bytesWritten() - kPushCullOpSize);
        return;
    }

    // op only
    uint32_t size = kUInt32Size;
    size_t initialOffset = this->addDraw(POP_CULL, &size);

    // update the cull skip offset to point past this op.
    fWriter.overwriteTAt<uint32_t>(cullSkipOffset, fWriter.bytesWritten());

    this->validate(initialOffset, size);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkPictureRecord::onNewSurface(const SkImageInfo& info) {
    return SkSurface::NewPicture(info.fWidth, info.fHeight);
}

void SkPictureRecord::trackBitmapUse(int bitmapID, size_t offset) {
#ifndef SK_ALLOW_BITMAP_TRACKING
    return;
#endif

    if (!(fRecordFlags & SkPicture::kOptimizeForClippedPlayback_RecordingFlag)) {
        return;
    }

    if (SkBitmapHeap::INVALID_SLOT == bitmapID) {
        return;
    }

    if (NULL == fBitmapUseOffsets) {
        fBitmapUseOffsets.reset(SkNEW(SkOffsetTable));
    }

    fBitmapUseOffsets->add(bitmapID, offset);
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
        fPathHeap = SkNEW(SkPathHeap);
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

void SkPictureRecord::addPicture(SkPicture& picture) {
    int index = fPictureRefs.find(&picture);
    if (index < 0) {    // not found
        index = fPictureRefs.count();
        *fPictureRefs.append() = &picture;
        picture.ref();
    }
    // follow the convention of recording a 1-based index
    this->addInt(index + 1);
}

void SkPictureRecord::addPoint(const SkPoint& point) {
#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.bytesWritten();
#endif
    fWriter.writePoint(point);
#ifdef SK_DEBUG_SIZE
    fPointBytes += fWriter.bytesWritten() - start;
    fPointWrites++;
#endif
}

void SkPictureRecord::addPoints(const SkPoint pts[], int count) {
    fWriter.writeMul4(pts, count * sizeof(SkPoint));
#ifdef SK_DEBUG_SIZE
    fPointBytes += count * sizeof(SkPoint);
    fPointWrites++;
#endif
}

void SkPictureRecord::addRect(const SkRect& rect) {
#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.bytesWritten();
#endif
    fWriter.writeRect(rect);
#ifdef SK_DEBUG_SIZE
    fRectBytes += fWriter.bytesWritten() - start;
    fRectWrites++;
#endif
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
#ifdef SK_DEBUG_SIZE
    size_t start = fWriter.bytesWritten();
#endif
    addInt(byteLength);
    fWriter.writePad(text, byteLength);
#ifdef SK_DEBUG_SIZE
    fTextBytes += fWriter.bytesWritten() - start;
    fTextWrites++;
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG_SIZE
size_t SkPictureRecord::size() const {
    size_t result = 0;
    size_t sizeData;
    bitmaps(&sizeData);
    result += sizeData;
    matrices(&sizeData);
    result += sizeData;
    paints(&sizeData);
    result += sizeData;
    paths(&sizeData);
    result += sizeData;
    pictures(&sizeData);
    result += sizeData;
    regions(&sizeData);
    result += sizeData;
    result += streamlen();
    return result;
}

int SkPictureRecord::bitmaps(size_t* size) const {
    size_t result = 0;
    int count = fBitmaps.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fBitmaps[index]) + fBitmaps[index]->size();
    *size = result;
    return count;
}

int SkPictureRecord::matrices(size_t* size) const {
    int count = fMatrices.count();
    *size = sizeof(fMatrices[0]) * count;
    return count;
}

int SkPictureRecord::paints(size_t* size) const {
    size_t result = 0;
    int count = fPaints.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fPaints[index]) + fPaints[index]->size();
    *size = result;
    return count;
}

int SkPictureRecord::paths(size_t* size) const {
    size_t result = 0;
    int count = fPaths.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fPaths[index]) + fPaths[index]->size();
    *size = result;
    return count;
}

int SkPictureRecord::regions(size_t* size) const {
    size_t result = 0;
    int count = fRegions.count();
    for (int index = 0; index < count; index++)
        result += sizeof(fRegions[index]) + fRegions[index]->size();
    *size = result;
    return count;
}

size_t SkPictureRecord::streamlen() const {
    return fWriter.size();
}
#endif

#ifdef SK_DEBUG_VALIDATE
void SkPictureRecord::validate(uint32_t initialOffset, uint32_t size) const {
    SkASSERT(fWriter.size() == initialOffset + size);

    validateBitmaps();
    validateMatrices();
    validatePaints();
    validatePaths();
    validateRegions();
}

void SkPictureRecord::validateBitmaps() const {
    int count = fBitmapHeap->count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkBitmap* bitPtr = fBitmapHeap->getBitmap(index);
        SkASSERT(bitPtr);
        bitPtr->validate();
    }
}

void SkPictureRecord::validateMatrices() const {
    int count = fMatrices.count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkFlatData* matrix = fMatrices[index];
        SkASSERT(matrix);
//        matrix->validate();
    }
}

void SkPictureRecord::validatePaints() const {
    int count = fPaints.count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkFlatData* paint = fPaints[index];
        SkASSERT(paint);
//            paint->validate();
    }
}

void SkPictureRecord::validatePaths() const {
    if (NULL == fPathHeap) {
        return;
    }

    int count = fPathHeap->count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkPath& path = (*fPathHeap)[index];
        path.validate();
    }
}

void SkPictureRecord::validateRegions() const {
    int count = fRegions.count();
    SkASSERT((unsigned) count < 0x1000);
    for (int index = 0; index < count; index++) {
        const SkFlatData* region = fRegions[index];
        SkASSERT(region);
//        region->validate();
    }
}
#endif

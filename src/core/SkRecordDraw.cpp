/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordDraw.h"
#include "SkTSort.h"

void SkRecordDraw(const SkRecord& record,
                  SkCanvas* canvas,
                  const SkBBoxHierarchy* bbh,
                  SkDrawPictureCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    if (NULL != bbh) {
        // Draw only ops that affect pixels in the canvas's current clip.
        SkIRect query;
#if 1   // TODO: Why is this the right way to make the query?  I'd think it'd be the else branch.
        SkRect clipBounds;
        canvas->getClipBounds(&clipBounds);
        clipBounds.roundOut(&query);
#else
        canvas->getClipDeviceBounds(&query);
#endif
        SkTDArray<void*> ops;
        bbh->search(query, &ops);

        SkRecords::Draw draw(canvas);
        for (int i = 0; i < ops.count(); i++) {
            if (NULL != callback && callback->abortDrawing()) {
                return;
            }
            record.visit<void>((uintptr_t)ops[i], draw);  // See FillBounds below.
        }
    } else {
        // Draw all ops.
        for (SkRecords::Draw draw(canvas); draw.index() < record.count(); draw.next()) {
            if (NULL != callback && callback->abortDrawing()) {
                return;
            }
            record.visit<void>(draw.index(), draw);
        }
    }
}

namespace SkRecords {

// FIXME: SkBitmaps are stateful, so we need to copy them to play back in multiple threads.
static SkBitmap shallow_copy(const SkBitmap& bitmap) {
    return bitmap;
}

// NoOps draw nothing.
template <> void Draw::draw(const NoOp&) {}

#define DRAW(T, call) template <> void Draw::draw(const T& r) { fCanvas->call; }
DRAW(Restore, restore());
DRAW(Save, save());
DRAW(SaveLayer, saveLayer(r.bounds, r.paint, r.flags));
DRAW(PopCull, popCull());
DRAW(PushCull, pushCull(r.rect));
DRAW(Clear, clear(r.color));
DRAW(Concat, concat(r.matrix));
DRAW(SetMatrix, setMatrix(SkMatrix::Concat(fInitialCTM, r.matrix)));

DRAW(ClipPath, clipPath(r.path, r.op, r.doAA));
DRAW(ClipRRect, clipRRect(r.rrect, r.op, r.doAA));
DRAW(ClipRect, clipRect(r.rect, r.op, r.doAA));
DRAW(ClipRegion, clipRegion(r.region, r.op));

DRAW(DrawBitmap, drawBitmap(shallow_copy(r.bitmap), r.left, r.top, r.paint));
DRAW(DrawBitmapMatrix, drawBitmapMatrix(shallow_copy(r.bitmap), r.matrix, r.paint));
DRAW(DrawBitmapNine, drawBitmapNine(shallow_copy(r.bitmap), r.center, r.dst, r.paint));
DRAW(DrawBitmapRectToRect,
        drawBitmapRectToRect(shallow_copy(r.bitmap), r.src, r.dst, r.paint, r.flags));
DRAW(DrawDRRect, drawDRRect(r.outer, r.inner, r.paint));
DRAW(DrawOval, drawOval(r.oval, r.paint));
DRAW(DrawPaint, drawPaint(r.paint));
DRAW(DrawPath, drawPath(r.path, r.paint));
DRAW(DrawPatch, drawPatch(r.cubics, r.colors, r.texCoords, r.xmode.get(), r.paint));
DRAW(DrawPicture, drawPicture(r.picture, r.matrix, r.paint));
DRAW(DrawPoints, drawPoints(r.mode, r.count, r.pts, r.paint));
DRAW(DrawPosText, drawPosText(r.text, r.byteLength, r.pos, r.paint));
DRAW(DrawPosTextH, drawPosTextH(r.text, r.byteLength, r.xpos, r.y, r.paint));
DRAW(DrawRRect, drawRRect(r.rrect, r.paint));
DRAW(DrawRect, drawRect(r.rect, r.paint));
DRAW(DrawSprite, drawSprite(shallow_copy(r.bitmap), r.left, r.top, r.paint));
DRAW(DrawText, drawText(r.text, r.byteLength, r.x, r.y, r.paint));
DRAW(DrawTextOnPath, drawTextOnPath(r.text, r.byteLength, r.path, r.matrix, r.paint));
DRAW(DrawVertices, drawVertices(r.vmode, r.vertexCount, r.vertices, r.texs, r.colors,
                                r.xmode.get(), r.indices, r.indexCount, r.paint));
#undef DRAW


// This is an SkRecord visitor that fills an SkBBoxHierarchy.
//
// The interesting part here is how to calculate bounds for ops which don't
// have intrinsic bounds.  What is the bounds of a Save or a Translate?
//
// We answer this by thinking about a particular definition of bounds: if I
// don't execute this op, pixels in this rectangle might draw incorrectly.  So
// the bounds of a Save, a Translate, a Restore, etc. are the union of the
// bounds of Draw* ops that they might have an effect on.  For any given
// Save/Restore block, the bounds of the Save, the Restore, and any other
// non-drawing ("control") ops inside are exactly the union of the bounds of
// the drawing ops inside that block.
//
// To implement this, we keep a stack of active Save blocks.  As we consume ops
// inside the Save/Restore block, drawing ops are unioned with the bounds of
// the block, and control ops are stashed away for later.  When we finish the
// block with a Restore, our bounds are complete, and we go back and fill them
// in for all the control ops we stashed away.
class FillBounds : SkNoncopyable {
public:
    FillBounds(const SkRecord& record, SkBBoxHierarchy* bbh) : fBounds(record.count()) {
        // Calculate bounds for all ops.  This won't go quite in order, so we'll need
        // to store the bounds separately then feed them in to the BBH later in order.
        const SkIRect largest = SkIRect::MakeLargest();
        fCTM.setIdentity();
        fCurrentClipBounds = largest;
        for (fCurrentOp = 0; fCurrentOp < record.count(); fCurrentOp++) {
            record.visit<void>(fCurrentOp, *this);
        }

        // If we have any lingering unpaired Saves, simulate restores to make
        // sure all ops in those Save blocks have their bounds calculated.
        while (!fSaveStack.isEmpty()) {
            this->popSaveBlock();
        }

        // Any control ops not part of any Save/Restore block draw everywhere.
        while (!fControlIndices.isEmpty()) {
            this->popControl(largest);
        }

        // Finally feed all stored bounds into the BBH.  They'll be returned in this order.
        SkASSERT(NULL != bbh);
        for (uintptr_t i = 0; i < record.count(); i++) {
            if (!fBounds[i].isEmpty()) {
                bbh->insert((void*)i, fBounds[i], true/*ok to defer*/);
            }
        }
        bbh->flushDeferredInserts();
    }

    template <typename T> void operator()(const T& op) {
        this->updateCTM(op);
        this->updateClipBounds(op);
        this->trackBounds(op);
    }

private:
    struct SaveBounds {
        int controlOps;        // Number of control ops in this Save block, including the Save.
        SkIRect bounds;        // Bounds of everything in the block.
        const SkPaint* paint;  // Unowned.  If set, adjusts the bounds of all ops in this block.
    };

    template <typename T> void updateCTM(const T&) { /* most ops don't change the CTM */ }
    void updateCTM(const Restore& op)   { fCTM = op.matrix; }
    void updateCTM(const SetMatrix& op) { fCTM = op.matrix; }
    void updateCTM(const Concat& op)    { fCTM.preConcat(op.matrix); }

    template <typename T> void updateClipBounds(const T&) { /* most ops don't change the clip */ }
    // Each of these devBounds fields is the state of the device bounds after the op.
    // So Restore's devBounds are those bounds saved by its paired Save or SaveLayer.
    void updateClipBounds(const Restore& op)    { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const ClipPath& op)   { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const ClipRRect& op)  { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const ClipRect& op)   { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const ClipRegion& op) { fCurrentClipBounds = op.devBounds; }
    void updateClipBounds(const SaveLayer& op)  {
        if (op.bounds) {
            fCurrentClipBounds.intersect(this->adjustAndMap(*op.bounds, op.paint));
        }
    }

    // The bounds of these ops must be calculated when we hit the Restore
    // from the bounds of the ops in the same Save block.
    void trackBounds(const Save&)          { this->pushSaveBlock(NULL); }
    // TODO: bounds of SaveLayer may be more complicated?
    void trackBounds(const SaveLayer& op)  { this->pushSaveBlock(op.paint); }
    void trackBounds(const Restore&) { fBounds[fCurrentOp] = this->popSaveBlock(); }

    void trackBounds(const Concat&)     { this->pushControl(); }
    void trackBounds(const SetMatrix&)  { this->pushControl(); }
    void trackBounds(const ClipRect&)   { this->pushControl(); }
    void trackBounds(const ClipRRect&)  { this->pushControl(); }
    void trackBounds(const ClipPath&)   { this->pushControl(); }
    void trackBounds(const ClipRegion&) { this->pushControl(); }

    // For all other ops, we can calculate and store the bounds directly now.
    template <typename T> void trackBounds(const T& op) {
        fBounds[fCurrentOp] = this->bounds(op);
        this->updateSaveBounds(fBounds[fCurrentOp]);
    }

    void pushSaveBlock(const SkPaint* paint) {
        // Starting a new Save block.  Push a new entry to represent that.
        SaveBounds sb = { 0, SkIRect::MakeEmpty(), paint };
        fSaveStack.push(sb);
        this->pushControl();
    }

    SkIRect popSaveBlock() {
        // We're done the Save block.  Apply the block's bounds to all control ops inside it.
        SaveBounds sb;
        fSaveStack.pop(&sb);
        while (sb.controlOps --> 0) {
            this->popControl(sb.bounds);
        }

        // This whole Save block may be part another Save block.
        this->updateSaveBounds(sb.bounds);

        // If called from a real Restore (not a phony one for balance), it'll need the bounds.
        return sb.bounds;
    }

    void pushControl() {
        fControlIndices.push(fCurrentOp);
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().controlOps++;
        }
    }

    void popControl(const SkIRect& bounds) {
        fBounds[fControlIndices.top()] = bounds;
        fControlIndices.pop();
    }

    void updateSaveBounds(const SkIRect& bounds) {
        // If we're in a Save block, expand its bounds to cover these bounds too.
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().bounds.join(bounds);
        }
    }

    // TODO: Remove this default when done bounding all ops.
    template <typename T> SkIRect bounds(const T&) { return fCurrentClipBounds; }
    SkIRect bounds(const Clear&) { return SkIRect::MakeLargest(); }  // Ignores the clip
    SkIRect bounds(const NoOp&)  { return SkIRect::MakeEmpty(); }    // NoOps don't draw anywhere.

    // Adjust rect for all paints that may affect its geometry, then map it to device space.
    SkIRect adjustAndMap(SkRect rect, const SkPaint* paint) {
        // Adjust rect for its own paint.
        if (paint) {
            if (paint->canComputeFastBounds()) {
                rect = paint->computeFastBounds(rect, &rect);
            } else {
                // The paint could do anything.  The only safe answer is the current clip.
                return fCurrentClipBounds;
            }
        }

        // Adjust rect for all the paints from the SaveLayers we're inside.
        // For SaveLayers, only image filters will affect the bounds.
        for (int i = fSaveStack.count() - 1; i >= 0; i--) {
            if (fSaveStack[i].paint && fSaveStack[i].paint->getImageFilter()) {
                if (paint->canComputeFastBounds()) {
                    rect = fSaveStack[i].paint->computeFastBounds(rect, &rect);
                } else {
                    // Same deal as above.
                    return fCurrentClipBounds;
                }
            }
        }

        // Map the rect back to device space.
        fCTM.mapRect(&rect);
        SkIRect devRect;
        rect.roundOut(&devRect);
        return devRect;
    }

    // Conservative device bounds for each op in the SkRecord.
    SkAutoTMalloc<SkIRect> fBounds;

    // We walk fCurrentOp through the SkRecord, as we go using updateCTM()
    // and updateClipBounds() to maintain the exact CTM (fCTM) and conservative
    // device bounds of the current clip (fCurrentClipBounds).
    unsigned fCurrentOp;
    SkMatrix fCTM;
    SkIRect fCurrentClipBounds;

    // Used to track the bounds of Save/Restore blocks and the control ops inside them.
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<unsigned>   fControlIndices;
};

}  // namespace SkRecords

void SkRecordFillBounds(const SkRecord& record, SkBBoxHierarchy* bbh) {
    SkRecords::FillBounds(record, bbh);
}

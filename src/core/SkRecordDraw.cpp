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
        SkIRect devBounds;
        canvas->getClipDeviceBounds(&devBounds);
        SkTDArray<void*> ops;
        bbh->search(devBounds, &ops);

        // FIXME: QuadTree doesn't send these back in the order we inserted them.  :(
        // Also remove the sort in SkPictureData::getActiveOps()?
        if (ops.count() > 0) {
            SkTQSort(ops.begin(), ops.end() - 1, SkTCompareLT<void*>());
        }

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
        fCTM.setIdentity();
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
            this->popControl(SkIRect::MakeLargest());
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

    template <typename T> void operator()(const T& r) {
        this->updateCTM(r);
        this->trackBounds(r);
    }

private:
    struct SaveBounds {
        int controlOps;  // Number of control ops in this Save block, including the Save.
        SkIRect bounds;  // Bounds of everything in the block.
    };

    template <typename T> void updateCTM(const T&) { /* most ops don't change the CTM */ }
    void updateCTM(const Restore& r)   { fCTM = r.matrix; }
    void updateCTM(const SetMatrix& r) { fCTM = r.matrix; }
    void updateCTM(const Concat& r)    { fCTM.preConcat(r.matrix); }

    // The bounds of these ops must be calculated when we hit the Restore
    // from the bounds of the ops in the same Save block.
    void trackBounds(const Save&)       { this->pushSaveBlock(); }
    // TODO: bounds of SaveLayer may be more complicated?
    void trackBounds(const SaveLayer&)  { this->pushSaveBlock(); }
    void trackBounds(const Restore&)    { fBounds[fCurrentOp] = this->popSaveBlock(); }

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

    // TODO: remove this trivially-safe default when done bounding all ops
    template <typename T> SkIRect bounds(const T&) { return SkIRect::MakeLargest(); }

    void pushSaveBlock() {
        // Starting a new Save block.  Push a new entry to represent that.
        SaveBounds sb = { 0, SkIRect::MakeEmpty() };
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

    SkIRect bounds(const NoOp&) { return SkIRect::MakeEmpty(); }  // NoOps don't draw anywhere.

    SkAutoTMalloc<SkIRect> fBounds;  // One for each op in the record.
    SkMatrix fCTM;
    unsigned fCurrentOp;
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<unsigned>   fControlIndices;
};

}  // namespace SkRecords

void SkRecordFillBounds(const SkRecord& record, SkBBoxHierarchy* bbh) {
    SkRecords::FillBounds(record, bbh);
}

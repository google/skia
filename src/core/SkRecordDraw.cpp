/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordDraw.h"
#include "SkPatchUtils.h"

void SkRecordDraw(const SkRecord& record,
                  SkCanvas* canvas,
                  const SkBBoxHierarchy* bbh,
                  SkDrawPictureCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    if (NULL != bbh) {
        // Draw only ops that affect pixels in the canvas's current clip.
        SkIRect query;

        // The SkRecord and BBH were recorded in identity space.  This canvas
        // is not necessarily in that same space.  getClipBounds() returns us
        // this canvas' clip bounds transformed back into identity space, which
        // lets us query the BBH.
        SkRect clipBounds = { 0, 0, 0, 0 };
        (void)canvas->getClipBounds(&clipBounds);
        clipBounds.roundOut(&query);

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
DRAW(SetMatrix, setMatrix(SkMatrix::Concat(fInitialCTM, r.matrix)));

DRAW(ClipPath, clipPath(r.path, r.op, r.doAA));
DRAW(ClipRRect, clipRRect(r.rrect, r.op, r.doAA));
DRAW(ClipRect, clipRect(r.rect, r.op, r.doAA));
DRAW(ClipRegion, clipRegion(r.region, r.op));

DRAW(BeginCommentGroup, beginCommentGroup(r.description));
DRAW(AddComment, addComment(r.key, r.value));
DRAW(EndCommentGroup, endCommentGroup());

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
DRAW(DrawTextBlob, drawTextBlob(r.blob, r.x, r.y, r.paint));
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
        fCTM = &SkMatrix::I();
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
    void updateCTM(const Restore& op)   { fCTM = &op.matrix; }
    void updateCTM(const SetMatrix& op) { fCTM = &op.matrix; }

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
    void trackBounds(const SaveLayer& op)  { this->pushSaveBlock(op.paint); }
    void trackBounds(const Restore&) { fBounds[fCurrentOp] = this->popSaveBlock(); }

    void trackBounds(const SetMatrix&)         { this->pushControl(); }
    void trackBounds(const ClipRect&)          { this->pushControl(); }
    void trackBounds(const ClipRRect&)         { this->pushControl(); }
    void trackBounds(const ClipPath&)          { this->pushControl(); }
    void trackBounds(const ClipRegion&)        { this->pushControl(); }
    void trackBounds(const PushCull&)          { this->pushControl(); }
    void trackBounds(const PopCull&)           { this->pushControl(); }
    void trackBounds(const BeginCommentGroup&) { this->pushControl(); }
    void trackBounds(const AddComment&)        { this->pushControl(); }
    void trackBounds(const EndCommentGroup&)   { this->pushControl(); }

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

    static bool PaintMayAffectTransparentBlack(const SkPaint* paint) {
        // FIXME: this is very conservative
        return paint && (paint->getImageFilter() || paint->getColorFilter());
    }

    SkIRect popSaveBlock() {
        // We're done the Save block.  Apply the block's bounds to all control ops inside it.
        SaveBounds sb;
        fSaveStack.pop(&sb);

        // If the paint affects transparent black, we can't trust any of our calculated bounds.
        const SkIRect& bounds =
            PaintMayAffectTransparentBlack(sb.paint) ? fCurrentClipBounds : sb.bounds;

        while (sb.controlOps --> 0) {
            this->popControl(bounds);
        }

        // This whole Save block may be part another Save block.
        this->updateSaveBounds(bounds);

        // If called from a real Restore (not a phony one for balance), it'll need the bounds.
        return bounds;
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

    // FIXME: this method could use better bounds
    SkIRect bounds(const DrawText&) const { return fCurrentClipBounds; }

    SkIRect bounds(const Clear&) const { return SkIRect::MakeLargest(); }  // Ignores the clip.
    SkIRect bounds(const DrawPaint&) const { return fCurrentClipBounds; }
    SkIRect bounds(const NoOp&)  const { return SkIRect::MakeEmpty(); }    // NoOps don't draw.

    SkIRect bounds(const DrawSprite& op) const {
        const SkBitmap& bm = op.bitmap;
        return SkIRect::MakeXYWH(op.left, op.top, bm.width(), bm.height());  // Ignores the matrix.
    }

    SkIRect bounds(const DrawRect& op) const { return this->adjustAndMap(op.rect, &op.paint); }
    SkIRect bounds(const DrawOval& op) const { return this->adjustAndMap(op.oval, &op.paint); }
    SkIRect bounds(const DrawRRect& op) const {
        return this->adjustAndMap(op.rrect.rect(), &op.paint);
    }
    SkIRect bounds(const DrawDRRect& op) const {
        return this->adjustAndMap(op.outer.rect(), &op.paint);
    }

    SkIRect bounds(const DrawBitmapRectToRect& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    SkIRect bounds(const DrawBitmapNine& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    SkIRect bounds(const DrawBitmap& op) const {
        const SkBitmap& bm = op.bitmap;
        return this->adjustAndMap(SkRect::MakeXYWH(op.left, op.top, bm.width(), bm.height()),
                                  op.paint);
    }
    SkIRect bounds(const DrawBitmapMatrix& op) const {
        const SkBitmap& bm = op.bitmap;
        SkRect dst = SkRect::MakeWH(bm.width(), bm.height());
        op.matrix.mapRect(&dst);
        return this->adjustAndMap(dst, op.paint);
    }

    SkIRect bounds(const DrawPath& op) const {
        return op.path.isInverseFillType() ? fCurrentClipBounds
                                           : this->adjustAndMap(op.path.getBounds(), &op.paint);
    }
    SkIRect bounds(const DrawPoints& op) const {
        SkRect dst;
        dst.set(op.pts, op.count);

        // Pad the bounding box a little to make sure hairline points' bounds aren't empty.
        SkScalar stroke = SkMaxScalar(op.paint.getStrokeWidth(), 0.01f);
        dst.outset(stroke/2, stroke/2);

        return this->adjustAndMap(dst, &op.paint);
    }
    SkIRect bounds(const DrawPatch& op) const {
        SkRect dst;
        dst.set(op.cubics, SkPatchUtils::kNumCtrlPts);
        return this->adjustAndMap(dst, &op.paint);
    }
    SkIRect bounds(const DrawVertices& op) const {
        SkRect dst;
        dst.set(op.vertices, op.vertexCount);
        return this->adjustAndMap(dst, &op.paint);
    }

    SkIRect bounds(const DrawPicture& op) const {
        SkRect dst = SkRect::MakeWH(op.picture->width(), op.picture->height());
        if (op.matrix) {
            op.matrix->mapRect(&dst);
        }
        return this->adjustAndMap(dst, op.paint);
    }

    SkIRect bounds(const DrawPosText& op) const {
        const int N = op.paint.countText(op.text, op.byteLength);
        if (N == 0) {
            return SkIRect::MakeEmpty();
        }

        SkRect dst;
        dst.set(op.pos, op.paint.countText(op.text, N));
        AdjustTextForFontMetrics(&dst, op.paint);
        return this->adjustAndMap(dst, &op.paint);
    }
    SkIRect bounds(const DrawPosTextH& op) const {
        const int N = op.paint.countText(op.text, op.byteLength);
        if (N == 0) {
            return SkIRect::MakeEmpty();
        }

        SkScalar left = op.xpos[0], right = op.xpos[0];
        for (int i = 1; i < N; i++) {
            left  = SkMinScalar(left,  op.xpos[i]);
            right = SkMaxScalar(right, op.xpos[i]);
        }
        SkRect dst = { left, op.y, right, op.y };
        AdjustTextForFontMetrics(&dst, op.paint);
        return this->adjustAndMap(dst, &op.paint);
    }
    SkIRect bounds(const DrawTextOnPath& op) const {
        SkRect dst = op.path.getBounds();

        // Pad all sides by the maximum padding in any direction we'd normally apply.
        SkRect pad = { 0, 0, 0, 0};
        AdjustTextForFontMetrics(&pad, op.paint);

        // That maximum padding happens to always be the right pad today.
        SkASSERT(pad.fLeft == -pad.fRight);
        SkASSERT(pad.fTop  == -pad.fBottom);
        SkASSERT(pad.fRight > pad.fBottom);
        dst.outset(pad.fRight, pad.fRight);

        return this->adjustAndMap(dst, &op.paint);
    }

    SkIRect bounds(const DrawTextBlob& op) const {
        SkRect dst = op.blob->bounds();
        dst.offset(op.x, op.y);
        // TODO: remove when implicit bounds are plumbed through
        if (dst.isEmpty()) {
            return fCurrentClipBounds;
        }
        return this->adjustAndMap(dst, &op.paint);
    }

    static void AdjustTextForFontMetrics(SkRect* rect, const SkPaint& paint) {
#ifdef SK_DEBUG
        SkRect correct = *rect;
#endif
        const SkScalar yPad = 2.0f * paint.getTextSize(),  // In practice, this seems to be enough.
                       xPad = 4.0f * yPad;                 // Hack for very wide Github logo font.
        rect->outset(xPad, yPad);
#ifdef SK_DEBUG
        SkPaint::FontMetrics metrics;
        paint.getFontMetrics(&metrics);
        correct.fLeft   += metrics.fXMin;
        correct.fTop    += metrics.fTop;
        correct.fRight  += metrics.fXMax;
        correct.fBottom += metrics.fBottom;
        // See skia:2862 for why we ignore small text sizes.
        SkASSERTF(paint.getTextSize() < 0.001f || rect->contains(correct),
                  "%f %f %f %f vs. %f %f %f %f\n",
                  -xPad, -yPad, +xPad, +yPad,
                  metrics.fXMin, metrics.fTop, metrics.fXMax, metrics.fBottom);
#endif
    }

    // Returns true if rect was meaningfully adjusted for the effects of paint,
    // false if the paint could affect the rect in unknown ways.
    static bool AdjustForPaint(const SkPaint* paint, SkRect* rect) {
        if (paint) {
            if (paint->canComputeFastBounds()) {
                *rect = paint->computeFastBounds(*rect, rect);
                return true;
            }
            return false;
        }
        return true;
    }

    // Adjust rect for all paints that may affect its geometry, then map it to device space.
    SkIRect adjustAndMap(SkRect rect, const SkPaint* paint) const {
        // Inverted rectangles really confuse our BBHs.
        rect.sort();

        // Adjust the rect for its own paint.
        if (!AdjustForPaint(paint, &rect)) {
            // The paint could do anything to our bounds.  The only safe answer is the current clip.
            return fCurrentClipBounds;
        }

        // Adjust rect for all the paints from the SaveLayers we're inside.
        for (int i = fSaveStack.count() - 1; i >= 0; i--) {
            if (!AdjustForPaint(fSaveStack[i].paint, &rect)) {
                // Same deal as above.
                return fCurrentClipBounds;
            }
        }

        // Map the rect back to device space.
        fCTM->mapRect(&rect);
        SkIRect devRect;
        rect.roundOut(&devRect);

        // Nothing can draw outside the current clip.
        // (Only bounded ops call into this method, so oddballs like Clear don't matter here.)
        devRect.intersect(fCurrentClipBounds);
        return devRect;
    }

    // Conservative device bounds for each op in the SkRecord.
    SkAutoTMalloc<SkIRect> fBounds;

    // We walk fCurrentOp through the SkRecord, as we go using updateCTM()
    // and updateClipBounds() to maintain the exact CTM (fCTM) and conservative
    // device bounds of the current clip (fCurrentClipBounds).
    unsigned fCurrentOp;
    const SkMatrix* fCTM;
    SkIRect fCurrentClipBounds;

    // Used to track the bounds of Save/Restore blocks and the control ops inside them.
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<unsigned>   fControlIndices;
};

}  // namespace SkRecords

void SkRecordFillBounds(const SkRecord& record, SkBBoxHierarchy* bbh) {
    SkRecords::FillBounds(record, bbh);
}

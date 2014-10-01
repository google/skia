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

    if (bbh) {
        // Draw only ops that affect pixels in the canvas's current clip.
        // The SkRecord and BBH were recorded in identity space.  This canvas
        // is not necessarily in that same space.  getClipBounds() returns us
        // this canvas' clip bounds transformed back into identity space, which
        // lets us query the BBH.
        SkRect query = { 0, 0, 0, 0 };
        (void)canvas->getClipBounds(&query);

        SkTDArray<void*> ops;
        bbh->search(query, &ops);

        SkRecords::Draw draw(canvas);
        for (int i = 0; i < ops.count(); i++) {
            if (callback && callback->abortDrawing()) {
                return;
            }
            record.visit<void>((uintptr_t)ops[i], draw);  // See FillBounds below.
        }
    } else {
        // Draw all ops.
        SkRecords::Draw draw(canvas);
        for (unsigned i = 0; i < record.count(); i++) {
            if (callback && callback->abortDrawing()) {
                return;
            }
            record.visit<void>(i, draw);
        }
    }
}

void SkRecordPartialDraw(const SkRecord& record,
                         SkCanvas* canvas,
                         const SkRect& clearRect,
                         unsigned start, unsigned stop,
                         const SkMatrix& initialCTM) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    stop = SkTMin(stop, record.count());
    SkRecords::PartialDraw draw(canvas, clearRect, initialCTM);
    for (unsigned i = start; i < stop; i++) {
        record.visit<void>(i, draw);
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
DRAW(DrawPatch, drawPatch(r.cubics, r.colors, r.texCoords, r.xmode, r.paint));
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
DRAW(DrawData, drawData(r.data, r.length));
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
        const Bounds largest = Bounds::MakeLargest();
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
        SkASSERT(bbh);
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
    // In this file, SkRect are in local coordinates, Bounds are translated back to identity space.
    typedef SkRect Bounds;

    struct SaveBounds {
        int controlOps;        // Number of control ops in this Save block, including the Save.
        Bounds bounds;         // Bounds of everything in the block.
        const SkPaint* paint;  // Unowned.  If set, adjusts the bounds of all ops in this block.
    };

    // Only Restore and SetMatrix change the CTM.
    template <typename T> void updateCTM(const T&) {}
    void updateCTM(const Restore& op)   { fCTM = &op.matrix; }
    void updateCTM(const SetMatrix& op) { fCTM = &op.matrix; }

    // Most ops don't change the clip.
    template <typename T> void updateClipBounds(const T&) {}

    // Clip{Path,RRect,Rect,Region} obviously change the clip.  They all know their bounds already.
    void updateClipBounds(const ClipPath&   op) { this->updateClipBoundsForClipOp(op.devBounds); }
    void updateClipBounds(const ClipRRect&  op) { this->updateClipBoundsForClipOp(op.devBounds); }
    void updateClipBounds(const ClipRect&   op) { this->updateClipBoundsForClipOp(op.devBounds); }
    void updateClipBounds(const ClipRegion& op) { this->updateClipBoundsForClipOp(op.devBounds); }

    // The bounds of clip ops need to be adjusted for the paints of saveLayers they're inside.
    void updateClipBoundsForClipOp(const SkIRect& devBounds) {
        Bounds clip = SkRect::Make(devBounds);
        // We don't call adjustAndMap() because as its last step it would intersect the adjusted
        // clip bounds with the previous clip, exactly what we can't do when the clip grows.
        fCurrentClipBounds = this->adjustForSaveLayerPaints(&clip) ? clip : Bounds::MakeLargest();
    }

    // Restore holds the devBounds for the clip after the {save,saveLayer}/restore block completes.
    void updateClipBounds(const Restore& op) {
        // This is just like the clip ops above, but we need to skip the effects (if any) of our
        // paired saveLayer (if it is one); it has not yet been popped off the save stack.  Our
        // devBounds reflect the state of the world after the saveLayer/restore block is done,
        // so they are not affected by the saveLayer's paint.
        const int kSavesToIgnore = 1;
        Bounds clip = SkRect::Make(op.devBounds);
        fCurrentClipBounds =
            this->adjustForSaveLayerPaints(&clip, kSavesToIgnore) ? clip : Bounds::MakeLargest();
    }

    // We also take advantage of SaveLayer bounds when present to further cut the clip down.
    void updateClipBounds(const SaveLayer& op)  {
        if (op.bounds) {
            // adjustAndMap() intersects these layer bounds with the previous clip for us.
            fCurrentClipBounds = this->adjustAndMap(*op.bounds, op.paint);
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
    void trackBounds(const DrawData&)          { this->pushControl(); }

    // For all other ops, we can calculate and store the bounds directly now.
    template <typename T> void trackBounds(const T& op) {
        fBounds[fCurrentOp] = this->bounds(op);
        this->updateSaveBounds(fBounds[fCurrentOp]);
    }

    void pushSaveBlock(const SkPaint* paint) {
        // Starting a new Save block.  Push a new entry to represent that.
        SaveBounds sb = { 0, Bounds::MakeEmpty(), paint };
        fSaveStack.push(sb);
        this->pushControl();
    }

    static bool PaintMayAffectTransparentBlack(const SkPaint* paint) {
        if (paint) {
            // FIXME: this is very conservative
            if (paint->getImageFilter() || paint->getColorFilter()) {
                return true;
            }

            // Unusual Xfermodes require us to process a saved layer
            // even with operations outisde the clip.
            // For example, DstIn is used by masking layers.
            // https://code.google.com/p/skia/issues/detail?id=1291
            // https://crbug.com/401593
            SkXfermode* xfermode = paint->getXfermode();
            SkXfermode::Mode mode;
            // SrcOver is ok, and is also the common case with a NULL xfermode.
            // So we should make that the fast path and bypass the mode extraction
            // and test.
            if (xfermode && xfermode->asMode(&mode)) {
                switch (mode) {
                    // For each of the following transfer modes, if the source
                    // alpha is zero (our transparent black), the resulting
                    // blended alpha is not necessarily equal to the original
                    // destination alpha.
                    case SkXfermode::kClear_Mode:
                    case SkXfermode::kSrc_Mode:
                    case SkXfermode::kSrcIn_Mode:
                    case SkXfermode::kDstIn_Mode:
                    case SkXfermode::kSrcOut_Mode:
                    case SkXfermode::kDstATop_Mode:
                    case SkXfermode::kModulate_Mode:
                        return true;
                        break;
                    default:
                        break;
                }
            }
        }
        return false;
    }

    Bounds popSaveBlock() {
        // We're done the Save block.  Apply the block's bounds to all control ops inside it.
        SaveBounds sb;
        fSaveStack.pop(&sb);

        // If the paint affects transparent black, we can't trust any of our calculated bounds.
        const Bounds& bounds =
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

    void popControl(const Bounds& bounds) {
        fBounds[fControlIndices.top()] = bounds;
        fControlIndices.pop();
    }

    void updateSaveBounds(const Bounds& bounds) {
        // If we're in a Save block, expand its bounds to cover these bounds too.
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().bounds.join(bounds);
        }
    }

    // FIXME: this method could use better bounds
    Bounds bounds(const DrawText&) const { return fCurrentClipBounds; }

    Bounds bounds(const Clear&) const { return Bounds::MakeLargest(); }  // Ignores the clip.
    Bounds bounds(const DrawPaint&) const { return fCurrentClipBounds; }
    Bounds bounds(const NoOp&)  const { return Bounds::MakeEmpty(); }    // NoOps don't draw.

    Bounds bounds(const DrawSprite& op) const {
        const SkBitmap& bm = op.bitmap;
        return Bounds::MakeXYWH(op.left, op.top, bm.width(), bm.height());  // Ignores the matrix.
    }

    Bounds bounds(const DrawRect& op) const { return this->adjustAndMap(op.rect, &op.paint); }
    Bounds bounds(const DrawOval& op) const { return this->adjustAndMap(op.oval, &op.paint); }
    Bounds bounds(const DrawRRect& op) const {
        return this->adjustAndMap(op.rrect.rect(), &op.paint);
    }
    Bounds bounds(const DrawDRRect& op) const {
        return this->adjustAndMap(op.outer.rect(), &op.paint);
    }

    Bounds bounds(const DrawBitmapRectToRect& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawBitmapNine& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawBitmap& op) const {
        const SkBitmap& bm = op.bitmap;
        return this->adjustAndMap(SkRect::MakeXYWH(op.left, op.top, bm.width(), bm.height()),
                                  op.paint);
    }
    Bounds bounds(const DrawBitmapMatrix& op) const {
        const SkBitmap& bm = op.bitmap;
        SkRect dst = SkRect::MakeWH(bm.width(), bm.height());
        op.matrix.mapRect(&dst);
        return this->adjustAndMap(dst, op.paint);
    }

    Bounds bounds(const DrawPath& op) const {
        return op.path.isInverseFillType() ? fCurrentClipBounds
                                           : this->adjustAndMap(op.path.getBounds(), &op.paint);
    }
    Bounds bounds(const DrawPoints& op) const {
        SkRect dst;
        dst.set(op.pts, op.count);

        // Pad the bounding box a little to make sure hairline points' bounds aren't empty.
        SkScalar stroke = SkMaxScalar(op.paint.getStrokeWidth(), 0.01f);
        dst.outset(stroke/2, stroke/2);

        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawPatch& op) const {
        SkRect dst;
        dst.set(op.cubics, SkPatchUtils::kNumCtrlPts);
        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawVertices& op) const {
        SkRect dst;
        dst.set(op.vertices, op.vertexCount);
        return this->adjustAndMap(dst, &op.paint);
    }

    Bounds bounds(const DrawPicture& op) const {
        SkRect dst = op.picture->cullRect();
        if (op.matrix) {
            op.matrix->mapRect(&dst);
        }
        return this->adjustAndMap(dst, op.paint);
    }

    Bounds bounds(const DrawPosText& op) const {
        const int N = op.paint.countText(op.text, op.byteLength);
        if (N == 0) {
            return Bounds::MakeEmpty();
        }

        SkRect dst;
        dst.set(op.pos, N);
        AdjustTextForFontMetrics(&dst, op.paint);
        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawPosTextH& op) const {
        const int N = op.paint.countText(op.text, op.byteLength);
        if (N == 0) {
            return Bounds::MakeEmpty();
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
    Bounds bounds(const DrawTextOnPath& op) const {
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

    Bounds bounds(const DrawTextBlob& op) const {
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

    bool adjustForSaveLayerPaints(SkRect* rect, int savesToIgnore = 0) const {
        for (int i = fSaveStack.count() - 1 - savesToIgnore; i >= 0; i--) {
            if (!AdjustForPaint(fSaveStack[i].paint, rect)) {
                return false;
            }
        }
        return true;
    }

    // Adjust rect for all paints that may affect its geometry, then map it to identity space.
    Bounds adjustAndMap(SkRect rect, const SkPaint* paint) const {
        // Inverted rectangles really confuse our BBHs.
        rect.sort();

        // Adjust the rect for its own paint.
        if (!AdjustForPaint(paint, &rect)) {
            // The paint could do anything to our bounds.  The only safe answer is the current clip.
            return fCurrentClipBounds;
        }

        // Adjust rect for all the paints from the SaveLayers we're inside.
        if (!this->adjustForSaveLayerPaints(&rect)) {
            // Same deal as above.
            return fCurrentClipBounds;
        }

        // Map the rect back to identity space.
        fCTM->mapRect(&rect);

        // Nothing can draw outside the current clip.
        // (Only bounded ops call into this method, so oddballs like Clear don't matter here.)
        rect.intersect(fCurrentClipBounds);
        return rect;
    }

    // Conservative identity-space bounds for each op in the SkRecord.
    SkAutoTMalloc<Bounds> fBounds;

    // We walk fCurrentOp through the SkRecord, as we go using updateCTM()
    // and updateClipBounds() to maintain the exact CTM (fCTM) and conservative
    // identity-space bounds of the current clip (fCurrentClipBounds).
    unsigned fCurrentOp;
    const SkMatrix* fCTM;
    Bounds fCurrentClipBounds;

    // Used to track the bounds of Save/Restore blocks and the control ops inside them.
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<unsigned>   fControlIndices;
};

}  // namespace SkRecords

void SkRecordFillBounds(const SkRecord& record, SkBBoxHierarchy* bbh) {
    SkRecords::FillBounds(record, bbh);
}

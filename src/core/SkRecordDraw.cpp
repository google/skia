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
                  SkPicture const* const drawablePicts[],
                  SkDrawable* const drawables[],
                  int drawableCount,
                  const SkBBoxHierarchy* bbh,
                  SkPicture::AbortCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    if (bbh) {
        // Draw only ops that affect pixels in the canvas's current clip.
        // The SkRecord and BBH were recorded in identity space.  This canvas
        // is not necessarily in that same space.  getLocalClipBounds() returns us
        // this canvas' clip bounds transformed back into identity space, which
        // lets us query the BBH.
        SkRect query = canvas->getLocalClipBounds();

        SkTDArray<int> ops;
        bbh->search(query, &ops);

        SkRecords::Draw draw(canvas, drawablePicts, drawables, drawableCount);
        for (int i = 0; i < ops.count(); i++) {
            if (callback && callback->abort()) {
                return;
            }
            // This visit call uses the SkRecords::Draw::operator() to call
            // methods on the |canvas|, wrapped by methods defined with the
            // DRAW() macro.
            record.visit(ops[i], draw);
        }
    } else {
        // Draw all ops.
        SkRecords::Draw draw(canvas, drawablePicts, drawables, drawableCount);
        for (int i = 0; i < record.count(); i++) {
            if (callback && callback->abort()) {
                return;
            }
            // This visit call uses the SkRecords::Draw::operator() to call
            // methods on the |canvas|, wrapped by methods defined with the
            // DRAW() macro.
            record.visit(i, draw);
        }
    }
}

void SkRecordPartialDraw(const SkRecord& record, SkCanvas* canvas,
                         SkPicture const* const drawablePicts[], int drawableCount,
                         int start, int stop,
                         const SkMatrix& initialCTM) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    stop = SkTMin(stop, record.count());
    SkRecords::Draw draw(canvas, drawablePicts, nullptr, drawableCount, &initialCTM);
    for (int i = start; i < stop; i++) {
        record.visit(i, draw);
    }
}

namespace SkRecords {

// NoOps draw nothing.
template <> void Draw::draw(const NoOp&) {}

#define DRAW(T, call) template <> void Draw::draw(const T& r) { fCanvas->call; }
DRAW(Restore, restore());
DRAW(Save, save());
DRAW(SaveLayer, saveLayer(SkCanvas::SaveLayerRec(r.bounds,
                                                 r.paint,
                                                 r.backdrop.get(),
                                                 r.saveLayerFlags)));
DRAW(SetMatrix, setMatrix(SkMatrix::Concat(fInitialCTM, r.matrix)));
DRAW(Concat, concat(r.matrix));
DRAW(Translate, translate(r.dx, r.dy));

DRAW(ClipPath, clipPath(r.path, r.opAA.op(), r.opAA.aa()));
DRAW(ClipRRect, clipRRect(r.rrect, r.opAA.op(), r.opAA.aa()));
DRAW(ClipRect, clipRect(r.rect, r.opAA.op(), r.opAA.aa()));
DRAW(ClipRegion, clipRegion(r.region, r.op));

#ifdef SK_EXPERIMENTAL_SHADOWING
DRAW(TranslateZ, SkCanvas::translateZ(r.z));
#else
template <> void Draw::draw(const TranslateZ& r) { }
#endif

DRAW(DrawArc, drawArc(r.oval, r.startAngle, r.sweepAngle, r.useCenter, r.paint));
DRAW(DrawDRRect, drawDRRect(r.outer, r.inner, r.paint));
DRAW(DrawImage, drawImage(r.image.get(), r.left, r.top, r.paint));

template <> void Draw::draw(const DrawImageLattice& r) {
    SkCanvas::Lattice lattice;
    lattice.fXCount = r.xCount;
    lattice.fXDivs = r.xDivs;
    lattice.fYCount = r.yCount;
    lattice.fYDivs = r.yDivs;
    lattice.fFlags = (0 == r.flagCount) ? nullptr : r.flags;
    lattice.fBounds = &r.src;
    fCanvas->drawImageLattice(r.image.get(), lattice, r.dst, r.paint);
}

DRAW(DrawImageRect, legacy_drawImageRect(r.image.get(), r.src, r.dst, r.paint, r.constraint));
DRAW(DrawImageNine, drawImageNine(r.image.get(), r.center, r.dst, r.paint));
DRAW(DrawOval, drawOval(r.oval, r.paint));
DRAW(DrawPaint, drawPaint(r.paint));
DRAW(DrawPath, drawPath(r.path, r.paint));
DRAW(DrawPatch, drawPatch(r.cubics, r.colors, r.texCoords, r.bmode, r.paint));
DRAW(DrawPicture, drawPicture(r.picture.get(), &r.matrix, r.paint));

#ifdef SK_EXPERIMENTAL_SHADOWING
DRAW(DrawShadowedPicture, drawShadowedPicture(r.picture.get(), &r.matrix, r.paint, r.params));
#else
template <> void Draw::draw(const DrawShadowedPicture& r) { }
#endif

DRAW(DrawPoints, drawPoints(r.mode, r.count, r.pts, r.paint));
DRAW(DrawPosText, drawPosText(r.text, r.byteLength, r.pos, r.paint));
DRAW(DrawPosTextH, drawPosTextH(r.text, r.byteLength, r.xpos, r.y, r.paint));
DRAW(DrawRRect, drawRRect(r.rrect, r.paint));
DRAW(DrawRect, drawRect(r.rect, r.paint));
DRAW(DrawRegion, drawRegion(r.region, r.paint));
DRAW(DrawText, drawText(r.text, r.byteLength, r.x, r.y, r.paint));
DRAW(DrawTextBlob, drawTextBlob(r.blob.get(), r.x, r.y, r.paint));
DRAW(DrawTextOnPath, drawTextOnPath(r.text, r.byteLength, r.path, &r.matrix, r.paint));
DRAW(DrawTextRSXform, drawTextRSXform(r.text, r.byteLength, r.xforms, r.cull, r.paint));
DRAW(DrawAtlas, drawAtlas(r.atlas.get(),
                          r.xforms, r.texs, r.colors, r.count, r.mode, r.cull, r.paint));
DRAW(DrawVertices, drawVertices(r.vertices, r.bmode, r.paint));
DRAW(DrawAnnotation, drawAnnotation(r.rect, r.key.c_str(), r.value.get()));
#undef DRAW

template <> void Draw::draw(const DrawDrawable& r) {
    SkASSERT(r.index >= 0);
    SkASSERT(r.index < fDrawableCount);
    if (fDrawables) {
        SkASSERT(nullptr == fDrawablePicts);
        fCanvas->drawDrawable(fDrawables[r.index], r.matrix);
    } else {
        fCanvas->drawPicture(fDrawablePicts[r.index], r.matrix, nullptr);
    }
}

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
    FillBounds(const SkRect& cullRect, const SkRecord& record, SkRect bounds[])
        : fNumRecords(record.count())
        , fCullRect(cullRect)
        , fBounds(bounds) {
        fCTM = SkMatrix::I();
        fCurrentClipBounds = fCullRect;
    }

    void cleanUp() {
        // If we have any lingering unpaired Saves, simulate restores to make
        // sure all ops in those Save blocks have their bounds calculated.
        while (!fSaveStack.isEmpty()) {
            this->popSaveBlock();
        }

        // Any control ops not part of any Save/Restore block draw everywhere.
        while (!fControlIndices.isEmpty()) {
            this->popControl(fCullRect);
        }
    }

    void setCurrentOp(int currentOp) { fCurrentOp = currentOp; }


    template <typename T> void operator()(const T& op) {
        this->updateCTM(op);
        this->updateClipBounds(op);
        this->trackBounds(op);
    }

    // In this file, SkRect are in local coordinates, Bounds are translated back to identity space.
    typedef SkRect Bounds;

    int currentOp() const { return fCurrentOp; }
    const SkMatrix& ctm() const { return fCTM; }
    const Bounds& getBounds(int index) const { return fBounds[index]; }

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
        fCTM.mapRect(&rect);

        // Nothing can draw outside the current clip.
        if (!rect.intersect(fCurrentClipBounds)) {
            return Bounds::MakeEmpty();
        }

        return rect;
    }

private:
    struct SaveBounds {
        int controlOps;        // Number of control ops in this Save block, including the Save.
        Bounds bounds;         // Bounds of everything in the block.
        const SkPaint* paint;  // Unowned.  If set, adjusts the bounds of all ops in this block.
        SkMatrix ctm;
    };

    // Only Restore, SetMatrix, Concat, and Translate change the CTM.
    template <typename T> void updateCTM(const T&) {}
    void updateCTM(const Restore& op)   { fCTM = op.matrix; }
    void updateCTM(const SetMatrix& op) { fCTM = op.matrix; }
    void updateCTM(const Concat& op)    { fCTM.preConcat(op.matrix); }
    void updateCTM(const Translate& op) { fCTM.preTranslate(op.dx, op.dy); }

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
        if (this->adjustForSaveLayerPaints(&clip)) {
            fCurrentClipBounds = clip.intersect(fCullRect) ? clip : Bounds::MakeEmpty();
        } else {
            fCurrentClipBounds = fCullRect;
        }
    }

    // Restore holds the devBounds for the clip after the {save,saveLayer}/restore block completes.
    void updateClipBounds(const Restore& op) {
        // This is just like the clip ops above, but we need to skip the effects (if any) of our
        // paired saveLayer (if it is one); it has not yet been popped off the save stack.  Our
        // devBounds reflect the state of the world after the saveLayer/restore block is done,
        // so they are not affected by the saveLayer's paint.
        const int kSavesToIgnore = 1;
        Bounds clip = SkRect::Make(op.devBounds);
        if (this->adjustForSaveLayerPaints(&clip, kSavesToIgnore)) {
            fCurrentClipBounds = clip.intersect(fCullRect) ? clip : Bounds::MakeEmpty();
        } else {
            fCurrentClipBounds = fCullRect;
        }
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
    void trackBounds(const Save&)          { this->pushSaveBlock(nullptr); }
    void trackBounds(const SaveLayer& op)  { this->pushSaveBlock(op.paint); }
    void trackBounds(const Restore&) { fBounds[fCurrentOp] = this->popSaveBlock(); }

    void trackBounds(const SetMatrix&)         { this->pushControl(); }
    void trackBounds(const Concat&)            { this->pushControl(); }
    void trackBounds(const Translate&)         { this->pushControl(); }
    void trackBounds(const TranslateZ&)        { this->pushControl(); }
    void trackBounds(const ClipRect&)          { this->pushControl(); }
    void trackBounds(const ClipRRect&)         { this->pushControl(); }
    void trackBounds(const ClipPath&)          { this->pushControl(); }
    void trackBounds(const ClipRegion&)        { this->pushControl(); }


    // For all other ops, we can calculate and store the bounds directly now.
    template <typename T> void trackBounds(const T& op) {
        fBounds[fCurrentOp] = this->bounds(op);
        this->updateSaveBounds(fBounds[fCurrentOp]);
    }

    void pushSaveBlock(const SkPaint* paint) {
        // Starting a new Save block.  Push a new entry to represent that.
        SaveBounds sb;
        sb.controlOps = 0;
        // If the paint affects transparent black, the bound shouldn't be smaller
        // than the current clip bounds.
        sb.bounds =
            PaintMayAffectTransparentBlack(paint) ? fCurrentClipBounds : Bounds::MakeEmpty();
        sb.paint = paint;
        sb.ctm = this->fCTM;

        fSaveStack.push(sb);
        this->pushControl();
    }

    static bool PaintMayAffectTransparentBlack(const SkPaint* paint) {
        if (paint) {
            // FIXME: this is very conservative
            if (paint->getImageFilter() || paint->getColorFilter()) {
                return true;
            }

            // Unusual blendmodes require us to process a saved layer
            // even with operations outisde the clip.
            // For example, DstIn is used by masking layers.
            // https://code.google.com/p/skia/issues/detail?id=1291
            // https://crbug.com/401593
            switch (paint->getBlendMode()) {
                // For each of the following transfer modes, if the source
                // alpha is zero (our transparent black), the resulting
                // blended alpha is not necessarily equal to the original
                // destination alpha.
                case SkBlendMode::kClear:
                case SkBlendMode::kSrc:
                case SkBlendMode::kSrcIn:
                case SkBlendMode::kDstIn:
                case SkBlendMode::kSrcOut:
                case SkBlendMode::kDstATop:
                case SkBlendMode::kModulate:
                    return true;
                    break;
                default:
                    break;
            }
        }
        return false;
    }

    Bounds popSaveBlock() {
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

    Bounds bounds(const DrawPaint&) const { return fCurrentClipBounds; }
    Bounds bounds(const NoOp&)  const { return Bounds::MakeEmpty(); }    // NoOps don't draw.

    Bounds bounds(const DrawRect& op) const { return this->adjustAndMap(op.rect, &op.paint); }
    Bounds bounds(const DrawRegion& op) const {
        SkRect rect = SkRect::Make(op.region.getBounds());
        return this->adjustAndMap(rect, &op.paint);
    }
    Bounds bounds(const DrawOval& op) const { return this->adjustAndMap(op.oval, &op.paint); }
    // Tighter arc bounds?
    Bounds bounds(const DrawArc& op) const { return this->adjustAndMap(op.oval, &op.paint); }
    Bounds bounds(const DrawRRect& op) const {
        return this->adjustAndMap(op.rrect.rect(), &op.paint);
    }
    Bounds bounds(const DrawDRRect& op) const {
        return this->adjustAndMap(op.outer.rect(), &op.paint);
    }
    Bounds bounds(const DrawImage& op) const {
        const SkImage* image = op.image.get();
        SkRect rect = SkRect::MakeXYWH(op.left, op.top, image->width(), image->height());

        return this->adjustAndMap(rect, op.paint);
    }
    Bounds bounds(const DrawImageLattice& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawImageRect& op) const {
        return this->adjustAndMap(op.dst, op.paint);
    }
    Bounds bounds(const DrawImageNine& op) const {
        return this->adjustAndMap(op.dst, op.paint);
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
        return this->adjustAndMap(op.vertices->bounds(), &op.paint);
    }

    Bounds bounds(const DrawAtlas& op) const {
        if (op.cull) {
            // TODO: <reed> can we pass nullptr for the paint? Isn't cull already "correct"
            // for the paint (by the caller)?
            return this->adjustAndMap(*op.cull, op.paint);
        } else {
            return fCurrentClipBounds;
        }
    }

    Bounds bounds(const DrawPicture& op) const {
        SkRect dst = op.picture->cullRect();
        op.matrix.mapRect(&dst);
        return this->adjustAndMap(dst, op.paint);
    }

    Bounds bounds(const DrawShadowedPicture& op) const {
        SkRect dst = op.picture->cullRect();
        op.matrix.mapRect(&dst);
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

    Bounds bounds(const DrawTextRSXform& op) const {
        if (op.cull) {
            return this->adjustAndMap(*op.cull, nullptr);
        } else {
            return fCurrentClipBounds;
        }
    }

    Bounds bounds(const DrawTextBlob& op) const {
        SkRect dst = op.blob->bounds();
        dst.offset(op.x, op.y);
        return this->adjustAndMap(dst, &op.paint);
    }

    Bounds bounds(const DrawDrawable& op) const {
        return this->adjustAndMap(op.worstCaseBounds, nullptr);
    }

    Bounds bounds(const DrawAnnotation& op) const {
        return this->adjustAndMap(op.rect, nullptr);
    }

    static void AdjustTextForFontMetrics(SkRect* rect, const SkPaint& paint) {
#ifdef SK_DEBUG
        SkRect correct = *rect;
#endif
        // crbug.com/373785 ~~> xPad = 4x yPad
        // crbug.com/424824 ~~> bump yPad from 2x text size to 2.5x
        const SkScalar yPad = 2.5f * paint.getTextSize(),
                       xPad = 4.0f * yPad;
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
            SkMatrix inverse;
            if (!fSaveStack[i].ctm.invert(&inverse)) {
                return false;
            }
            inverse.mapRect(rect);
            if (!AdjustForPaint(fSaveStack[i].paint, rect)) {
                return false;
            }
            fSaveStack[i].ctm.mapRect(rect);
        }
        return true;
    }

    const int fNumRecords;

    // We do not guarantee anything for operations outside of the cull rect
    const SkRect fCullRect;

    // Conservative identity-space bounds for each op in the SkRecord.
    Bounds* fBounds;

    // We walk fCurrentOp through the SkRecord, as we go using updateCTM()
    // and updateClipBounds() to maintain the exact CTM (fCTM) and conservative
    // identity-space bounds of the current clip (fCurrentClipBounds).
    int fCurrentOp;
    SkMatrix fCTM;
    Bounds fCurrentClipBounds;

    // Used to track the bounds of Save/Restore blocks and the control ops inside them.
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<int>   fControlIndices;
};

}  // namespace SkRecords

void SkRecordFillBounds(const SkRect& cullRect, const SkRecord& record, SkRect bounds[]) {
    SkRecords::FillBounds visitor(cullRect, record, bounds);
    for (int curOp = 0; curOp < record.count(); curOp++) {
        visitor.setCurrentOp(curOp);
        record.visit(curOp, visitor);
    }
    visitor.cleanUp();
}


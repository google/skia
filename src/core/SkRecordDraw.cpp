/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "include/core/SkImage.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkRecordDraw.h"
#include "src/utils/SkPatchUtils.h"

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

        std::vector<int> ops;
        bbh->search(query, &ops);

        SkRecords::Draw draw(canvas, drawablePicts, drawables, drawableCount);
        for (int i = 0; i < (int)ops.size(); i++) {
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

    stop = std::min(stop, record.count());
    SkRecords::Draw draw(canvas, drawablePicts, nullptr, drawableCount, &initialCTM);
    for (int i = start; i < stop; i++) {
        record.visit(i, draw);
    }
}

namespace SkRecords {

// NoOps draw nothing.
template <> void Draw::draw(const NoOp&) {}

#define DRAW(T, call) template <> void Draw::draw(const T& r) { fCanvas->call; }
DRAW(Flush, flush());
DRAW(Restore, restore());
DRAW(Save, save());
DRAW(SaveLayer, saveLayer(SkCanvas::SaveLayerRec(r.bounds,
                                                 r.paint,
                                                 r.backdrop.get(),
                                                 r.clipMask.get(),
                                                 r.clipMatrix,
                                                 r.saveLayerFlags)));

template <> void Draw::draw(const SaveBehind& r) {
    SkCanvasPriv::SaveBehind(fCanvas, r.subset);
}

template <> void Draw::draw(const DrawBehind& r) {
    SkCanvasPriv::DrawBehind(fCanvas, r.paint);
}

DRAW(SetMatrix, setMatrix(SkMatrix::Concat(fInitialCTM, r.matrix)));
DRAW(Concat44, concat44(r.matrix));
DRAW(Concat, concat(r.matrix));
DRAW(Translate, translate(r.dx, r.dy));
DRAW(Scale, scale(r.sx, r.sy));

DRAW(ClipPath, clipPath(r.path, r.opAA.op(), r.opAA.aa()));
DRAW(ClipRRect, clipRRect(r.rrect, r.opAA.op(), r.opAA.aa()));
DRAW(ClipRect, clipRect(r.rect, r.opAA.op(), r.opAA.aa()));
DRAW(ClipRegion, clipRegion(r.region, r.op));
DRAW(ClipShader, clipShader(r.shader, r.op));

DRAW(DrawArc, drawArc(r.oval, r.startAngle, r.sweepAngle, r.useCenter, r.paint));
DRAW(DrawDRRect, drawDRRect(r.outer, r.inner, r.paint));
DRAW(DrawImage, drawImage(r.image.get(), r.left, r.top, r.paint));

template <> void Draw::draw(const DrawImageLattice& r) {
    SkCanvas::Lattice lattice;
    lattice.fXCount = r.xCount;
    lattice.fXDivs = r.xDivs;
    lattice.fYCount = r.yCount;
    lattice.fYDivs = r.yDivs;
    lattice.fRectTypes = (0 == r.flagCount) ? nullptr : r.flags;
    lattice.fColors = (0 == r.flagCount) ? nullptr : r.colors;
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
DRAW(DrawPoints, drawPoints(r.mode, r.count, r.pts, r.paint));
DRAW(DrawRRect, drawRRect(r.rrect, r.paint));
DRAW(DrawRect, drawRect(r.rect, r.paint));
DRAW(DrawRegion, drawRegion(r.region, r.paint));
DRAW(DrawTextBlob, drawTextBlob(r.blob.get(), r.x, r.y, r.paint));
DRAW(DrawAtlas, drawAtlas(r.atlas.get(),
                          r.xforms, r.texs, r.colors, r.count, r.mode, r.cull, r.paint));
DRAW(DrawVertices, drawVertices(r.vertices, r.bmode, r.paint));
DRAW(DrawShadowRec, private_draw_shadow_rec(r.path, r.rec));
DRAW(DrawAnnotation, drawAnnotation(r.rect, r.key.c_str(), r.value.get()));

DRAW(DrawEdgeAAQuad, experimental_DrawEdgeAAQuad(
        r.rect, r.clip, r.aa, r.color, r.mode));
DRAW(DrawEdgeAAImageSet, experimental_DrawEdgeAAImageSet(
        r.set.get(), r.count, r.dstClips, r.preViewMatrices, r.paint, r.constraint));

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
    FillBounds(const SkRect& cullRect, const SkRecord& record,
               SkRect bounds[], SkBBoxHierarchy::Metadata meta[])
        : fCullRect(cullRect)
        , fBounds(bounds)
        , fMeta(meta) {
        fCTM = SkMatrix::I();

        // We push an extra save block to track the bounds of any top-level control operations.
        fSaveStack.push_back({ 0, Bounds::MakeEmpty(), nullptr, fCTM });
    }

    ~FillBounds() {
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
        this->trackBounds(op);
    }

    // In this file, SkRect are in local coordinates, Bounds are translated back to identity space.
    typedef SkRect Bounds;

    // Adjust rect for all paints that may affect its geometry, then map it to identity space.
    Bounds adjustAndMap(SkRect rect, const SkPaint* paint) const {
        // Inverted rectangles really confuse our BBHs.
        rect.sort();

        // Adjust the rect for its own paint.
        if (!AdjustForPaint(paint, &rect)) {
            // The paint could do anything to our bounds.  The only safe answer is the cull.
            return fCullRect;
        }

        // Adjust rect for all the paints from the SaveLayers we're inside.
        if (!this->adjustForSaveLayerPaints(&rect)) {
            // Same deal as above.
            return fCullRect;
        }

        // Map the rect back to identity space.
        fCTM.mapRect(&rect);

        // Nothing can draw outside the cull rect.
        if (!rect.intersect(fCullRect)) {
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
    void updateCTM(const Concat44& op)  { fCTM.preConcat(op.matrix.asM33()); }
    void updateCTM(const Concat& op)    { fCTM.preConcat(op.matrix); }
    void updateCTM(const Scale& op)     { fCTM.preScale(op.sx, op.sy); }
    void updateCTM(const Translate& op) { fCTM.preTranslate(op.dx, op.dy); }

    // The bounds of these ops must be calculated when we hit the Restore
    // from the bounds of the ops in the same Save block.
    void trackBounds(const Save&)          { this->pushSaveBlock(nullptr); }
    void trackBounds(const SaveLayer& op)  { this->pushSaveBlock(op.paint); }
    void trackBounds(const SaveBehind&)    { this->pushSaveBlock(nullptr); }
    void trackBounds(const Restore&) {
        const bool isSaveLayer = fSaveStack.top().paint != nullptr;
        fBounds[fCurrentOp] = this->popSaveBlock();
        fMeta  [fCurrentOp].isDraw = isSaveLayer;
    }

    void trackBounds(const SetMatrix&)         { this->pushControl(); }
    void trackBounds(const Concat&)            { this->pushControl(); }
    void trackBounds(const Concat44&)          { this->pushControl(); }
    void trackBounds(const Scale&)             { this->pushControl(); }
    void trackBounds(const Translate&)         { this->pushControl(); }
    void trackBounds(const ClipRect&)          { this->pushControl(); }
    void trackBounds(const ClipRRect&)         { this->pushControl(); }
    void trackBounds(const ClipPath&)          { this->pushControl(); }
    void trackBounds(const ClipRegion&)        { this->pushControl(); }
    void trackBounds(const ClipShader&)        { this->pushControl(); }


    // For all other ops, we can calculate and store the bounds directly now.
    template <typename T> void trackBounds(const T& op) {
        fBounds[fCurrentOp] = this->bounds(op);
        fMeta  [fCurrentOp].isDraw = true;
        this->updateSaveBounds(fBounds[fCurrentOp]);
    }

    void pushSaveBlock(const SkPaint* paint) {
        // Starting a new Save block.  Push a new entry to represent that.
        SaveBounds sb;
        sb.controlOps = 0;
        // If the paint affects transparent black,
        // the bound shouldn't be smaller than the cull.
        sb.bounds =
            PaintMayAffectTransparentBlack(paint) ? fCullRect : Bounds::MakeEmpty();
        sb.paint = paint;
        sb.ctm = this->fCTM;

        fSaveStack.push_back(sb);
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
        fControlIndices.push_back(fCurrentOp);
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().controlOps++;
        }
    }

    void popControl(const Bounds& bounds) {
        fBounds[fControlIndices.top()] = bounds;
        fMeta  [fControlIndices.top()].isDraw = false;
        fControlIndices.pop();
    }

    void updateSaveBounds(const Bounds& bounds) {
        // If we're in a Save block, expand its bounds to cover these bounds too.
        if (!fSaveStack.isEmpty()) {
            fSaveStack.top().bounds.join(bounds);
        }
    }

    Bounds bounds(const Flush&) const { return fCullRect; }

    Bounds bounds(const DrawPaint&) const { return fCullRect; }
    Bounds bounds(const DrawBehind&) const { return fCullRect; }
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
        return op.path.isInverseFillType() ? fCullRect
                                           : this->adjustAndMap(op.path.getBounds(), &op.paint);
    }
    Bounds bounds(const DrawPoints& op) const {
        SkRect dst;
        dst.setBounds(op.pts, op.count);

        // Pad the bounding box a little to make sure hairline points' bounds aren't empty.
        SkScalar stroke = std::max(op.paint.getStrokeWidth(), 0.01f);
        dst.outset(stroke/2, stroke/2);

        return this->adjustAndMap(dst, &op.paint);
    }
    Bounds bounds(const DrawPatch& op) const {
        SkRect dst;
        dst.setBounds(op.cubics, SkPatchUtils::kNumCtrlPts);
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
            return fCullRect;
        }
    }

    Bounds bounds(const DrawShadowRec& op) const {
        SkRect bounds;
        SkDrawShadowMetrics::GetLocalBounds(op.path, op.rec, fCTM, &bounds);
        return this->adjustAndMap(bounds, nullptr);
    }

    Bounds bounds(const DrawPicture& op) const {
        SkRect dst = op.picture->cullRect();
        op.matrix.mapRect(&dst);
        return this->adjustAndMap(dst, op.paint);
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
    Bounds bounds(const DrawEdgeAAQuad& op) const {
        SkRect bounds = op.rect;
        if (op.clip) {
            bounds.setBounds(op.clip, 4);
        }
        return this->adjustAndMap(bounds, nullptr);
    }
    Bounds bounds(const DrawEdgeAAImageSet& op) const {
        SkRect rect = SkRect::MakeEmpty();
        int clipIndex = 0;
        for (int i = 0; i < op.count; ++i) {
            SkRect entryBounds = op.set[i].fDstRect;
            if (op.set[i].fHasClip) {
                entryBounds.setBounds(op.dstClips + clipIndex, 4);
                clipIndex += 4;
            }
            if (op.set[i].fMatrixIndex >= 0) {
                op.preViewMatrices[op.set[i].fMatrixIndex].mapRect(&entryBounds);
            }
            rect.join(this->adjustAndMap(entryBounds, nullptr));
        }
        return rect;
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

    // We do not guarantee anything for operations outside of the cull rect
    const SkRect fCullRect;

    // Conservative identity-space bounds for each op in the SkRecord.
    Bounds* fBounds;

    // Parallel array to fBounds, holding metadata for each bounds rect.
    SkBBoxHierarchy::Metadata* fMeta;

    // We walk fCurrentOp through the SkRecord,
    // as we go using updateCTM() to maintain the exact CTM (fCTM).
    int fCurrentOp;
    SkMatrix fCTM;

    // Used to track the bounds of Save/Restore blocks and the control ops inside them.
    SkTDArray<SaveBounds> fSaveStack;
    SkTDArray<int>   fControlIndices;
};

}  // namespace SkRecords

void SkRecordFillBounds(const SkRect& cullRect, const SkRecord& record,
                        SkRect bounds[], SkBBoxHierarchy::Metadata meta[]) {
    {
        SkRecords::FillBounds visitor(cullRect, record, bounds, meta);
        for (int i = 0; i < record.count(); i++) {
            visitor.setCurrentOp(i);
            record.visit(i, visitor);
        }
    }
}


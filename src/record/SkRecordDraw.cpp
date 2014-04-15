/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordDraw.h"

namespace {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas) : fCanvas(canvas), fIndex(0), fClipEmpty(false) {}

    unsigned index() const { return fIndex; }
    void next() { ++fIndex; }

    template <typename T> void operator()(const T& r) {
        if (!this->canSkip(r)) {
            this->draw(r);
            this->updateClip<T>();
        }
    }

private:
    // Can we skip this command right now?
    template <typename T> bool canSkip(const T&) const {
        // We can skip most commands if the clip is empty.  Exceptions are specialized below.
        return fClipEmpty;
    }

    // No base case, so we'll be compile-time checked that we implemented all possibilities below.
    template <typename T> void draw(const T&);

    // Update fClipEmpty if necessary.
    template <typename T> void updateClip() {
        // Most commands don't change the clip.  Exceptions are specialized below.
    }

    SkCanvas* fCanvas;
    unsigned fIndex;
    bool fClipEmpty;
};

// These commands may change the clip.
#define UPDATE_CLIP(T) template <> void Draw::updateClip<SkRecords::T>() \
    { fClipEmpty = fCanvas->isClipEmpty(); }
UPDATE_CLIP(Restore);
UPDATE_CLIP(SaveLayer);
UPDATE_CLIP(ClipPath);
UPDATE_CLIP(ClipRRect);
UPDATE_CLIP(ClipRect);
UPDATE_CLIP(ClipRegion);
#undef UPDATE_CLIP

// These commands must always run.
#define CAN_SKIP(T) template <> bool Draw::canSkip(const SkRecords::T&) const { return false; }
CAN_SKIP(Restore);
CAN_SKIP(Save);
CAN_SKIP(SaveLayer);
CAN_SKIP(Clear);
CAN_SKIP(PushCull);
CAN_SKIP(PopCull);
#undef CAN_SKIP

// We can skip these commands if they're intersecting with a clip that's already empty.
#define CAN_SKIP(T) template <> bool Draw::canSkip(const SkRecords::T& r) const \
    { return fClipEmpty && SkRegion::kIntersect_Op == r.op; }
CAN_SKIP(ClipPath);
CAN_SKIP(ClipRRect);
CAN_SKIP(ClipRect);
CAN_SKIP(ClipRegion);
#undef CAN_SKIP

static bool can_skip_text(const SkCanvas& c, const SkPaint& p, SkScalar minY, SkScalar maxY) {
    // If we're drawing vertical text, none of the checks we're about to do make any sense.
    // We'll need to call SkPaint::computeFastBounds() later, so bail out if that's not possible.
    if (p.isVerticalText() || !p.canComputeFastBounds()) {
        return false;
    }

    // Rather than checking the top and bottom font metrics, we guess.  Actually looking up the top
    // and bottom metrics is slow, and this overapproximation should be good enough.
    const SkScalar buffer = p.getTextSize() * 1.5f;
    SkDEBUGCODE(SkPaint::FontMetrics metrics;)
    SkDEBUGCODE(p.getFontMetrics(&metrics);)
    SkASSERT(-buffer <= metrics.fTop);
    SkASSERT(+buffer >= metrics.fBottom);

    // Let the paint adjust the text bounds.  We don't care about left and right here, so we use
    // 0 and 1 respectively just so the bounds rectangle isn't empty.
    SkRect bounds;
    bounds.set(0, -buffer, SK_Scalar1, buffer);
    SkRect adjusted = p.computeFastBounds(bounds, &bounds);
    return c.quickRejectY(minY + adjusted.fTop, maxY + adjusted.fBottom);
}

template <> bool Draw::canSkip(const SkRecords::DrawPosTextH& r) const {
    return fClipEmpty || can_skip_text(*fCanvas, r.paint, r.y, r.y);
}

template <> bool Draw::canSkip(const SkRecords::DrawPosText& r) const {
    if (fClipEmpty) {
        return true;
    }

    // TODO(mtklein): may want to move this minY/maxY calculation into a one-time pass
    const unsigned points = r.paint.countText(r.text, r.byteLength);
    if (points == 0) {
        return true;
    }
    SkScalar minY = SK_ScalarInfinity, maxY = SK_ScalarNegativeInfinity;
    for (unsigned i = 0; i < points; i++) {
        minY = SkTMin(minY, r.pos[i].fY);
        maxY = SkTMax(maxY, r.pos[i].fY);
    }

    return can_skip_text(*fCanvas, r.paint, minY, maxY);
}

#define DRAW(T, call) template <> void Draw::draw(const SkRecords::T& r) { fCanvas->call; }
DRAW(Restore, restore());
DRAW(Save, save(r.flags));
DRAW(SaveLayer, saveLayer(r.bounds, r.paint, r.flags));
DRAW(PopCull, popCull());
DRAW(Clear, clear(r.color));
DRAW(Concat, concat(r.matrix));
DRAW(SetMatrix, setMatrix(r.matrix));

DRAW(ClipPath, clipPath(r.path, r.op, r.doAA));
DRAW(ClipRRect, clipRRect(r.rrect, r.op, r.doAA));
DRAW(ClipRect, clipRect(r.rect, r.op, r.doAA));
DRAW(ClipRegion, clipRegion(r.region, r.op));

DRAW(DrawBitmap, drawBitmap(r.bitmap, r.left, r.top, r.paint));
DRAW(DrawBitmapMatrix, drawBitmapMatrix(r.bitmap, r.matrix, r.paint));
DRAW(DrawBitmapNine, drawBitmapNine(r.bitmap, r.center, r.dst, r.paint));
DRAW(DrawBitmapRectToRect, drawBitmapRectToRect(r.bitmap, r.src, r.dst, r.paint, r.flags));
DRAW(DrawDRRect, drawDRRect(r.outer, r.inner, r.paint));
DRAW(DrawOval, drawOval(r.oval, r.paint));
DRAW(DrawPaint, drawPaint(r.paint));
DRAW(DrawPath, drawPath(r.path, r.paint));
DRAW(DrawPoints, drawPoints(r.mode, r.count, r.pts, r.paint));
DRAW(DrawPosText, drawPosText(r.text, r.byteLength, r.pos, r.paint));
DRAW(DrawPosTextH, drawPosTextH(r.text, r.byteLength, r.xpos, r.y, r.paint));
DRAW(DrawRRect, drawRRect(r.rrect, r.paint));
DRAW(DrawRect, drawRect(r.rect, r.paint));
DRAW(DrawSprite, drawSprite(r.bitmap, r.left, r.top, r.paint));
DRAW(DrawText, drawText(r.text, r.byteLength, r.x, r.y, r.paint));
DRAW(DrawTextOnPath, drawTextOnPath(r.text, r.byteLength, r.path, r.matrix, r.paint));
DRAW(DrawVertices, drawVertices(r.vmode, r.vertexCount, r.vertices, r.texs, r.colors,
                                r.xmode.get(), r.indices, r.indexCount, r.paint));
#undef DRAW

// PushCull is a bit of a oddball.  We might be able to just skip until just past its popCull.
template <> void Draw::draw(const SkRecords::PushCull& r) {
    if (r.popOffset != SkRecords::kUnsetPopOffset && fCanvas->quickReject(r.rect)) {
        fIndex += r.popOffset;
    } else {
        fCanvas->pushCull(r.rect);
    }
}

}  // namespace

void SkRecordDraw(const SkRecord& record, SkCanvas* canvas) {
    for (Draw draw(canvas); draw.index() < record.count(); draw.next()) {
        record.visit(draw.index(), draw);
    }
}

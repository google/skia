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
        if (!this->skip(r)) {
            this->draw(r);
            this->updateClip<T>();
        }
    }

private:
    // Return true if we can skip this command, false if not.
    // Update fIndex here directly to skip more than just this one command.
    template <typename T> bool skip(const T&) {
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

// TODO(mtklein): do this specialization with template traits instead of macros

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
#define SKIP(T) template <> bool Draw::skip(const SkRecords::T&) { return false; }
SKIP(Restore);
SKIP(Save);
SKIP(SaveLayer);
SKIP(Clear);
SKIP(PushCull);
SKIP(PopCull);
#undef SKIP

// We can skip these commands if they're intersecting with a clip that's already empty.
#define SKIP(T) template <> bool Draw::skip(const SkRecords::T& r) \
    { return fClipEmpty && SkRegion::kIntersect_Op == r.op; }
SKIP(ClipPath);
SKIP(ClipRRect);
SKIP(ClipRect);
SKIP(ClipRegion);
#undef SKIP

// NoOps can always be skipped and draw nothing.
template <> bool Draw::skip(const SkRecords::NoOp&) { return true; }
template <> void Draw::draw(const SkRecords::NoOp&) {}

#define DRAW(T, call) template <> void Draw::draw(const SkRecords::T& r) { fCanvas->call; }
DRAW(Restore, restore());
DRAW(Save, save(r.flags));
DRAW(SaveLayer, saveLayer(r.bounds, r.paint, r.flags));
DRAW(PopCull, popCull());
DRAW(PushCull, pushCull(r.rect));
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

// Added by SkRecordAnnotateCullingPairs.
template <> bool Draw::skip(const SkRecords::PairedPushCull& r) {
    if (fCanvas->quickReject(r.base->rect)) {
        fIndex += r.skip;
        return true;
    }
    return false;
}

// Added by SkRecordBoundDrawPosTextH
template <> bool Draw::skip(const SkRecords::BoundedDrawPosTextH& r) {
    return fClipEmpty || fCanvas->quickRejectY(r.minY, r.maxY);
}

// These draw by proxying to the commands they wrap.  (All the optimization is for skip().)
#define DRAW(T) template <> void Draw::draw(const SkRecords::T& r) { this->draw(*r.base); }
DRAW(PairedPushCull);
DRAW(BoundedDrawPosTextH);
#undef DRAW

}  // namespace

void SkRecordDraw(const SkRecord& record, SkCanvas* canvas) {
    for (Draw draw(canvas); draw.index() < record.count(); draw.next()) {
        record.visit(draw.index(), draw);
    }
}

/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordDraw.h"

void SkRecordDraw(const SkRecord& record, SkCanvas* canvas, SkDrawPictureCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);
    for (SkRecords::Draw draw(canvas); draw.index() < record.count(); draw.next()) {
        if (NULL != callback && callback->abortDrawing()) {
            return;
        }
        record.visit<void>(draw.index(), draw);
    }
}

namespace SkRecords {

bool Draw::skip(const PairedPushCull& r) {
    if (fCanvas->quickReject(r.base->rect)) {
        fIndex += r.skip;
        return true;
    }
    return false;
}

bool Draw::skip(const BoundedDrawPosTextH& r) {
    return fCanvas->quickRejectY(r.minY, r.maxY);
}

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

template <> void Draw::draw(const PairedPushCull& r) { this->draw(*r.base); }
template <> void Draw::draw(const BoundedDrawPosTextH& r) { this->draw(*r.base); }

}  // namespace SkRecords

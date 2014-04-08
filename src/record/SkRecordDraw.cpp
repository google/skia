#include "SkRecordDraw.h"

namespace {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
struct Draw {
    unsigned index;
    SkCanvas* canvas;

    // No base case, so we'll be compile-time checked that we implemented all possibilities below.
    template <typename T> void operator()(const T&);
};

template <> void Draw::operator()(const SkRecords::PushCull& pushCull) {
    if (pushCull.popOffset != SkRecords::kUnsetPopOffset &&
        canvas->quickReject(pushCull.rect)) {
        // We skip to the popCull, then the loop moves us just beyond it.
        index += pushCull.popOffset;
    } else {
        canvas->pushCull(pushCull.rect);
    }
}

// Nothing fancy below here.

#define CASE(T) template <> void Draw::operator()(const SkRecords::T& r)

CASE(Restore) { canvas->restore(); }
CASE(Save) { canvas->save(r.flags); }
CASE(SaveLayer) { canvas->saveLayer(r.bounds, r.paint, r.flags); }

CASE(PopCull) { canvas->popCull(); }

CASE(Concat) { canvas->concat(r.matrix); }
CASE(SetMatrix) { canvas->setMatrix(r.matrix); }

CASE(ClipPath) { canvas->clipPath(r.path, r.op, r.doAA); }
CASE(ClipRRect) { canvas->clipRRect(r.rrect, r.op, r.doAA); }
CASE(ClipRect) { canvas->clipRect(r.rect, r.op, r.doAA); }
CASE(ClipRegion) { canvas->clipRegion(r.region, r.op); }

CASE(Clear) { canvas->clear(r.color); }
CASE(DrawBitmap) { canvas->drawBitmap(r.bitmap, r.left, r.top, r.paint); }
CASE(DrawBitmapMatrix) { canvas->drawBitmapMatrix(r.bitmap, r.matrix, r.paint); }
CASE(DrawBitmapNine) { canvas->drawBitmapNine(r.bitmap, r.center, r.dst, r.paint); }
CASE(DrawBitmapRectToRect) {
    canvas->drawBitmapRectToRect(r.bitmap, r.src, r.dst, r.paint, r.flags);
}
CASE(DrawDRRect) { canvas->drawDRRect(r.outer, r.inner, r.paint); }
CASE(DrawOval) { canvas->drawOval(r.oval, r.paint); }
CASE(DrawPaint) { canvas->drawPaint(r.paint); }
CASE(DrawPath) { canvas->drawPath(r.path, r.paint); }
CASE(DrawPoints) { canvas->drawPoints(r.mode, r.count, r.pts, r.paint); }
CASE(DrawPosText) { canvas->drawPosText(r.text, r.byteLength, r.pos, r.paint); }
CASE(DrawPosTextH) { canvas->drawPosTextH(r.text, r.byteLength, r.xpos, r.y, r.paint); }
CASE(DrawRRect) { canvas->drawRRect(r.rrect, r.paint); }
CASE(DrawRect) { canvas->drawRect(r.rect, r.paint); }
CASE(DrawSprite) { canvas->drawSprite(r.bitmap, r.left, r.top, r.paint); }
CASE(DrawText) { canvas->drawText(r.text, r.byteLength, r.x, r.y, r.paint); }
CASE(DrawTextOnPath) { canvas->drawTextOnPath(r.text, r.byteLength, r.path, r.matrix, r.paint); }
CASE(DrawVertices) {
    canvas->drawVertices(r.vmode, r.vertexCount, r.vertices, r.texs, r.colors,
                         r.xmode.get(), r.indices, r.indexCount, r.paint);
}
#undef CASE

}  // namespace

void SkRecordDraw(const SkRecord& record, SkCanvas* canvas) {
    Draw draw;
    draw.canvas = canvas;

    for (draw.index = 0; draw.index < record.count(); draw.index++) {
        record.visit(draw.index, draw);
    }
}



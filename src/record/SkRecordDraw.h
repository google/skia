#ifndef SkRecordDraw_DEFINED
#define SkRecordDraw_DEFINED

#include "SkRecord.h"
#include "SkRecords.h"
#include "SkCanvas.h"

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.

struct SkRecordDraw {
    explicit SkRecordDraw(SkCanvas* canvas) : canvas(canvas) {}

    // No base case, so we'll be compile-time checked that we implemented all possibilities below.
    template <typename T> void operator()(const T& record);

    SkCanvas* canvas;
};

// Nothing fancy here.
// The structs in SkRecord are completely isomorphic to their corresponding SkCanvas calls.

#define CASE(T) template <> void SkRecordDraw::operator()(const SkRecords::T& r)

CASE(Restore) { canvas->restore(); }
CASE(Save) { canvas->save(r.flags); }
CASE(SaveLayer) { canvas->saveLayer(r.bounds, r.paint, r.flags); }

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

#endif//SkRecordDraw_DEFINED

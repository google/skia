#include "SkRecordDraw.h"

namespace {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas) : fCanvas(canvas), fIndex(0), fClipEmpty(false) {}

    unsigned index() const { return fIndex; }
    void next() { ++fIndex; }

    // No base case, so we'll be compile-time checked that we implemented all possibilities below.
    template <typename T> void operator()(const T&);

private:
    // Must be called after any potential clip change.
    void updateClip() { fClipEmpty = fCanvas->isClipEmpty(); }

    SkCanvas* fCanvas;
    unsigned fIndex;
    bool fClipEmpty;
};

template <> void Draw::operator()(const SkRecords::PushCull& r) {
    if (r.popOffset != SkRecords::kUnsetPopOffset &&
        fCanvas->quickReject(r.rect)) {
        // We skip to the popCull, then the loop moves us just beyond it.
        fIndex += r.popOffset;
    } else {
        fCanvas->pushCull(r.rect);
    }
}

// These commands might change the clip.
#define CASE(T, call) \
    template <> void Draw::operator()(const SkRecords::T& r) { fCanvas->call; this->updateClip(); }
CASE(Restore, restore());
CASE(SaveLayer, saveLayer(r.bounds, r.paint, r.flags));
#undef CASE

// These certainly do change the clip,
// but we can skip them if they're intersecting with a clip that's already empty.
#define CASE(T, call)  template <> void Draw::operator()(const SkRecords::T& r) {                \
    if (!(fClipEmpty && SkRegion::kIntersect_Op == r.op)) { fCanvas->call; this->updateClip(); } \
}
CASE(ClipPath, clipPath(r.path, r.op, r.doAA));
CASE(ClipRRect, clipRRect(r.rrect, r.op, r.doAA));
CASE(ClipRect, clipRect(r.rect, r.op, r.doAA));
CASE(ClipRegion, clipRegion(r.region, r.op));
#undef CASE

// Commands which must run regardless of the clip, but don't change it themselves.
#define CASE(T, call) \
    template <> void Draw::operator()(const SkRecords::T& r) { fCanvas->call; }
CASE(Save, save(r.flags));
CASE(Clear, clear(r.color));
CASE(PopCull, popCull());
#undef CASE

// Nothing fancy below here.  These commands respect and don't change the clip.
#define CASE(T, call) \
    template <> void Draw::operator()(const SkRecords::T& r) { if (!fClipEmpty) fCanvas->call; }
CASE(Concat, concat(r.matrix));
CASE(SetMatrix, setMatrix(r.matrix));

CASE(DrawBitmap, drawBitmap(r.bitmap, r.left, r.top, r.paint));
CASE(DrawBitmapMatrix, drawBitmapMatrix(r.bitmap, r.matrix, r.paint));
CASE(DrawBitmapNine, drawBitmapNine(r.bitmap, r.center, r.dst, r.paint));
CASE(DrawBitmapRectToRect, drawBitmapRectToRect(r.bitmap, r.src, r.dst, r.paint, r.flags));
CASE(DrawDRRect, drawDRRect(r.outer, r.inner, r.paint));
CASE(DrawOval, drawOval(r.oval, r.paint));
CASE(DrawPaint, drawPaint(r.paint));
CASE(DrawPath, drawPath(r.path, r.paint));
CASE(DrawPoints, drawPoints(r.mode, r.count, r.pts, r.paint));
CASE(DrawPosText, drawPosText(r.text, r.byteLength, r.pos, r.paint));
CASE(DrawPosTextH, drawPosTextH(r.text, r.byteLength, r.xpos, r.y, r.paint));
CASE(DrawRRect, drawRRect(r.rrect, r.paint));
CASE(DrawRect, drawRect(r.rect, r.paint));
CASE(DrawSprite, drawSprite(r.bitmap, r.left, r.top, r.paint));
CASE(DrawText, drawText(r.text, r.byteLength, r.x, r.y, r.paint));
CASE(DrawTextOnPath, drawTextOnPath(r.text, r.byteLength, r.path, r.matrix, r.paint));
CASE(DrawVertices, drawVertices(r.vmode, r.vertexCount, r.vertices, r.texs, r.colors,
                                r.xmode.get(), r.indices, r.indexCount, r.paint));
#undef CASE

}  // namespace

void SkRecordDraw(const SkRecord& record, SkCanvas* canvas) {
    for (Draw draw(canvas); draw.index() < record.count(); draw.next()) {
        record.visit(draw.index(), draw);
    }
}

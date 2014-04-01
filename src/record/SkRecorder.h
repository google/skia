#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "SkCanvas.h"
#include "SkRecord.h"
#include "SkRecords.h"

// SkRecorder provides an SkCanvas interface for recording into an SkRecord.

class SkRecorder : public SkCanvas {
public:
    // Does not take ownership of the SkRecord.
    SkRecorder(SkRecord*, int width, int height);

    void clear(SkColor);
    void drawPaint(const SkPaint& paint);
    void drawPoints(PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint);
    void drawRect(const SkRect& rect, const SkPaint& paint);
    void drawOval(const SkRect& oval, const SkPaint&);
    void drawRRect(const SkRRect& rrect, const SkPaint& paint);
    void drawPath(const SkPath& path, const SkPaint& paint);
    void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                    const SkPaint* paint = NULL);
    void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                              const SkPaint* paint = NULL,
                              DrawBitmapRectFlags flags = kNone_DrawBitmapRectFlag);
    void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m, const SkPaint* paint = NULL);
    void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                        const SkPaint* paint = NULL);
    void drawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint = NULL);
    void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                  const SkPaint& paint);
    void drawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                     const SkPaint& paint);
    void drawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[], SkScalar constY,
                      const SkPaint& paint);
    void drawTextOnPath(const void* text, size_t byteLength,
                        const SkPath& path, const SkMatrix* matrix, const SkPaint& paint);
    void drawPicture(SkPicture& picture);
    void drawVertices(VertexMode vmode,
                      int vertexCount, const SkPoint vertices[],
                      const SkPoint texs[], const SkColor colors[],
                      SkXfermode* xmode,
                      const uint16_t indices[], int indexCount,
                      const SkPaint& paint);

    void willSave(SkCanvas::SaveFlags);
    SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SkCanvas::SaveFlags);
    void willRestore();

    void didConcat(const SkMatrix&);
    void didSetMatrix(const SkMatrix&);

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&);
    void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle);
    void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle);
    void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle);
    void onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op);

private:
    template <typename T>
    T* copy(const T*);

    template <typename T>
    T* copy(const T[], unsigned count);

    SkRecord* fRecord;
};

#endif//SkRecorder_DEFINED

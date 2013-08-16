#ifndef EXPERIMENTAL_PDFVIEWER_SKNULCANVAS_H_
#define EXPERIMENTAL_PDFVIEWER_SKNULCANVAS_H_

#include "SkCanvas.h"

class SK_API SkNulCanvas : public SkCanvas {
public:
    SK_DECLARE_INST_COUNT(SkNulCanvas);

    SkNulCanvas() {}
    explicit SkNulCanvas(SkDevice* device) : SkCanvas(device) {}

    explicit SkNulCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap) {}
    virtual ~SkNulCanvas() {}

    virtual int save(SaveFlags flags = kMatrixClip_SaveFlag) {return 0;}
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags = kARGB_ClipLayer_SaveFlag) {return 0;}
    int saveLayerAlpha(const SkRect* bounds, U8CPU alpha,
                       SaveFlags flags = kARGB_ClipLayer_SaveFlag) {return 0;}
    virtual void restore() {}
    int getSaveCount() const {return 0;}
    virtual bool isDrawingToLayer() const {return false;}
    virtual bool translate(SkScalar dx, SkScalar dy) {return true;}
    virtual bool scale(SkScalar sx, SkScalar sy) {return true;}
    virtual bool rotate(SkScalar degrees) {return true;}
    virtual bool skew(SkScalar sx, SkScalar sy) {return true;}
    virtual bool concat(const SkMatrix& matrix) {return true;}
    virtual void setMatrix(const SkMatrix& matrix) {}
    virtual bool clipRect(const SkRect& rect,
                          SkRegion::Op op = SkRegion::kIntersect_Op,
                          bool doAntiAlias = false) {return true;}
    virtual bool clipRRect(const SkRRect& rrect,
                           SkRegion::Op op = SkRegion::kIntersect_Op,
                           bool doAntiAlias = false) {return true;}
    virtual bool clipPath(const SkPath& path,
                          SkRegion::Op op = SkRegion::kIntersect_Op,
                          bool doAntiAlias = false) {return true;}
    virtual bool clipRegion(const SkRegion& deviceRgn,
                            SkRegion::Op op = SkRegion::kIntersect_Op) {return true;}
    virtual void clear(SkColor) {}
    virtual void drawPaint(const SkPaint& paint) {}
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) {}
    virtual void drawRect(const SkRect& rect, const SkPaint& paint) {}
    virtual void drawOval(const SkRect& oval, const SkPaint&) {}
    virtual void drawRRect(const SkRRect& rrect, const SkPaint& paint) {}
    virtual void drawPath(const SkPath& path, const SkPaint& paint) {}
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL) {}
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint,
                                      DrawBitmapRectFlags flags) {}
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint = NULL) {}
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint = NULL) {}
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint = NULL) {}
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint) {}
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) {}
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) {}
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) {}
    virtual void drawPicture(SkPicture& picture) {}
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {}
    virtual void drawData(const void* data, size_t length) {}
    virtual void beginCommentGroup(const char* description) {}
    virtual void addComment(const char* kywd, const char* value) {}
    virtual void endCommentGroup() {}
    virtual SkBounder* setBounder(SkBounder* bounder) {return NULL;}
    virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter) {return NULL;}

protected:
    virtual SkCanvas* canvasForDrawIter() {return NULL;}
    virtual SkDevice* setDevice(SkDevice* device) {return NULL;}

private:
    typedef SkCanvas INHERITED;
};

#endif  // EXPERIMENTAL_PDFVIEWER_SKNULCANVAS_H_

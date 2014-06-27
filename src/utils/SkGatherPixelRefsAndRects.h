/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGatherPixelRefsAndRects_DEFINED
#define SkGatherPixelRefsAndRects_DEFINED

#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkDraw.h"
#include "SkPictureUtils.h"
#include "SkRasterClip.h"
#include "SkRefCnt.h"
#include "SkRRect.h"
#include "SkTypes.h"

// This GatherPixelRefs device passes all discovered pixel refs and their
// device bounds to the user provided SkPixelRefContainer-derived object
class SkGatherPixelRefsAndRectsDevice : public SkBaseDevice {
public:
    SK_DECLARE_INST_COUNT(SkGatherPixelRefsAndRectsDevice)

    SkGatherPixelRefsAndRectsDevice(int width, int height,
                                    SkPictureUtils::SkPixelRefContainer* prCont) {
        fSize.set(width, height);
        fPRCont = prCont;
        SkSafeRef(fPRCont);
        fEmptyBitmap.setInfo(SkImageInfo::MakeUnknown(width, height));
    }

    virtual ~SkGatherPixelRefsAndRectsDevice() {
        SkSafeUnref(fPRCont);
    }

    virtual SkImageInfo imageInfo() const SK_OVERRIDE {
        return fEmptyBitmap.info();
    }

protected:
    virtual void clear(SkColor color) SK_OVERRIDE {
        NothingToDo();
    }
    virtual void drawPaint(const SkDraw& draw, const SkPaint& paint) SK_OVERRIDE {
        SkBitmap bm;

        if (GetBitmapFromPaint(paint, &bm)) {
            SkRect clipRect = SkRect::Make(draw.fRC->getBounds());
            fPRCont->add(bm.pixelRef(), clipRect);
        }
    }
    virtual void drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                            const SkPoint points[], const SkPaint& paint) SK_OVERRIDE {
        SkBitmap bm;
        if (!GetBitmapFromPaint(paint, &bm)) {
            return;
        }

        if (0 == count) {
            return;
        }

        SkPoint min = points[0];
        SkPoint max = points[0];
        for (size_t i = 1; i < count; ++i) {
            const SkPoint& point = points[i];

            min.set(SkMinScalar(min.x(), point.x()), SkMinScalar(min.y(), point.y()));
            max.set(SkMaxScalar(max.x(), point.x()), SkMaxScalar(max.y(), point.y()));
        }

        SkRect bounds = SkRect::MakeLTRB(min.x(), min.y(), max.x()+1, max.y()+1);

        this->drawRect(draw, bounds, paint);
    }
    virtual void drawRect(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) SK_OVERRIDE {
        SkBitmap bm;
        if (GetBitmapFromPaint(paint, &bm)) {
            SkRect mappedRect;
            draw.fMatrix->mapRect(&mappedRect, rect);
            SkRect clipRect = SkRect::Make(draw.fRC->getBounds());
            mappedRect.intersect(clipRect);
            fPRCont->add(bm.pixelRef(), mappedRect);
        }
    }
    virtual void drawOval(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) SK_OVERRIDE {
        this->drawRect(draw, rect, paint);
    }
    virtual void drawRRect(const SkDraw& draw, const SkRRect& rrect,
                           const SkPaint& paint) SK_OVERRIDE {
        this->drawRect(draw, rrect.rect(), paint);
    }
    virtual void drawPath(const SkDraw& draw, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE {
        SkBitmap bm;
        if (!GetBitmapFromPaint(paint, &bm)) {
            return;
        }

        SkRect pathBounds = path.getBounds();
        if (NULL != prePathMatrix) {
            prePathMatrix->mapRect(&pathBounds);
        }

        this->drawRect(draw, pathBounds, paint);
    }
    virtual void drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint& paint) SK_OVERRIDE {
        SkMatrix totMatrix;
        totMatrix.setConcat(*draw.fMatrix, matrix);

        SkRect bitmapRect = SkRect::MakeWH(SkIntToScalar(bitmap.width()),
                                           SkIntToScalar(bitmap.height()));
        SkRect mappedRect;
        totMatrix.mapRect(&mappedRect, bitmapRect);
        fPRCont->add(bitmap.pixelRef(), mappedRect);

        SkBitmap paintBitmap;
        if (GetBitmapFromPaint(paint, &paintBitmap)) {
            fPRCont->add(paintBitmap.pixelRef(), mappedRect);
        }
    }
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) SK_OVERRIDE {
        // Sprites aren't affected by current matrix, so we can't reuse drawRect.
        SkMatrix matrix;
        matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));

        SkRect bitmapRect = SkRect::MakeWH(SkIntToScalar(bitmap.width()),
                                           SkIntToScalar(bitmap.height()));
        SkRect mappedRect;
        matrix.mapRect(&mappedRect, bitmapRect);
        fPRCont->add(bitmap.pixelRef(), mappedRect);

        SkBitmap paintBitmap;
        if (GetBitmapFromPaint(paint, &paintBitmap)) {
            fPRCont->add(paintBitmap.pixelRef(), mappedRect);
        }
    }
    virtual void drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::DrawBitmapRectFlags flags) SK_OVERRIDE {
        SkRect bitmapRect = SkRect::MakeWH(SkIntToScalar(bitmap.width()),
                                           SkIntToScalar(bitmap.height()));
        SkMatrix matrix;
        matrix.setRectToRect(bitmapRect, dst, SkMatrix::kFill_ScaleToFit);
        this->drawBitmap(draw, bitmap, matrix, paint);
    }
    virtual void drawText(const SkDraw& draw, const void* text, size_t len,
                          SkScalar x, SkScalar y,
                          const SkPaint& paint) SK_OVERRIDE {
        SkBitmap bitmap;
        if (!GetBitmapFromPaint(paint, &bitmap)) {
            return;
        }

        // Math is borrowed from SkBBoxRecord
        SkRect bounds;
        paint.measureText(text, len, &bounds);
        SkPaint::FontMetrics metrics;
        paint.getFontMetrics(&metrics);

        if (paint.isVerticalText()) {
            SkScalar h = bounds.fBottom - bounds.fTop;
            if (paint.getTextAlign() == SkPaint::kCenter_Align) {
                bounds.fTop -= h / 2;
                bounds.fBottom -= h / 2;
            }
            bounds.fBottom += metrics.fBottom;
            bounds.fTop += metrics.fTop;
        } else {
            SkScalar w = bounds.fRight - bounds.fLeft;
            if (paint.getTextAlign() == SkPaint::kCenter_Align) {
                bounds.fLeft -= w / 2;
                bounds.fRight -= w / 2;
            } else if (paint.getTextAlign() == SkPaint::kRight_Align) {
                bounds.fLeft -= w;
                bounds.fRight -= w;
            }
            bounds.fTop = metrics.fTop;
            bounds.fBottom = metrics.fBottom;
        }

        SkScalar pad = (metrics.fBottom - metrics.fTop) / 2;
        bounds.fLeft -= pad;
        bounds.fRight += pad;
        bounds.offset(x, y);

        this->drawRect(draw, bounds, paint);
    }
    virtual void drawPosText(const SkDraw& draw, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint& paint) SK_OVERRIDE {
        SkBitmap bitmap;
        if (!GetBitmapFromPaint(paint, &bitmap)) {
            return;
        }

        if (0 == len) {
            return;
        }

        // Similar to SkDraw asserts.
        SkASSERT(scalarsPerPos == 1 || scalarsPerPos == 2);

        SkScalar y = scalarsPerPos == 1 ? constY : constY + pos[1];

        SkPoint min, max;
        min.set(pos[0], y);
        max.set(pos[0], y);

        for (size_t i = 1; i < len; ++i) {
            SkScalar x = pos[i * scalarsPerPos];
            SkScalar y = constY;
            if (2 == scalarsPerPos) {
                y += pos[i * scalarsPerPos + 1];
            }

            min.set(SkMinScalar(x, min.x()), SkMinScalar(y, min.y()));
            max.set(SkMaxScalar(x, max.x()), SkMaxScalar(y, max.y()));
        }

        SkRect bounds = SkRect::MakeLTRB(min.x(), min.y(), max.x(), max.y());

        // Math is borrowed from SkBBoxRecord
        SkPaint::FontMetrics metrics;
        paint.getFontMetrics(&metrics);

        bounds.fTop += metrics.fTop;
        bounds.fBottom += metrics.fBottom;

        SkScalar pad = (metrics.fTop - metrics.fBottom) / 2;
        bounds.fLeft -= pad;
        bounds.fRight += pad;

        this->drawRect(draw, bounds, paint);
    }
    virtual void drawTextOnPath(const SkDraw& draw, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE {
        SkBitmap bitmap;
        if (!GetBitmapFromPaint(paint, &bitmap)) {
            return;
        }

        // Math is borrowed from SkBBoxRecord
        SkRect bounds = path.getBounds();
        SkPaint::FontMetrics metrics;
        paint.getFontMetrics(&metrics);

        SkScalar pad = metrics.fTop;
        // TODO: inset?!
        bounds.fLeft += pad;
        bounds.fRight -= pad;
        bounds.fTop += pad;
        bounds.fBottom -= pad;

        this->drawRect(draw, bounds, paint);
    }
    virtual void drawVertices(const SkDraw& draw, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE {
        this->drawPoints(draw, SkCanvas::kPolygon_PointMode, vertexCount, verts, paint);
    }
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE {
        NothingToDo();
    }
    // TODO: allow this call to return failure, or move to SkBitmapDevice only.
    virtual const SkBitmap& onAccessBitmap() SK_OVERRIDE {
        return fEmptyBitmap;
    }
    virtual void lockPixels() SK_OVERRIDE { NothingToDo(); }
    virtual void unlockPixels() SK_OVERRIDE { NothingToDo(); }
    virtual bool allowImageFilter(const SkImageFilter*) SK_OVERRIDE { return false; }
    virtual bool canHandleImageFilter(const SkImageFilter*) SK_OVERRIDE { return false; }
    virtual bool filterImage(const SkImageFilter*, const SkBitmap&, const SkImageFilter::Context&,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE {
        return false;
    }

private:
    SkPictureUtils::SkPixelRefContainer* fPRCont;
    SkISize                              fSize;

    SkBitmap                             fEmptyBitmap; // legacy -- need to remove

    static bool GetBitmapFromPaint(const SkPaint &paint, SkBitmap* bitmap) {
        SkShader* shader = paint.getShader();
        if (NULL != shader) {
            if (SkShader::kNone_GradientType == shader->asAGradient(NULL)) {
                return SkShader::kNone_BitmapType != shader->asABitmap(bitmap, NULL, NULL);
            }
        }
        return false;
    }

    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) SK_OVERRIDE {
        NotSupported();
    }

    virtual SkBaseDevice* onCreateDevice(const SkImageInfo& info, Usage usage) SK_OVERRIDE {
        // we expect to only get called via savelayer, in which case it is fine.
        SkASSERT(kSaveLayer_Usage == usage);
        return SkNEW_ARGS(SkGatherPixelRefsAndRectsDevice,
                          (info.width(), info.height(), fPRCont));
    }

    static void NotSupported() {
        SkDEBUGFAIL("this method should never be called");
    }

    static void NothingToDo() {}

    typedef SkBaseDevice INHERITED;
};

#endif // SkGatherPixelRefsAndRects_DEFINED

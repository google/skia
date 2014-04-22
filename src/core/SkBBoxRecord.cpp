
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxRecord.h"

void SkBBoxRecord::drawOval(const SkRect& rect, const SkPaint& paint) {
    if (this->transformBounds(rect, &paint)) {
        INHERITED::drawOval(rect, paint);
    }
}

void SkBBoxRecord::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    if (this->transformBounds(rrect.rect(), &paint)) {
        INHERITED::drawRRect(rrect, paint);
    }
}

void SkBBoxRecord::drawRect(const SkRect& rect, const SkPaint& paint) {
    if (this->transformBounds(rect, &paint)) {
        INHERITED::drawRect(rect, paint);
    }
}

void SkBBoxRecord::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                const SkPaint& paint) {
    if (this->transformBounds(outer.rect(), &paint)) {
        this->INHERITED::onDrawDRRect(outer, inner, paint);
    }
}

void SkBBoxRecord::drawPath(const SkPath& path, const SkPaint& paint) {
    if (path.isInverseFillType()) {
        // If path is inverse filled, use the current clip bounds as the
        // path's device-space bounding box.
        SkIRect clipBounds;
        if (this->getClipDeviceBounds(&clipBounds)) {
            this->handleBBox(SkRect::Make(clipBounds));
            INHERITED::drawPath(path, paint);
        }
    } else if (this->transformBounds(path.getBounds(), &paint)) {
        INHERITED::drawPath(path, paint);
    }
}

void SkBBoxRecord::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                              const SkPaint& paint) {
    SkRect bbox;
    bbox.set(pts, SkToInt(count));
    // Small min width value, just to ensure hairline point bounding boxes aren't empty.
    // Even though we know hairline primitives are drawn one pixel wide, we do not use a
    // minimum of 1 because the playback scale factor is unknown at record time. Later
    // outsets will take care of adding additional padding for antialiasing and rounding out
    // to integer device coordinates, guaranteeing that the rasterized pixels will be included
    // in the computed bounds.
    // Note: The device coordinate outset in SkBBoxHierarchyRecord::handleBBox is currently
    // done in the recording coordinate space, which is wrong.
    // http://code.google.com/p/skia/issues/detail?id=1021
    static const SkScalar kMinWidth = 0.01f;
    SkScalar halfStrokeWidth = SkMaxScalar(paint.getStrokeWidth(), kMinWidth) / 2;
    bbox.outset(halfStrokeWidth, halfStrokeWidth);
    if (this->transformBounds(bbox, &paint)) {
        INHERITED::drawPoints(mode, count, pts, paint);
    }
}

void SkBBoxRecord::drawPaint(const SkPaint& paint) {
    SkRect bbox;
    if (this->getClipBounds(&bbox)) {
        if (this->transformBounds(bbox, &paint)) {
            INHERITED::drawPaint(paint);
        }
    }
}

void SkBBoxRecord::clear(SkColor color) {
    SkISize size = this->getDeviceSize();
    SkRect bbox = {0, 0, SkIntToScalar(size.width()), SkIntToScalar(size.height())};
    this->handleBBox(bbox);
    INHERITED::clear(color);
}

void SkBBoxRecord::drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    SkRect bbox;
    paint.measureText(text, byteLength, &bbox);
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);

    // Vertical and aligned text need to be offset
    if (paint.isVerticalText()) {
        SkScalar h = bbox.fBottom - bbox.fTop;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            bbox.fTop    -= h / 2;
            bbox.fBottom -= h / 2;
        }
        // Pad top and bottom with max extents from FontMetrics
        bbox.fBottom += metrics.fBottom;
        bbox.fTop += metrics.fTop;
    } else {
        SkScalar w = bbox.fRight - bbox.fLeft;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            bbox.fLeft  -= w / 2;
            bbox.fRight -= w / 2;
        } else if (paint.getTextAlign() == SkPaint::kRight_Align) {
            bbox.fLeft  -= w;
            bbox.fRight -= w;
        }
        // Set vertical bounds to max extents from font metrics
        bbox.fTop = metrics.fTop;
        bbox.fBottom = metrics.fBottom;
    }

    // Pad horizontal bounds on each side by half of max vertical extents (this is sort of
    // arbitrary, but seems to produce reasonable results, if there were a way of getting max
    // glyph X-extents to pad by, that may be better here, but FontMetrics fXMin and fXMax seem
    // incorrect on most platforms (too small in Linux, never even set in Windows).
    SkScalar pad = (metrics.fBottom - metrics.fTop) / 2;
    bbox.fLeft  -= pad;
    bbox.fRight += pad;

    bbox.fLeft += x;
    bbox.fRight += x;
    bbox.fTop += y;
    bbox.fBottom += y;
    if (this->transformBounds(bbox, &paint)) {
        INHERITED::drawText(text, byteLength, x, y, paint);
    }
}

void SkBBoxRecord::drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                              const SkPaint* paint) {
    SkRect bbox = {left, top, left + bitmap.width(), top + bitmap.height()};
    if (this->transformBounds(bbox, paint)) {
        INHERITED::drawBitmap(bitmap, left, top, paint);
    }
}

void SkBBoxRecord::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                        const SkRect& dst, const SkPaint* paint,
                                        DrawBitmapRectFlags flags) {
    if (this->transformBounds(dst, paint)) {
        INHERITED::drawBitmapRectToRect(bitmap, src, dst, paint, flags);
    }
}

void SkBBoxRecord::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& mat,
                                    const SkPaint* paint) {
    SkMatrix m = mat;
    SkRect bbox = {0, 0, SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height())};
    m.mapRect(&bbox);
    if (this->transformBounds(bbox, paint)) {
        INHERITED::drawBitmapMatrix(bitmap, mat, paint);
    }
}

void SkBBoxRecord::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                  const SkRect& dst, const SkPaint* paint) {
    if (this->transformBounds(dst, paint)) {
        INHERITED::drawBitmapNine(bitmap, center, dst, paint);
    }
}

void SkBBoxRecord::drawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint) {
    SkRect bbox;
    bbox.set(pos, paint.countText(text, byteLength));
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);
    bbox.fTop += metrics.fTop;
    bbox.fBottom += metrics.fBottom;

    // pad on left and right by half of max vertical glyph extents
    SkScalar pad = (metrics.fTop - metrics.fBottom) / 2;
    bbox.fLeft += pad;
    bbox.fRight -= pad;

    if (this->transformBounds(bbox, &paint)) {
        INHERITED::drawPosText(text, byteLength, pos, paint);
    }
}

void SkBBoxRecord::drawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                SkScalar constY, const SkPaint& paint) {
    size_t numChars = paint.countText(text, byteLength);
    if (numChars == 0) {
        return;
    }

    const SkFlatData* flatPaintData = this->getFlatPaintData(paint);
    WriteTopBot(paint, *flatPaintData);

    SkScalar top = flatPaintData->topBot()[0];
    SkScalar bottom = flatPaintData->topBot()[1];
    SkScalar pad = top - bottom;

    SkRect bbox;
    bbox.fLeft = SK_ScalarMax;
    bbox.fRight = SK_ScalarMin;

    for (size_t i = 0; i < numChars; ++i) {
        if (xpos[i] < bbox.fLeft) {
            bbox.fLeft = xpos[i];
        }
        if (xpos[i] > bbox.fRight) {
            bbox.fRight = xpos[i];
        }
    }

    // pad horizontally by max glyph height
    bbox.fLeft  += pad;
    bbox.fRight -= pad;

    bbox.fTop    = top + constY;
    bbox.fBottom = bottom + constY;

    if (!this->transformBounds(bbox, &paint)) {
        return;
    }
    // This is the equivalent of calling:
    //  INHERITED::drawPosTextH(text, byteLength, xpos, constY, paint);
    // but we filled our flat paint beforehand so that we could get font metrics.
    drawPosTextHImpl(text, byteLength, xpos, constY, paint, flatPaintData);
}

void SkBBoxRecord::drawSprite(const SkBitmap& bitmap, int left, int top,
                              const SkPaint* paint) {
    SkRect bbox;
    bbox.set(SkIRect::MakeXYWH(left, top, bitmap.width(), bitmap.height()));
    this->handleBBox(bbox); // directly call handleBBox, matrix is ignored
    INHERITED::drawSprite(bitmap, left, top, paint);
}

void SkBBoxRecord::drawTextOnPath(const void* text, size_t byteLength,
                                  const SkPath& path, const SkMatrix* matrix,
                                  const SkPaint& paint) {
    SkRect bbox = path.getBounds();
    SkPaint::FontMetrics metrics;
    paint.getFontMetrics(&metrics);

    // pad out all sides by the max glyph height above baseline
    SkScalar pad = metrics.fTop;
    bbox.fLeft += pad;
    bbox.fRight -= pad;
    bbox.fTop += pad;
    bbox.fBottom -= pad;

    if (this->transformBounds(bbox, &paint)) {
        INHERITED::drawTextOnPath(text, byteLength, path, matrix, paint);
    }
}

void SkBBoxRecord::drawVertices(VertexMode mode, int vertexCount,
                                const SkPoint vertices[], const SkPoint texs[],
                                const SkColor colors[], SkXfermode* xfer,
                                const uint16_t indices[], int indexCount,
                                const SkPaint& paint) {
    SkRect bbox;
    bbox.set(vertices, vertexCount);
    if (this->transformBounds(bbox, &paint)) {
        INHERITED::drawVertices(mode, vertexCount, vertices, texs,
                                colors, xfer, indices, indexCount, paint);
    }
}

void SkBBoxRecord::drawPicture(SkPicture& picture) {
    if (picture.width() > 0 && picture.height() > 0 &&
        this->transformBounds(SkRect::MakeWH(picture.width(), picture.height()), NULL)) {
        INHERITED::drawPicture(picture);
    }
}

bool SkBBoxRecord::transformBounds(const SkRect& bounds, const SkPaint* paint) {
    SkRect outBounds = bounds;
    outBounds.sort();

    if (paint) {
        // account for stroking, path effects, shadows, etc
        if (paint->canComputeFastBounds()) {
            SkRect temp;
            outBounds = paint->computeFastBounds(outBounds, &temp);
        } else {
            // set bounds to current clip
            if (!this->getClipBounds(&outBounds)) {
                // current clip is empty
                return false;
            }
        }
    }

    if (!outBounds.isEmpty() && !this->quickReject(outBounds)) {
        this->getTotalMatrix().mapRect(&outBounds);
        this->handleBBox(outBounds);
        return true;
    }

    return false;
}

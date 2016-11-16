/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkOverdrawDevice.h"
#include "SkSpecialImage.h"

class ProcessOneGlyphBounds {
public:
    ProcessOneGlyphBounds(SkOverdrawDevice* device, const SkDraw& draw)
        : fDevice(device)
        , fDraw(draw)
    {}

    void operator()(const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
        int left = SkScalarFloorToInt(position.fX) + glyph.fLeft;
        int top = SkScalarFloorToInt(position.fY) + glyph.fTop;
        int right = left + glyph.fWidth;
        int bottom = top + glyph.fHeight;
        fDevice->drawRect(fDraw, SkRect::MakeLTRB(left, top, right, bottom), SkPaint());
    }

private:
    SkOverdrawDevice* fDevice;
    const SkDraw&     fDraw;
};

sk_sp<SkGpuDevice> SkOverdrawDevice::Make(GrContext* context, SkBudgeted budgeted,
                                          const SkImageInfo& info, int sampleCount,
                                          GrSurfaceOrigin origin,
                                          const SkSurfaceProps* props, InitContents init) {
    unsigned flags;
    if (!SkGpuDevice::CheckAlphaTypeAndGetFlags(&info, init, &flags)) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(MakeRenderTargetContext(context, budgeted,
                                                                             info, sampleCount,
                                                                             origin, props));
    if (!renderTargetContext) {
        return nullptr;
    }

    return sk_sp<SkGpuDevice>(new SkOverdrawDevice(std::move(renderTargetContext),
                                                   info.width(), info.height(), flags));
}

SkOverdrawDevice::SkOverdrawDevice(sk_sp<GrRenderTargetContext> ctx, int width, int height,
                                   unsigned flags)
    : INHERITED(std::move(ctx), width, height, flags)
{
    static constexpr float kIncrementAlpha[] = {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    fPaint.setAntiAlias(false);
    fPaint.setBlendMode(SkBlendMode::kPlus);
    fPaint.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(kIncrementAlpha));
}

void SkOverdrawDevice::drawPaint(const SkDraw& draw, const SkPaint&) {
    INHERITED::drawPaint(draw, fPaint);
}

void SkOverdrawDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                                  const SkPoint points[], const SkPaint&) {
    INHERITED::drawPoints(draw, mode, count, points, fPaint);
}

void SkOverdrawDevice::drawRect(const SkDraw& draw, const SkRect& rect, const SkPaint&) {
    INHERITED::drawRect(draw, rect, fPaint);
}

void SkOverdrawDevice::drawRRect(const SkDraw& draw, const SkRRect& rect, const SkPaint&) {
    INHERITED::drawRRect(draw, rect, fPaint);
}

void SkOverdrawDevice::drawDRRect(const SkDraw& draw, const SkRRect& outer, const SkRRect& inner,
                                  const SkPaint&) {
    INHERITED::drawDRRect(draw, outer, inner, fPaint);
}

void SkOverdrawDevice::drawRegion(const SkDraw& draw, const SkRegion& region, const SkPaint&) {
    INHERITED::drawRegion(draw, region, fPaint);
}

void SkOverdrawDevice::drawOval(const SkDraw& draw, const SkRect& oval, const SkPaint&) {
    INHERITED::drawOval(draw, oval, fPaint);
}

void SkOverdrawDevice::drawArc(const SkDraw& draw, const SkRect& oval, SkScalar startAngle,
                               SkScalar sweepAngle, bool useCenter, const SkPaint&) {
    INHERITED::drawArc(draw, oval, startAngle, sweepAngle, useCenter, fPaint);
}

void SkOverdrawDevice::drawPath(const SkDraw& draw, const SkPath& path, const SkPaint&,
                                const SkMatrix* prePathMatrix, bool pathIsMutable) {
    INHERITED::drawPath(draw, path, fPaint, prePathMatrix, pathIsMutable);
}

void SkOverdrawDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bm, const SkMatrix& matrix,
                                  const SkPaint&) {
    SkRect srcRect = SkRect::MakeXYWH(0.0f, 0.0f, bm.width(), bm.height());
    SkRect dstRect;
    matrix.mapRect(&dstRect, srcRect);
    INHERITED::drawRect(draw, dstRect, fPaint);
}

void SkOverdrawDevice::drawBitmapRect(const SkDraw& draw, const SkBitmap&, const SkRect* srcRect,
                                      const SkRect& dstRect, const SkPaint&,
                                      SkCanvas::SrcRectConstraint) {
    INHERITED::drawRect(draw, dstRect, fPaint);
}

void SkOverdrawDevice::drawSprite(const SkDraw& draw, const SkBitmap& bm, int x, int y,
                                  const SkPaint&) {
    INHERITED::drawRect(draw, SkRect::MakeXYWH(x, y, bm.width(), bm.height()), fPaint);
}

void SkOverdrawDevice::drawBitmapNine(const SkDraw& draw, const SkBitmap&, const SkIRect&,
                                      const SkRect& dst, const SkPaint&) {
    INHERITED::drawRect(draw, dst, fPaint);
}

void SkOverdrawDevice::drawBitmapLattice(const SkDraw& draw, const SkBitmap&,
                                         const SkCanvas::Lattice&, const SkRect& dst,
                                         const SkPaint&) {
    INHERITED::drawRect(draw, dst, fPaint);
}

void SkOverdrawDevice::drawImage(const SkDraw& draw, const SkImage* image, SkScalar x, SkScalar y,
                                 const SkPaint&) {
    INHERITED::drawRect(draw, SkRect::MakeXYWH(x, y, image->width(), image->height()), fPaint);
}

void SkOverdrawDevice::drawImageRect(const SkDraw& draw, const SkImage*, const SkRect*,
                                     const SkRect& dst, const SkPaint&,
                                     SkCanvas::SrcRectConstraint constraint) {
    INHERITED::drawRect(draw, dst, fPaint);
}

void SkOverdrawDevice::drawImageNine(const SkDraw& draw, const SkImage*, const SkIRect&,
                                     const SkRect& dst, const SkPaint&) {
    INHERITED::drawRect(draw, dst, fPaint);
}

void SkOverdrawDevice::drawImageLattice(const SkDraw& draw, const SkImage*, const SkCanvas::Lattice&,
                                        const SkRect& dst, const SkPaint&) {
    INHERITED::drawRect(draw, dst, fPaint);
}

void SkOverdrawDevice::drawSpecial(const SkDraw& draw, SkSpecialImage* image, int x, int y,
                                   const SkPaint&) {
    INHERITED::drawRect(draw, SkRect::MakeXYWH(x, y, image->width(), image->height()), fPaint);
}

void SkOverdrawDevice::drawText(const SkDraw& draw, const void* text, size_t byteLength, SkScalar x,
                                SkScalar y, const SkPaint& paint) {
    ProcessOneGlyphBounds processBounds(this, draw);
    SkAutoGlyphCache cache(paint, &this->surfaceProps(), 0, draw.fMatrix);
    SkFindAndPlaceGlyph::ProcessText(paint.getTextEncoding(), (const char*) text, byteLength,
                                     SkPoint::Make(x, y), SkMatrix(), paint.getTextAlign(),
                                     cache.get(), processBounds);
}

void SkOverdrawDevice::drawPosText(const SkDraw& draw, const void* text, size_t byteLength,
                                   const SkScalar pos[], int scalarsPerPos, const SkPoint& offset,
                                   const SkPaint& paint) {
    ProcessOneGlyphBounds processBounds(this, draw);
    SkAutoGlyphCache cache(paint, &this->surfaceProps(), 0, draw.fMatrix);
    SkFindAndPlaceGlyph::ProcessPosText(paint.getTextEncoding(), (const char*) text, byteLength,
                                        offset, SkMatrix(), pos, scalarsPerPos,
                                        paint.getTextAlign(), cache.get(), processBounds);
}

void SkOverdrawDevice::drawTextBlob(const SkDraw& draw, const SkTextBlob* blob, SkScalar x,
                                    SkScalar y, const SkPaint& paint, SkDrawFilter* drawFilter) {
    SkBaseDevice::drawTextBlob(draw, blob, x, y, paint, drawFilter);
}

void SkOverdrawDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode mode, int vertCount,
                                    const SkPoint verts[], const SkPoint texs[],
                                    const SkColor colors[], SkBlendMode blendMode,
                                    const uint16_t indices[], int indexCount,
                                    const SkPaint&) {
    INHERITED::drawVertices(draw, mode, vertCount, verts, texs, colors, blendMode, indices,
                            indexCount, fPaint);
}

void SkOverdrawDevice::drawAtlas(const SkDraw& draw, const SkImage* atlas, const SkRSXform xform[],
                                 const SkRect tex[], const SkColor colors[], int count,
                                 SkBlendMode mode, const SkPaint& paint) {
    SkBaseDevice::drawAtlas(draw, atlas, xform, tex, colors, count, mode, paint);
}

void SkOverdrawDevice::drawDevice(const SkDraw& draw, SkBaseDevice* device, int x, int y,
                                  const SkPaint&) {
    INHERITED::drawRect(draw, SkRect::MakeXYWH(x, y, device->width(), device->height()), fPaint);
}

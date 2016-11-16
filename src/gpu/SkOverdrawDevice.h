/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef OverdrawDevice_DEFINED
#define OverdrawDevice_DEFINED

#include "SkGpuDevice.h"

/**
 *  Captures all drawing commands.  Rather than draw the actual content, this device
 *  increments the alpha channel of each pixel every time it would have been touched
 *  by a draw call.  This is useful for detecting overdraw.
 *  This needs to override all of the draw calls that are overridden by SkGpuDevice.
 */
class SK_API SkOverdrawDevice : public SkGpuDevice {
public:
    static sk_sp<SkGpuDevice> Make(GrContext*, SkBudgeted, const SkImageInfo&,
                                   int sampleCount, GrSurfaceOrigin,
                                   const SkSurfaceProps*, InitContents);

protected:
    void drawPaint(const SkDraw&, const SkPaint&) override;
    void drawPoints(const SkDraw&, SkCanvas::PointMode, size_t, const SkPoint[],
                    const SkPaint&) override;
    void drawRect(const SkDraw&, const SkRect&, const SkPaint&) override;
    void drawRRect(const SkDraw&, const SkRRect&, const SkPaint&) override;
    void drawDRRect(const SkDraw&, const SkRRect&, const SkRRect&, const SkPaint&) override;
    void drawRegion(const SkDraw&, const SkRegion&, const SkPaint&) override;
    void drawOval(const SkDraw&, const SkRect&, const SkPaint&) override;
    void drawArc(const SkDraw&, const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void drawPath(const SkDraw&, const SkPath& path, const SkPaint&,
                  const SkMatrix*, bool pathIsMutable) override;
    void drawBitmap(const SkDraw&, const SkBitmap&, const SkMatrix&, const SkPaint&) override;
    void drawBitmapRect(const SkDraw&, const SkBitmap&, const SkRect*, const SkRect&,
                        const SkPaint&, SkCanvas::SrcRectConstraint) override;
    void drawSprite(const SkDraw&, const SkBitmap&, int, int, const SkPaint&) override;
    void drawBitmapNine(const SkDraw&, const SkBitmap&, const SkIRect&, const SkRect&,
                        const SkPaint&) override;
    void drawBitmapLattice(const SkDraw&, const SkBitmap&, const SkCanvas::Lattice&, const SkRect&,
                           const SkPaint&) override;
    void drawImage(const SkDraw&, const SkImage*, SkScalar, SkScalar, const SkPaint&) override;
    void drawImageRect(const SkDraw&, const SkImage*, const SkRect*, const SkRect&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;
    void drawImageNine(const SkDraw&, const SkImage*, const SkIRect&, const SkRect&,
                       const SkPaint&) override;
    void drawImageLattice(const SkDraw&, const SkImage*, const SkCanvas::Lattice&, const SkRect&,
                          const SkPaint&) override;
    void drawSpecial(const SkDraw&, SkSpecialImage*, int, int, const SkPaint&) override;
    void drawText(const SkDraw&, const void*, size_t, SkScalar, SkScalar, const SkPaint&) override;
    void drawPosText(const SkDraw&, const void*, size_t, const SkScalar[], int, const SkPoint&,
                     const SkPaint&) override;
    void drawTextBlob(const SkDraw&, const SkTextBlob*, SkScalar, SkScalar, const SkPaint&,
                      SkDrawFilter*) override;
    void drawVertices(const SkDraw&, SkCanvas::VertexMode, int, const SkPoint[], const SkPoint[],
                      const SkColor[], SkBlendMode, const uint16_t[], int, const SkPaint&) override;
    void drawAtlas(const SkDraw&, const SkImage*, const SkRSXform[], const SkRect[],
                   const SkColor[], int, SkBlendMode, const SkPaint&) override;
    void drawDevice(const SkDraw&, SkBaseDevice*, int, int, const SkPaint&) override;

private:
    SkOverdrawDevice(sk_sp<GrRenderTargetContext> ctx, int width, int height, unsigned flags);

    SkPaint fPaint;

    friend class ProcessOneGlyphBounds;
    typedef SkGpuDevice INHERITED;
};

#endif

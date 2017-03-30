/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadedBMPDevice_DEFINED
#define SkThreadedBMPDevice_DEFINED

#include "SkDraw.h"
#include "SkBitmapDevice.h"

///////////////////////////////////////////////////////////////////////////////
class SkThreadedBMPDevice : public SkBitmapDevice {
public:
    SkThreadedBMPDevice(const SkBitmap& bitmap, int threads);

protected:
    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;

    void drawPath(const SkPath&, const SkPaint&, const SkMatrix* prePathMatrix,
                  bool pathIsMutable) override;
    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkPaint&) override;
    void drawSprite(const SkBitmap&, int x, int y, const SkPaint&) override;

    void drawText(const void* text, size_t len, SkScalar x, SkScalar y,
                  const SkPaint&) override;
    void drawPosText(const void* text, size_t len, const SkScalar pos[],
                     int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void drawDevice(SkBaseDevice*, int x, int y, const SkPaint&) override;

    void flush() override;

private:
    struct DrawElement {
        SkIRect fDrawBounds;
        std::function<void(const SkIRect& threadBounds)> fDrawFn;
    };

    struct DrawState;

    SkIRect transformDrawBounds(const SkRect& drawBounds) const;

    const int fThreadCnt;
    SkTArray<SkIRect> fThreadBounds;
    SkTArray<DrawElement> fQueue;

    typedef SkBitmapDevice INHERITED;
};

#endif // SkThreadedBMPDevice_DEFINED

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
class SK_API SkThreadedBMPDevice : public SkBitmapDevice {
public:
    /**
     *  Construct a new device with the specified bitmap as its backend. It is
     *  valid for the bitmap to have no pixels associated with it. In that case,
     *  any drawing to this device will have no effect.
     */
    SkThreadedBMPDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps,
                   void* externalHandle = nullptr);

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
        // As mtklein@ suggested, we may later have std::atomic<std::funciton*> fInitFn
    };

    // Having this captured in lambda seems to be faster than saving this in DrawElement
    struct DrawState {
        SkPixmap fDst;
        SkMatrix fMatrix;
        SkRasterClip fRC;

        DrawState(SkThreadedBMPDevice* dev) {
            dev->setDrawDst(fDst);
            fMatrix = dev->ctm();
            fRC = dev->fRCStack.rc();
        }

        SK_ALWAYS_INLINE
        SkDraw getThreadDraw(SkRasterClip& threadRC, const SkIRect& threadBounds) const {
            SkDraw draw;
            draw.fDst = fDst;
            draw.fMatrix = &fMatrix;
            threadRC = fRC;
            threadRC.op(threadBounds, SkRegion::kIntersect_Op);
            draw.fRC = &threadRC;
            return draw;
        }
    };

    inline SkIRect transformDrawBounds(const SkRect& drawBounds) const {
        if (drawBounds.isLargest())
            return SkIRect::MakeLargest();
        SkRect transformedBounds;
        this->ctm().mapRect(&transformedBounds, drawBounds);
        return transformedBounds.roundOut();
    }

    const int fThreadCnt;
    SkTArray<SkIRect> fThreadBounds;
    SkTArray<DrawElement> fQueue;

    typedef SkBitmapDevice INHERITED;
};

#endif // SkThreadedBMPDevice_DEFINED

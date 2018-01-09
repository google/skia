/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadedBMPDevice_DEFINED
#define SkThreadedBMPDevice_DEFINED

#include "SkBitmapDevice.h"
#include "SkDraw.h"
#include "SkTaskGroup2D.h"

class SkThreadedBMPDevice : public SkBitmapDevice {
public:
    // When threads = 0, we make fThreadCnt = tiles. Otherwise fThreadCnt = threads.
    // When executor = nullptr, we manages the thread pool. Otherwise, the caller manages it.
    SkThreadedBMPDevice(const SkBitmap& bitmap, int tiles, int threads = 0,
                        SkExecutor* executor = nullptr);

    ~SkThreadedBMPDevice() override { fQueue.finish(); }

protected:
    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;

    void drawPath(const SkPath&, const SkPaint&, const SkMatrix* prePathMatrix,
                  bool pathIsMutable) override;
    void drawBitmap(const SkBitmap&, SkScalar x, SkScalar y, const SkPaint&) override;
    void drawSprite(const SkBitmap&, int x, int y, const SkPaint&) override;

    void drawText(const void* text, size_t len, SkScalar x, SkScalar y,
                  const SkPaint&) override;
    void drawPosText(const void* text, size_t len, const SkScalar pos[],
                     int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void drawDevice(SkBaseDevice*, int x, int y, const SkPaint&) override;

    void flush() override;

private:
    struct DrawState {
        SkPixmap fDst;
        SkMatrix fMatrix;
        SkRasterClip fRC;

        DrawState() {}
        explicit DrawState(SkThreadedBMPDevice* dev);

        SkDraw getDraw() const;
    };

    class TileDraw : public SkDraw {
        public: TileDraw(const DrawState& ds, const SkIRect& tileBounds);
        private: SkRasterClip fTileRC;
    };

    struct DrawElement {
        using DrawFn = std::function<void(SkArenaAlloc* threadAlloc, const DrawState& ds,
                                          const SkIRect& tileBounds)>;

        DrawFn      fDrawFn;
        DrawState   fDS;
        SkIRect     fDrawBounds;
    };

    class DrawQueue {
    public:
        static constexpr int MAX_QUEUE_SIZE = 100000;

        DrawQueue(SkThreadedBMPDevice* device) : fDevice(device) {}
        void reset();

        // For ~SkThreadedBMPDevice() to shutdown tasks, we use this instead of reset because reset
        // will start new tasks.
        void finish() { fTasks->finish(); }

        SK_ALWAYS_INLINE void push(const SkRect& rawDrawBounds,
                                   DrawElement::DrawFn&& drawFn) {
            if (fSize == MAX_QUEUE_SIZE) {
                this->reset();
            }
            SkASSERT(fSize < MAX_QUEUE_SIZE);

            DrawElement* element = &fElements[fSize++];
            element->fDS = DrawState(fDevice);
            element->fDrawFn = std::move(drawFn);
            element->fDrawBounds = fDevice->transformDrawBounds(rawDrawBounds);
            fTasks->addColumn();
        }

    private:
        SkThreadedBMPDevice*            fDevice;
        std::unique_ptr<SkTaskGroup2D>  fTasks;
        DrawElement                     fElements[MAX_QUEUE_SIZE];
        int                             fSize;
    };

    SkIRect transformDrawBounds(const SkRect& drawBounds) const;

    const int fTileCnt;
    const int fThreadCnt;
    SkTArray<SkIRect> fTileBounds;

    /**
     * This can either be
     * 1. fInternalExecutor.get() which means that we're managing the thread pool's life cycle.
     * 2. provided by our caller which means that our caller is managing the threads' life cycle.
     * In the 2nd case, fInternalExecutor == nullptr.
     */
    SkExecutor* fExecutor = nullptr;
    std::unique_ptr<SkExecutor> fInternalExecutor;

    DrawQueue fQueue;

    typedef SkBitmapDevice INHERITED;
};

#endif // SkThreadedBMPDevice_DEFINED

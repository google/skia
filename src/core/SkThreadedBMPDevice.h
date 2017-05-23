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

#include <future>
#include <condition_variable>
#include <mutex>

class TiledDrawScheduler {
public:
    TiledDrawScheduler(int tiles) : fTileCnt(tiles), fIsFinishing(false), fDrawCnt(0) {
        for(int i = 0; i < tiles; ++i) {
            fDrawIndexes.push_back(0);
        }
    }
    virtual ~TiledDrawScheduler() {}

    virtual void signal() = 0; // signal that one more draw is available for all tiles

    // Wait until a draw is available at some tile.
    // Some scheduler may be able to pick the tile while others see tileIndex as given and fixed.
    // We ensure that for each tile, the drawIndex returned is increasing over time
    // (so our draw order is correct for each tile).
    // When returned false, we have no more draw and the threads are finishing.
    virtual bool waitForNext(int& tileIndex, int& drawIndex) = 0;

    // A single draw command finished on a tile. This is used to release the lock
    // in some scheduler.
    virtual void onSingleDrawFinished(int tileIndex) {}

    virtual void finish() = 0;

protected:
    const int fTileCnt;
    std::atomic<bool> fIsFinishing;
    std::atomic<int> fDrawCnt;
    SkTArray<int> fDrawIndexes; // next draw index for each tile
};

///////////////////////////////////////////////////////////////////////////////
class SkThreadedBMPDevice : public SkBitmapDevice {
public:
    // When threads = 0, we make fThreadCnt = fTileCnt
    SkThreadedBMPDevice(const SkBitmap& bitmap, int tiles, int threads = 0);
    ~SkThreadedBMPDevice() { finishThreads(); }

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

    void startThreads();
    void finishThreads();

    static constexpr int MAX_QUEUE_SIZE = 100000;

    const int fTileCnt;
    const int fThreadCnt;
    std::unique_ptr<TiledDrawScheduler> fScheduler;
    SkTArray<SkIRect> fTileBounds;
    SkTArray<std::future<void>> fThreadFutures;
    DrawElement fQueue[MAX_QUEUE_SIZE];
    int fQueueSize;

    typedef SkBitmapDevice INHERITED;
};

#endif // SkThreadedBMPDevice_DEFINED

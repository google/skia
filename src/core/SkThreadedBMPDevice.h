/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadedBMPDevice_DEFINED
#define SkThreadedBMPDevice_DEFINED

#include "SkTaskGroup.h"
#include "SkDraw.h"
#include "SkBitmapDevice.h"

class TiledDrawScheduler {
public:
    using WorkFunc = std::function<void(int, int)>;

    virtual ~TiledDrawScheduler() {}

    virtual void signal() = 0; // signal that one more draw is available for all tiles

    // Tell scheduler that no more draw calls will be added (no signal will be called).
    virtual void finish() = 0;

    // Handle the next draw available. This method will block until
    //   (1) the next draw is finished, or
    //   (2) the finish is called
    // The method will return true for case (1) and false for case (2).
    // When there's no draw available and we haven't called finish, we will just wait.
    // In many cases, the parameter tileIndex specifies the tile that the next draw should happen.
    // However, for some schedulers, that tileIndex may only be a hint and the scheduler is free
    // to find another tile to draw. In that case, tileIndex will be changed to the actual tileIndex
    // where the draw happens.
    virtual bool next(int& tileIndex) = 0;
};

///////////////////////////////////////////////////////////////////////////////
class SkThreadedBMPDevice : public SkBitmapDevice {
public:
    // When threads = 0, we make fThreadCnt = tiles. Otherwise fThreadCnt = threads.
    // When executor = nullptr, we manages the thread pool. Otherwise, the caller manages it.
    SkThreadedBMPDevice(const SkBitmap& bitmap, int tiles, int threads = 0,
                        SkExecutor* executor = nullptr);

    ~SkThreadedBMPDevice() override { finishThreads(); }

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

    /**
     * This can either be
     * 1. fInternalExecutor.get() which means that we're managing the thread pool's life cycle.
     * 2. provided by our caller which means that our caller is managing the threads' life cycle.
     * In the 2nd case, fInternalExecutor == nullptr.
     */
    SkExecutor* fExecutor = nullptr;
    std::unique_ptr<SkExecutor> fInternalExecutor;

    std::unique_ptr<SkTaskGroup> fTaskGroup; // generated from fExecutor

    DrawElement fQueue[MAX_QUEUE_SIZE];
    int fQueueSize;

    typedef SkBitmapDevice INHERITED;
};

#endif // SkThreadedBMPDevice_DEFINED

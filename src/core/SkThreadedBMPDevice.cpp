/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedBMPDevice.h"

#include "SkPath.h"
#include "SkTaskGroup.h"
#include "SkVertices.h"

#include <mutex>
#include <vector>

constexpr int MAX_CACHE_LINE = 64;

// Some basic logics and data structures that are shared across the current experimental schedulers.
class TiledDrawSchedulerBase : public TiledDrawScheduler {
public:
    TiledDrawSchedulerBase(int tiles, WorkFunc work)
            : fTileCnt(tiles), fIsFinishing(false), fDrawCnt(0), fWork(std::move(work)) {}

    void signal() override {
        fDrawCnt++;
    }
    void finish() override {
        fIsFinishing.store(true, std::memory_order_relaxed);
    }

protected:
    const int                   fTileCnt;
    std::atomic<bool>           fIsFinishing;
    std::atomic<int>            fDrawCnt;
    WorkFunc                    fWork;
};

class TiledDrawSchedulerBySpinning : public TiledDrawSchedulerBase {
public:
    TiledDrawSchedulerBySpinning(int tiles, WorkFunc work)
            : TiledDrawSchedulerBase(tiles, std::move(work)), fScheduleData(tiles) {}

    void signal() final { this->TiledDrawSchedulerBase::signal(); }
    void finish() final { this->TiledDrawSchedulerBase::finish(); }

    bool next(int& tileIndex) final {
        int& drawIndex = fScheduleData[tileIndex].fDrawIndex;
        SkASSERT(drawIndex <= fDrawCnt);
        while (true) {
            bool isFinishing = fIsFinishing.load(std::memory_order_relaxed);
            if (isFinishing && drawIndex >= fDrawCnt) {
                return false;
            } else if (drawIndex < fDrawCnt) {
                fWork(tileIndex, drawIndex++);
                return true;
            }
        }
    }

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) TileScheduleData {
        TileScheduleData() : fDrawIndex(0) {}

        int fDrawIndex; // next draw index for this tile
    };

    std::vector<TileScheduleData>  fScheduleData;
};

class TiledDrawSchedulerFlexible : public TiledDrawSchedulerBase {
public:
    TiledDrawSchedulerFlexible(int tiles, WorkFunc work)
            : TiledDrawSchedulerBase(tiles, std::move(work)), fScheduleData(tiles) {}

    void signal() final { this->TiledDrawSchedulerBase::signal(); }
    void finish() final { this->TiledDrawSchedulerBase::finish(); }

    bool next(int& tileIndex) final {
        int failCnt = 0;
        while (true) {
            TileScheduleData& scheduleData = fScheduleData[tileIndex];
            bool locked = scheduleData.fMutex.try_lock();
            bool processed = false;

            if (locked) {
                if (scheduleData.fDrawIndex < fDrawCnt) {
                    fWork(tileIndex, scheduleData.fDrawIndex++);
                    processed = true;
                } else {
                    failCnt += fIsFinishing.load(std::memory_order_relaxed);
                }
                scheduleData.fMutex.unlock();
            }

            if (processed) {
                return true;
            } else {
                if (failCnt >= fTileCnt) {
                    return false;
                }
                tileIndex = (tileIndex + 1) % fTileCnt;
            }
        }
    }

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) TileScheduleData {
        TileScheduleData() : fDrawIndex(0) {}

        int         fDrawIndex; // next draw index for this tile
        std::mutex  fMutex;     // the mutex for the thread to acquire
    };

    std::vector<TileScheduleData>  fScheduleData;
};

class TiledDrawSchedulerBySemaphores : public TiledDrawSchedulerBase {
public:
    TiledDrawSchedulerBySemaphores(int tiles, WorkFunc work)
            : TiledDrawSchedulerBase(tiles, std::move(work)), fScheduleData(tiles) {}


    void signal() final {
        this->TiledDrawSchedulerBase::signal();
        signalRoot();
    }

    void finish() final {
        this->TiledDrawSchedulerBase::finish();
        signalRoot();
    }

    bool next(int& tileIndex) final {
        SkASSERT(tileIndex >= 0 && tileIndex < fTileCnt);
        TileScheduleData& scheduleData = fScheduleData[tileIndex];
        while (true) {
            scheduleData.fSemaphore.wait();
            int leftChild = (tileIndex + 1) * 2 - 1;
            int rightChild = leftChild + 1;
            if (leftChild < fTileCnt) {
                fScheduleData[leftChild].fSemaphore.signal();
            }
            if (rightChild < fTileCnt) {
                fScheduleData[rightChild].fSemaphore.signal();
            }

            bool isFinishing = fIsFinishing.load(std::memory_order_relaxed);
            if (isFinishing && scheduleData.fDrawIndex >= fDrawCnt) {
                return false;
            } else {
                SkASSERT(scheduleData.fDrawIndex < fDrawCnt);
                fWork(tileIndex, scheduleData.fDrawIndex++);
                return true;
            }
        }
    }

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) TileScheduleData {
        TileScheduleData() : fDrawIndex(0) {}

        int         fDrawIndex;
        SkSemaphore fSemaphore;
    };

    void signalRoot() {
        SkASSERT(fTileCnt > 0);
        fScheduleData[0].fSemaphore.signal();
    }

    std::vector<TileScheduleData> fScheduleData;
};

void SkThreadedBMPDevice::startThreads() {
    SkASSERT(fThreadFutures.count() == 0);
    SkASSERT(fQueueSize == 0);

    TiledDrawScheduler::WorkFunc work = [this](int tileIndex, int drawIndex){
        auto& element = fQueue[drawIndex];
        if (SkIRect::Intersects(fTileBounds[tileIndex], element.fDrawBounds)) {
            element.fDrawFn(fTileBounds[tileIndex]);
        }
    };

    // using Scheduler = TiledDrawSchedulerBySemaphores;
    // using Scheduler = TiledDrawSchedulerBySpinning;
    using Scheduler = TiledDrawSchedulerFlexible;
    fScheduler.reset(new Scheduler(fTileCnt, work));
    for(int i = 0; i < fThreadCnt; ++i) {
        fThreadFutures.push_back(std::async(std::launch::async, [this, i]() {
            int tileIndex = i;
            while (fScheduler->next(tileIndex)) {}
        }));
    }
}

void SkThreadedBMPDevice::finishThreads() {
    fScheduler->finish();
    for(auto& future : fThreadFutures) {
        future.wait();
    }
    fThreadFutures.reset();
    fQueueSize = 0;
    fScheduler.reset(nullptr);
}

SkThreadedBMPDevice::SkThreadedBMPDevice(const SkBitmap& bitmap, int tiles, int threads)
        : INHERITED(bitmap)
        , fTileCnt(tiles)
        , fThreadCnt(threads <= 0 ? tiles : threads)
{
    // Tiling using stripes for now; we'll explore better tiling in the future.
    int h = (bitmap.height() + fTileCnt - 1) / SkTMax(fTileCnt, 1);
    int w = bitmap.width();
    int top = 0;
    for(int tid = 0; tid < fTileCnt; ++tid, top += h) {
        fTileBounds.push_back(SkIRect::MakeLTRB(0, top, w, top + h));
    }
    fQueueSize = 0;
    startThreads();
}

void SkThreadedBMPDevice::flush() {
    finishThreads();
    startThreads();
}

// Having this captured in lambda seems to be faster than saving this in DrawElement
struct SkThreadedBMPDevice::DrawState {
    SkPixmap fDst;
    SkMatrix fMatrix;
    SkRasterClip fRC;

    explicit DrawState(SkThreadedBMPDevice* dev) {
        // we need fDst to be set, and if we're actually drawing, to dirty the genID
        if (!dev->accessPixels(&fDst)) {
            // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
            fDst.reset(dev->imageInfo(), nullptr, 0);
        }
        fMatrix = dev->ctm();
        fRC = dev->fRCStack.rc();
    }

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

SkIRect SkThreadedBMPDevice::transformDrawBounds(const SkRect& drawBounds) const {
    if (drawBounds.isLargest()) {
        return SkIRect::MakeLargest();
    }
    SkRect transformedBounds;
    this->ctm().mapRect(&transformedBounds, drawBounds);
    return transformedBounds.roundOut();
}

// The do {...} while (false) is to enforce trailing semicolon as suggested by mtklein@
#define THREADED_DRAW(drawBounds, actualDrawCall)                                                  \
    do {                                                                                           \
        DrawState ds(this);                                                                        \
        SkASSERT(fQueueSize < MAX_QUEUE_SIZE);                                                     \
        fQueue[fQueueSize++] = {                                                                   \
            this->transformDrawBounds(drawBounds),                                                 \
            [=](const SkIRect& tileBounds) {                                                       \
                SkRasterClip tileRC;                                                               \
                SkDraw draw = ds.getThreadDraw(tileRC, tileBounds);                                \
                draw.actualDrawCall;                                                               \
            },                                                                                     \
        };                                                                                         \
        fScheduler->signal();                                                                      \
    } while (false)

static inline SkRect get_fast_bounds(const SkRect& r, const SkPaint& p) {
    SkRect result;
    if (p.canComputeFastBounds()) {
        result = p.computeFastBounds(r, &result);
    } else {
        result = SkRect::MakeLargest();
    }
    return result;
}

void SkThreadedBMPDevice::drawPaint(const SkPaint& paint) {
    THREADED_DRAW(SkRect::MakeLargest(), drawPaint(paint));
}

void SkThreadedBMPDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    // TODO tighter drawBounds
    SkRect drawBounds = SkRect::MakeLargest();
    THREADED_DRAW(drawBounds, drawPoints(mode, count, pts, paint, nullptr));
}

void SkThreadedBMPDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    SkRect drawBounds = get_fast_bounds(r, paint);
    THREADED_DRAW(drawBounds, drawRect(r, paint));
}

void SkThreadedBMPDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    SkPath  path;

    path.addRRect(rrect);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(path, paint, nullptr, false);
#else
    SkRect drawBounds = get_fast_bounds(rrect.getBounds(), paint);
    THREADED_DRAW(drawBounds, drawRRect(rrect, paint));
#endif
}

void SkThreadedBMPDevice::drawPath(const SkPath& path, const SkPaint& paint,
        const SkMatrix* prePathMatrix, bool pathIsMutable) {
    SkRect drawBounds = path.isInverseFillType() ? SkRect::MakeLargest()
                                                 : get_fast_bounds(path.getBounds(), paint);
    // For thread safety, make path imutable
    THREADED_DRAW(drawBounds, drawPath(path, paint, prePathMatrix, false));
}

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    SkMatrix matrix = SkMatrix::MakeTrans(x, y);
    LogDrawScaleFactor(SkMatrix::Concat(this->ctm(), matrix), paint.getFilterQuality());
    SkRect drawBounds = SkRect::MakeWH(bitmap.width(), bitmap.height());
    matrix.mapRect(&drawBounds);
    THREADED_DRAW(drawBounds, drawBitmap(bitmap, matrix, nullptr, paint));
}

void SkThreadedBMPDevice::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeXYWH(x, y, bitmap.width(), bitmap.height());
    THREADED_DRAW(drawBounds, drawSprite(bitmap, x, y, paint));
}

void SkThreadedBMPDevice::drawText(const void* text, size_t len, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawText((const char*)text, len, x, y, paint, &this->surfaceProps()));
}

void SkThreadedBMPDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
        int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawPosText((const char*)text, len, xpos, scalarsPerPos, offset,
                                          paint, &surfaceProps()));
}

void SkThreadedBMPDevice::drawVertices(const SkVertices* vertices, SkBlendMode bmode,
        const SkPaint& paint) {
    SkRect drawBounds = SkRect::MakeLargest(); // TODO tighter drawBounds
    THREADED_DRAW(drawBounds, drawVertices(vertices->mode(), vertices->vertexCount(),
                                           vertices->positions(), vertices->texCoords(),
                                           vertices->colors(), bmode, vertices->indices(),
                                           vertices->indexCount(), paint));
}

void SkThreadedBMPDevice::drawDevice(SkBaseDevice* device, int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());
    SkRect drawBounds = SkRect::MakeXYWH(x, y, device->width(), device->height());
    THREADED_DRAW(drawBounds,
                  drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap, x, y, paint));
}

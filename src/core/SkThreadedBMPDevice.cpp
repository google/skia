/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadedBMPDevice.h"

#include "SkDrawProcs.h"
#include "SkPath.h"
#include "SkTaskGroup.h"
#include "SkVertices.h"

#include <mutex>
#include <vector>

// Some basic logics and data structures that are shared across the current experimental schedulers.
class ThreadedDrawSchedulerBase : public ThreadedDrawScheduler {
public:
    ThreadedDrawSchedulerBase(int tiles, SkThreadedBMPDevice* device)
            : fTileCnt(tiles), fIsFinishing(false), fDrawCnt(0), fDevice(device) {}

    void signal() override {
        fDrawCnt++;
    }
    void finish() override {
        fIsFinishing.store(true, std::memory_order_relaxed);
    }

    // initIndex should be thread-local or exclusively owned by the thread
    void tryInit(int& initIndex, int threadId) {
        while (initIndex < fDrawCnt) {
            DrawElement& initElement = fDevice->fQueue[initIndex];
            if (!initElement.fInitialized && initElement.fNeedInit) {
                bool needInit = true;
                // TODO maybe try exchange_weak
                if (initElement.fNeedInit.compare_exchange_strong(
                        needInit, false)) {
                    SkArenaAlloc* threadAlloc = &fDevice->fThreadAllocs[threadId];
                    initElement.fInitFn(threadAlloc);
                    initElement.fInitialized = true;
                }
            }
            initIndex++;
        }
    }

    // The drawIndex should be thread-local, or exclusively owned by this thread.
    // The drawIndex is incremented if drawn (including empty draw).
    // Return true if drawn and false otherwise.
    bool tryDraw(int& drawIndex, int tileIndex, int threadId) {
        SkASSERT(drawIndex < fDrawCnt);
        DrawElement& drawElement = fDevice->fQueue[drawIndex];
        if (!SkIRect::Intersects(fDevice->fTileBounds[tileIndex], drawElement.fDrawBounds)) {
            drawIndex++;
            return true;
        }
        if (drawElement.fInitialized) {
            SkArenaAlloc* threadAlloc = &fDevice->fThreadAllocs[threadId];
            drawElement.fDrawFn(threadAlloc, fDevice->fTileBounds[tileIndex]);
            drawIndex++;
            return true;
        }
        return false;
    }

protected:
    const int                   fTileCnt;
    std::atomic<bool>           fIsFinishing;
    std::atomic<int>            fDrawCnt;
    SkThreadedBMPDevice*        fDevice;
};

class SpinningScheduler : public ThreadedDrawSchedulerBase {
public:
    // the number of tiles and the number of threads are equal for this scheduler
    SpinningScheduler(int tiles, SkThreadedBMPDevice* device)
            : ThreadedDrawSchedulerBase(tiles, device), fScheduleData(tiles) {}

    void signal() final { this->ThreadedDrawSchedulerBase::signal(); }
    void finish() final { this->ThreadedDrawSchedulerBase::finish(); }

    bool next(int threadId) final {
        int& drawIndex = fScheduleData[threadId].fDrawIndex;

        while (true) {
            SkASSERT(drawIndex <= fDrawCnt);
            bool isFinishing = fIsFinishing.load(std::memory_order_relaxed);
            if (isFinishing && drawIndex >= fDrawCnt) {
                return false;
            }

            if (drawIndex < fDrawCnt && this->tryDraw(drawIndex, threadId, threadId)) {
                return true;
            }

            // In this scheduler, we first draw and then try to init; we can also first try to init
            // everything before attemp to draw.
            this->tryInit(fScheduleData[threadId].fInitIndex, threadId);
        }
    }

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) ScheduleData {
        ScheduleData() : fDrawIndex(0), fInitIndex(0) {}

        int fDrawIndex; // next draw index for this tile
        int fInitIndex; // next init index for this thread (tile = thread)
    };

    std::vector<ScheduleData>  fScheduleData;
};

class FlexibleScheduler : public ThreadedDrawSchedulerBase {
public:
    FlexibleScheduler(int tiles, int threads, SkThreadedBMPDevice* device)
            : ThreadedDrawSchedulerBase(tiles, device), fTileData(tiles), fThreadData(threads) {
        for (int i = 0; i < threads; ++i) {
            fThreadData[i].fTileIndex = i;
        }
    }

    void signal() final { this->ThreadedDrawSchedulerBase::signal(); }
    void finish() final { this->ThreadedDrawSchedulerBase::finish(); }

    bool next(int threadId) final {
        int failCnt = 0;
        int& tileIndex = fThreadData[threadId].fTileIndex;
        while (true) {
            TileData& tileData = fTileData[tileIndex];
            bool locked = tileData.fMutex.try_lock();
            bool processed = false;

            if (locked) {
                if (tileData.fDrawIndex < fDrawCnt) {
                    processed = this->tryDraw(tileData.fDrawIndex, tileIndex, threadId);
                } else {
                    failCnt += fIsFinishing.load(std::memory_order_relaxed);
                }
                tileData.fMutex.unlock();
            }

            if (processed) {
                return true;
            } else {
                if (failCnt >= fTileCnt) {
                    return false;
                }
                tileIndex = (tileIndex + 1) % fTileCnt;
            }

            this->tryInit(fThreadData[threadId].fInitIndex, threadId);
        }
    }

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) TileData {
        TileData() : fDrawIndex(0) {}

        int         fDrawIndex; // next draw index for this tile
        std::mutex  fMutex;     // the mutex for the thread to acquire
    };

    struct alignas(MAX_CACHE_LINE) ThreadData {
        ThreadData() : fInitIndex(0), fTileIndex(0) {}

        int         fInitIndex; // next draw index for this tile
        int         fTileIndex; // the tile that the current thread is working on
    };

    std::vector<TileData>   fTileData;
    std::vector<ThreadData> fThreadData;
};

class SemaphoreTreeScheduler : public ThreadedDrawSchedulerBase {
public:
    SemaphoreTreeScheduler(int tiles, SkThreadedBMPDevice* device)
            : ThreadedDrawSchedulerBase(tiles, device), fScheduleData(tiles) {}


    void signal() final {
        this->ThreadedDrawSchedulerBase::signal();
        signalRoot();
    }

    void finish() final {
        this->ThreadedDrawSchedulerBase::finish();
        signalRoot();
    }

    bool next(int threadId) final {
        int tileIndex = threadId;
        SkASSERT(tileIndex >= 0 && tileIndex < fTileCnt);
        ScheduleData& scheduleData = fScheduleData[tileIndex];
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
                // We have to init first so we won't be blocked by uninitialized draw
                this->tryInit(scheduleData.fInitIndex, threadId);
                // Some other threds might be initializing this draw, so try until initialized
                // and drawn.
                while (!this->tryDraw(scheduleData.fDrawIndex, tileIndex, threadId)) {}
                return true;
            }
        }
    }

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) ScheduleData {
        ScheduleData() : fDrawIndex(0), fInitIndex(0) {}

        int         fDrawIndex;
        int         fInitIndex;
        SkSemaphore fSemaphore;
    };

    void signalRoot() {
        SkASSERT(fTileCnt > 0);
        fScheduleData[0].fSemaphore.signal();
    }

    std::vector<ScheduleData> fScheduleData;
};

void SkThreadedBMPDevice::startThreads() {
    SkASSERT(fQueueSize == 0);

    // fScheduler.reset(new SemaphoreTreeScheduler(fTileCnt, this));
    // fScheduler.reset(new SpinningScheduler(fTileCnt, this));
    fScheduler.reset(new FlexibleScheduler(fTileCnt, fThreadCnt, this));

    fTaskGroup->batch(fThreadCnt, [this](int threadId){
        while (fScheduler->next(threadId)) {}
    });
}

void SkThreadedBMPDevice::finishThreads() {
    fScheduler->finish();
    fTaskGroup->wait();
    fQueueSize = 0;
    fScheduler.reset(nullptr);
}

SkThreadedBMPDevice::SkThreadedBMPDevice(const SkBitmap& bitmap,
                                         int tiles,
                                         int threads,
                                         SkExecutor* executor)
        : INHERITED(bitmap)
        , fTileCnt(tiles)
        , fThreadCnt(threads <= 0 ? tiles : threads)
        , fThreadAllocs(threads <= 0 ? tiles : threads)
{
    if (executor == nullptr) {
        fInternalExecutor = SkExecutor::MakeFIFOThreadPool(fThreadCnt);
        executor = fInternalExecutor.get();
    }
    fExecutor = executor;

    for(int i = 0; i < fThreadCnt; ++i) {
        fThreadAllocs.emplace_back();
    }

    // Tiling using stripes for now; we'll explore better tiling in the future.
    int h = (bitmap.height() + fTileCnt - 1) / SkTMax(fTileCnt, 1);
    int w = bitmap.width();
    int top = 0;
    for(int tid = 0; tid < fTileCnt; ++tid, top += h) {
        fTileBounds.push_back(SkIRect::MakeLTRB(0, top, w, top + h));
    }
    fQueueSize = 0;
    fTaskGroup.reset(new SkTaskGroup(*fExecutor));
    startThreads();
}

void SkThreadedBMPDevice::flush() {
    finishThreads();
    startThreads();
}


DrawState::DrawState(SkThreadedBMPDevice* dev) {
    // we need fDst to be set, and if we're actually drawing, to dirty the genID
    if (!dev->accessPixels(&fDst)) {
        // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
        fDst.reset(dev->imageInfo(), nullptr, 0);
    }
    fMatrix = dev->ctm();
    fRC = dev->getRCStack()->rc();
}

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
        if (fQueueSize == MAX_QUEUE_SIZE) {                                                        \
            this->flush();                                                                         \
        }                                                                                          \
        SkASSERT(fQueueSize < MAX_QUEUE_SIZE);                                                     \
        DrawElement* element = &fQueue[fQueueSize++];                                               \
        element->fDS = DrawState(this);                                                             \
        element->fDrawBounds = this->transformDrawBounds(drawBounds);                               \
        element->fInitialized = true;                                                               \
        element->fDrawFn = [=](SkArenaAlloc*, const SkIRect& tileBounds) {                   \
            SkRasterClip tileRC;                                                                   \
            SkDraw draw = element->fDS.getThreadDraw(tileRC, tileBounds);                          \
            draw.actualDrawCall;                                                                   \
        };                                                                                         \
        fScheduler->signal();                                                                      \
    } while (false)

#define THREADED_DRAW_WITH_INIT(drawBounds, initCall)                                              \
    do {                                                                                           \
        if (fQueueSize == MAX_QUEUE_SIZE) {                                                        \
            this->flush();                                                                         \
        }                                                                                          \
        SkASSERT(fQueueSize < MAX_QUEUE_SIZE);                                                     \
        DrawElement* element = &fQueue[fQueueSize++];                                               \
        element->fDS = DrawState(this);                                                             \
        element->fDrawBounds = this->transformDrawBounds(drawBounds);                               \
        element->fInitialized = false;                                                              \
        element->fNeedInit = true;                                                                  \
        element->fInitFn = [=](SkArenaAlloc* alloc) {                         \
            element->fDS.getDraw().initCall;                                                \
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
    SkScalar coverage;
    if (path.countVerbs() < 100 || SkDrawTreatAsHairline(paint, this->ctm(), &coverage)) {
        THREADED_DRAW(drawBounds, drawPath(path, paint, prePathMatrix, false));
    } else {
        THREADED_DRAW_WITH_INIT(
                drawBounds, drawPath(path, paint, prePathMatrix, false, false, nullptr, alloc,
                                     element));
    }
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

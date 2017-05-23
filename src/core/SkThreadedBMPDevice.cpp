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

class TiledDrawSchedulerFlexible : public TiledDrawScheduler {
public:
    using MPtr = std::unique_ptr<std::mutex>;

    TiledDrawSchedulerFlexible(int tiles) : TiledDrawScheduler(tiles) {
        for(int i = 0; i < tiles; ++i) {
            fMutexes.emplace_back(new std::mutex);
        }
    }

    void signal() override {
        fDrawCnt++;
    }
    void finish() override {
        fIsFinishing = true;
    }

    void onSingleDrawFinished(int tileIndex) override {
        fMutexes[tileIndex]->unlock();
    }

    // Here we assume that the tileIndex is only a hint. We may change it.
    bool waitForNext(int& tileIndex, int& drawIndex) override {
        int failCnt = 0;
        while (true) {
            bool locked = fMutexes[tileIndex]->try_lock();

            if (!locked) {
                tileIndex = (tileIndex + 1) % fTileCnt;
                continue;
            }

            if (fDrawIndexes[tileIndex] < fDrawCnt) {
                drawIndex = fDrawIndexes[tileIndex]++;
                // Do not unlock as the current thread is processing this tile
                return true;
            } else {
                // No work to do; move to next tile.
                fMutexes[tileIndex]->unlock();
                tileIndex = (tileIndex + 1) % fTileCnt;
                failCnt += fIsFinishing;
            }

            if (failCnt >= fTileCnt) {
                fMutexes[tileIndex]->unlock();
                return false;
            }
        }
    }

private:
    SkTArray<MPtr> fMutexes;
};

class TiledDrawSchedulerBySpinning : public TiledDrawScheduler {
public:
    TiledDrawSchedulerBySpinning(int tiles) : TiledDrawScheduler(tiles) {
        for(int i = 0; i < tiles; ++i) {
            fHitCount.push_back(0);
            fReportCount.push_back(0);
        }
    }

    ~TiledDrawSchedulerBySpinning() override {
        float averageHit = getAverageHit();
        if (averageHit > 0) {
            SkDebugf("\33[2K\r");
            SkDebugf("Average hit: %f", averageHit);
        }
    }

    void signal() override {
        fDrawCnt++;
    }
    void finish() override {
        fIsFinishing = true;
    }

    // Here we assume that the tileIndex is fixed and given. We will return with the
    // new drawIndex once it's available.
    bool waitForNext(int& tileIndex, int& drawIndex) override {
        SkASSERT(fDrawIndexes[tileIndex] <= fDrawCnt);

        int mask = drawIndex;
        while (true) {
            if (fIsFinishing && fDrawIndexes[tileIndex] >= fDrawCnt) {
                return false;
            } else if (fDrawIndexes[tileIndex] < fDrawCnt) {
                drawIndex = fDrawIndexes[tileIndex]++;
                this->reportHit(tileIndex, mask, true);
                return true;
            }
            this->reportHit(tileIndex, mask, false);
            mask++;
        }
    }

private:
    inline void reportHit(int tileIndex, int mask, bool isHit) {
        if ((mask & REPORT_MASK) == 0) {
            fHitCount[tileIndex] += isHit;
            fReportCount[tileIndex]++;
        }
    }

    inline float getAverageHit() const {
        int reportCount = 0;
        int hitCount = 0;
        for(int i = 0; i < fTileCnt; ++i) {
            reportCount += fReportCount[i];
            hitCount += fHitCount[i];
        }
        return reportCount == 0 ? -1 : (float)hitCount / reportCount;
    }

    SkTArray<int> fHitCount;
    SkTArray<int> fReportCount;

    static constexpr int REPORT_MASK = (1 << 8) - 1;
};

class TiledDrawSchedulerByConditionVariable : public TiledDrawScheduler {
public:
    TiledDrawSchedulerByConditionVariable(int tiles) : TiledDrawScheduler(tiles) {}

    void signal() override {
        std::unique_lock<std::mutex> lock(fMutex);
        fDrawCnt++;
        lock.unlock();
        fHasNewDraw.notify_all();
    }
    void finish() override {
        std::unique_lock<std::mutex> lock(fMutex);
        fIsFinishing = true;
        lock.unlock();
        fHasNewDraw.notify_all();
    }

    // Here we assume that the tileIndex is fixed and given. We will return with the
    // new drawIndex once it's available.
    bool waitForNext(int& tileIndex, int& drawIndex) override {
        SkASSERT(fDrawIndexes[tileIndex] <= fDrawCnt);

        while (true) {
            if (fIsFinishing && fDrawIndexes[tileIndex] >= fDrawCnt) {
                return false;
            }

            if (fDrawIndexes[tileIndex] < fDrawCnt) {
                drawIndex = fDrawIndexes[tileIndex]++;
                return true;
            }

            std::unique_lock<std::mutex> lock(fMutex);
            if (fIsFinishing && fDrawIndexes[tileIndex] >= fDrawCnt) {
                return false;
            }

            if (fDrawIndexes[tileIndex] < fDrawCnt) {
                drawIndex = fDrawIndexes[tileIndex]++;
                return true;
            } else {
                fHasNewDraw.wait(lock);
            }
        }
    }

private:
    std::mutex fMutex;
    std::condition_variable fHasNewDraw;
};

class TiledDrawSchedulerBySemaphoreTree : public TiledDrawScheduler {
public:
    using SemPtr = std::unique_ptr<SkSemaphore>;

    TiledDrawSchedulerBySemaphoreTree(int tiles) : TiledDrawScheduler(tiles) {
        for(int i = 0; i < tiles; ++i) {
            fSemaphores.emplace_back(new SkSemaphore);
        }
    }

    void signal() override {
        fDrawCnt++;
        signalRoot();
    }
    void finish() override {
        fIsFinishing = true;
        signalRoot();
    }

    // Here we assume that the tileIndex is fixed and given. We will return with the
    // new drawIndex once it's available.
    bool waitForNext(int& tileIndex, int& drawIndex) override {
        SkASSERT(tileIndex >= 0 && tileIndex < fTileCnt);
        while (true) {
            fSemaphores[tileIndex]->wait();
            int leftChild = (tileIndex + 1) * 2 -1;
            int rightChild = leftChild + 1;
            if (leftChild < fTileCnt) {
                fSemaphores[leftChild]->signal();
            }
            if (rightChild < fTileCnt) {
                fSemaphores[rightChild]->signal();
            }

            if (fIsFinishing && fDrawIndexes[tileIndex] >= fDrawCnt) {
                return false;
            } else if (fDrawIndexes[tileIndex] < fDrawCnt) {
                drawIndex = fDrawIndexes[tileIndex]++;
                return true;
            }
        }
    }

private:
    SkTArray<SemPtr> fSemaphores;

    inline void signalRoot() {
        SkASSERT(fTileCnt > 0); // ensure that fSemaphores[0] is initialized
        fSemaphores[0]->signal();
    }
};

void SkThreadedBMPDevice::startThreads() {
    SkASSERT(fThreadFutures.count() == 0);
    SkASSERT(fQueueSize == 0);

    if (!fScheduler) {
        // fScheduler.reset(new TiledDrawSchedulerFlexible(fTileCnt));
        fScheduler.reset(new TiledDrawSchedulerBySpinning(fTileCnt));
        // fScheduler.reset(new TiledDrawSchedulerByConditionVariable(fTileCnt));
        // fScheduler.reset(new TiledDrawSchedulerBySemaphoreTree(fTileCnt));
    }

    for(int i = 0; i < fThreadCnt; ++i) {
        fThreadFutures.push_back(std::async(std::launch::async, [this, i]() {
            int tileIndex = i, drawIndex;
            while (fScheduler->waitForNext(tileIndex, drawIndex)) {
                auto& element = fQueue[drawIndex];
                if (SkIRect::Intersects(fTileBounds[tileIndex], element.fDrawBounds)) {
                    element.fDrawFn(fTileBounds[tileIndex]);
                }
                fScheduler->onSingleDrawFinished(tileIndex);
            }
        }));
    }
}

void SkThreadedBMPDevice::finishThreads() {
    if (fScheduler) {
        fScheduler->finish();
    }
    for(auto& future : fThreadFutures) {
        future.wait();
    }
    fThreadFutures.reset();
    fQueueSize = 0;
    if (fScheduler) {
        fScheduler->reset();
    }
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
}

void SkThreadedBMPDevice::flush() {
    finishThreads();
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
        if (fThreadFutures.count() == 0) {                                                         \
            startThreads();                                                                        \
        }                                                                                          \
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

void SkThreadedBMPDevice::drawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
        const SkPaint& paint) {
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

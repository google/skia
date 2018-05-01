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
    // We store DrawState inside DrawElement because inifFn and drawFn both want to use it
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

    class DrawElement {
    public:
        using InitFn = std::function<void(SkArenaAlloc* threadAlloc, DrawElement* element)>;
        using DrawFn = std::function<void(SkArenaAlloc* threadAlloc, const DrawState& ds,
                                          const SkIRect& tileBounds)>;

        DrawElement() {}
        DrawElement(SkThreadedBMPDevice* device, DrawFn&& drawFn, const SkRect& rawDrawBounds)
                : fInitialized(true)
                , fDrawFn(std::move(drawFn))
                , fDS(device)
                , fDrawBounds(device->transformDrawBounds(rawDrawBounds)) {}
        DrawElement(SkThreadedBMPDevice* device, InitFn&& initFn, const SkRect& rawDrawBounds)
                : fInitialized(false)
                , fNeedInit(true)
                , fInitFn(std::move(initFn))
                , fDS(device)
                , fDrawBounds(device->transformDrawBounds(rawDrawBounds)) {}

        SK_ALWAYS_INLINE bool tryInitOnce(SkArenaAlloc* alloc) {
            bool t = true;
            // If there are multiple threads reaching this point simutaneously,
            // compare_exchange_strong ensures that only one thread can enter the if condition and
            // do the initialization.
            if (!fInitialized && fNeedInit && fNeedInit.compare_exchange_strong(t, false)) {
#ifdef SK_DEBUG
                fDrawFn = 0; // Invalidate fDrawFn
#endif
                fInitFn(alloc, this);
                fInitialized = true;
                SkASSERT(fDrawFn != 0); // Ensure that fInitFn does populate fDrawFn
                return true;
            }
            return false;
        }

        SK_ALWAYS_INLINE bool tryDraw(const SkIRect& tileBounds, SkArenaAlloc* alloc) {
            if (!SkIRect::Intersects(tileBounds, fDrawBounds)) {
                return true;
            }
            if (fInitialized) {
                fDrawFn(alloc, fDS, tileBounds);
                return true;
            }
            return false;
        }

        SkDraw getDraw() const { return fDS.getDraw(); }
        void setDrawFn(DrawFn&& fn) { fDrawFn = std::move(fn); }

    private:
        std::atomic<bool>   fInitialized;
        std::atomic<bool>   fNeedInit;
        InitFn              fInitFn;
        DrawFn              fDrawFn;
        DrawState           fDS;
        SkIRect             fDrawBounds;
    };

    class DrawQueue : public SkWorkKernel2D {
    public:
        static constexpr int MAX_QUEUE_SIZE = 100000;

        DrawQueue(SkThreadedBMPDevice* device) : fDevice(device) {}
        void reset();

        // For ~SkThreadedBMPDevice() to shutdown tasks, we use this instead of reset because reset
        // will start new tasks.
        void finish() { fTasks->finish(); }

        // Push a draw command into the queue. If Fn is DrawFn, we're pushing an element without
        // the need of initialization. If Fn is InitFn, we're pushing an element with init-once
        // and the InitFn will generate the DrawFn during initialization.
        template<typename Fn>
        SK_ALWAYS_INLINE void push(const SkRect& rawDrawBounds, Fn&& fn) {
            if (fSize == MAX_QUEUE_SIZE) {
                this->reset();
            }
            SkASSERT(fSize < MAX_QUEUE_SIZE);
            new (&fElements[fSize++]) DrawElement(fDevice, std::move(fn), rawDrawBounds);
            fTasks->addColumn();
        }

        // SkWorkKernel2D
        bool initColumn(int column, int thread) override;
        bool work2D(int row, int column, int thread) override;

    private:
        SkThreadedBMPDevice*                fDevice;
        std::unique_ptr<SkTaskGroup2D>      fTasks;
        SkTArray<SkSTArenaAlloc<8 << 10>>   fThreadAllocs; // 8k stack size
        DrawElement                         fElements[MAX_QUEUE_SIZE];
        int                                 fSize;
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

    SkSTArenaAlloc<8 << 10> fAlloc; // so we can allocate memory that lives until flush

    DrawQueue fQueue;

    friend struct SkInitOnceData;   // to access DrawElement and DrawState
    friend class SkDraw;            // to access DrawState

    typedef SkBitmapDevice INHERITED;
};

// Passed to SkDraw::drawXXX to enable threaded draw with init-once. The goal is to reuse as much
// code as possible from SkDraw. (See SkDraw::drawPath and SkDraw::drawDevPath for an example.)
struct SkInitOnceData {
    SkArenaAlloc* fAlloc;
    SkThreadedBMPDevice::DrawElement* fElement;

    void setEmptyDrawFn() {
        fElement->setDrawFn([](SkArenaAlloc* threadAlloc, const SkThreadedBMPDevice::DrawState& ds,
                               const SkIRect& tileBounds){});
    }
};

#endif // SkThreadedBMPDevice_DEFINED

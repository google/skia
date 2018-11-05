// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Jitter GMs
//
// Re-execute rendering tests with slight translational changes and see if
// there is a significant change.  Print `1` if the named test has no
// significant change, `0` otherwise

#include "gm.h"
#include "SkGraphics.h"

#include <atomic>
#include <cstdio>
#include <mutex>
#include <thread>

// Error tolerance distance in 8888 color space with Manhattan metric on color channel.
static constexpr uint8_t kSkiaSkqpGlobalErrorTolerance = 8;

// Number of times to jitter the canvas.
static constexpr int kNumberOfJitters = 7;

// Distance to translate the canvas in each jitter (direction will be different each time).
static constexpr float kJitterMagiitude = 0.03125f;

// The `kNumberOfJitters` different runs will each go in a different direction.
// this is the angle (in radians) for the first one.
static constexpr float kPhase = 0.3f;

static inline uint32_t color(const SkPixmap& pm, SkIPoint p) {
    return *pm.addr32(p.x(), p.y());
}

static inline bool inside(SkIPoint point, SkISize dimensions) {
    return (unsigned)point.x() < (unsigned)dimensions.width() &&
           (unsigned)point.y() < (unsigned)dimensions.height();
}

static SkBitmap make_bitmap(SkISize size) {
    SkBitmap b;
    b.allocN32Pixels(size.width(), size.height());
    return b;
}

namespace {  // code copied from SkQPv2
struct Result {
    int64_t fTotalError = 0;
    int fBadPixelCount = 0;
    int fMaxError = 0;
};
}

static Result check(const SkPixmap& minImg, const SkPixmap& maxImg, const SkPixmap& img,
                    unsigned tolerance, SkBitmap* errorOut) {
    Result result = {0, 0, 0};
    SkISize dim = img.info().dimensions();
    SkASSERT(minImg.info().dimensions() == dim);
    SkASSERT(maxImg.info().dimensions() == dim);
    static const SkIPoint kNeighborhood[9] = {
        { 0,  0},
        {-1,  0}, { 1,  0}, { 0, -1}, { 0,  1},
        {-1, -1}, { 1, -1}, {-1,  1}, { 1,  1},
    };
    for (int y = 0; y < dim.height(); ++y) {
        for (int x = 0; x < dim.width(); ++x) {
            SkIPoint xy{x, y};
            uint32_t c = color(img, xy);
            int error = INT_MAX;
            // loop over neighborhood (halo);
            for (SkIPoint delta : kNeighborhood) {
                SkIPoint point = xy + delta;
                if (inside(point, dim)) {
                    int err = 0;
                    // loop over four color channels.
                    // Return Manhattan distance in channel-space.
                    for (int j : {0, 8, 16, 24}) {
                        uint8_t v    = (c                    >> j) & 0xFF,
                                vmin = (color(minImg, point) >> j) & 0xFF,
                                vmax = (color(maxImg, point) >> j) & 0xFF;
                        err = SkMax32(err, SkMax32((int)v - (int)vmax, (int)vmin - (int)v));
                    }
                    error = SkMin32(error, err);
                }
            }
            if (error > (int)tolerance) {
                ++result.fBadPixelCount;
                result.fTotalError += error;
                result.fMaxError = SkMax32(error, result.fMaxError);
                if (errorOut) {
                    if (!errorOut->getPixels()) {
                        errorOut->allocPixels(SkImageInfo::Make(
                                    dim.width(), dim.height(),
                                    kBGRA_8888_SkColorType,
                                    kOpaque_SkAlphaType));
                        errorOut->eraseColor(SK_ColorWHITE);
                    }
                    SkASSERT((unsigned)error < 256);
                    *(errorOut->getAddr32(x, y)) = SkColorSetARGB(0xFF, (uint8_t)error, 0, 0);
                }
            }
        }
    }
    return result;
}

static void do_gm(SkBitmap* bm, skiagm::GM* gm, SkPoint jitter = {0, 0}) {
    SkASSERT(bm);
    SkASSERT(gm);
    SkASSERT(bm->dimensions() == gm->getISize());
    SkCanvas canvas(*bm);
    SkAutoCanvasRestore autoCanvasRestore(&canvas, true);
    canvas.clear(SK_ColorWHITE);
    canvas.translate(jitter.x(), jitter.y());
    gm->draw(&canvas);
    canvas.flush();
}

static bool test_jitter(skiagm::GM* gm) {
    SkASSERT(gm);
    SkISize size = gm->getISize();
    SkBitmap control = make_bitmap(size);
    SkBitmap experimental =  make_bitmap(size);
    do_gm(&control, gm);

    for (int i = 0; i < kNumberOfJitters; ++i) {
        float angle = i * (6.2831853f / kNumberOfJitters) + kPhase;
        do_gm(&experimental, gm, SkPoint{kJitterMagiitude * cosf(angle),
                                         kJitterMagiitude * sinf(angle)});
        Result result = check(control.pixmap(), control.pixmap(), experimental.pixmap(),
                              kSkiaSkqpGlobalErrorTolerance, nullptr);
        if (result.fTotalError > 0) {
            return false;
        }
    }
    return true;
}

static std::atomic<int> gGoodcount;
static std::atomic<int> gBadcount;
static std::mutex gOutputMutex;

static void do_test(skiagm::GMFactory factory) {
    std::unique_ptr<skiagm::GM> gm(factory(nullptr));
    if (0 == strcmp("p3", gm->getName())) {
        return;
    }
    bool j = test_jitter(gm.get());
    if (j) { ++gGoodcount; } else { ++gBadcount; }
    std::lock_guard<std::mutex> lock(gOutputMutex);
    fprintf(stdout, "%d %s\n", j, gm->getName());
    fflush(stdout);
}

static void run_in_threads(void(*f)(unsigned, unsigned)) {
    const unsigned threadcount = std::thread::hardware_concurrency();
    std::unique_ptr<std::thread[]> threads(new std::thread[threadcount]);
    fprintf(stderr, "spinning up %u threads\n", threadcount);
    for (unsigned i = 0; i < threadcount; ++i) {
        threads[i] = std::thread(f, i, threadcount);
    }
    for (unsigned i = 0; i < threadcount; ++i) {
        threads[i].join();
    }
}

int main() {
    SkGraphics::Init();
    gGoodcount = 0;
    gBadcount = 0;
    run_in_threads([](unsigned index, unsigned count) {
        unsigned i = 0;
        for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
            if (i % count == index) {
                do_test(factory);
            }
            ++i;
        }
    });
    fprintf(stderr, "good = %d\nbad = %d\n\n", gGoodcount.load(), gBadcount.load());
}

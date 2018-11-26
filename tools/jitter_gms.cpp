// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Jitter GMs
//
// Re-execute rendering tests with slight translational changes and see if
// there is a significant change.  Print `1` if the named test has no
// significant change, `0` otherwise

#include "gm.h"
#include "SkGraphics.h"
#include "SkExecutor.h"
#include "SkSemaphore.h"

#include "skqp_model.h"

#include <cstdio>
#include <mutex>

// Error tolerance distance in 8888 color space with Manhattan metric on color channel.
static constexpr uint8_t kSkiaSkqpGlobalErrorTolerance = 8;

// Number of times to jitter the canvas.
static constexpr int kNumberOfJitters = 7;

// Distance to translate the canvas in each jitter (direction will be different each time).
static constexpr float kJitterMagnitude = 0.03125f;

// The `kNumberOfJitters` different runs will each go in a different direction.
// this is the angle (in radians) for the first one.
static constexpr float kPhase = 0.3f;

static void do_gm(SkBitmap* bm, skiagm::GM* gm, SkPoint jitter) {
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

// Return true if passes jitter test.
static bool test_jitter(skiagm::GM* gm) {
    SkASSERT(gm);
    SkISize size = gm->getISize();
    SkBitmap control, experimental;
    control.allocN32Pixels(size.width(), size.height());
    experimental.allocN32Pixels(size.width(), size.height());
    do_gm(&control, gm, {0, 0});
    for (int i = 0; i < kNumberOfJitters; ++i) {
        float angle = i * (6.2831853f / kNumberOfJitters) + kPhase;
        do_gm(&experimental, gm, SkPoint{kJitterMagnitude * cosf(angle),
                                         kJitterMagnitude * sinf(angle)});
        SkQP::RenderOutcome result = skqp::Check(
                control.pixmap(), control.pixmap(), experimental.pixmap(),
                kSkiaSkqpGlobalErrorTolerance, nullptr);
        if (result.fTotalError > 0) {
            return false;
        }
    }
    return true;
}

int main() {
    SkGraphics::Init();
    std::mutex mutex;
    int goodCount = 0;
    int badCount = 0;
    int total = 0;
    SkSemaphore semaphore;
    auto executor = SkExecutor::MakeFIFOThreadPool();
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        ++total;
        executor->add([factory, &mutex, &goodCount, &badCount, &semaphore](){
            std::unique_ptr<skiagm::GM> gm(factory(nullptr));
            const char* name = gm->getName();
            if (0 != strcmp("p3", name)) {
                bool success = test_jitter(gm.get());
                std::lock_guard<std::mutex> lock(mutex);
                if (success) {
                    ++goodCount;
                } else {
                    ++badCount;
                }
                fprintf(stdout, "%s,%d\n", name, success ? 0 : -1);
                fflush(stdout);
            }
            semaphore.signal();
        });
    }
    while (total-- > 0) { semaphore.wait(); }
    fprintf(stderr, "good = %d\nbad = %d\n\n", goodCount, badCount);
}

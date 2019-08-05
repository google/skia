// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Jitter GMs
//
// Re-execute rendering tests with slight translational changes and see if
// there is a significant change.  Print `1` if the named test has no
// significant change, `0` otherwise

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSemaphore.h"
#include "tools/Registry.h"
#include "tools/skqp/src/skqp.h"
#include "tools/skqp/src/skqp_model.h"

#include <math.h>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

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

static bool do_this_test(const char* name,
                    const std::vector<std::string>& doNotRun,
                    const std::vector<std::string>& testOnlyThese) {
    for (const std::string& bad : doNotRun) {
        if (bad == name) {
            return false;
        }
    }
    for (const std::string& good : testOnlyThese) {
        if (good == name) {
            return true;
        }
    }
    return testOnlyThese.empty();
}


int main(int argc, char** argv) {
    std::vector<std::string> doNotRun;
    std::vector<std::string> testOnlyThese;
    if (argc > 1) {
        std::ifstream ifs(argv[1]);
        if (ifs.is_open()) {
            std::string str;
            while (std::getline(ifs, str)) {
                doNotRun.push_back(str);
            }
        }
    }
    if (argc > 2) {
        for (int i = 2; i < argc; ++i) {
            testOnlyThese.emplace_back(argv[i]);
        }
    }
    SkGraphics::Init();
    std::mutex mutex;
    std::vector<std::string> goodResults;
    std::vector<std::string> badResults;

    int total = 0;
    SkSemaphore semaphore;
    auto executor = SkExecutor::MakeFIFOThreadPool();
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        ++total;
        executor->add([factory, &mutex, &goodResults, &badResults,
                       &semaphore, &doNotRun, &testOnlyThese](){
            std::unique_ptr<skiagm::GM> gm(factory());
            const char* name = gm->getName();
            if (do_this_test(name, doNotRun, testOnlyThese)) {
                bool success = test_jitter(gm.get());
                std::lock_guard<std::mutex> lock(mutex);
                if (success) {
                    goodResults.emplace_back(name);
                } else {
                    badResults.emplace_back(name);
                }
                fputc('.', stderr);
                fflush(stderr);
            }
            semaphore.signal();
        });
    }
    while (total-- > 0) { semaphore.wait(); }
    fputc('\n', stderr);
    fflush(stderr);
    std::sort(goodResults.begin(), goodResults.end());
    std::sort(badResults.begin(), badResults.end());
    std::ofstream good("good.txt");
    std::ofstream bad("bad.txt");
    for (const std::string& s : goodResults) { good << s << '\n'; }
    for (const std::string& s : badResults) { bad << s << '\n'; }
    fprintf(stderr, "good = %u\nbad = %u\n\n",
            (unsigned)goodResults.size(), (unsigned)badResults.size());
}

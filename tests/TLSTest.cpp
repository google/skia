/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkTLS.h"
#include "Test.h"
#include <atomic>
#include <thread>

static void thread_main() {
    SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);

    const char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    size_t len = strlen(text);

    SkPaint paint;

    for (int j = 0; j < 10; ++j) {
        for (int i = 9; i <= 48; ++i) {
            paint.setTextSize(SkIntToScalar(i));
            paint.setAntiAlias(false);
            paint.measureText(text, len);
            paint.setAntiAlias(true);
            paint.measureText(text, len);
        }
    }
}

template <typename Fn>
static void test_threads(Fn fn) {
    std::thread threads[8];

    for (auto& thread : threads) {
        thread = std::thread(fn);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

static std::atomic<int> gCounter{0};

static void* fake_create_TLS() {
    gCounter++;
    return nullptr;
}
static void fake_delete_TLS(void*) {
    gCounter--;
}

DEF_TEST(TLS, reporter) {
    // TODO: Disabled for now to work around
    // http://code.google.com/p/skia/issues/detail?id=619
    // ('flaky segfault in TLS test on Shuttle_Ubuntu12 buildbots')
    if( false ) test_threads(&thread_main);

    // Test to ensure that at thread destruction, TLS destructors
    // have been called.
    test_threads([] {
        SkTLS::Get(fake_create_TLS, fake_delete_TLS);
    });
    REPORTER_ASSERT(reporter, 0 == gCounter.load());
}

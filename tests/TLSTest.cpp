/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkTLS.h"
#include "SkThreadUtils.h"
#include "Test.h"

static void thread_main(void*) {
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

static void test_threads(SkThread::entryPointProc proc) {
    SkThread* threads[8];
    int N = SK_ARRAY_COUNT(threads);
    int i;

    for (i = 0; i < N; ++i) {
        threads[i] = new SkThread(proc);
    }

    for (i = 0; i < N; ++i) {
        threads[i]->start();
    }

    for (i = 0; i < N; ++i) {
        threads[i]->join();
    }

    for (i = 0; i < N; ++i) {
        delete threads[i];
    }
}

static int32_t gCounter;

static void* FakeCreateTLS() {
    sk_atomic_inc(&gCounter);
    return nullptr;
}

static void FakeDeleteTLS(void*) {
    sk_atomic_dec(&gCounter);
}

static void testTLSDestructor(void*) {
    SkTLS::Get(FakeCreateTLS, FakeDeleteTLS);
}

DEF_TEST(TLS, reporter) {
    // TODO: Disabled for now to work around
    // http://code.google.com/p/skia/issues/detail?id=619
    // ('flaky segfault in TLS test on Shuttle_Ubuntu12 buildbots')
    if( false ) test_threads(&thread_main);

    // Test to ensure that at thread destruction, TLS destructors
    // have been called.
    test_threads(&testTLSDestructor);
    REPORTER_ASSERT(reporter, 0 == gCounter);
}

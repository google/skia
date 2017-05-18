/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTemplates.h"
#include "Test.h"

// Tests for some of the helpers in SkTemplates.h
static void test_automalloc_realloc(skiatest::Reporter* reporter) {
    SkAutoSTMalloc<1, int> array;

    // test we have a valid pointer, should not crash
    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);

    // using realloc for init
    array.realloc(1);

    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);

    // verify realloc can grow
    array.realloc(2);
    REPORTER_ASSERT(reporter, array[0] == 1);

    // realloc can shrink
    array.realloc(1);
    REPORTER_ASSERT(reporter, array[0] == 1);

    // should not crash
    array.realloc(0);

    // grow and shrink again
    array.realloc(10);
    for (int i = 0; i < 10; i++) {
        array[i] = 10 - i;
    }
    array.realloc(20);
    for (int i = 0; i < 10; i++) {
        REPORTER_ASSERT(reporter, array[i] == 10 - i);
    }
    array.realloc(10);
    for (int i = 0; i < 10; i++) {
        REPORTER_ASSERT(reporter, array[i] == 10 - i);
    }

    array.realloc(1);
    REPORTER_ASSERT(reporter, array[0] = 10);

    // resets mixed with realloc, below stack alloc size
    array.reset(0);
    array.realloc(1);
    array.reset(1);

    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);

    // reset and realloc > stack size
    array.reset(2);
    array.realloc(3);
    array[0] = 1;
    REPORTER_ASSERT(reporter, array[0] == 1);
    array.realloc(1);
    REPORTER_ASSERT(reporter, array[0] == 1);
}

DEF_TEST(Templates, reporter) {
    test_automalloc_realloc(reporter);
}

constexpr int static kStackPreallocCount = 10;

// Ensures the containers in SkTemplates.h all have a consistent api.
template<typename TContainer, typename TCount>
static void test_container_apis(skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, !TContainer((TCount)0).get());
    REPORTER_ASSERT(reporter, TContainer((TCount)1).get());
    REPORTER_ASSERT(reporter, TContainer((TCount)kStackPreallocCount).get());
    REPORTER_ASSERT(reporter, TContainer((TCount)kStackPreallocCount + 1).get());

    TContainer container;
    // The default constructor may or may not init to empty, depending on the type of container.

    container.reset((TCount)1);
    REPORTER_ASSERT(reporter, container.get());

    container.reset((TCount)kStackPreallocCount);
    REPORTER_ASSERT(reporter, container.get());

    container.reset((TCount)kStackPreallocCount + 1);
    REPORTER_ASSERT(reporter, container.get());

    container.reset((TCount)0);
    REPORTER_ASSERT(reporter, !container.get());
}

DEF_TEST(TemplateContainerAPIs, reporter) {
    test_container_apis<SkAutoTArray<int>, int>(reporter);
    test_container_apis<SkAutoSTArray<kStackPreallocCount, int>, int>(reporter);
    test_container_apis<SkAutoTMalloc<int>, size_t>(reporter);
    test_container_apis<SkAutoSTMalloc<kStackPreallocCount, int>, size_t>(reporter);
}

// Ensures that realloc(0) results in a null pointer.
template<typename TAutoMalloc> static void test_realloc_to_zero(skiatest::Reporter* reporter) {
    TAutoMalloc autoMalloc(kStackPreallocCount);
    REPORTER_ASSERT(reporter, autoMalloc.get());

    autoMalloc.realloc(0);
    REPORTER_ASSERT(reporter, !autoMalloc.get());

    autoMalloc.realloc(kStackPreallocCount + 1);
    REPORTER_ASSERT(reporter, autoMalloc.get());

    autoMalloc.realloc(0);
    REPORTER_ASSERT(reporter, !autoMalloc.get());

    autoMalloc.realloc(kStackPreallocCount);
    REPORTER_ASSERT(reporter, autoMalloc.get());
}

DEF_TEST(AutoReallocToZero, reporter) {
    test_realloc_to_zero<SkAutoTMalloc<int> >(reporter);
    test_realloc_to_zero<SkAutoSTMalloc<kStackPreallocCount, int> >(reporter);
}

DEF_TEST(SkAutoTMallocSelfMove, r) {
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wself-move"
#endif

    SkAutoTMalloc<int> foo(20);
    REPORTER_ASSERT(r, foo.get());

    foo = std::move(foo);
    REPORTER_ASSERT(r, foo.get());

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
}

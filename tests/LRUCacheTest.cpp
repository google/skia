/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkLRUCache.h"
#include "tests/Test.h"

#include <memory>

struct Value {
    Value(int value, int* counter)
    : fValue(value)
    , fCounter(counter) {
        (*fCounter)++;
    }

    ~Value() {
        (*fCounter)--;
    }

    int fValue;
    int* fCounter;
};

DEF_TEST(LRUCacheSequential, r) {
    int instances = 0;
    {
        static const int kSize = 100;
        SkLRUCache<int, std::unique_ptr<Value>> test(kSize);
        for (int i = 1; i < kSize * 2; i++) {
            REPORTER_ASSERT(r, !test.find(i));
            test.insert(i, std::make_unique<Value>(i * i, &instances));
            REPORTER_ASSERT(r, test.find(i));
            REPORTER_ASSERT(r, i * i == (*test.find(i))->fValue);
            if (i > kSize) {
                REPORTER_ASSERT(r, kSize == instances);
                REPORTER_ASSERT(r, !test.find(i - kSize));
            } else {
                REPORTER_ASSERT(r, i == instances);
            }
            REPORTER_ASSERT(r, (int) test.count() == instances);
        }
    }
    REPORTER_ASSERT(r, 0 == instances);
}

DEF_TEST(LRUCacheRandom, r) {
    struct MoveOnlyKey {
        explicit MoveOnlyKey(int value) : fValue(value) {}
        MoveOnlyKey(const MoveOnlyKey&) = delete;
        MoveOnlyKey& operator=(const MoveOnlyKey&) = delete;
        MoveOnlyKey(MoveOnlyKey&&) = default;
        MoveOnlyKey& operator=(MoveOnlyKey&&) = delete;

        bool operator==(const MoveOnlyKey& that) const {
            return fValue == that.fValue;
        }
        int fValue;
    };
    int instances = 0;
    {
        int seq[] = { 0, 1, 2, 3, 4, 1, 6, 2, 7, 5, 3, 2, 2, 3, 1, 7 };
        int expected[] = { 7, 1, 3, 2, 5 };
        static const int kSize = 5;
        SkLRUCache<MoveOnlyKey, std::unique_ptr<Value>> test(kSize);
        for (int i = 0; i < (int) (sizeof(seq) / sizeof(int)); i++) {
            int k = seq[i];
            MoveOnlyKey key(k);
            if (!test.find(key)) {
                test.insert(std::move(key), std::make_unique<Value>(k, &instances));
            }
        }
        REPORTER_ASSERT(r, kSize == instances);
        REPORTER_ASSERT(r, kSize == test.count());
        for (int i = 0; i < kSize; i++) {
            int k = expected[i];
            MoveOnlyKey key(k);
            REPORTER_ASSERT(r, test.find(key));
            REPORTER_ASSERT(r, k == (*test.find(key))->fValue);
        }
    }
    REPORTER_ASSERT(r, 0 == instances);
}

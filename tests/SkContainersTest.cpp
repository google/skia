// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/private/base/SkContainers.h"

#include "tests/Test.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>

struct SkContainerAllocatorTestingPeer {
    static size_t RoundUpCapacity(const SkContainerAllocator& a, int64_t capacity){
        return a.roundUpCapacity(capacity);
    }

    static size_t GrowthFactorCapacity(
            const SkContainerAllocator& a, int capacity, double growthFactor) {
        return a.growthFactorCapacity(capacity, growthFactor);
    }

    static constexpr int64_t kCapacityMultiple = SkContainerAllocator::kCapacityMultiple;
};

static int max_capacity(size_t sizeOfT) {
    return std::min(SIZE_MAX / sizeOfT, (size_t)INT_MAX);
}

DEF_TEST(SkContainerAllocatorBasic, reporter) {

    /* Comment out until a solution can be found.
    // The following test drive the 32-bit machines out of memory so skip them.
    if constexpr (SIZE_MAX > UINT_MAX) {
        {
            SkContainerAllocator a{1, max_capacity(1)};

            SkSpan<std::byte> span = a.allocate(0, 1.0);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(0, 1.5);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(max_capacity(1), 1.0);
            REPORTER_ASSERT(reporter, span.size() >= SkToSizeT(max_capacity(1)));
            sk_free(span.data());
        }

        {
            SkContainerAllocator a{16, max_capacity(16)};

            SkSpan<std::byte> span = a.allocate(0, 1.0);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(0, 1.5);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(max_capacity(16), 1.0);
            REPORTER_ASSERT(reporter, span.size() >= SkToSizeT(max_capacity(16)) * 16);
            sk_free(span.data());
        }
    }  // end skipped on 32-bit Windows tests.
     */

    for (int i = 1; i < 33; ++i) {
        SkContainerAllocator a{(size_t)i, max_capacity(i)};
        int64_t r = SkContainerAllocatorTestingPeer::RoundUpCapacity(a, 1);
        REPORTER_ASSERT(reporter, r == SkContainerAllocatorTestingPeer::kCapacityMultiple);

        r = SkContainerAllocatorTestingPeer::RoundUpCapacity(a, INT_MAX);
        REPORTER_ASSERT(reporter, r == max_capacity(i));

        r = SkContainerAllocatorTestingPeer::RoundUpCapacity(a, max_capacity(i));
        REPORTER_ASSERT(reporter, r == max_capacity(i));
    }

    for (int i = 1; i < 33; ++i) {
        SkContainerAllocator a{(size_t)i, max_capacity(i)};
        int64_t r = SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, 1, 1.5);
        REPORTER_ASSERT(reporter, r == SkContainerAllocatorTestingPeer::kCapacityMultiple);

        r = SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, INT_MAX, 1.5);
        REPORTER_ASSERT(reporter, r == max_capacity(i));

        r = SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, max_capacity(i), 1.5);
        REPORTER_ASSERT(reporter, r == max_capacity(i));
    }
}


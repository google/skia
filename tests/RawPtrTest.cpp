/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkAssert.h"
#include "include/private/SkTypeTraits.h"
#include "src/partition_alloc/raw_ptr.h"
#include "src/partition_alloc/raw_ref.h"
#include "tests/Test.h"

#include <memory>
#include <tuple>

#if defined(SK_USE_PARTITION_ALLOC)
#include <partition_alloc/buildflags.h>
#if PA_BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
#include <partition_alloc/dangling_raw_ptr_checks.h>

namespace {
class ScopedDanglingPointerTracker {
public:
    ScopedDanglingPointerTracker()
            : fOldDetectedFn(partition_alloc::GetDanglingRawPtrDetectedFn())
            , fOldReleasedFn(partition_alloc::GetDanglingRawPtrReleasedFn()) {
        gDetected = false;
        gReleased = false;
        partition_alloc::SetDanglingRawPtrDetectedFn([](uintptr_t) { gDetected = true; });
        partition_alloc::SetDanglingRawPtrReleasedFn([](uintptr_t) { gReleased = true; });
    }

    ~ScopedDanglingPointerTracker() {
        partition_alloc::SetDanglingRawPtrDetectedFn(fOldDetectedFn);
        partition_alloc::SetDanglingRawPtrReleasedFn(fOldReleasedFn);
    }

    bool detected() const { return gDetected; }
    bool released() const { return gReleased; }

private:
    static inline bool gDetected = false;
    static inline bool gReleased = false;

    partition_alloc::DanglingRawPtrDetectedFn* fOldDetectedFn;
    partition_alloc::DanglingRawPtrReleasedFn* fOldReleasedFn;
};
}  // namespace

// This test temporarily affects global PartitionAlloc state by replacing
// detection and release handlers, so it cannot run in parallel.
DEF_SERIAL_TEST(RawPtr_DanglingPointerDetected, reporter) {
    ScopedDanglingPointerTracker tracker;

    std::unique_ptr<int> owner = std::make_unique<int>(42);
    raw_ptr<int> ptr = owner.get();
    REPORTER_ASSERT(reporter, !tracker.detected());
    REPORTER_ASSERT(reporter, !tracker.released());

    owner.reset();
    REPORTER_ASSERT(reporter, tracker.detected());
    REPORTER_ASSERT(reporter, !tracker.released());

    ptr = nullptr;
    REPORTER_ASSERT(reporter, tracker.detected());
    REPORTER_ASSERT(reporter, tracker.released());
}
#endif
#endif

DEF_TEST(RawPtr_DisableDanglingPtrDetection, reporter) {
    std::unique_ptr<int> owner = std::make_unique<int>(42);
    raw_ptr<int, DisableDanglingPtrDetection> ptr = owner.get();
    std::ignore = ptr;
    owner.reset();
}

DEF_TEST(RawPtr_TriviallyRelocatable, reporter) {
    static_assert(sk_is_trivially_relocatable_v<raw_ptr<int>>);
    static_assert(sk_is_trivially_relocatable_v<raw_ptr<int, DisableDanglingPtrDetection>>);
    static_assert(sk_is_trivially_relocatable_v<raw_ref<int>>);
    static_assert(sk_is_trivially_relocatable_v<raw_ref<int, DisableDanglingPtrDetection>>);
}

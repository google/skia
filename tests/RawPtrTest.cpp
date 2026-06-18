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
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>

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

namespace {
struct Base {
    virtual ~Base() = default;
    int base_val = 1;
};

struct Derived : public Base {
    int derived_val = 2;
};
}  // namespace

DEF_TEST(RawPtr_DisableDanglingPtrDetection, reporter) {
    std::unique_ptr<int> owner = std::make_unique<int>(42);
    raw_ptr<int, DisableDanglingPtrDetection> ptr = owner.get();
    std::ignore = ptr;
    owner.reset();
}

static_assert(sk_is_trivially_relocatable<raw_ptr<int>>::value);
static_assert(sk_is_trivially_relocatable<raw_ptr<int, DisableDanglingPtrDetection>>::value);
static_assert(sk_is_trivially_relocatable<raw_ref<int>>::value);
static_assert(sk_is_trivially_relocatable<raw_ref<int, DisableDanglingPtrDetection>>::value);
DEF_TEST(RawPtr_DefaultConstructor, reporter) {
    raw_ptr<int> p;
    REPORTER_ASSERT(reporter, p == nullptr);
    REPORTER_ASSERT(reporter, !p);
}

DEF_TEST(RawPtr_NullptrConstructor, reporter) {
    raw_ptr<int> p = nullptr;
    REPORTER_ASSERT(reporter, p == nullptr);
    REPORTER_ASSERT(reporter, !p);
}

DEF_TEST(RawPtr_RawPointerConstructor, reporter) {
    int val = 42;
    raw_ptr<int> p = &val;
    REPORTER_ASSERT(reporter, p == &val);
    REPORTER_ASSERT(reporter, p);
    REPORTER_ASSERT(reporter, *p == 42);
}

DEF_TEST(RawPtr_CopyAndMove, reporter) {
    int val = 42;
    raw_ptr<int> p1 = &val;

    // Copy.
    raw_ptr<int> p2 = p1;
    REPORTER_ASSERT(reporter, p2 == &val);
    REPORTER_ASSERT(reporter, p1 == &val);

    // Move.
    raw_ptr<int> p3 = std::move(p2);
    REPORTER_ASSERT(reporter, p3 == &val);
    REPORTER_ASSERT(reporter, p2 == nullptr);  // NOLINT(bugprone-use-after-move)
}

DEF_TEST(RawPtr_Assignment, reporter) {
    int val1 = 42;
    int val2 = 84;
    raw_ptr<int> p1 = &val1;
    raw_ptr<int> p2 = &val2;

    p1 = p2;
    REPORTER_ASSERT(reporter, p1 == &val2);

    p1 = nullptr;
    REPORTER_ASSERT(reporter, p1 == nullptr);

    p1 = &val1;
    REPORTER_ASSERT(reporter, p1 == &val1);
}

DEF_TEST(RawPtr_SelfAssignment, reporter) {
    int val = 42;
    raw_ptr<int> p = &val;

    // Self-copy-assignment.
    raw_ptr<int>* p_ptr = &p;
    p = *p_ptr;
    REPORTER_ASSERT(reporter, p == &val);

    // Self-move-assignment.
    p = std::move(*p_ptr);
    REPORTER_ASSERT(reporter, p == &val);
}

DEF_TEST(RawPtr_Upcasting, reporter) {
    Derived derived;
    raw_ptr<Derived> p_derived = &derived;

    // Copy construct Derived -> Base.
    raw_ptr<Base> p_base(p_derived);
    REPORTER_ASSERT(reporter, p_base == &derived);
    REPORTER_ASSERT(reporter, p_base->base_val == 1);

    // Assignment Derived -> Base.
    raw_ptr<Base> p_base2;
    p_base2 = p_derived;
    REPORTER_ASSERT(reporter, p_base2 == &derived);

    // Move construct Derived -> Base.
    raw_ptr<Derived> p_derived_move = &derived;
    raw_ptr<Base> p_base_move(std::move(p_derived_move));
    REPORTER_ASSERT(reporter, p_base_move == &derived);
    REPORTER_ASSERT(reporter, p_derived_move == nullptr);  // NOLINT(bugprone-use-after-move)

    // Move assignment Derived -> Base.
    raw_ptr<Derived> p_derived_move2 = &derived;
    raw_ptr<Base> p_base_move2;
    p_base_move2 = std::move(p_derived_move2);
    REPORTER_ASSERT(reporter, p_base_move2 == &derived);
    REPORTER_ASSERT(reporter, p_derived_move2 == nullptr);  // NOLINT(bugprone-use-after-move)
}

DEF_TEST(RawPtr_DereferenceAndMemberAccess, reporter) {
    struct Foo {
        int x = 42;
    };
    Foo foo;
    raw_ptr<Foo> p = &foo;
    REPORTER_ASSERT(reporter, p->x == 42);
    REPORTER_ASSERT(reporter, (*p).x == 42);
    REPORTER_ASSERT(reporter, p == &foo);
}

DEF_TEST(RawPtr_PointerArithmetic, reporter) {
    int arr[] = {10, 20, 30};
    int* expected = arr;
    raw_ptr<int, AllowPtrArithmetic> p = arr;

    // Verify the initial state.
    REPORTER_ASSERT(reporter, p == expected);

    // Test pre-increment.
    ++expected;
    raw_ptr<int, AllowPtrArithmetic>& p_ref = ++p;
    REPORTER_ASSERT(reporter, &p_ref == &p);
    REPORTER_ASSERT(reporter, p == expected);

    // Test post-increment.
    int* expected_old = expected++;
    raw_ptr<int, AllowPtrArithmetic> p_old = p++;
    REPORTER_ASSERT(reporter, p_old == expected_old);
    REPORTER_ASSERT(reporter, p == expected);

    // Test pre-decrement.
    --expected;
    raw_ptr<int, AllowPtrArithmetic>& p_ref2 = --p;
    REPORTER_ASSERT(reporter, &p_ref2 == &p);
    REPORTER_ASSERT(reporter, p == expected);

    // Test post-decrement.
    expected_old = expected--;
    p_old = p--;
    REPORTER_ASSERT(reporter, p_old == expected_old);
    REPORTER_ASSERT(reporter, p == expected);

    // Test addition.
    REPORTER_ASSERT(reporter, (p + 1) == (expected + 1));
    REPORTER_ASSERT(reporter, (p + 2) == (expected + 2));

    // Test subtraction.
    raw_ptr<int, AllowPtrArithmetic> p2 = p + 2;
    int* expected2 = expected + 2;
    REPORTER_ASSERT(reporter, (p2 - 1) == (expected2 - 1));
    REPORTER_ASSERT(reporter, (p2 - 2) == (expected2 - 2));

    // Test difference.
    REPORTER_ASSERT(reporter, (p2 - p) == (expected2 - expected));
}

DEF_TEST(RawPtr_EqualityComparisons, reporter) {
    int val1 = 42;
    int val2 = 84;
    raw_ptr<int> p1 = &val1;
    raw_ptr<int> p2 = &val2;
    raw_ptr<int> null_p = nullptr;

    REPORTER_ASSERT(reporter, p1 == p1);
    REPORTER_ASSERT(reporter, !(p1 == p2));
    REPORTER_ASSERT(reporter, p1 != p2);

    REPORTER_ASSERT(reporter, p1 == &val1);
    REPORTER_ASSERT(reporter, &val1 == p1);
    REPORTER_ASSERT(reporter, !(p1 == &val2));
    REPORTER_ASSERT(reporter, !(&val2 == p1));

    REPORTER_ASSERT(reporter, null_p == nullptr);
    REPORTER_ASSERT(reporter, nullptr == null_p);
    REPORTER_ASSERT(reporter, !(p1 == nullptr));
    REPORTER_ASSERT(reporter, !(nullptr == p1));
}

DEF_TEST(RawPtr_RelationalComparisons, reporter) {
    int arr[2] = {42, 84};
    raw_ptr<int> p0 = &arr[0];
    raw_ptr<int> p1_arr = &arr[1];

    REPORTER_ASSERT(reporter, p0 < p1_arr);
    REPORTER_ASSERT(reporter, p0 <= p1_arr);
    REPORTER_ASSERT(reporter, p1_arr > p0);
    REPORTER_ASSERT(reporter, p1_arr >= p0);

    REPORTER_ASSERT(reporter, !(p1_arr < p0));
    REPORTER_ASSERT(reporter, !(p1_arr <= p0));
    REPORTER_ASSERT(reporter, !(p0 > p1_arr));
    REPORTER_ASSERT(reporter, !(p0 >= p1_arr));

    REPORTER_ASSERT(reporter, p0 < &arr[1]);
    REPORTER_ASSERT(reporter, p0 <= &arr[1]);
    REPORTER_ASSERT(reporter, &arr[1] > p0);
    REPORTER_ASSERT(reporter, &arr[1] >= p0);
}

DEF_TEST(RawPtr_Void, reporter) {
    int val = 42;
    raw_ptr<void> p = &val;
    REPORTER_ASSERT(reporter, p == &val);
    REPORTER_ASSERT(reporter, p);

    raw_ptr<const void> cp = &val;
    REPORTER_ASSERT(reporter, cp == &val);
}

DEF_TEST(RawPtr_Stl, reporter) {
    int a = 1;
    int b = 2;
    raw_ptr<int> pa = &a;
    raw_ptr<int> pb = &b;

    // std::set.
    std::set<raw_ptr<int>> ptr_set;
    ptr_set.insert(pa);
    ptr_set.insert(pb);
    REPORTER_ASSERT(reporter, ptr_set.size() == 2);
    REPORTER_ASSERT(reporter, ptr_set.find(pa) != ptr_set.end());
    REPORTER_ASSERT(reporter, ptr_set.find(&a) != ptr_set.end());

    // std::unordered_set.
    std::unordered_set<raw_ptr<int>> ptr_uset;
    ptr_uset.insert(pa);
    ptr_uset.insert(pb);
    REPORTER_ASSERT(reporter, ptr_uset.size() == 2);
    REPORTER_ASSERT(reporter, ptr_uset.find(pa) != ptr_uset.end());

    // std::to_address.
    REPORTER_ASSERT(reporter, std::to_address(pa) == &a);
}

static_assert(sizeof(raw_ptr<int>) == sizeof(int*));

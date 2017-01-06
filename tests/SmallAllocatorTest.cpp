/*
 * Copyright 2014 Google, Inc
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSmallAllocator.h"
#include "SkTypes.h"
#include "Test.h"

class CountingClass {
public:
    CountingClass() {
        kCount++;
    }

    ~CountingClass() {
        kCount--;
    }

    static int GetCount() { return kCount; }

private:
    static int kCount;
};

int CountingClass::kCount;

template<uint32_t kMaxObjects, size_t kBytes> void test_allocator(skiatest::Reporter* reporter) {
    {
        SkSmallAllocator<kMaxObjects, kBytes> alloc;
        for (uint32_t i = 0; i < kMaxObjects + 1; ++i) {
            CountingClass* c = alloc.template createT<CountingClass>();
            REPORTER_ASSERT(reporter, c != nullptr);
            REPORTER_ASSERT(reporter, CountingClass::GetCount() == static_cast<int>(i+1));
        }
    }
    REPORTER_ASSERT(reporter, CountingClass::GetCount() == 0);
}

// Tests that ensure that the destructor is called, whether the objects
// were created in fStorage or on the heap.
DEF_TEST(SmallAllocator_destructor, reporter) {
    // Four times as many bytes as objects will never require any heap
    // allocations (since SkAlign4(sizeof(CountingClass)) == 4).
    test_allocator<5, 20>(reporter);
    test_allocator<10, 40>(reporter);
    test_allocator<20, 80>(reporter);

    // Allowing less bytes than objects means some will be allocated on the
    // heap. Don't run these in debug where we assert.
    test_allocator<50, 20>(reporter);
    test_allocator<100, 20>(reporter);
}

class Dummy {
};

class DummyContainer {
public:
    explicit DummyContainer(Dummy* d)
        :fDummy(d)
    {}

    Dummy* getDummy() const { return fDummy; }

private:
    Dummy* fDummy;
};

// Test that using a createT with a constructor taking a pointer as a
// parameter works as expected.
DEF_TEST(SmallAllocator_pointer, reporter) {
    SkSmallAllocator<1, 8> alloc;
    Dummy d;
    DummyContainer* container = alloc.createT<DummyContainer>(&d);
    REPORTER_ASSERT(reporter, container != nullptr);
    REPORTER_ASSERT(reporter, container->getDummy() == &d);
}

// Test that using a createWithIniterT works as expected.
DEF_TEST(SmallAllocator_initer, reporter) {
    SkSmallAllocator<1, 8> alloc;
    Dummy d;
    DummyContainer* container = alloc.createWithIniter(
        sizeof(DummyContainer),
        [&](void* storage) {
            return new (storage) DummyContainer(&d);
        });
    REPORTER_ASSERT(reporter, container != nullptr);
    REPORTER_ASSERT(reporter, container->getDummy() == &d);
}

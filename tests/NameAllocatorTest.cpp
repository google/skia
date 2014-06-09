/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "gl/GrGLNameAllocator.h"
#include "Test.h"

////////////////////////////////////////////////////////////////////////////////

class NameLeakTest {
    static const GrGLuint kFirstName = 101;
    static const GrGLuint kRange = 1013;

public:
    NameLeakTest(skiatest::Reporter* reporter)
        : fReporter(reporter),
          fAllocator(kFirstName, kFirstName + kRange),
          fAllocatedCount(0),
          fRandomName(kFirstName + 4 * kRange / 7) {
        memset(fAllocatedNames, 0, sizeof(fAllocatedNames));
    }

    bool run() {
        if (!this->allocateAllRemaining()) {
            return false;
        }

        for (GrGLuint freeCount = 1; freeCount <= kRange; ++freeCount) {
            if (!this->freeRandomNames(freeCount)) {
                return false;
            }
            if (!this->allocateAllRemaining()) {
                return false;
            }
        }

        return true;
    }

private:
    bool isAllocated(GrGLuint name) const {
        return fAllocatedNames[name - kFirstName];
    }

    void setAllocated(GrGLuint name, bool allocated) {
        fAllocatedNames[name - kFirstName] = allocated;
    }

    bool allocateAllRemaining() {
        for (; fAllocatedCount < kRange; ++fAllocatedCount) {
            GrGLuint name = fAllocator.allocateName();
            if (0 == name) {
                ERRORF(fReporter,
                       "Name allocate failed, but there should still be %u free names",
                       kRange - fAllocatedCount);
                return false;
            }
            if (name < kFirstName || name >= kFirstName + kRange) {
                ERRORF(fReporter,
                       "Name allocate returned name %u outside its bounds [%u, %u)",
                       name, kFirstName, kFirstName + kRange);
                return false;
            }
            if (this->isAllocated(name)) {
                ERRORF(fReporter, "Name allocate returned name that is already allocated");
                return false;
            }

            this->setAllocated(name, true);
        }

        // Ensure it returns 0 once all the names are allocated.
        GrGLuint name = fAllocator.allocateName();
        if (0 != name) {
            ERRORF(fReporter,
                   "Name allocate did not fail when all names were already in use");
            return false;
        }

        // Ensure every unique name is allocated.
        for (GrGLuint i = 0; i < kRange; ++i) {
            if (!this->isAllocated(kFirstName + i)) {
                ERRORF(fReporter, "Not all unique names are allocated after allocateAllRemaining()");
                return false;
            }
        }

        return true;
    }

    bool freeRandomNames(GrGLuint count) {
        // The values a and c make up an LCG (pseudo-random generator). These
        // values must satisfy the Hull-Dobell Theorem (with m=kRange):
        // http://en.wikipedia.org/wiki/Linear_congruential_generator
        // We use our own generator to guarantee it hits each unique value
        // within kRange exactly once before repeating.
        const GrGLuint seed = (count + fRandomName) / 2;
        const GrGLuint a = seed * kRange + 1;
        const GrGLuint c = (seed * 743) % kRange;

        for (GrGLuint i = 0; i < count; ++i) {
            fRandomName = (a * fRandomName + c) % kRange;
            const GrGLuint name = kFirstName + fRandomName;
            if (!this->isAllocated(name)) {
                ERRORF(fReporter, "Test bug: Should not free a not-allocated name at this point (%u)", i);
                return false;
            }

            fAllocator.free(name);
            this->setAllocated(name, false);
            --fAllocatedCount;
        }

        return true;
    }

    skiatest::Reporter* fReporter;
    GrGLNameAllocator fAllocator;
    bool fAllocatedNames[kRange];
    GrGLuint fAllocatedCount;
    GrGLuint fRandomName;
};

DEF_GPUTEST(NameAllocator, reporter, factory) {
    // Ensure no names are leaked or double-allocated during heavy usage.
    {
        NameLeakTest nameLeakTest(reporter);
        nameLeakTest.run();
    }

    static const GrGLuint range = 32;
    GrGLNameAllocator allocator(1, 1 + range);
    for (GrGLuint i = 1; i <= range; ++i) {
        allocator.allocateName();
    }
    REPORTER_ASSERT(reporter, 0 == allocator.allocateName());

    // Test freeing names out of range.
    allocator.free(allocator.firstName() - 1);
    allocator.free(allocator.endName());
    REPORTER_ASSERT(reporter, 0 == allocator.allocateName());

    // Test freeing not-allocated names.
    for (GrGLuint i = 1; i <= range/2; i += 2) {
        allocator.free(i);
    }
    for (GrGLuint i = 1; i <= range/2; i += 2) {
        // None of these names will be allocated.
        allocator.free(i);
    }
    for (GrGLuint i = 1; i <= range/2; ++i) {
        // Every other name will not be be allocated.
        allocator.free(i);
    }
    for (GrGLuint i = 1; i <= range/2; ++i) {
        if (0 == allocator.allocateName()) {
            ERRORF(reporter, "Name allocate failed when there should be free names");
            break;
        }
    }
    REPORTER_ASSERT(reporter, 0 == allocator.allocateName());
}

#endif

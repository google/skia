/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/GrAffinedStorage.h"

DEF_TEST(GrAffinedStorageTest, reporter) {
    class Owner {};
    Owner owner1, owner2;
    GrAffinedStorage<void, int> storage{0};
    REPORTER_ASSERT(reporter,   storage.set(&owner1, 1));
    REPORTER_ASSERT(reporter,  !storage.set(&owner2, 2));
    REPORTER_ASSERT(reporter,   1 == *storage.get(&owner1));
    REPORTER_ASSERT(reporter,   storage.set(&owner1, 2));
    REPORTER_ASSERT(reporter,   2 == *storage.get(&owner1));
    REPORTER_ASSERT(reporter,  !storage.release(&owner2));
    REPORTER_ASSERT(reporter,   storage.set(&owner1, 3));
    REPORTER_ASSERT(reporter,   3 == *storage.get(&owner1));
    REPORTER_ASSERT(reporter,   storage.release(&owner1));
    REPORTER_ASSERT(reporter,  !storage.get(&owner1));
    REPORTER_ASSERT(reporter,   storage.set(&owner2, 4));
    REPORTER_ASSERT(reporter,   4 == *storage.get(&owner2));
}

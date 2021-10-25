/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "tests/Test.h"

enum class Flags {
    kNone = 0,
    kA = 1,
    kB = 2,
    kC = 4
};

SKGPU_MAKE_MASK_OPS(Flags);

using namespace skgpu;

DEF_GRAPHITE_TEST(skgpu_Mask, r) {
    Mask<Flags> flags = Flags::kNone;
    REPORTER_ASSERT(r, !flags);
    flags |= Flags::kA;
    REPORTER_ASSERT(r, flags);
    REPORTER_ASSERT(r, !(flags & Flags::kB));
    REPORTER_ASSERT(r, (flags & Flags::kA));
    flags |= (Flags::kB | Flags::kC);
    auto mask = Flags::kB | Flags::kC;
    REPORTER_ASSERT(r, (flags & mask) == mask);
    REPORTER_ASSERT(r, flags == (Flags::kA | Flags::kB | Flags::kC));
    flags &= ~Flags::kC;
    REPORTER_ASSERT(r, flags == (Flags::kA | Flags::kB));
    REPORTER_ASSERT(r, (flags & mask) != mask);
    REPORTER_ASSERT(r, (flags & mask) != Flags::kNone);
    REPORTER_ASSERT(r, (flags & mask));
    REPORTER_ASSERT(r, flags);
    flags &= ~Flags::kB;
    REPORTER_ASSERT(r, (flags & mask) == Flags::kNone);
    REPORTER_ASSERT(r, !(flags & mask));
    REPORTER_ASSERT(r, flags);
    flags = (flags | flags) ^ (flags);
    REPORTER_ASSERT(r, !flags);
    flags ^= mask;
    REPORTER_ASSERT(r, flags == mask);
    REPORTER_ASSERT(r, !(Flags::kA & Flags::kB));
    REPORTER_ASSERT(r, (Flags::kA ^ Flags::kB) == (Flags::kA | Flags::kB));
}

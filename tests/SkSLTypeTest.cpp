/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <limits>

#include "src/gpu/GrCaps.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "tests/Test.h"

DEF_TEST(SkSLTypeLimits, r) {
    GrShaderCaps caps(GrContextOptions{});
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::Context context(errors, caps);

    using int_limits = std::numeric_limits<int32_t>;
    REPORTER_ASSERT(r, context.fTypes.fInt->minimumValue() == int_limits::min());
    REPORTER_ASSERT(r, context.fTypes.fInt->maximumValue() == int_limits::max());

    using short_limits = std::numeric_limits<int16_t>;
    REPORTER_ASSERT(r, context.fTypes.fShort->minimumValue() == short_limits::min());
    REPORTER_ASSERT(r, context.fTypes.fShort->maximumValue() == short_limits::max());

    using uint_limits = std::numeric_limits<uint32_t>;
    REPORTER_ASSERT(r, context.fTypes.fUInt->minimumValue() == uint_limits::min());
    REPORTER_ASSERT(r, context.fTypes.fUInt->maximumValue() == uint_limits::max());

    using ushort_limits = std::numeric_limits<uint16_t>;
    REPORTER_ASSERT(r, context.fTypes.fUShort->minimumValue() == ushort_limits::min());
    REPORTER_ASSERT(r, context.fTypes.fUShort->maximumValue() == ushort_limits::max());
}

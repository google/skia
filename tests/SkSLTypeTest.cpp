/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/ir/SkSLType.h"
#include "tests/Test.h"

#include <cstdint>
#include <limits>
#include <memory>

DEF_TEST(SkSLTypeLimits, r) {
    SkSL::BuiltinTypes types;

    using int_limits = std::numeric_limits<int32_t>;
    REPORTER_ASSERT(r, types.fInt->minimumValue() == int_limits::lowest());
    REPORTER_ASSERT(r, types.fInt->maximumValue() == int_limits::max());

    using short_limits = std::numeric_limits<int16_t>;
    REPORTER_ASSERT(r, types.fShort->minimumValue() == short_limits::lowest());
    REPORTER_ASSERT(r, types.fShort->maximumValue() == short_limits::max());

    using uint_limits = std::numeric_limits<uint32_t>;
    REPORTER_ASSERT(r, types.fUInt->minimumValue() == uint_limits::lowest());
    REPORTER_ASSERT(r, types.fUInt->maximumValue() == uint_limits::max());

    using ushort_limits = std::numeric_limits<uint16_t>;
    REPORTER_ASSERT(r, types.fUShort->minimumValue() == ushort_limits::lowest());
    REPORTER_ASSERT(r, types.fUShort->maximumValue() == ushort_limits::max());

    using float_limits = std::numeric_limits<float>;
    REPORTER_ASSERT(r, types.fFloat->minimumValue() == float_limits::lowest());
    REPORTER_ASSERT(r, types.fFloat->maximumValue() == float_limits::max());
}

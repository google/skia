//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicHLSL unittests:
//   Tests for DynamicHLSL and related classes.
//

#include <gtest/gtest.h>

#include "libANGLE/renderer/d3d/DynamicHLSL.h"

using namespace rx;

namespace
{

TEST(PackedVarying, DefaultInitialization)
{
    sh::Varying defaultVarying;
    PackedVarying pv(defaultVarying);

    EXPECT_EQ(&defaultVarying, pv.varying);
    EXPECT_EQ(GL_INVALID_INDEX, pv.registerIndex);
    EXPECT_EQ(0, pv.columnIndex);
    EXPECT_FALSE(pv.vertexOnly);
}

}  // anonymous namespace

//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// API_test.cpp:
//   Some tests for the compiler API.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

TEST(APITest, CompareShBuiltInResources)
{
    ShBuiltInResources a_resources;
    memset(&a_resources, 88, sizeof(a_resources));
    ShInitBuiltInResources(&a_resources);

    ShBuiltInResources b_resources;
    memset(&b_resources, 77, sizeof(b_resources));
    ShInitBuiltInResources(&b_resources);

    EXPECT_TRUE(memcmp(&a_resources, &b_resources, sizeof(a_resources)) == 0);
}


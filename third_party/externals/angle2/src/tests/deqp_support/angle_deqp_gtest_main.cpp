//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_deqp_gtest_main:
//   Entry point for standalone dEQP tests.

#include <gtest/gtest.h>

#include "angle_deqp_libtester.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    int rt = RUN_ALL_TESTS();
    deqp_libtester_shutdown_platform();
    return rt;
}

//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef DEQP_TESTS_DEQP_TESTS_H_
#define DEQP_TESTS_DEQP_TESTS_H_

#include "gtest/gtest.h"

#include <EGL/egl.h>

#include <string>

struct DEQPConfig
{
    size_t width;
    size_t height;
    bool hidden;
    EGLNativeDisplayType displayType;
};

void SetCurrentConfig(const DEQPConfig& config);
const DEQPConfig& GetCurrentConfig();

void RunDEQPTest(const std::string &testPath, const DEQPConfig& config);

#endif // DEQP_TESTS_DEQP_TESTS_H_

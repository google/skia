//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"

#include "MockDiagnostics.h"
#include "MockDirectiveHandler.h"
#include "compiler/preprocessor/Preprocessor.h"

#ifndef PREPROCESSOR_TESTS_PREPROCESSOR_TEST_H_
#define PREPROCESSOR_TESTS_PREPROCESSOR_TEST_H_

class PreprocessorTest : public testing::Test
{
  protected:
    PreprocessorTest() : mPreprocessor(&mDiagnostics, &mDirectiveHandler) { }

    // Preprocesses the input string and verifies that it matches
    // expected output.
    void preprocess(const char* input, const char* expected);

    MockDiagnostics mDiagnostics;
    MockDirectiveHandler mDirectiveHandler;
    pp::Preprocessor mPreprocessor;
};

#endif  // PREPROCESSOR_TESTS_PREPROCESSOR_TEST_H_

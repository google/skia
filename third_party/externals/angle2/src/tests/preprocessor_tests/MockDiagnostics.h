//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef PREPROCESSOR_TESTS_MOCK_DIAGNOSTICS_H_
#define PREPROCESSOR_TESTS_MOCK_DIAGNOSTICS_H_

#include "gmock/gmock.h"
#include "compiler/preprocessor/DiagnosticsBase.h"

class MockDiagnostics : public pp::Diagnostics
{
  public:
    MOCK_METHOD3(print,
        void(ID id, const pp::SourceLocation& loc, const std::string& text));
};

#endif  // PREPROCESSOR_TESTS_MOCK_DIAGNOSTICS_H_

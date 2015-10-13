//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

class CompilerTestEnvironment : public testing::Environment
{
  public:
    virtual void SetUp()
    {
        if (!ShInitialize())
        {
            FAIL() << "Failed to initialize the compiler.";
        }
    }

    virtual void TearDown()
    {
        if (!ShFinalize())
        {
            FAIL() << "Failed to finalize the compiler.";
        }
    }
};

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new CompilerTestEnvironment());
    int rt = RUN_ALL_TESTS();
    return rt;
}

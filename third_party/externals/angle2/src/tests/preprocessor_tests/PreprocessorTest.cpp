//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

void PreprocessorTest::preprocess(const char* input, const char* expected)
{
    ASSERT_TRUE(mPreprocessor.init(1, &input, NULL));

    int line = 1;
    pp::Token token;
    std::stringstream stream;
    do
    {
        mPreprocessor.lex(&token);
        for (; line < token.location.line; ++line)
        {
            stream << "\n";
        }
        stream << token;
    } while (token.type != pp::Token::LAST);

    std::string actual = stream.str();
    EXPECT_STREQ(expected, actual.c_str());
}

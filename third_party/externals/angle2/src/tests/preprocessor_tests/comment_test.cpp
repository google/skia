//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

class CommentTest : public PreprocessorTest,
                    public testing::WithParamInterface<const char*>
{
};

TEST_P(CommentTest, CommentIgnored)
{
    const char* str = GetParam();

    ASSERT_TRUE(mPreprocessor.init(1, &str, 0));

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::LAST, token.type);
}

INSTANTIATE_TEST_CASE_P(LineComment, CommentTest,
                        testing::Values("//foo\n", // With newline.
                                        "//foo",   // Without newline.
                                        "//**/",   // Nested block comment.
                                        "////",    // Nested line comment.
                                        "//\""));  // Invalid character.  

INSTANTIATE_TEST_CASE_P(BlockComment, CommentTest,
                        testing::Values("/*foo*/",
                                        "/*foo\n*/", // With newline.
                                        "/*//*/",    // Nested line comment.
                                        "/*/**/",    // Nested block comment.
                                        "/***/",     // With lone '*'.
                                        "/*\"*/"));  // Invalid character.

class BlockCommentTest : public PreprocessorTest
{
};

TEST_F(BlockCommentTest, CommentReplacedWithSpace)
{
    const char* str = "/*foo*/bar";

    ASSERT_TRUE(mPreprocessor.init(1, &str, 0));

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::IDENTIFIER, token.type);
    EXPECT_EQ("bar", token.text);
    EXPECT_TRUE(token.hasLeadingSpace());
}

TEST_F(BlockCommentTest, UnterminatedComment)
{
    const char* str = "/*foo";

    ASSERT_TRUE(mPreprocessor.init(1, &str, 0));

    using testing::_;
    EXPECT_CALL(mDiagnostics, print(pp::Diagnostics::PP_EOF_IN_COMMENT, _, _));

    pp::Token token;
    mPreprocessor.lex(&token);
}

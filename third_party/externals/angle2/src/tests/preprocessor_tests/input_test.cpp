//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Input.h"
#include "compiler/preprocessor/Token.h"

class InitTest : public PreprocessorTest
{
};

TEST_F(InitTest, ZeroCount)
{
    EXPECT_TRUE(mPreprocessor.init(0, NULL, NULL));

    pp::Token token;
    mPreprocessor.lex(&token);
    EXPECT_EQ(pp::Token::LAST, token.type);
}

TEST_F(InitTest, NullString)
{
    EXPECT_FALSE(mPreprocessor.init(1, NULL, NULL));
}

TEST(InputTest, DefaultConstructor)
{
    pp::Input input;
    EXPECT_EQ(0u, input.count());
    int lineNo = 0;
    EXPECT_EQ(0u, input.read(NULL, 1, &lineNo));
}

TEST(InputTest, NullLength)
{
    const char* str[] = {"foo"};
    pp::Input input(1, str, NULL);
    EXPECT_EQ(3u, input.length(0));
}

TEST(InputTest, NegativeLength)
{
    const char* str[] = {"foo"};
    int length[] = {-1};
    pp::Input input(1, str, length);
    EXPECT_EQ(3u, input.length(0));
}

TEST(InputTest, ActualLength)
{
    const char* str[] = {"foobar"};
    int length[] = {3};
    pp::Input input(1, str, length);
    // Note that strlen(str[0]) != length[0].
    // Even then Input should just accept any non-negative number.
    EXPECT_EQ(static_cast<size_t>(length[0]), input.length(0));
}

TEST(InputTest, String)
{
    const char* str[] = {"foo"};
    pp::Input input(1, str, NULL);
    EXPECT_STREQ(str[0], input.string(0));
}

TEST(InputTest, ReadSingleString)
{
    int count = 1;
    const char* str[] = {"foo"};
    char buf[4] = {'\0', '\0', '\0', '\0'};

    int maxSize = 1;
    int lineNo = 0;
    pp::Input input1(count, str, NULL);
    EXPECT_EQ(1u, input1.read(buf, maxSize, &lineNo));
    EXPECT_EQ('f', buf[0]);
    EXPECT_EQ(1u, input1.read(buf, maxSize, &lineNo));
    EXPECT_EQ('o', buf[0]);
    EXPECT_EQ(1u, input1.read(buf, maxSize, &lineNo));
    EXPECT_EQ('o', buf[0]);
    EXPECT_EQ(0u, input1.read(buf, maxSize, &lineNo));

    maxSize = 2;
    pp::Input input2(count, str, NULL);
    EXPECT_EQ(2u, input2.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("fo", buf);
    EXPECT_EQ(1u, input2.read(buf, maxSize, &lineNo));
    EXPECT_EQ('o', buf[0]);
    EXPECT_EQ(0u, input2.read(buf, maxSize, &lineNo));

    maxSize = 3;
    pp::Input input3(count, str, NULL);
    EXPECT_EQ(3u, input3.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("foo", buf);
    EXPECT_EQ(0u, input3.read(buf, maxSize, &lineNo));

    maxSize = 4;
    pp::Input input4(count, str, NULL);
    EXPECT_EQ(3u, input4.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("foo", buf);
    EXPECT_EQ(0u, input4.read(buf, maxSize, &lineNo));
}

TEST(InputTest, ReadMultipleStrings)
{
    int count = 3;
    const char* str[] = {"f", "o", "o"};
    char buf[4] = {'\0', '\0', '\0', '\0'};

    int maxSize = 1;
    int lineNo = 0;
    pp::Input input1(count, str, NULL);
    EXPECT_EQ(1u, input1.read(buf, maxSize, &lineNo));
    EXPECT_EQ('f', buf[0]);
    EXPECT_EQ(1u, input1.read(buf, maxSize, &lineNo));
    EXPECT_EQ('o', buf[0]);
    EXPECT_EQ(1u, input1.read(buf, maxSize, &lineNo));
    EXPECT_EQ('o', buf[0]);
    EXPECT_EQ(0u, input1.read(buf, maxSize, &lineNo));

    maxSize = 2;
    pp::Input input2(count, str, NULL);
    EXPECT_EQ(2u, input2.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("fo", buf);
    EXPECT_EQ(1u, input2.read(buf, maxSize, &lineNo));
    EXPECT_EQ('o', buf[0]);
    EXPECT_EQ(0u, input2.read(buf, maxSize, &lineNo));

    maxSize = 3;
    pp::Input input3(count, str, NULL);
    EXPECT_EQ(3u, input3.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("foo", buf);
    EXPECT_EQ(0u, input3.read(buf, maxSize, &lineNo));

    maxSize = 4;
    pp::Input input4(count, str, NULL);
    EXPECT_EQ(3u, input4.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("foo", buf);
    EXPECT_EQ(0u, input4.read(buf, maxSize, &lineNo));
}

TEST(InputTest, ReadStringsWithLength)
{
    int count = 2;
    const char* str[] = {"foo", "bar"};
    // Note that the length for the first string is 2 which is less than
    // strlen(str[0]. We want to make sure that the last character is ignored.
    int length[] = {2, 3};
    char buf[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
    size_t maxSize = 5;
    int lineNo = 0;

    pp::Input input(count, str, length);
    EXPECT_EQ(maxSize, input.read(buf, maxSize, &lineNo));
    EXPECT_STREQ("fobar", buf);
}

TEST(InputTest, ReadStringsWithLineContinuation)
{
    int count = 2;
    const char* str[] = {"foo\\", "\nba\\\r\nr"};
    int length[] = {4, 7};
    char buf[11] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    size_t maxSize = 11;
    int lineNo = 0;

    pp::Input input(count, str, length);
    EXPECT_EQ(3u, input.read(buf, maxSize, &lineNo));
    EXPECT_EQ(0, lineNo);
    EXPECT_EQ(2u, input.read(buf + 3, maxSize - 3, &lineNo));
    EXPECT_EQ(1, lineNo);
    EXPECT_EQ(1u, input.read(buf + 5, maxSize - 5, &lineNo));
    EXPECT_EQ(2, lineNo);
    EXPECT_STREQ("foobar", buf);
}

/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "tests/Test.h"
#include <array>
#include <vector>

DEF_TEST(SkSpanBasicTemplateGuide, reporter) {
    // Test constness preservation for SkMakeSpan.
    {
        std::vector<int> v = {{1, 2, 3, 4, 5}};
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> t = {{1, 2, 3, 4, 5}};
        const std::vector<int>& v = t;
        auto s = SkSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> v = {{1, 2, 3, 4, 5}};
        auto s = SkSpan(v); // {1, 2, 3, 4, 5}
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100; // {1, 2, 3, 100, 5}
        REPORTER_ASSERT(reporter, s[3] == 100);
        auto s1 = s.subspan(1,3); // {2, 3, 100}
        REPORTER_ASSERT(reporter, s1.size() == 3);
        REPORTER_ASSERT(reporter, s1.front() == 2);
        REPORTER_ASSERT(reporter, s1.back() == 100);
        auto s2 = s.subspan(2); // {3, 100, 5}
        REPORTER_ASSERT(reporter, s2.size() == 3);
        REPORTER_ASSERT(reporter, s2.front() == 3);
        REPORTER_ASSERT(reporter, s2.back() == 5);
    }

    {
        std::array<int, 5> t = {{1, 2, 3, 4, 5}};
        const std::array<int, 5>& v = t;
        auto s = SkSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> v;
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, s.empty());
    }

    {
        auto s = SkSpan({1, 2, 3});
        REPORTER_ASSERT(reporter, s.size() == 3);
    }
}

DEF_TEST(SkSpanBasicMakeSpan, reporter) {
    // Test constness preservation for SkMakeSpan.
    {
        std::vector<int> v = {{1, 2, 3, 4, 5}};
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> t = {{1, 2, 3, 4, 5}};
        const std::vector<int>& v = t;
        auto s = SkMakeSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::array<int, 5> v = {{1, 2, 3, 4, 5}};
        auto s = SkMakeSpan(v); // {1, 2, 3, 4, 5}
        REPORTER_ASSERT(reporter, s[3] == 4);
        s[3] = 100; // {1, 2, 3, 100, 5}
        REPORTER_ASSERT(reporter, s[3] == 100);
        auto s1 = s.subspan(1,3); // {2, 3, 100}
        REPORTER_ASSERT(reporter, s1.size() == 3);
        REPORTER_ASSERT(reporter, s1.front() == 2);
        REPORTER_ASSERT(reporter, s1.back() == 100);
        auto s2 = s.subspan(2); // {3, 100, 5}
        REPORTER_ASSERT(reporter, s2.size() == 3);
        REPORTER_ASSERT(reporter, s2.front() == 3);
        REPORTER_ASSERT(reporter, s2.back() == 5);
    }

    {
        std::array<int, 5> t = {{1, 2, 3, 4, 5}};
        const std::array<int, 5>& v = t;
        auto s = SkMakeSpan(v);
        //s[3] = 100; // Should fail to compile
        REPORTER_ASSERT(reporter, s[3] == 4);
        REPORTER_ASSERT(reporter, t[3] == 4);
        t[3] = 100;
        REPORTER_ASSERT(reporter, s[3] == 100);
    }

    {
        std::vector<int> v;
        auto s = SkMakeSpan(v);
        REPORTER_ASSERT(reporter, s.empty());
    }
}

static bool test_span_parameter(SkSpan<const int> s) {
    return s[0] == 1 && s[1] == 2 && s[2] == 3;
}

DEF_TEST(SkSpanDeduceParam, reporter) {
    {
        std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
    }

    {
        int v[]{1, 2, 3};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
    }

    {
        test_span_parameter({1, 2, 3});
        REPORTER_ASSERT(reporter, test_span_parameter({1, 2, 3}));
    }

    {
        int v[]{1, 2, 3};
        auto s = SkSpan(v);
        REPORTER_ASSERT(reporter, test_span_parameter(s));
    }
}

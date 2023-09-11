/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

DEF_TEST(SkSpanBasicTemplateGuide, reporter) {
    // Test constness preservation for SkSpan.
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

    auto routine = [&](SkSpan<const int> a) {
        REPORTER_ASSERT(reporter, a.size() == 4);
    };
    routine({1,2,3,4});
}

static bool test_span_parameter(SkSpan<const int> s) {
    return s[0] == 1 && s[1] == 2 && s[2] == 3;
}

DEF_TEST(SkSpanDeduceParam, reporter) {
    {
        std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
        REPORTER_ASSERT(reporter, test_span_parameter(std::move(v)));
    }

    {
        const std::vector<int> v = {{1, 2, 3}};
        REPORTER_ASSERT(reporter, test_span_parameter(v));
        REPORTER_ASSERT(reporter, test_span_parameter(std::move(v)));
    }

    REPORTER_ASSERT(reporter, test_span_parameter(std::vector<int>{1,2,3}));

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

DEF_TEST(SkSpanDeduceSize, reporter) {
    int d[] = {1, 2, 3, 4, 5};
    {
        int s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        uint32_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        size_t s = std::size(d);
        SkSpan span = SkSpan{d, s};
        REPORTER_ASSERT(reporter, span.size() == std::size(d));
    }
    {
        struct C {
            int* data() { return nullptr; }
            int size() const { return 0; }
        };

        C c;
        SkSpan span = SkSpan(c);
        REPORTER_ASSERT(reporter, span.size() == 0);
    }
}

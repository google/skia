/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStringView.h"

#include "tests/Test.h"

DEF_TEST(SkStringViewConstructors, r) {
    skstd::string_view empty;
    REPORTER_ASSERT(r, empty.data() == nullptr);
    REPORTER_ASSERT(r, empty.length() == 0);

    const char* str = "Hello, World!";
    skstd::string_view helloWorld(str);
    REPORTER_ASSERT(r, helloWorld.data() == str);
    REPORTER_ASSERT(r, helloWorld.length() == strlen(str));

    skstd::string_view hello(str, 5);
    REPORTER_ASSERT(r, hello.data() == str);
    REPORTER_ASSERT(r, hello.length() == 5);

    skstd::string_view copy(hello);
    REPORTER_ASSERT(r, copy.data() == str);
    REPORTER_ASSERT(r, copy.length() == 5);

    copy = helloWorld;
    REPORTER_ASSERT(r, copy.data() == str);
    REPORTER_ASSERT(r, copy.length() == strlen(str));
}

DEF_TEST(SkStringViewBasics, r) {
    skstd::string_view empty("");
    REPORTER_ASSERT(r, empty.empty());
    REPORTER_ASSERT(r, !empty.starts_with('x'));
    REPORTER_ASSERT(r, !empty.ends_with('x'));
    REPORTER_ASSERT(r, !empty.starts_with("x"));
    REPORTER_ASSERT(r, !empty.ends_with("x"));
    REPORTER_ASSERT(r, empty.starts_with(""));
    REPORTER_ASSERT(r, empty.ends_with(""));

    skstd::string_view xyz("xyz");
    REPORTER_ASSERT(r, !xyz.empty());
    REPORTER_ASSERT(r, xyz.front() == 'x');
    REPORTER_ASSERT(r, xyz.back() == 'z');
    REPORTER_ASSERT(r, xyz.length() == 3);

    REPORTER_ASSERT(r, xyz.starts_with('x'));
    REPORTER_ASSERT(r, !xyz.starts_with('y'));
    REPORTER_ASSERT(r, xyz.ends_with('z'));
    REPORTER_ASSERT(r, !xyz.ends_with('y'));

    REPORTER_ASSERT(r, xyz.starts_with(""));
    REPORTER_ASSERT(r, xyz.ends_with(""));
    REPORTER_ASSERT(r, xyz.starts_with("x"));
    REPORTER_ASSERT(r, xyz.ends_with("z"));
    REPORTER_ASSERT(r, !xyz.starts_with("xa"));
    REPORTER_ASSERT(r, !xyz.ends_with("az"));
    REPORTER_ASSERT(r, xyz.starts_with("xy"));
    REPORTER_ASSERT(r, xyz.ends_with("yz"));
    REPORTER_ASSERT(r, xyz.starts_with("xyz"));
    REPORTER_ASSERT(r, xyz.ends_with("xyz"));
    REPORTER_ASSERT(r, !xyz.starts_with("wxyz"));
    REPORTER_ASSERT(r, !xyz.ends_with("wxyz"));

    xyz.swap(empty);
    REPORTER_ASSERT(r, xyz == "");
    REPORTER_ASSERT(r, empty == "xyz");
}

DEF_TEST(SkStringViewIterator, r) {
    skstd::string_view str("abc");
    skstd::string_view::iterator iter = str.begin();
    REPORTER_ASSERT(r, *(iter++) == 'a');
    REPORTER_ASSERT(r, *(iter++) == 'b');
    REPORTER_ASSERT(r, *(iter++) == 'c');
    REPORTER_ASSERT(r, iter == str.end());
    REPORTER_ASSERT(r, *(--iter) == 'c');
    REPORTER_ASSERT(r, *(--iter) == 'b');
    REPORTER_ASSERT(r, *(--iter) == 'a');
    REPORTER_ASSERT(r, iter == str.begin());

    skstd::string_view empty;
    REPORTER_ASSERT(r, empty.begin() == empty.end());
}

DEF_TEST(SkStringViewOperators, r) {
    skstd::string_view empty;
    REPORTER_ASSERT(r, empty == empty);
    REPORTER_ASSERT(r, empty == "");
    REPORTER_ASSERT(r, "" == empty);
    REPORTER_ASSERT(r, !(empty != ""));

    skstd::string_view str("abc");
    REPORTER_ASSERT(r, str == str);
    REPORTER_ASSERT(r, str == "abc");
    REPORTER_ASSERT(r, "abc" == str);
    REPORTER_ASSERT(r, str != "");
    REPORTER_ASSERT(r, "" != str);
    REPORTER_ASSERT(r, str != "abcd");
    REPORTER_ASSERT(r, "abcd" != str);

    skstd::string_view str2("abcd");
    REPORTER_ASSERT(r, str < str2);
    REPORTER_ASSERT(r, str <= str2);
    REPORTER_ASSERT(r, str <= str);
    REPORTER_ASSERT(r, str2 > str);
    REPORTER_ASSERT(r, str2 >= str);
    REPORTER_ASSERT(r, str >= str);
    REPORTER_ASSERT(r, !(str2 < str));
    REPORTER_ASSERT(r, !(str < str));
    REPORTER_ASSERT(r, !(str2 <= str));
    REPORTER_ASSERT(r, !(str > str2));
    REPORTER_ASSERT(r, !(str > str));
    REPORTER_ASSERT(r, !(str >= str2));

    REPORTER_ASSERT(r, str < "b");
    REPORTER_ASSERT(r, str <= "b");
    REPORTER_ASSERT(r, str <= str);
    REPORTER_ASSERT(r, "b" > str);
    REPORTER_ASSERT(r, "b" >= str);
    REPORTER_ASSERT(r, str >= str);
    REPORTER_ASSERT(r, !("b" < str));
    REPORTER_ASSERT(r, !(str < str));
    REPORTER_ASSERT(r, !("b" <= str));
    REPORTER_ASSERT(r, !(str > "b"));
    REPORTER_ASSERT(r, !(str > str));
    REPORTER_ASSERT(r, !(str >= "b"));
}

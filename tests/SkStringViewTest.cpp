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

DEF_TEST(SkStringViewSubstr, r) {
    skstd::string_view xyz("xyz");
    REPORTER_ASSERT(r, xyz.substr() == xyz);
    REPORTER_ASSERT(r, xyz.substr(0, 1) == "x");
    REPORTER_ASSERT(r, xyz.substr(0, 2) == "xy");
    REPORTER_ASSERT(r, xyz.substr(0, 3) == "xyz");
    REPORTER_ASSERT(r, xyz.substr(0, 4) == "xyz");

    REPORTER_ASSERT(r, xyz.substr(1) == "yz");
    REPORTER_ASSERT(r, xyz.substr(1, 1) == "y");
    REPORTER_ASSERT(r, xyz.substr(1, 2) == "yz");
    REPORTER_ASSERT(r, xyz.substr(1, 3) == "yz");

    REPORTER_ASSERT(r, xyz.substr(2) == "z");
    REPORTER_ASSERT(r, xyz.substr(2, 1) == "z");
    REPORTER_ASSERT(r, xyz.substr(2, 2) == "z");

    REPORTER_ASSERT(r, xyz.substr(0, 0).empty());
    REPORTER_ASSERT(r, xyz.substr(1, 0).empty());
    REPORTER_ASSERT(r, xyz.substr(2, 0).empty());
    REPORTER_ASSERT(r, xyz.substr(3, 0).empty());

    REPORTER_ASSERT(r, xyz.substr(3).empty());
    REPORTER_ASSERT(r, xyz.substr(4).empty());
}

DEF_TEST(SkStringViewFind, r) {
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("abcdef") == 0);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("abcdefg") == skstd::string_view::npos);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("") == 0);
    REPORTER_ASSERT(r, skstd::string_view("").find("") == 0);
    REPORTER_ASSERT(r, skstd::string_view("").find("a") == skstd::string_view::npos);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("b") == 1);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("f") == 5);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("q") == skstd::string_view::npos);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("bcd") == 1);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("bcd", 2) == skstd::string_view::npos);
    REPORTER_ASSERT(r, skstd::string_view("abcdef").find("bce") == skstd::string_view::npos);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").find("test") == 3);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").find("test", 1) == 3);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").find("test", 3) == 3);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").find("test", 4) == 10);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").find("test2") == 10);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").find("test3") == 17);
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").contains("test"));
    REPORTER_ASSERT(r, skstd::string_view("ttttest1tttest2tttest3").contains("test3"));
    REPORTER_ASSERT(r, !skstd::string_view("ttttest1tttest2tttest3").contains("test4"));
    REPORTER_ASSERT(r, skstd::string_view("").contains(""));
    REPORTER_ASSERT(r, !skstd::string_view("").contains("a"));
    REPORTER_ASSERT(r, skstd::string_view("abcabcd").contains("abcd"));
    REPORTER_ASSERT(r, skstd::string_view("abc").contains(""));
    REPORTER_ASSERT(r, skstd::string_view("abc").contains("a"));
    REPORTER_ASSERT(r, skstd::string_view("abc").contains("b"));
    REPORTER_ASSERT(r, skstd::string_view("abc").contains("c"));
    REPORTER_ASSERT(r, skstd::string_view("abc").contains("ab"));
    REPORTER_ASSERT(r, skstd::string_view("abc").contains("bc"));
    REPORTER_ASSERT(r, !skstd::string_view("abc").contains("ac"));
    REPORTER_ASSERT(r, !skstd::string_view("abc").contains("cb"));
    REPORTER_ASSERT(r, !skstd::string_view("abc").contains("abcd"));
}

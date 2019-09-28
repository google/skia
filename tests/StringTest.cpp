/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkString.h"
#include "src/core/SkStringUtils.h"

#include <stdarg.h>
#include <stdio.h>
#include <thread>

static const char* gThirtyWideDecimal = "%30d";

DEF_TEST(String, reporter) {
    SkString    a;
    SkString    b((size_t)0);
    SkString    c("");
    SkString    d(nullptr, 0);

    REPORTER_ASSERT(reporter, a.isEmpty());
    REPORTER_ASSERT(reporter, a == b && a == c && a == d);

    a.set("hello");
    b.set("hellox", 5);
    c.set(a);
    d.resize(5);
    memcpy(d.writable_str(), "helloz", 5);

    REPORTER_ASSERT(reporter, !a.isEmpty());
    REPORTER_ASSERT(reporter, a.size() == 5);
    REPORTER_ASSERT(reporter, a == b && a == c && a == d);
    REPORTER_ASSERT(reporter, a.equals("hello", 5));
    REPORTER_ASSERT(reporter, a.equals("hello"));
    REPORTER_ASSERT(reporter, !a.equals("help"));

    REPORTER_ASSERT(reporter,  a.startsWith("hell"));
    REPORTER_ASSERT(reporter,  a.startsWith('h'));
    REPORTER_ASSERT(reporter, !a.startsWith( "ell"));
    REPORTER_ASSERT(reporter, !a.startsWith( 'e'));
    REPORTER_ASSERT(reporter,  a.startsWith(""));
    REPORTER_ASSERT(reporter,  a.endsWith("llo"));
    REPORTER_ASSERT(reporter,  a.endsWith('o'));
    REPORTER_ASSERT(reporter, !a.endsWith("ll" ));
    REPORTER_ASSERT(reporter, !a.endsWith('l'));
    REPORTER_ASSERT(reporter,  a.endsWith(""));
    REPORTER_ASSERT(reporter,  a.contains("he"));
    REPORTER_ASSERT(reporter,  a.contains("ll"));
    REPORTER_ASSERT(reporter,  a.contains("lo"));
    REPORTER_ASSERT(reporter,  a.contains("hello"));
    REPORTER_ASSERT(reporter, !a.contains("hellohello"));
    REPORTER_ASSERT(reporter,  a.contains(""));
    REPORTER_ASSERT(reporter,  a.contains('e'));
    REPORTER_ASSERT(reporter, !a.contains('z'));

    SkString    e(a);
    SkString    f("hello");
    SkString    g("helloz", 5);

    REPORTER_ASSERT(reporter, a == e && a == f && a == g);

    b.set("world");
    c = b;
    REPORTER_ASSERT(reporter, a != b && a != c && b == c);

    a.append(" world");
    e.append("worldz", 5);
    e.insert(5, " ");
    f.set("world");
    f.prepend("hello ");
    REPORTER_ASSERT(reporter, a.equals("hello world") && a == e && a == f);

    a.reset();
    b.resize(0);
    REPORTER_ASSERT(reporter, a.isEmpty() && b.isEmpty() && a == b);

    a.set("a");
    a.set("ab");
    a.set("abc");
    a.set("abcd");

    a.set("");
    a.appendS32(0x7FFFFFFFL);
    REPORTER_ASSERT(reporter, a.equals("2147483647"));
    a.set("");
    a.appendS32(0x80000001L);
    REPORTER_ASSERT(reporter, a.equals("-2147483647"));
    a.set("");
    a.appendS32(0x80000000L);
    REPORTER_ASSERT(reporter, a.equals("-2147483648"));

    a.set("");
    a.appendU32(0x7FFFFFFFUL);
    REPORTER_ASSERT(reporter, a.equals("2147483647"));
    a.set("");
    a.appendU32(0x80000001UL);
    REPORTER_ASSERT(reporter, a.equals("2147483649"));
    a.set("");
    a.appendU32(0xFFFFFFFFUL);
    REPORTER_ASSERT(reporter, a.equals("4294967295"));

    a.set("");
    a.appendS64(0x7FFFFFFFFFFFFFFFLL, 0);
    REPORTER_ASSERT(reporter, a.equals("9223372036854775807"));
    a.set("");
    a.appendS64(0x8000000000000001LL, 0);
    REPORTER_ASSERT(reporter, a.equals("-9223372036854775807"));
    a.set("");
    a.appendS64(0x8000000000000000LL, 0);
    REPORTER_ASSERT(reporter, a.equals("-9223372036854775808"));
    a.set("");
    a.appendS64(0x0000000001000000LL, 15);
    REPORTER_ASSERT(reporter, a.equals("000000016777216"));
    a.set("");
    a.appendS64(0xFFFFFFFFFF000000LL, 15);
    REPORTER_ASSERT(reporter, a.equals("-000000016777216"));

    a.set("");
    a.appendU64(0x7FFFFFFFFFFFFFFFULL, 0);
    REPORTER_ASSERT(reporter, a.equals("9223372036854775807"));
    a.set("");
    a.appendU64(0x8000000000000001ULL, 0);
    REPORTER_ASSERT(reporter, a.equals("9223372036854775809"));
    a.set("");
    a.appendU64(0xFFFFFFFFFFFFFFFFULL, 0);
    REPORTER_ASSERT(reporter, a.equals("18446744073709551615"));
    a.set("");
    a.appendU64(0x0000000001000000ULL, 15);
    REPORTER_ASSERT(reporter, a.equals("000000016777216"));

    a.printf("%i", 0);
    REPORTER_ASSERT(reporter, a.equals("0"));
    a.printf("%g", 3.14);
    REPORTER_ASSERT(reporter, a.equals("3.14"));
    a.printf("hello %s", "skia");
    REPORTER_ASSERT(reporter, a.equals("hello skia"));

    static const struct {
        SkScalar    fValue;
        const char* fString;
    } gRec[] = {
        { 0,            "0" },
        { SK_Scalar1,   "1" },
        { -SK_Scalar1,  "-1" },
        { SK_Scalar1/2, "0.5" },
  #if defined(SK_BUILD_FOR_WIN) && (_MSC_VER < 1900)
        { 3.4028234e38f,   "3.4028235e+038" },
        { -3.4028234e38f, "-3.4028235e+038" },
  #else
        { 3.4028234e38f,   "3.4028235e+38" },
        { -3.4028234e38f, "-3.4028235e+38" },
  #endif
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        a.reset();
        a.appendScalar(gRec[i].fValue);
        REPORTER_ASSERT(reporter, a.size() <= SkStrAppendScalar_MaxSize);
        if (!a.equals(gRec[i].fString)) {
            ERRORF(reporter, "received <%s> expected <%s>\n", a.c_str(), gRec[i].fString);
        }
    }

    REPORTER_ASSERT(reporter, SkStringPrintf("%i", 0).equals("0"));

    char buffer [40];
    memset(buffer, 'a', 40);
    REPORTER_ASSERT(reporter, buffer[18] == 'a');
    REPORTER_ASSERT(reporter, buffer[19] == 'a');
    REPORTER_ASSERT(reporter, buffer[20] == 'a');
    snprintf(buffer, 20, gThirtyWideDecimal, 0);
    REPORTER_ASSERT(reporter, buffer[18] == ' ');
    REPORTER_ASSERT(reporter, buffer[19] == 0);
    REPORTER_ASSERT(reporter, buffer[20] == 'a');

    REPORTER_ASSERT(reporter, SkStringPrintf("%i", 0).equals("0"));

    // 2000 is larger than the static buffer size inside SkString.cpp
    a = SkStringPrintf("%2000s", " ");
    REPORTER_ASSERT(reporter, a.size() == 2000);
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != ' ') {
            ERRORF(reporter, "SkStringPrintf fail: a[%d] = '%c'", i, a[i]);
            break;
        }
    }
    a.reset();
    a.printf("%2000s", " ");
    REPORTER_ASSERT(reporter, a.size() == 2000);
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != ' ') {
            ERRORF(reporter, "SkString::printf fail: a[%d] = '%c'", i, a[i]);
            break;
        }
    }
    a.appendf("%2000s", " ");
    REPORTER_ASSERT(reporter, a.size() == 4000);
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != ' ') {
            ERRORF(reporter, "SkString::appendf fail: a[%d] = '%c'", i, a[i]);
            break;
        }
    }
}

DEF_TEST(String_SkStrSplit, r) {
    SkTArray<SkString> results;

    SkStrSplit("a-_b_c-dee--f-_-_-g-", "-_", &results);
    REPORTER_ASSERT(r, results.count() == 6);
    REPORTER_ASSERT(r, results[0].equals("a"));
    REPORTER_ASSERT(r, results[1].equals("b"));
    REPORTER_ASSERT(r, results[2].equals("c"));
    REPORTER_ASSERT(r, results[3].equals("dee"));
    REPORTER_ASSERT(r, results[4].equals("f"));
    REPORTER_ASSERT(r, results[5].equals("g"));

    results.reset();
    SkStrSplit("\n", "\n", &results);
    REPORTER_ASSERT(r, results.count() == 0);

    results.reset();
    SkStrSplit("", "\n", &results);
    REPORTER_ASSERT(r, results.count() == 0);

    results.reset();
    SkStrSplit("a", "\n", &results);
    REPORTER_ASSERT(r, results.count() == 1);
    REPORTER_ASSERT(r, results[0].equals("a"));
}
DEF_TEST(String_SkStrSplit_All, r) {
    SkTArray<SkString> results;
    SkStrSplit("a-_b_c-dee--f-_-_-g-", "-_", kStrict_SkStrSplitMode, &results);
    REPORTER_ASSERT(r, results.count() == 13);
    REPORTER_ASSERT(r, results[0].equals("a"));
    REPORTER_ASSERT(r, results[1].equals(""));
    REPORTER_ASSERT(r, results[2].equals("b"));
    REPORTER_ASSERT(r, results[3].equals("c"));
    REPORTER_ASSERT(r, results[4].equals("dee"));
    REPORTER_ASSERT(r, results[5].equals(""));
    REPORTER_ASSERT(r, results[6].equals("f"));
    REPORTER_ASSERT(r, results[7].equals(""));
    REPORTER_ASSERT(r, results[8].equals(""));
    REPORTER_ASSERT(r, results[9].equals(""));
    REPORTER_ASSERT(r, results[10].equals(""));
    REPORTER_ASSERT(r, results[11].equals("g"));
    REPORTER_ASSERT(r, results[12].equals(""));

    results.reset();
    SkStrSplit("\n", "\n", kStrict_SkStrSplitMode, &results);
    REPORTER_ASSERT(r, results.count() == 2);
    REPORTER_ASSERT(r, results[0].equals(""));
    REPORTER_ASSERT(r, results[1].equals(""));

    results.reset();
    SkStrSplit("", "\n", kStrict_SkStrSplitMode, &results);
    REPORTER_ASSERT(r, results.count() == 0);

    results.reset();
    SkStrSplit("a", "\n", kStrict_SkStrSplitMode, &results);
    REPORTER_ASSERT(r, results.count() == 1);
    REPORTER_ASSERT(r, results[0].equals("a"));

    results.reset();
    SkStrSplit(",,", ",", kStrict_SkStrSplitMode, &results);
    REPORTER_ASSERT(r, results.count() == 3);
    REPORTER_ASSERT(r, results[0].equals(""));
    REPORTER_ASSERT(r, results[1].equals(""));
    REPORTER_ASSERT(r, results[2].equals(""));

    results.reset();
    SkStrSplit(",a,b,", ",", kStrict_SkStrSplitMode, &results);
    REPORTER_ASSERT(r, results.count() == 4);
    REPORTER_ASSERT(r, results[0].equals(""));
    REPORTER_ASSERT(r, results[1].equals("a"));
    REPORTER_ASSERT(r, results[2].equals("b"));
    REPORTER_ASSERT(r, results[3].equals(""));
}

// https://bugs.chromium.org/p/skia/issues/detail?id=7107
DEF_TEST(String_Threaded, r) {
    SkString str("foo");

    std::thread threads[5];
    for (auto& thread : threads) {
        thread = std::thread([&] {
            SkString copy = str;
            (void)copy.equals("test");
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

// Ensure that the string allocate doesn't internally overflow any calculations, and accidentally
// let us create a string with a requested length longer than we can manage.
DEF_TEST(String_huge, r) {
    // start testing slightly below max 32
    size_t size = UINT32_MAX - 16;
    // See where we crash, and manually check that its at the right point.
    //
    //  To test, change the false to true
    while (false) {
        // On a 64bit build, this should crash when size == 1 << 32, since we can't store
        // that length in the string's header (which has a u32 slot for the length).
        //
        // On a 32bit build, this should crash the first time around, since we can't allocate
        // anywhere near this amount.
        //
        SkString str(size);
        size += 1;
    }
}

DEF_TEST(String_fromUTF16, r) {
    // test data produced with `iconv`.
    const uint16_t test1[] = {
        0xD835, 0xDCD0, 0xD835, 0xDCD1, 0xD835, 0xDCD2, 0xD835, 0xDCD3, 0xD835, 0xDCD4, 0x0020,
        0xD835, 0xDCD5, 0xD835, 0xDCD6, 0xD835, 0xDCD7, 0xD835, 0xDCD8, 0xD835, 0xDCD9
    };
    REPORTER_ASSERT(r, SkStringFromUTF16(test1, SK_ARRAY_COUNT(test1)).equals("𝓐𝓑𝓒𝓓𝓔 𝓕𝓖𝓗𝓘𝓙"));

    const uint16_t test2[] = {
        0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0020, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A,
    };
    REPORTER_ASSERT(r, SkStringFromUTF16(test2, SK_ARRAY_COUNT(test2)).equals("ABCDE FGHIJ"));

    const uint16_t test3[] = {
        0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x0020, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA,
    };
    REPORTER_ASSERT(r, SkStringFromUTF16(test3, SK_ARRAY_COUNT(test3)).equals("αβγδε ζηθικ"));
}


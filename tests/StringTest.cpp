/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkString.h"
#include <stdarg.h>
#include <stdio.h>

// Windows vsnprintf doesn't 0-terminate safely), but is so far
// encapsulated in SkString that we can't test it directly.

#ifdef SK_BUILD_FOR_WIN
    #define VSNPRINTF(buffer, size, format, args)   \
        vsnprintf_s(buffer, size, _TRUNCATE, format, args)
#else
    #define VSNPRINTF   vsnprintf
#endif

#define ARGS_TO_BUFFER(format, buffer, size)        \
    do {                                            \
        va_list args;                               \
        va_start(args, format);                     \
        VSNPRINTF(buffer, size, format, args);      \
        va_end(args);                               \
    } while (0)

static void printfAnalog(char* buffer, int size, const char format[], ...) {
    ARGS_TO_BUFFER(format, buffer, size);
}

DEF_TEST(String, reporter) {
    SkString    a;
    SkString    b((size_t)0);
    SkString    c("");
    SkString    d(NULL, 0);

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

    static const struct {
        SkScalar    fValue;
        const char* fString;
    } gRec[] = {
        { 0,            "0" },
        { SK_Scalar1,   "1" },
        { -SK_Scalar1,  "-1" },
        { SK_Scalar1/2, "0.5" },
#ifdef SK_SCALAR_IS_FLOAT
  #ifdef SK_BUILD_FOR_WIN
        { 3.4028234e38f,   "3.4028235e+038" },
        { -3.4028234e38f, "-3.4028235e+038" },
  #else
        { 3.4028234e38f,   "3.4028235e+38" },
        { -3.4028234e38f, "-3.4028235e+38" },
  #endif
#endif
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        a.reset();
        a.appendScalar(gRec[i].fValue);
        REPORTER_ASSERT(reporter, a.size() <= SkStrAppendScalar_MaxSize);
//        SkDebugf(" received <%s> expected <%s>\n", a.c_str(), gRec[i].fString);
        REPORTER_ASSERT(reporter, a.equals(gRec[i].fString));
    }

    REPORTER_ASSERT(reporter, SkStringPrintf("%i", 0).equals("0"));

    char buffer [40];
    memset(buffer, 'a', 40);
    REPORTER_ASSERT(reporter, buffer[18] == 'a');
    REPORTER_ASSERT(reporter, buffer[19] == 'a');
    REPORTER_ASSERT(reporter, buffer[20] == 'a');
    printfAnalog(buffer, 20, "%30d", 0);
    REPORTER_ASSERT(reporter, buffer[18] == ' ');
    REPORTER_ASSERT(reporter, buffer[19] == 0);
    REPORTER_ASSERT(reporter, buffer[20] == 'a');

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
}

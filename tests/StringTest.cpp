
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkString.h"
#include <stdarg.h>


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

void printfAnalog(char* buffer, int size, const char format[], ...) {
    ARGS_TO_BUFFER(format, buffer, size);
}



static void TestString(skiatest::Reporter* reporter) {
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
    a.appendS64(72036854775808LL, 0);
    REPORTER_ASSERT(reporter, a.equals("72036854775808"));

    a.set("");
    a.appendS64(-1844674407370LL, 0);
    REPORTER_ASSERT(reporter, a.equals("-1844674407370"));

    a.set("");
    a.appendS64(73709551616LL, 15);
    REPORTER_ASSERT(reporter, a.equals("000073709551616"));

    a.set("");
    a.appendS64(-429496729612LL, 15);
    REPORTER_ASSERT(reporter, a.equals("-000429496729612"));

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

#include "TestClassDef.h"
DEFINE_TESTCLASS("String", StringTestClass, TestString)

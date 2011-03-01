#include "Test.h"
#include "SkString.h"

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
        { 3.4028234e38f,   "3.4028235e+38" },
        { -3.4028234e38f, "-3.4028235e+38" },
#endif
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        a.reset();
        a.appendScalar(gRec[i].fValue);
        REPORTER_ASSERT(reporter, a.size() <= SkStrAppendScalar_MaxSize);
//        SkDebugf(" received <%s> expected <%s>\n", a.c_str(), gRec[i].fString);
        REPORTER_ASSERT(reporter, a.equals(gRec[i].fString));
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("String", StringTestClass, TestString)

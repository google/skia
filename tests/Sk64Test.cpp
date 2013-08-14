
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkRandom.h"
#include <math.h>

struct BoolTable {
    int8_t  zero, pos, neg, toBool, sign;
};

static void bool_table_test(skiatest::Reporter* reporter,
                            const Sk64& a, const BoolTable& table)
{
    REPORTER_ASSERT(reporter, a.isZero() != a.nonZero());

    REPORTER_ASSERT(reporter, !a.isZero() == !table.zero);
    REPORTER_ASSERT(reporter, !a.isPos() == !table.pos);
    REPORTER_ASSERT(reporter, !a.isNeg() == !table.neg);
    REPORTER_ASSERT(reporter, a.getSign() == table.sign);
}

static void TestSk64(skiatest::Reporter* reporter) {
    enum BoolTests {
        kZero_BoolTest,
        kPos_BoolTest,
        kNeg_BoolTest
    };
    static const BoolTable gBoolTable[] = {
        { 1, 0, 0, 0, 0 },
        { 0, 1, 0, 1, 1 },
        { 0, 0, 1, 1, -1 }
    };

    Sk64    a, b, c;

    a.fHi = a.fLo = 0;
    b.set(0);
    c.setZero();
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, a == c);
    bool_table_test(reporter, a, gBoolTable[kZero_BoolTest]);

    a.fHi = 0;  a.fLo = 5;
    b.set(5);
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, a.is32() && a.get32() == 5 && !a.is64());
    bool_table_test(reporter, a, gBoolTable[kPos_BoolTest]);

    a.fHi = -1; a.fLo = (uint32_t)-5;
    b.set(-5);
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, a.is32() && a.get32() == -5 && !a.is64());
    bool_table_test(reporter, a, gBoolTable[kNeg_BoolTest]);

    a.setZero();
    b.set(6);
    c.set(-6);
    REPORTER_ASSERT(reporter, a != b && b != c && a != c);
    REPORTER_ASSERT(reporter, !(a == b) && !(a == b) && !(a == b));
    REPORTER_ASSERT(reporter, a < b && b > a && a <= b && b >= a);
    REPORTER_ASSERT(reporter, c < a && a > c && c <= a && a >= c);
    REPORTER_ASSERT(reporter, c < b && b > c && c <= b && b >= c);

    // Now test add/sub

    SkMWCRandom    rand;
    int         i;

    for (i = 0; i < 1000; i++)
    {
        int aa = rand.nextS() >> 1;
        int bb = rand.nextS() >> 1;
        a.set(aa);
        b.set(bb);
        REPORTER_ASSERT(reporter, a.get32() == aa && b.get32() == bb);
        c = a; c.add(bb);
        REPORTER_ASSERT(reporter, c.get32() == aa + bb);
        c = a; c.add(-bb);
        REPORTER_ASSERT(reporter, c.get32() == aa - bb);
        c = a; c.add(b);
        REPORTER_ASSERT(reporter, c.get32() == aa + bb);
        c = a; c.sub(b);
        REPORTER_ASSERT(reporter, c.get32() == aa - bb);
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Sk64", Sk64TestClass, TestSk64)

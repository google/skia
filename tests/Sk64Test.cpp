/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
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

#ifdef SkLONGLONG
    static SkLONGLONG asLL(const Sk64& a)
    {
        return ((SkLONGLONG)a.fHi << 32) | a.fLo;
    }
#endif

DEF_TEST(Sk64Test, reporter) {
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

    SkRandom    rand;
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

#ifdef SkLONGLONG
    for (i = 0; i < 1000; i++)
    {
        rand.next64(&a); //a.fHi >>= 1; // avoid overflow
        rand.next64(&b); //b.fHi >>= 1; // avoid overflow

        if (!(i & 3))   // want to explicitly test these cases
        {
            a.fLo = 0;
            b.fLo = 0;
        }
        else if (!(i & 7))  // want to explicitly test these cases
        {
            a.fHi = 0;
            b.fHi = 0;
        }

        SkLONGLONG aa = asLL(a);
        SkLONGLONG bb = asLL(b);

        REPORTER_ASSERT(reporter, (a < b) == (aa < bb));
        REPORTER_ASSERT(reporter, (a <= b) == (aa <= bb));
        REPORTER_ASSERT(reporter, (a > b) == (aa > bb));
        REPORTER_ASSERT(reporter, (a >= b) == (aa >= bb));
        REPORTER_ASSERT(reporter, (a == b) == (aa == bb));
        REPORTER_ASSERT(reporter, (a != b) == (aa != bb));

        c = a; c.add(b);
        REPORTER_ASSERT(reporter, asLL(c) == aa + bb);
        c = a; c.sub(b);
        REPORTER_ASSERT(reporter, asLL(c) == aa - bb);
        c = a; c.rsub(b);
        REPORTER_ASSERT(reporter, asLL(c) == bb - aa);
        c = a; c.negate();
        REPORTER_ASSERT(reporter, asLL(c) == -aa);

        int bits = rand.nextU() & 63;
        c = a; c.shiftLeft(bits);
        REPORTER_ASSERT(reporter, asLL(c) == (aa << bits));
        c = a; c.shiftRight(bits);
        REPORTER_ASSERT(reporter, asLL(c) == (aa >> bits));
        c = a; c.roundRight(bits);

        SkLONGLONG tmp;

        tmp = aa;
        if (bits > 0)
            tmp += (SkLONGLONG)1 << (bits - 1);
        REPORTER_ASSERT(reporter, asLL(c) == (tmp >> bits));

        c.setMul(a.fHi, b.fHi);
        tmp = (SkLONGLONG)a.fHi * b.fHi;
        REPORTER_ASSERT(reporter, asLL(c) == tmp);
    }


    for (i = 0; i < 100000; i++)
    {
        Sk64    wide;
        int32_t denom = rand.nextS();

        while (denom == 0)
            denom = rand.nextS();
        wide.setMul(rand.nextS(), rand.nextS());
        SkLONGLONG check = wide.getLongLong();

        wide.div(denom, Sk64::kTrunc_DivOption);
        check /= denom;
        SkLONGLONG w = wide.getLongLong();

        REPORTER_ASSERT(reporter, check == w);

        wide.setMul(rand.nextS(), rand.nextS());
        wide.abs();
        denom = wide.getSqrt();
        int32_t ck = (int32_t)sqrt((double)wide.getLongLong());
        int diff = denom - ck;
        REPORTER_ASSERT(reporter, SkAbs32(diff) <= 1);

        wide.setMul(rand.nextS(), rand.nextS());
        Sk64    dwide;
        dwide.setMul(rand.nextS(), rand.nextS());
        SkFixed fixdiv = wide.getFixedDiv(dwide);
        double dnumer = (double)wide.getLongLong();
        double ddenom = (double)dwide.getLongLong();
        double ddiv = dnumer / ddenom;
        SkFixed dfixdiv;
        if (ddiv >= (double)SK_MaxS32 / (double)SK_Fixed1)
            dfixdiv = SK_MaxS32;
        else if (ddiv <= -(double)SK_MaxS32 / (double)SK_Fixed1)
            dfixdiv = SK_MinS32;
        else
            dfixdiv = SkFloatToFixed(dnumer / ddenom);
        diff = fixdiv - dfixdiv;

        if (SkAbs32(diff) > 1) {
            SkDebugf(" %d === numer %g denom %g div %g xdiv %x fxdiv %x\n",
                     i, dnumer, ddenom, ddiv, dfixdiv, fixdiv);
        }
        REPORTER_ASSERT(reporter, SkAbs32(diff) <= 1);
    }
#endif
}

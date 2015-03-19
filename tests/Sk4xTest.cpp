#include "Test.h"
#include "Sk4x.h"

#define ASSERT_EQ(a, b) REPORTER_ASSERT(r, a.equal(b).allTrue())
#define ASSERT_NE(a, b) REPORTER_ASSERT(r, a.notEqual(b).allTrue())

DEF_TEST(Sk4x_Construction, r) {
    Sk4f uninitialized;
    Sk4f zero(0,0,0,0);
    Sk4f foo(1,2,3,4),
         bar(foo),
         baz = bar;
    ASSERT_EQ(foo, bar);
    ASSERT_EQ(bar, baz);
    ASSERT_EQ(baz, foo);
}

struct AlignedFloats {
    Sk4f forces16ByteAlignment;   // On 64-bit machines, the stack starts 128-bit aligned,
    float fs[5];                  // but not necessarily so on 32-bit.  Adding an Sk4f forces it.
};

DEF_TEST(Sk4x_LoadStore, r) {
    AlignedFloats aligned;
    // fs will be 16-byte aligned, fs+1 not.
    float* fs = aligned.fs;
    for (int i = 0; i < 5; i++) {  // set to 5,6,7,8,9
        fs[i] = float(i+5);
    }

    Sk4f foo = Sk4f::Load(fs);
    Sk4f bar = Sk4f::LoadAligned(fs);
    ASSERT_EQ(foo, bar);

    foo = Sk4f::Load(fs+1);
    ASSERT_NE(foo, bar);

    foo.storeAligned(fs);
    bar.store(fs+1);
    REPORTER_ASSERT(r, fs[0] == 6 &&
                       fs[1] == 5 &&
                       fs[2] == 6 &&
                       fs[3] == 7 &&
                       fs[4] == 8);
}

DEF_TEST(Sk4x_Conversions, r) {
    // Assuming IEEE floats.
    Sk4f zerof(0,0,0,0);
    Sk4i zeroi(0,0,0,0);
    ASSERT_EQ(zeroi, zerof.cast<Sk4i>());
    ASSERT_EQ(zeroi, zerof.reinterpret<Sk4i>());
    ASSERT_EQ(zerof, zeroi.cast<Sk4f>());
    ASSERT_EQ(zerof, zeroi.reinterpret<Sk4f>());

    Sk4f twof(2,2,2,2);
    Sk4i twoi(2,2,2,2);
    ASSERT_EQ(twoi, twof.cast<Sk4i>());
    ASSERT_NE(twoi, twof.reinterpret<Sk4i>());
    ASSERT_EQ(twof, twoi.cast<Sk4f>());
    ASSERT_NE(twof, twoi.reinterpret<Sk4f>());
}

DEF_TEST(Sk4x_Bits, r) {
    ASSERT_EQ(Sk4i(0,0,0,0).bitNot(), Sk4i(-1,-1,-1,-1));

    Sk4i a(2,3,4,5),
         b(1,3,5,7);
    ASSERT_EQ(Sk4i(0,3,4,5), a.bitAnd(b));
    ASSERT_EQ(Sk4i(3,3,5,7), a.bitOr(b));
}

DEF_TEST(Sk4x_Arith, r) {
    ASSERT_EQ(Sk4f(4,6,8,10),    Sk4f(1,2,3,4).add(Sk4f(3,4,5,6)));
    ASSERT_EQ(Sk4f(-2,-2,-2,-2), Sk4f(1,2,3,4).subtract(Sk4f(3,4,5,6)));
    ASSERT_EQ(Sk4f(3,8,15,24),   Sk4f(1,2,3,4).multiply(Sk4f(3,4,5,6)));

    float third = 1.0f/3.0f;
    ASSERT_EQ(Sk4f(1*third, 0.5f, 0.6f, 2*third), Sk4f(1,2,3,4).divide(Sk4f(3,4,5,6)));
    ASSERT_EQ(Sk4i(4,6,8,10),    Sk4i(1,2,3,4).add(Sk4i(3,4,5,6)));
    ASSERT_EQ(Sk4i(-2,-2,-2,-2), Sk4i(1,2,3,4).subtract(Sk4i(3,4,5,6)));
    ASSERT_EQ(Sk4i(3,8,15,24),   Sk4i(1,2,3,4).multiply(Sk4i(3,4,5,6)));
}

DEF_TEST(Sk4x_ImplicitPromotion, r) {
    ASSERT_EQ(Sk4f(2,4,6,8), Sk4f(1,2,3,4).multiply(Sk4f(2.0f)));
}

DEF_TEST(Sk4x_Sqrt, r) {
    Sk4f squares(4, 16, 25, 121),
           roots(2,  4,  5,  11);
    // .sqrt() should be pretty precise.
    Sk4f error = roots.subtract(squares.sqrt());
    REPORTER_ASSERT(r, error.greaterThanEqual(Sk4f(0.0f)).allTrue());
    REPORTER_ASSERT(r, error.lessThan(Sk4f(0.000001f)).allTrue());

    // .rsqrt() isn't so precise (for SSE), but should be pretty close.
    error = roots.subtract(squares.multiply(squares.rsqrt()));
    REPORTER_ASSERT(r, error.greaterThanEqual(Sk4f(0.0f)).allTrue());
    REPORTER_ASSERT(r, error.lessThan(Sk4f(0.01f)).allTrue());
}

DEF_TEST(Sk4x_Comparison, r) {
    ASSERT_EQ(Sk4f(1,2,3,4), Sk4f(1,2,3,4));
    ASSERT_NE(Sk4f(4,3,2,1), Sk4f(1,2,3,4));

    ASSERT_EQ(Sk4i(-1,-1,0,-1), Sk4f(1,2,5,4).equal(Sk4f(1,2,3,4)));

    ASSERT_EQ(Sk4i(-1,-1,-1,-1), Sk4f(1,2,3,4).lessThan(Sk4f(2,3,4,5)));
    ASSERT_EQ(Sk4i(-1,-1,-1,-1), Sk4f(1,2,3,4).lessThanEqual(Sk4f(2,3,4,5)));
    ASSERT_EQ(Sk4i(0,0,0,0),     Sk4f(1,2,3,4).greaterThan(Sk4f(2,3,4,5)));
    ASSERT_EQ(Sk4i(0,0,0,0),     Sk4f(1,2,3,4).greaterThanEqual(Sk4f(2,3,4,5)));

    ASSERT_EQ(Sk4i(1,2,3,4), Sk4i(1,2,3,4));
    ASSERT_NE(Sk4i(4,3,2,1), Sk4i(1,2,3,4));

    ASSERT_EQ(Sk4i(-1,-1,0,-1), Sk4i(1,2,5,4).equal(Sk4i(1,2,3,4)));

    ASSERT_EQ(Sk4i(-1,-1,-1,-1), Sk4i(1,2,3,4).lessThan(Sk4i(2,3,4,5)));
    ASSERT_EQ(Sk4i(-1,-1,-1,-1), Sk4i(1,2,3,4).lessThanEqual(Sk4i(2,3,4,5)));
    ASSERT_EQ(Sk4i(0,0,0,0),     Sk4i(1,2,3,4).greaterThan(Sk4i(2,3,4,5)));
    ASSERT_EQ(Sk4i(0,0,0,0),     Sk4i(1,2,3,4).greaterThanEqual(Sk4i(2,3,4,5)));
}

DEF_TEST(Sk4x_MinMax, r) {
    ASSERT_EQ(Sk4f(1,2,2,1), Sk4f::Min(Sk4f(1,2,3,4), Sk4f(4,3,2,1)));
    ASSERT_EQ(Sk4f(4,3,3,4), Sk4f::Max(Sk4f(1,2,3,4), Sk4f(4,3,2,1)));
    ASSERT_EQ(Sk4i(1,2,2,1), Sk4i::Min(Sk4i(1,2,3,4), Sk4i(4,3,2,1)));
    ASSERT_EQ(Sk4i(4,3,3,4), Sk4i::Max(Sk4i(1,2,3,4), Sk4i(4,3,2,1)));
}

DEF_TEST(Sk4x_Swizzle, r) {
    ASSERT_EQ(Sk4f(3,4,1,2), Sk4f(1,2,3,4).zwxy());
    ASSERT_EQ(Sk4f(1,2,5,6), Sk4f::XYAB(Sk4f(1,2,3,4), Sk4f(5,6,7,8)));
    ASSERT_EQ(Sk4f(3,4,7,8), Sk4f::ZWCD(Sk4f(1,2,3,4), Sk4f(5,6,7,8)));
    ASSERT_EQ(Sk4i(3,4,1,2), Sk4i(1,2,3,4).zwxy());
    ASSERT_EQ(Sk4i(1,2,5,6), Sk4i::XYAB(Sk4i(1,2,3,4), Sk4i(5,6,7,8)));
    ASSERT_EQ(Sk4i(3,4,7,8), Sk4i::ZWCD(Sk4i(1,2,3,4), Sk4i(5,6,7,8)));
}

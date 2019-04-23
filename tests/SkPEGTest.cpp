/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_XML)
#include "experimental/svg/model/SkPEG.h"
#include "include/private/SkTo.h"

using namespace skpeg;

namespace {

struct Alpha {
    using V = char;
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        static constexpr unsigned kAlphaRange = 'z' - 'a';
        return static_cast<unsigned>(*in - 'a') <= kAlphaRange
            || static_cast<unsigned>(*in - 'A') <= kAlphaRange
            ? MatchT(in + 1, *in)
            : nullptr;
    }
};

struct Digit {
    using V = uint8_t;
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        static constexpr unsigned kDigitRange = '9' - '0';
        return static_cast<unsigned>(*in - '0') <= kDigitRange
            ? MatchT(in + 1, SkTo<uint8_t>(*in - '0'))
            : nullptr;
    }
};

void test_EOS(skiatest::Reporter* r) {
    static const struct {
        const char* fInput;
        bool        fMatch;
    } gTests[] = {
        { ""   , true  },
        { " "  , false },
        { "\0" , true  },
        { "foo", false },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        const auto match = EOS::Match(gTests[i].fInput);
        REPORTER_ASSERT(r, match == gTests[i].fMatch);
        REPORTER_ASSERT(r, match.fNext == (match ? gTests[i].fInput : nullptr));
    }
}

void test_LIT(skiatest::Reporter* r) {
    static const struct {
        const char* fInput;
        bool        fMatch;
    } gTests[] = {
        { ""  , false },
        { " " , false },
        { "x" , false },
        { "X" , true  },
        { "xX", false },
        { "Xx", true  },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        const auto match = LIT<'X'>::Match(gTests[i].fInput);
        REPORTER_ASSERT(r, match == gTests[i].fMatch);
        REPORTER_ASSERT(r, match.fNext == (match ? gTests[i].fInput + 1 : nullptr));
    }

    REPORTER_ASSERT(r, !(LIT<'F', 'o', 'o'>::Match("")));
    REPORTER_ASSERT(r, !(LIT<'F', 'o', 'o'>::Match("Fo")));
    REPORTER_ASSERT(r, !(LIT<'F', 'o', 'o'>::Match("FoO")));
    REPORTER_ASSERT(r,  (LIT<'F', 'o', 'o'>::Match("Foo")));
    REPORTER_ASSERT(r,  (LIT<'F', 'o', 'o'>::Match("Foobar")));
}

void test_Alpha(skiatest::Reporter* r) {
    static const struct {
        const char* fInput;
        bool        fMatch;
        char        fMatchValue;
    } gTests[] = {
        { ""  , false,  0  },
        { "\r", false,  0  },
        { "\n", false,  0  },
        { "\t", false,  0  },
        { "0" , false,  0  },
        { "9" , false,  0  },
        { "a" , true , 'a' },
        { "a" , true , 'a' },
        { "z" , true , 'z' },
        { "A" , true , 'A' },
        { "Z" , true , 'Z' },
        { "az", true , 'a' },
        { "a0", true , 'a' },
        { "0a", false,  0  },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        const auto match = Alpha::Match(gTests[i].fInput);
        REPORTER_ASSERT(r, match == gTests[i].fMatch);
        REPORTER_ASSERT(r, match.fNext == (match ? gTests[i].fInput + 1 : nullptr));
        if (match) {
            REPORTER_ASSERT(r, *match == gTests[i].fMatchValue);
        }
    }
}

void test_Digit(skiatest::Reporter* r) {
    static const struct {
        const char* fInput;
        bool        fMatch;
        uint8_t     fMatchValue;
    } gTests[] = {
        { ""   , false, 0 },
        { "/"  , false, 0 },
        { ":"  , false, 0 },
        { "x"  , false, 0 },
        { "x0" , false, 0 },
        { "0"  , true , 0 },
        { "1x" , true , 1 },
        { "9 a", true , 9 },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        const auto match = Digit::Match(gTests[i].fInput);
        REPORTER_ASSERT(r, match == gTests[i].fMatch);
        REPORTER_ASSERT(r, match.fNext == (match ? gTests[i].fInput + 1 : nullptr));
        if (match) {
            REPORTER_ASSERT(r, *match == gTests[i].fMatchValue);
        }
    }
}

void test_Opt(skiatest::Reporter* r) {
    static const struct {
        const char* fInput;
        bool        fMatch;
    } gTests[] = {
        { ""       , false },
        { "fo"     , false },
        { " foo"   , false },
        { "foo"    , true },
        { "foobar" , true },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        const auto m = Opt<LIT<'f', 'o', 'o'>>::Match(gTests[i].fInput);
        REPORTER_ASSERT(r, m);
        REPORTER_ASSERT(r, m->fValue.isValid() == gTests[i].fMatch);
    }
}

void test_Seq(skiatest::Reporter* r) {
    REPORTER_ASSERT(r,  (Seq<LIT<'X'>, EOS>::Match("X")));
    REPORTER_ASSERT(r, !(Seq<LIT<'X'>, EOS>::Match("x")));
    REPORTER_ASSERT(r, !(Seq<LIT<'X'>, EOS>::Match("xX")));
    REPORTER_ASSERT(r, !(Seq<LIT<'X'>, EOS>::Match("XX")));
    REPORTER_ASSERT(r,  (Seq<LIT<'X'>, Seq<LIT<'X'>, EOS>>::Match("XX")));
    REPORTER_ASSERT(r,  (Seq<LIT<'X'>, Seq<LIT<'X'>, EOS>>::Match("XX")));

    REPORTER_ASSERT(r, !(Seq<LIT<'F', 'o', 'o'>, EOS>::Match("FooBar")));
    REPORTER_ASSERT(r,  (Seq<LIT<'F', 'o', 'o'>, EOS>::Match("Foo")));

    {
        const auto m = Seq<LIT<'x'>, Digit>::Match("x5");
        REPORTER_ASSERT(r, m);
        REPORTER_ASSERT(r, m->get<1>() == 5);
    }
    {
        const auto m = Seq<Digit, Digit>::Match("42");
        REPORTER_ASSERT(r, m);
        REPORTER_ASSERT(r, m->get<0>() == 4);
        REPORTER_ASSERT(r, m->get<1>() == 2);
    }
}

void test_Choice(skiatest::Reporter* r) {
    REPORTER_ASSERT(r, !(Choice<Digit,Alpha>::Match("")));
    REPORTER_ASSERT(r, !(Choice<Digit,Alpha>::Match("\t")));
    REPORTER_ASSERT(r, !(Choice<Digit,Alpha>::Match(" ")));
    REPORTER_ASSERT(r,  (Choice<Digit,Alpha>::Match("a")));
    REPORTER_ASSERT(r,  (Choice<Digit,Alpha>::Match("3")));
    REPORTER_ASSERT(r,  (Choice<Digit,Alpha>::Match("a ")));
    REPORTER_ASSERT(r,  (Choice<Digit,Alpha>::Match("3 ")));
    REPORTER_ASSERT(r, !(Choice<Digit,Alpha>::Match(" a ")));
    REPORTER_ASSERT(r, !(Choice<Digit,Alpha>::Match(" 3 ")));

    {
        const auto m = Choice<Alpha, Digit>::Match("x");
        REPORTER_ASSERT(r,  m);
        REPORTER_ASSERT(r,  m->v1.isValid());
        REPORTER_ASSERT(r, !m->v2.isValid());
        REPORTER_ASSERT(r, *m->v1.get() == 'x');
    }

    {
        const auto m = Choice<Alpha, Digit>::Match("7");
        REPORTER_ASSERT(r,  m);
        REPORTER_ASSERT(r, !m->v1.isValid());
        REPORTER_ASSERT(r,  m->v2.isValid());
        REPORTER_ASSERT(r, *m->v2.get() == 7);
    }
}

void test_AnySome(skiatest::Reporter* r) {
    static const struct {
        const char* fInput;
        int         fCount;
    } gTests[] = {
        { ""      , 0 },
        { "fo"    , 0 },
        { "Foo"   , 0 },
        { "foo"   , 1 },
        { "foofoo", 2 },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        const auto matchAny = Any<LIT<'f', 'o', 'o'>>::Match(gTests[i].fInput);
        REPORTER_ASSERT(r, matchAny);
        REPORTER_ASSERT(r, matchAny->fValues.count() == gTests[i].fCount);

        const auto matchSome = Some<LIT<'f', 'o', 'o'>>::Match(gTests[i].fInput);
        REPORTER_ASSERT(r,  matchSome == (gTests[i].fCount > 0));
        REPORTER_ASSERT(r, !matchSome ||
                            matchSome->get<1>().fValues.count() == gTests[i].fCount - 1);
    }

    {
        const auto m = Any<Digit>::Match("0123456789foo");
        REPORTER_ASSERT(r, m);
        REPORTER_ASSERT(r, m->fValues.count() == 10);
        for (int i = 0; i < m->fValues.count(); ++i) {
            REPORTER_ASSERT(r, m->fValues[i] == i);
        }
    }
}

void test_Complex(skiatest::Reporter* r) {
    // [0-9]+(,[0-9]+)?$
    using P0 =
        Seq<
          Some<Digit>,
          Opt<Seq<
            LIT<','>,
            Some<Digit>>>,
          EOS>;

    REPORTER_ASSERT(r, !P0::Match(""));
    REPORTER_ASSERT(r, !P0::Match(","));
    REPORTER_ASSERT(r, !P0::Match("1,"));
    REPORTER_ASSERT(r, !P0::Match(",1"));
    REPORTER_ASSERT(r,  P0::Match("1"));
    REPORTER_ASSERT(r,  P0::Match("1,2"));
    REPORTER_ASSERT(r, !P0::Match("1,2 "));
    REPORTER_ASSERT(r,  P0::Match("123,456"));

    // [ ]*[Ff]oo([Bb]ar)+[Bb]az[ ]*$
    using P1 =
        Seq<
          Any<LIT<' '>>,
          Choice<LIT<'F'>, LIT<'f'>>,
          LIT<'o', 'o'>,
          Some<Seq<
            Choice<LIT<'B'>, LIT<'b'>>,
            LIT<'a', 'r'>>>,
          Choice<LIT<'B'>, LIT<'b'>>,
          LIT<'a', 'z'>,
          Any<LIT<' '>>,
          EOS>;

    REPORTER_ASSERT(r, !P1::Match(""));
    REPORTER_ASSERT(r, !P1::Match("FooBar"));
    REPORTER_ASSERT(r, !P1::Match("FooBaz"));
    REPORTER_ASSERT(r,  P1::Match("FooBarBaz"));
    REPORTER_ASSERT(r,  P1::Match("foobarbaz"));
    REPORTER_ASSERT(r,  P1::Match("  FooBarbaz     "));
    REPORTER_ASSERT(r,  P1::Match(" FooBarbarbarBaz "));
}

} // anonymous ns

DEF_TEST(SkPEG, r) {
    test_EOS(r);
    test_LIT(r);
    test_Alpha(r);
    test_Digit(r);
    test_Opt(r);
    test_Seq(r);
    test_Choice(r);
    test_AnySome(r);
    test_Complex(r);
}

#endif // SK_XML

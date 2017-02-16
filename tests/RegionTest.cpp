/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "Test.h"

static void Union(SkRegion* rgn, const SkIRect& rect) {
    rgn->op(rect, SkRegion::kUnion_Op);
}

#define TEST_NO_INTERSECT(rgn, rect)    REPORTER_ASSERT(reporter, !rgn.intersects(rect))
#define TEST_INTERSECT(rgn, rect)       REPORTER_ASSERT(reporter, rgn.intersects(rect))
#define TEST_NO_CONTAINS(rgn, rect)     REPORTER_ASSERT(reporter, !rgn.contains(rect))

// inspired by http://code.google.com/p/skia/issues/detail?id=958
//
static void test_fromchrome(skiatest::Reporter* reporter) {
    SkRegion r;
    Union(&r, SkIRect::MakeXYWH(0, 0, 1, 1));
    TEST_NO_INTERSECT(r, SkIRect::MakeXYWH(0, 0, 0, 0));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, 0, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(-1, 0, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(-1, -1, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, -1, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(-1, -1, 3, 3));

    Union(&r, SkIRect::MakeXYWH(0, 0, 3, 3));
    Union(&r, SkIRect::MakeXYWH(10, 0, 3, 3));
    Union(&r, SkIRect::MakeXYWH(0, 10, 13, 3));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(-1, -1, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(2, -1, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(2, 2, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(-1, 2, 2, 2));

    TEST_INTERSECT(r, SkIRect::MakeXYWH(9, -1, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(12, -1, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(12, 2, 2, 2));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(9, 2, 2, 2));

    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, -1, 13, 5));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(1, -1, 11, 5));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(2, -1, 9, 5));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(2, -1, 8, 5));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(3, -1, 8, 5));

    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, 1, 13, 1));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(1, 1, 11, 1));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(2, 1, 9, 1));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(2, 1, 8, 1));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(3, 1, 8, 1));

    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, 0, 13, 13));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, 1, 13, 11));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, 2, 13, 9));
    TEST_INTERSECT(r, SkIRect::MakeXYWH(0, 2, 13, 8));


    // These test SkRegion::contains(Rect) and SkRegion::contains(Region)

    SkRegion container;
    Union(&container, SkIRect::MakeXYWH(0, 0, 40, 20));
    Union(&container, SkIRect::MakeXYWH(30, 20, 10, 20));
    TEST_NO_CONTAINS(container, SkIRect::MakeXYWH(0, 0, 10, 39));
    TEST_NO_CONTAINS(container, SkIRect::MakeXYWH(29, 0, 10, 39));

    {
        SkRegion rgn;
        Union(&rgn, SkIRect::MakeXYWH(0, 0, 10, 10));
        Union(&rgn, SkIRect::MakeLTRB(5, 10, 20, 20));
        TEST_INTERSECT(rgn, SkIRect::MakeXYWH(15, 0, 5, 11));
    }
}

static void test_empties(skiatest::Reporter* reporter) {
    SkRegion valid(SkIRect::MakeWH(10, 10));
    SkRegion empty, empty2;

    REPORTER_ASSERT(reporter, empty.isEmpty());
    REPORTER_ASSERT(reporter, !valid.isEmpty());

    // test intersects
    REPORTER_ASSERT(reporter, !empty.intersects(empty2));
    REPORTER_ASSERT(reporter, !valid.intersects(empty));

    // test contains
    REPORTER_ASSERT(reporter, !empty.contains(empty2));
    REPORTER_ASSERT(reporter, !valid.contains(empty));
    REPORTER_ASSERT(reporter, !empty.contains(valid));

    SkPath emptyPath;
    emptyPath.moveTo(1, 5);
    emptyPath.close();
    SkRegion openClip;
    openClip.setRect(-16000, -16000, 16000, 16000);
    empty.setPath(emptyPath, openClip);  // should not assert
}

enum {
    W = 256,
    H = 256
};

static SkIRect randRect(SkRandom& rand) {
    int x = rand.nextU() % W;
    int y = rand.nextU() % H;
    int w = rand.nextU() % W;
    int h = rand.nextU() % H;
    return SkIRect::MakeXYWH(x, y, w >> 1, h >> 1);
}

static void randRgn(SkRandom& rand, SkRegion* rgn, int n) {
    rgn->setEmpty();
    for (int i = 0; i < n; ++i) {
        rgn->op(randRect(rand), SkRegion::kUnion_Op);
    }
}

static bool slow_contains(const SkRegion& outer, const SkRegion& inner) {
    SkRegion tmp;
    tmp.op(outer, inner, SkRegion::kUnion_Op);
    return outer == tmp;
}

static bool slow_contains(const SkRegion& outer, const SkIRect& r) {
    SkRegion tmp;
    tmp.op(outer, SkRegion(r), SkRegion::kUnion_Op);
    return outer == tmp;
}

static bool slow_intersects(const SkRegion& outer, const SkRegion& inner) {
    SkRegion tmp;
    return tmp.op(outer, inner, SkRegion::kIntersect_Op);
}

static void test_contains_iter(skiatest::Reporter* reporter, const SkRegion& rgn) {
    SkRegion::Iterator iter(rgn);
    while (!iter.done()) {
        SkIRect r = iter.rect();
        REPORTER_ASSERT(reporter, rgn.contains(r));
        r.inset(-1, -1);
        REPORTER_ASSERT(reporter, !rgn.contains(r));
        iter.next();
    }
}

static void contains_proc(skiatest::Reporter* reporter,
                          const SkRegion& a, const SkRegion& b) {
    // test rgn
    bool c0 = a.contains(b);
    bool c1 = slow_contains(a, b);
    REPORTER_ASSERT(reporter, c0 == c1);

    // test rect
    SkIRect r = a.getBounds();
    r.inset(r.width()/4, r.height()/4);
    c0 = a.contains(r);
    c1 = slow_contains(a, r);
    REPORTER_ASSERT(reporter, c0 == c1);

    test_contains_iter(reporter, a);
    test_contains_iter(reporter, b);
}

static void test_intersects_iter(skiatest::Reporter* reporter, const SkRegion& rgn) {
    SkRegion::Iterator iter(rgn);
    while (!iter.done()) {
        SkIRect r = iter.rect();
        REPORTER_ASSERT(reporter, rgn.intersects(r));
        r.inset(-1, -1);
        REPORTER_ASSERT(reporter, rgn.intersects(r));
        iter.next();
    }
}

static void intersects_proc(skiatest::Reporter* reporter,
                          const SkRegion& a, const SkRegion& b) {
    bool c0 = a.intersects(b);
    bool c1 = slow_intersects(a, b);
    REPORTER_ASSERT(reporter, c0 == c1);

    test_intersects_iter(reporter, a);
    test_intersects_iter(reporter, b);
}

static void test_proc(skiatest::Reporter* reporter,
                      void (*proc)(skiatest::Reporter*,
                                   const SkRegion& a, const SkRegion&)) {
    SkRandom rand;
    for (int i = 0; i < 10000; ++i) {
        SkRegion outer;
        randRgn(rand, &outer, 8);
        SkRegion inner;
        randRgn(rand, &inner, 2);
        proc(reporter, outer, inner);
    }
}

static void rand_rect(SkIRect* rect, SkRandom& rand) {
    int bits = 6;
    int shift = 32 - bits;
    rect->set(rand.nextU() >> shift, rand.nextU() >> shift,
              rand.nextU() >> shift, rand.nextU() >> shift);
    rect->sort();
}

static bool test_rects(const SkIRect rect[], int count) {
    SkRegion rgn0, rgn1;

    for (int i = 0; i < count; i++) {
        rgn0.op(rect[i], SkRegion::kUnion_Op);
    }
    rgn1.setRects(rect, count);

    if (rgn0 != rgn1) {
        SkDebugf("\n");
        for (int i = 0; i < count; i++) {
            SkDebugf(" { %d, %d, %d, %d },\n",
                     rect[i].fLeft, rect[i].fTop,
                     rect[i].fRight, rect[i].fBottom);
        }
        SkDebugf("\n");
        return false;
    }
    return true;
}

DEF_TEST(Region, reporter) {
    const SkIRect r2[] = {
        { 0, 0, 1, 1 },
        { 2, 2, 3, 3 },
    };
    REPORTER_ASSERT(reporter, test_rects(r2, SK_ARRAY_COUNT(r2)));

    const SkIRect rects[] = {
        { 0, 0, 1, 2 },
        { 2, 1, 3, 3 },
        { 4, 0, 5, 1 },
        { 6, 0, 7, 4 },
    };
    REPORTER_ASSERT(reporter, test_rects(rects, SK_ARRAY_COUNT(rects)));

    SkRandom rand;
    for (int i = 0; i < 1000; i++) {
        SkRegion rgn0, rgn1;

        const int N = 8;
        SkIRect rect[N];
        for (int j = 0; j < N; j++) {
            rand_rect(&rect[j], rand);
        }
        REPORTER_ASSERT(reporter, test_rects(rect, N));
    }

    test_proc(reporter, contains_proc);
    test_proc(reporter, intersects_proc);
    test_empties(reporter);
    test_fromchrome(reporter);
}

// Test that writeToMemory reports the same number of bytes whether there was a
// buffer to write to or not.
static void test_write(const SkRegion& region, skiatest::Reporter* r) {
    const size_t bytesNeeded = region.writeToMemory(nullptr);
    SkAutoMalloc storage(bytesNeeded);
    const size_t bytesWritten = region.writeToMemory(storage.get());
    REPORTER_ASSERT(r, bytesWritten == bytesNeeded);

    // Also check that the bytes are meaningful.
    SkRegion copy;
    REPORTER_ASSERT(r, copy.readFromMemory(storage.get(), bytesNeeded));
    REPORTER_ASSERT(r, region == copy);
}

DEF_TEST(Region_writeToMemory, r) {
    // Test an empty region.
    SkRegion region;
    REPORTER_ASSERT(r, region.isEmpty());
    test_write(region, r);

    // Test a rectangular region
    bool nonEmpty = region.setRect(0, 0, 50, 50);
    REPORTER_ASSERT(r, nonEmpty);
    REPORTER_ASSERT(r, region.isRect());
    test_write(region, r);

    // Test a complex region
    nonEmpty = region.op(50, 50, 100, 100, SkRegion::kUnion_Op);
    REPORTER_ASSERT(r, nonEmpty);
    REPORTER_ASSERT(r, region.isComplex());
    test_write(region, r);
}

DEF_TEST(Region_readFromMemory_bad, r) {
    // These assume what our binary format is: conceivably we could change it
    // and might need to remove or change some of these tests.
    SkRegion region;

    static const char data0[] =
        "\10\0\0\0\207\256Z\337\217E}9\34\200\255\313|\351\270\32\357\343\373"
        "\224\343hU]\273\377)\220\36bp\256\3239\250WK\323\200\32\33\243G\1\366"
        "\5\254\204\317\25Y\223!\4\371\335\315\361\351\327";
    size_t data0length = 64;
    REPORTER_ASSERT(r, !region.readFromMemory(data0, data0length));

    static const char data13[] =
        "\363\247\377\n\0\0A\365\315\347\251\217\27I\370\242\334\2\377\177\242"
        "a'IdS\331\335\326\372\334A\277\25\25\25\25\25\25\25\25\25\25\25\25"
        "\25\25\25\25`b\327\315\227M\305.\304\224\237\222\351\336\262\324Y\317"
        "2\254$<\2515s!0\26o,T\352p\4\263\212\35\227K\27\237/\341\313\177\0"
        "\17Leq\342@\322$\317\346Iw`\331\335\4\356\252\32=\200;\236}\177\377"
        "\204\303\214\16N\32a\354\335C\2139\270\237\375c\0077\362\0\200\377"
        "\377k\22`\0\4\0\0\374\265\342\243yL\275\215\270*9\266\4\364\10\217"
        "|y\325J\10\262\262\262\262\262\262\262\363Q\362\222\6&!\0\200\212\35"
        "\227K\27\237/\341\313\177\0\17Leq\342@\322cig\257'\271\377\177\377"
        "\377e\206\231J\343\277\345\3\3067S9h\240\344\25\31'\245\355\4Hw\307"
        "\317\r\317\4\343\305\304\215H\312\34\2\371O\333X\215>\212\243\323\377"
        "\0\0\0\277\5(\35O\276\277\336\6gR\334Ok\245\225\1~\17\22\314\202X\215"
        ">7E\304&\7\233\234N\32@\355H\327\34\337\222V\30\t\225\35\225\233\216"
        "Y[a\362\224\13\330t\233\336\243\267N]K8g\27\244.\374-\270\304\361\361"
        "\361\361\0\200\361\361\361\361\361\361\361\345\361\262\5\24_\377\223"
        "\253\351L\30\372\373\243\240g\0367V\336O\20\212=;\366\314\36\260c\213"
        "\272v\243\203\316w{.\306>f2i\244\0\0\177\26y\215\215\200\215R\21\177"
        "\251\275\225\251I\277\365b\341\343\316\3\21\377\177\274M\370\201\243"
        "^@\343\236JaS\204\3212\244\301\327\r\352KI\207\350z\300\250$\26\14"
        "\2\233K\330\16\251\230\263\r\"\243\271\17)\260\262\377\344\307\376"
        "h\23\177<Q\375\34\324\203c\2735\3115\206uN\32@\355H\327\34\337\222"
        "V\30\t\225\35\225\233\216\0144Z\310\352\346\333\222S\240\203\375\374"
        "\332\247 \302I\240H\374\251lk\22`\0375\324W\376\265\342\243yL\232\215"
        "\270ZL\217wf8C\333\370_.\277A\277^\\\313\207\342\340\213\210\244\272"
        "\33\256\360\301\347\315}\6G\272k\243\20,\"\240\347\2430\361n~\323\27"
        "/\324(~\263=\304\333~\323\27/\324(~\263=\304\333\260\205!8B\314L\3"
        "\341Y\236\364\302\357xR:Q\\\303\364\265\332\267\242&\303s\350\324\32"
        "\330\263\263\263\263\263\2636\361$\\(\265W[F\326\5\377\377\5\306>f"
        "2i\244\242=Y\236\364\302\357xR:Q\\\303\364\265\332\267\242&\303s\350"
        "\324\32\330\263\263\263\263\263\263\263\263\263\263\263\263\263\263"
        "\263\313\243yL\232\215\270ZL\217\23\300D\246\307\266\256\37/M\207";
    size_t data13length = 732;
    REPORTER_ASSERT(r, !region.readFromMemory(data13, data13length));

    static const char data14[] =
        "\10\0\0\0\207\256Z\337\217E}9\34\200\255\313|\351\270\32\357\343\373"
        "\224\343hU]\273\377)\220\36bp\256\3239\250WK\323\200\32\33\243G\1\366"
        "\5\254\204\317\25Y\223!\4\371\335\315\361\351\327";
    size_t data14length = 64;
    REPORTER_ASSERT(r, !region.readFromMemory(data14, data14length));

    static const char data15[] =
        "\10\0\0\0\207\256Z\337\217E}9\34\200\255\313|\351\270\32\357\343\373"
        "\224\343hU]\273\377)\220\36bp\256\3239\250WK\323\200\32\33\243G\1\366"
        "\5\254\204\317\25Y\223!\4\371\335\315\361\351\327";
    size_t data15length = 64;
    REPORTER_ASSERT(r, !region.readFromMemory(data15, data15length));

    static const char data16[] =
        "\10\0\0\0\207\256Z\337\217E}9\34\200\255\313|\351\270\32\357\343\373"
        "\224\343hU]\273\377)\220\36bp\256\3239\250WK\323\200\32\33\243G\1\366"
        "\5\254\204\317\25Y\223!\4\371\335\315\361\351\327";
    size_t data16length = 64;
    REPORTER_ASSERT(r, !region.readFromMemory(data16, data16length));

    static const char data25[] =
        "\7\0\0\0\207\256Z\337\217E}8\374\200\255\313|\351\270\32\357\343\373"
        "\224\343hU]\272\217E}9\34\200\255\325\377\377\377\351\270\32\357\343"
        "\373\224\343hU]\273\217E\343\373\224\343hU]\273\317\25\361\321\327";
    size_t data25length = 67;
    REPORTER_ASSERT(r, !region.readFromMemory(data25, data25length));

    static const char data26[] =
        "@\0\0\0\277]\345\222\\\2G\252\0\177'\10\203\236\211>\377\340@\351!"
        "\370y\3\31\232r\353\0\0\327\7H\333\246\213\33\33\33\33\33\33\177\377"
        "\33\33\33\33\33\33\301\212\35\227K\27\215ZZZZZZZZZ\0R\334Ok\245\225"
        "a\371\224\13\330t\233\376\243\267\251\301\225\251I\277\365b\341\343"
        "\336Ja\177\377\377\377\244\301\362:Q\\\0\0\1\200\263\214\374\276\336"
        "P\225^\230\20UH N\265\357\177\240\0\306\377\177\346\222S \0\375\0\332"
        "\247 \302I\240H\374\200lk\r`\0375\324W\215\270tE^,\224n\310fy\377\231"
        "AH\16\235A\371\315\347\360\265\372r\232\301\216\35\227:\265]\32\20"
        "W\263yc\207\246\270tE^,\224n\310sy\2\0A\14\241SQ\\\303\364\0\0\1\200"
        "\0\0\374k\r`\0375\324Wp\270\267\313\313\313\313\313@\277\365b\341\343"
        "\336Ja\357~\263\0\2\333\260\220\\\303\364\265\332\267\242\325nlX\367"
        "\27I4444;\266\256\37/M\207";
    size_t data26length = 285;
    REPORTER_ASSERT(r, !region.readFromMemory(data26, data26length));

    static const char data41[] =
        "\t\0\0\0\0\4\0\0W\0\0\0\0\207\256Zd\0\0\0\200\32\33\243\223 \370\1"
        "\335\315\315\361\t\0\0\0\0\4\0\0W\0\0\0\0\207\256Zd\0\0\0\200\32\33"
        "\243\223 \370\1\335\315\315\361\351\327\351\327";
    size_t data41length = 68;
    REPORTER_ASSERT(r, !region.readFromMemory(data41, data41length));

    static const char data42[] =
        "\7\0\0\0\207\256Z\250W\0\0\0\2\0\0\0d\0\0\0\200\32\33\243\223 \370"
        "\1\7\0\0\0\207\256Z\250W\0\0\0\2\0\0\0d\0\0\0\200\32\33\243\223 \370"
        "\1\335\315\315\361\6\336\315\315\361\351\327";
    size_t data42length = 67;
    REPORTER_ASSERT(r, !region.readFromMemory(data42, data42length));

    static const char data43[] =
        "\22\0\0\0`\6I\207\7z\300\250\372\26\14\2\223K\330\16\251\230\223\r"
        "\"\243\271\17)\260\262\2[a.*/4\14\256\177\336\\&\303\200\344\5\377"
        "\377\363p\335C\2139\270\237\375c\7\0\362\370\344\5\304}\315\356\0\362"
        "\377\377\377\177\344\5\304}\316\366/\342\214\253\331\\\222\0\0\0\0"
        "u\301@\367\310\246\307\266\256\37/M\207";
    size_t data43length = 106;
    REPORTER_ASSERT(r, !region.readFromMemory(data43, data43length));

    static const char data44[] =
        "\7\0\0\0\0\0\4\371\0\200\377\377\377\1774T\2002\0\0\377\377\177ZRZ"
        "\0\4\00022\2002\2\0\4\371\356\7\0\0\0\0\0\4\371\356\177\377\377\377"
        "\1774T\2002\0\0\377\377\177ZRZ\0\4\0=2\2000\2\0\5\22\356\177\377\377"
        "\377\1774T\0262\0\0\24ZmZR\0\0\0\0\245!\00422\0\32\0\200\377!\4\177"
        "\377\377\377\1774T\2002\251\0\24ZmZR\0\0\0\0\245!\00422\0\0\0\200\377"
        "!\4\371\335";
    size_t data44length = 142;
    REPORTER_ASSERT(r, !region.readFromMemory(data44, data44length));

    static const char data45[] =
        " \0\0\0\207\256Z\250W\0\0\0\1\0\0\0X\0\0\0\200\32\33\224\343hU]\272"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0!\4\371\217E}9\34\200\255\323\377\250WK\323\200\32\33\243G\1\366"
        "\5\254\204\317\25Y\223\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0!\4\371\217E}9\34\200\255\323\377\250W\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0";
    size_t data45length = 156;
    REPORTER_ASSERT(r, !region.readFromMemory(data45, data45length));

    static const char data46[] =
        "\20\0\0\0`\0\0\0d\0d\372\0\0\372\32\33\243\0\0\200\32\33\243\223\240"
        "\370\1\335\351\315\1\335\177\377\377\377\177\377\377\223\"\370\377"
        "\177\377\377)\177\0\0\0d\0\0\0\200\32\5 \0\0}\32\33\243\223\240\372"
        "\32\33\243\0\0\200\32\33\243|\240\367\352\335\351\315\1\335\177\377"
        "\377\377\177\377\377\223\t\370\377\177\377\377)\177\0\0\0d\0\0\0\177"
        "\377\5\243\0\0\200\32\33\243\223\250\370\1\335\351\315\1\335\177\377"
        "\377\377\177\377\377\223\"\370\377\177\377\377)\177\0\372\0\0\372\32"
        "\33\243\0\377\377\177\33\243\211\240\370\1\335\315\315\1\315\177\377"
        "\377\377\177\377\377\223\370\1\335\351\315\1\335\177\377\377\377\177"
        "\377\377\223\"\370\343\177\377\377)\177\0\372\0\0\372\32\33\243\0\0"
        "\200\32\33\243\211\240\370\1\335\315\315\1\315\177\377\377\377\177"
        "\377\377\223 \370\377\177\377\377L\177\0\0\0d\0\0\0\200\32\33\261\223"
        "\t\370\1\335\315\315\1\335\315\315\361\351\327";
    size_t data46length = 263;
    REPORTER_ASSERT(r, !region.readFromMemory(data46, data46length));
}

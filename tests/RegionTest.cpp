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
        "\2\0\0\0\277]\345\222\\\2G\252\0\177'\10\203\236\211>\377\340@\351"
        "!\370y\3\31\232r\353\343\336Ja\177\377\377\377\244\301\362:Q\\\0\0"
        "\1\200\263\214\374\276\336P\225^\230\20UH N\265\357\177\240\0\306\377"
        "\177\346\222S \0\375\0\332\247 \302I\240H\374\200lk\r`\0375\324W\215"
        "\270tE^,\224n\310fy\377\231AH\16\235A\371\315\347\360\265\372r\232"
        "\301\216\35\227:\265]\32\20W\263yc\207\246\270tE^,\224n\310sy\2\0A"
        "\14\241SQ\\\303\364\0\0\1\200\0\0\374k\r`\0375\324Wp\270\267\313\313"
        "\313\313\313@\277\365b\341\343\336Ja\357~\263\0\2\333\260\220\\\303"
        "\364\265\332\267\242\325nlX\367\27I4444;\266\256\37/M\207";
    size_t data0length = 221;
    REPORTER_ASSERT(r, 0 == region.readFromMemory(data0, data0length));

    static const char data1[] =
        "\2\0\0\0\\\2G\252\0\177'\10\247 \302I\240H\374\200lk\r`\0375\324Wr"
        "\232\301\216\35\227:\265]\32\20W\263yc\207\246\270tE^,\224n\310sy\2"
        "\0A\14\241SQ\\\303\364\0\0\1\200\0\0\374k\r`\0375\324Wp\270\267\313"
        "\313\313\313\313@\277\365b\341\343\336Ja\357~\263\0\2\333\260\220\\"
        "\303\364\265\332\267\242\325nlX\367\27I4444;\266\256\37/M\207";
    size_t data1length = 129;
    REPORTER_ASSERT(r, 0 == region.readFromMemory(data1, data1length));

    static const char data2[] =
        " \0\0\0`\6\363\234AH\26\235\0\0\0\0\251\217\27I\27C\361,\320u\3171"
        "\10.\206\277]\345\222\334\2C\252\242a'\10\251\31\326\372\334A\277\30"
        "\240M\275v\201\271\3527\215{)S\3771{\345Z\250\23\213\331\23j@\13\220"
        "\200Z^-\20\212=;\355\314\36\260c\224M\16\271Szy\373\204M\21\177\251"
        "\275\r\274M\370\201\243^@\343\236JaS\204\3212\244\301\327\22\352KI"
        "\207\350z\300\250\372\26\14\2\233K\330\16\251\230\223\r\"\243\271\17"
        ")\260\262\2[a.*.4\14\344\307\350\3\0\0-\350G!\31\300\205\205\205\205"
        "\205\205\205\205\205\205\205\205\205\205\205\205\305m\311<Q\347\30"
        "\324\203f\2614\3115\206\214@:\346n\254\37\225\263\214\374\276\336\23"
        "\270\304\262\25\24_\342\223\253\351L\30\372\373\243\240g\0367V\336"
        "P\7-1{\345Z\250\23\213P\225^\230\27UH\206N\265\357\177\262\302\306"
        "kk\7\233\234N\32@\355H\327\34\337\0V\30 \225\35\225\233\253\0144>\310"
        "\352\346L\232\215\270t[^,\224l\312f\2025?}\1ZL\217wf8C\346\222S\240"
        "\203\375\374\332\247 \302I\271H\0\0lk\22`\0375\324W\374\265\342\243"
        "yL\211\215\270tE^,\224l\312f\2025?}\1ZL\217wf8C\333\370_.\277A\277"
        "^\\\313!\342\340\213\210\244\272\33\275\360\301\347\315\377\6a\272"
        "kyi:W\332\366\5\312F\217c\243\20,\"\240\347o\375\277\317}HEji\367\374"
        "\331\214\314\242x\356\340\350\362r$\222\266\325\201\234\267P\243N\361"
        "++++++++\370+@++\205!8B\255L\3\3416\335$\\\r\265W[F\326\316w{.\306"
        ">f2i\244\242=Y\236\364\302\357xR:Q\\\303\364\265\332\200\242\325nl"
        "X\373\307\5<-";
    size_t data2length = 512;
    REPORTER_ASSERT(r, 0 == region.readFromMemory(data2, data2length));
}

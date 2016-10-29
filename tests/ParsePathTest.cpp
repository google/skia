/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkParsePath.h"
#include "Test.h"

static void test_to_from(skiatest::Reporter* reporter, const SkPath& path) {
    SkString str, str2;
    SkParsePath::ToSVGString(path, &str);

    SkPath path2;
    bool success = SkParsePath::FromSVGString(str.c_str(), &path2);
    REPORTER_ASSERT(reporter, success);

    SkParsePath::ToSVGString(path2, &str2);
    REPORTER_ASSERT(reporter, str == str2);
#if 0 // closed paths are not equal, the iter explicitly gives the closing
      // edge, even if it is not in the path.
    REPORTER_ASSERT(reporter, path == path2);
    if (path != path2) {
        SkDebugf("str1=%s\nstr2=%s\n", str.c_str(), str2.c_str());
    }
#endif
}

static struct {
    const char* fStr;
    const SkRect fBounds;
} gRec[] = {
    { "M1,1 l-2.58-2.828-3.82-0.113, 1.9-3.3223-1.08-3.6702, 3.75,0.7744,3.16-2.1551,"
       "0.42,3.8008,3.02,2.3384-3.48,1.574-1.29,3.601z",
        { -5.39999962f, -10.3142f, 5.77000046f, 1.f } },
    { "", { 0, 0, 0, 0 } },
    { "M0,0L10,10", { 0, 0, SkIntToScalar(10), SkIntToScalar(10) } },
    { "M-5.5,-0.5 Q 0 0 6,6.50",
        { -5.5f, -0.5f,
          6, 6.5f } }
};

DEF_TEST(ParsePath, reporter) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        SkPath  path;
        bool success = SkParsePath::FromSVGString(gRec[i].fStr, &path);
        REPORTER_ASSERT(reporter, success);
        const SkRect& expectedBounds = gRec[i].fBounds;
        const SkRect& pathBounds = path.getBounds();
        REPORTER_ASSERT(reporter, expectedBounds == pathBounds);

        test_to_from(reporter, path);
    }

    SkRect r;
    r.set(0, 0, 10, 10.5f);
    SkPath p;
    p.addRect(r);
    test_to_from(reporter, p);
    p.addOval(r);
    test_to_from(reporter, p);
    p.addRoundRect(r, 4, 4.5f);
    test_to_from(reporter, p);
}

DEF_TEST(ParsePath_invalid, r) {
    SkPath path;
    // This is an invalid SVG string, but the test verifies that we do not
    // crash.
    bool success = SkParsePath::FromSVGString("M 5", &path);
    REPORTER_ASSERT(r, !success);
}

#include "random_parse_path.h"
#include "SkRandom.h"

DEF_TEST(ParsePathRandom, r) {
    SkRandom rand;
    for (int index = 0; index < 1000; ++index) {
        SkPath path, path2;
        SkString spec;
        uint32_t count = rand.nextRangeU(0, 10);
        for (uint32_t i = 0; i < count; ++i) {
            spec.append(MakeRandomParsePathPiece(&rand));
        }
        bool success = SkParsePath::FromSVGString(spec.c_str(), &path);
        REPORTER_ASSERT(r, success);
    }
}

DEF_TEST(ParsePathOptionalCommand, r) {
    struct {
        const char* fStr;
        int         fVerbs;
        int         fPoints;
    } gTests[] = {
        { "", 0, 0 },

        { "H100 200 ", 3, 3 },
        { "H-100-200", 3, 3 },
        { "H+100+200", 3, 3 },
        { "H.10.20"  , 3, 3 },
        { "H-.10-.20", 3, 3 },
        { "H+.10+.20", 3, 3 },

        { "L100 100 200 200" , 3, 3 },
        { "L-100-100-200-200", 3, 3 },
        { "L+100+100+200+200", 3, 3 },
        { "L.10.10.20.20"    , 3, 3 },
        { "L-.10-.10-.20-.20", 3, 3 },
        { "L+.10+.10+.20+.20", 3, 3 },

        { "C100 100 200 200 300 300 400 400 500 500 600 600" , 3, 7 },
        { "C100-100-200-200-300-300-400-400-500-500-600-600" , 3, 7 },
        { "C100+100+200+200+300+300+400+400+500+500+600+600" , 3, 7 },
        { "C.10.10.20.20.30.30.40.40.50.50.60.60"            , 3, 7 },
        { "C-.10-.10-.20-.20-.30-.30-.40-.40-.50-.50-.60-.60", 3, 7 },
        { "C+.10+.10+.20+.20+.30+.30+.40+.40+.50+.50+.60+.60", 3, 7 },

        { "c-1.49.71-2.12 2.5-1.4 4 .71 1.49 2.5 2.12 4 1.4z", 4, 7 },
    };

    SkPath path;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        REPORTER_ASSERT(r, SkParsePath::FromSVGString(gTests[i].fStr, &path));
        REPORTER_ASSERT(r, path.countVerbs() == gTests[i].fVerbs);
        REPORTER_ASSERT(r, path.countPoints() == gTests[i].fPoints);
    }
}

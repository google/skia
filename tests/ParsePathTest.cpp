/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkParsePath.h"

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

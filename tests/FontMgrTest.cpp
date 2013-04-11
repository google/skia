/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkCommandLineFlags.h"
#include "SkFontMgr.h"
#include "SkTypeface.h"

static void test_fontiter(skiatest::Reporter* reporter, bool verbose) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    int count = fm->countFamilies();

    for (int i = 0; i < count; ++i) {
        SkString fname;
        fm->getFamilyName(i, &fname);
        REPORTER_ASSERT(reporter, fname.size() > 0);

        SkAutoTUnref<SkFontStyleSet> set(fm->createStyleSet(i));

        if (verbose) {
            SkDebugf("[%2d] %s\n", i, fname.c_str());
        }

        for (int j = 0; j < set->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            set->getStyle(j, &fs, &sname);
            REPORTER_ASSERT(reporter, sname.size() > 0);

            SkAutoTUnref<SkTypeface> face(set->createTypeface(j));
            REPORTER_ASSERT(reporter, face.get());

            if (verbose) {
                SkDebugf("\t[%d] %s [%3d %d %d]\n", j, sname.c_str(),
                         fs.weight(), fs.width(), fs.isItalic());
            }
        }
    }
}

DEFINE_bool(verboseFontMgr, false, "run verbose fontmgr tests.");

static void TestFontMgr(skiatest::Reporter* reporter) {
    test_fontiter(reporter, FLAGS_verboseFontMgr);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("FontMgr", FontMgrClass, TestFontMgr)

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkCommandLineFlags.h"
#include "SkFontMgr.h"
#include "SkTypeface.h"

/*
 *  If the font backend is going to "alias" some font names to other fonts
 *  (e.g. sans -> Arial) then we want to at least get the same typeface back
 *  if we request the alias name multiple times.
 */
static void test_alias_names(skiatest::Reporter* reporter) {
    const char* inNames[] = {
        "sans", "sans-serif", "serif", "monospace", "times", "helvetica"
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(inNames); ++i) {
        SkAutoTUnref<SkTypeface> first(SkTypeface::CreateFromName(inNames[i],
                                                          SkTypeface::kNormal));
        if (NULL == first.get()) {
            continue;
        }
        for (int j = 0; j < 10; ++j) {
            SkAutoTUnref<SkTypeface> face(SkTypeface::CreateFromName(inNames[i],
                                                         SkTypeface::kNormal));
    #if 0
            SkString name;
            face->getFamilyName(&name);
            printf("request %s, received %s, first id %x received %x\n",
                   inNames[i], name.c_str(), first->uniqueID(), face->uniqueID());
    #endif
            REPORTER_ASSERT(reporter, first->uniqueID() == face->uniqueID());
        }
    }
}

static void test_fontiter(skiatest::Reporter* reporter, bool verbose) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    int count = fm->countFamilies();

    for (int i = 0; i < count; ++i) {
        SkString fname;
        fm->getFamilyName(i, &fname);
        REPORTER_ASSERT(reporter, fname.size() > 0);

        SkAutoTUnref<SkFontStyleSet> fnset(fm->matchFamily(fname.c_str()));
        SkAutoTUnref<SkFontStyleSet> set(fm->createStyleSet(i));
        REPORTER_ASSERT(reporter, fnset->count() == set->count());

        if (verbose) {
            SkDebugf("[%2d] %s\n", i, fname.c_str());
        }

        for (int j = 0; j < set->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            set->getStyle(j, &fs, &sname);
//            REPORTER_ASSERT(reporter, sname.size() > 0);

            SkAutoTUnref<SkTypeface> face(set->createTypeface(j));
//            REPORTER_ASSERT(reporter, face.get());

            if (verbose) {
                SkDebugf("\t[%d] %s [%3d %d %d]\n", j, sname.c_str(),
                         fs.weight(), fs.width(), fs.isItalic());
            }
        }
    }
}

DEFINE_bool(verboseFontMgr, false, "run verbose fontmgr tests.");

DEF_TEST(FontMgr, reporter) {
    test_fontiter(reporter, FLAGS_verboseFontMgr);
    test_alias_names(reporter);
}

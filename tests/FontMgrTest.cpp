/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkFontMgr.h"
#include "SkTypeface.h"
#include "Test.h"

#include "SkFont.h"
#include "SkPaint.h"
static void test_font(skiatest::Reporter* reporter) {
    uint32_t flags = 0;
    SkAutoTUnref<SkFont> font(SkFont::Create(NULL, 24, SkFont::kA8_MaskType, flags));

    REPORTER_ASSERT(reporter, font->getTypeface());
    REPORTER_ASSERT(reporter, 24 == font->getSize());
    REPORTER_ASSERT(reporter, 1 == font->getScaleX());
    REPORTER_ASSERT(reporter, 0 == font->getSkewX());
    REPORTER_ASSERT(reporter, SkFont::kA8_MaskType == font->getMaskType());

    uint16_t glyphs[5];
    sk_bzero(glyphs, sizeof(glyphs));

    int count = font->textToGlyphs("Hello", 5, kUTF8_SkTextEncoding, glyphs, SK_ARRAY_COUNT(glyphs));

    REPORTER_ASSERT(reporter, 5 == count);
    for (int i = 0; i < count; ++i) {
        REPORTER_ASSERT(reporter, 0 != glyphs[i]);
    }
    REPORTER_ASSERT(reporter, glyphs[0] != glyphs[1]); // 'h' != 'e'
    REPORTER_ASSERT(reporter, glyphs[2] == glyphs[3]); // 'l' == 'l'

    SkAutoTUnref<SkFont> newFont(font->cloneWithSize(36));
    REPORTER_ASSERT(reporter, newFont.get());
    REPORTER_ASSERT(reporter, font->getTypeface() == newFont->getTypeface());
    REPORTER_ASSERT(reporter, 36 == newFont->getSize());   // double check we haven't changed
    REPORTER_ASSERT(reporter, 24 == font->getSize());   // double check we haven't changed

    SkPaint paint;
    paint.setTextSize(18);
    font.reset(SkFont::Testing_CreateFromPaint(paint));
    REPORTER_ASSERT(reporter, font.get());
    REPORTER_ASSERT(reporter, font->getSize() == paint.getTextSize());
    REPORTER_ASSERT(reporter, SkFont::kBW_MaskType == font->getMaskType());
}

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
    test_font(reporter);
}

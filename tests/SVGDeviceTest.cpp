/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkDOM.h"
#include "SkParse.h"
#include "SkStream.h"
#include "SkSVGCanvas.h"
#include "SkXMLWriter.h"
#include "Test.h"

#include <string.h>

namespace {

void check_text_node(skiatest::Reporter* reporter,
                     const SkDOM& dom,
                     const SkDOM::Node* root,
                     const SkPoint& offset,
                     unsigned scalarsPerPos,
                     const char* expected) {
    if (root == nullptr) {
        ERRORF(reporter, "root element not found.");
        return;
    }

    const SkDOM::Node* textElem = dom.getFirstChild(root, "text");
    if (textElem == nullptr) {
        ERRORF(reporter, "<text> element not found.");
        return;
    }
    REPORTER_ASSERT(reporter, dom.getType(textElem) == SkDOM::kElement_Type);

    const SkDOM::Node* textNode= dom.getFirstChild(textElem);
    REPORTER_ASSERT(reporter, textNode != nullptr);
    if (textNode != nullptr) {
        REPORTER_ASSERT(reporter, dom.getType(textNode) == SkDOM::kText_Type);
        REPORTER_ASSERT(reporter, strcmp(expected, dom.getName(textNode)) == 0);
    }

    int textLen = SkToInt(strlen(expected));

    const char* x = dom.findAttr(textElem, "x");
    REPORTER_ASSERT(reporter, x != nullptr);
    if (x != nullptr) {
        int xposCount = (scalarsPerPos < 1) ? 1 : textLen;
        REPORTER_ASSERT(reporter, SkParse::Count(x) == xposCount);

        SkAutoTMalloc<SkScalar> xpos(xposCount);
        SkParse::FindScalars(x, xpos.get(), xposCount);
        if (scalarsPerPos < 1) {
            REPORTER_ASSERT(reporter, xpos[0] == offset.x());
        } else {
            for (int i = 0; i < xposCount; ++i) {
                REPORTER_ASSERT(reporter, xpos[i] == SkIntToScalar(expected[i]));
            }
        }
    }

    const char* y = dom.findAttr(textElem, "y");
    REPORTER_ASSERT(reporter, y != nullptr);
    if (y != nullptr) {
        int yposCount = (scalarsPerPos < 2) ? 1 : textLen;
        REPORTER_ASSERT(reporter, SkParse::Count(y) == yposCount);

        SkAutoTMalloc<SkScalar> ypos(yposCount);
        SkParse::FindScalars(y, ypos.get(), yposCount);
        if (scalarsPerPos < 2) {
            REPORTER_ASSERT(reporter, ypos[0] == offset.y());
        } else {
            for (int i = 0; i < yposCount; ++i) {
                REPORTER_ASSERT(reporter, ypos[i] == -SkIntToScalar(expected[i]));
            }
        }
    }
}

void test_whitespace_pos(skiatest::Reporter* reporter,
                         const char* txt,
                         const char* expected) {
    size_t len = strlen(txt);

    SkDOM dom;
    SkPaint paint;
    SkPoint offset = SkPoint::Make(10, 20);

    {
        SkXMLParserWriter writer(dom.beginParsing());
        std::unique_ptr<SkCanvas> svgCanvas = SkSVGCanvas::Make(SkRect::MakeWH(100, 100), &writer);
        svgCanvas->drawText(txt, len, offset.x(), offset.y(), paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 0, expected);

    {
        SkAutoTMalloc<SkScalar> xpos(len);
        for (int i = 0; i < SkToInt(len); ++i) {
            xpos[i] = SkIntToScalar(txt[i]);
        }

        SkXMLParserWriter writer(dom.beginParsing());
        std::unique_ptr<SkCanvas> svgCanvas = SkSVGCanvas::Make(SkRect::MakeWH(100, 100), &writer);
        svgCanvas->drawPosTextH(txt, len, xpos, offset.y(), paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 1, expected);

    {
        SkAutoTMalloc<SkPoint> pos(len);
        for (int i = 0; i < SkToInt(len); ++i) {
            pos[i] = SkPoint::Make(SkIntToScalar(txt[i]), -SkIntToScalar(txt[i]));
        }

        SkXMLParserWriter writer(dom.beginParsing());
        std::unique_ptr<SkCanvas> svgCanvas = SkSVGCanvas::Make(SkRect::MakeWH(100, 100), &writer);
        svgCanvas->drawPosText(txt, len, pos, paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 2, expected);
}

}

DEF_TEST(SVGDevice_whitespace_pos, reporter) {
    static const struct {
        const char* tst_in;
        const char* tst_out;
    } tests[] = {
        { "abcd"      , "abcd" },
        { "ab cd"     , "ab cd" },
        { "ab \t\t cd", "ab cd" },
        { " abcd"     , "abcd" },
        { "  abcd"    , "abcd" },
        { " \t\t abcd", "abcd" },
        { "abcd "     , "abcd " }, // we allow one trailing whitespace char
        { "abcd  "    , "abcd " }, // because it makes no difference and
        { "abcd\t  "  , "abcd\t" }, // simplifies the implementation
        { "\t\t  \t ab \t\t  \t cd \t\t   \t  ", "ab cd " },
    };

    for (unsigned i = 0; i < SK_ARRAY_COUNT(tests); ++i) {
        test_whitespace_pos(reporter, tests[i].tst_in, tests[i].tst_out);
    }
}

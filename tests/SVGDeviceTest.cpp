/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define ABORT_TEST(r, cond, ...)                                   \
    do {                                                           \
        if (cond) {                                                \
            REPORT_FAILURE(r, #cond, SkStringPrintf(__VA_ARGS__)); \
            return;                                                \
        }                                                          \
    } while (0)

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageShader.h"
#include "SkMakeUnique.h"
#include "SkParse.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTo.h"
#include "Test.h"

#include <string.h>

#ifdef SK_XML

#include "SkDOM.h"
#include "../src/svg/SkSVGDevice.h"
#include "SkXMLWriter.h"

static std::unique_ptr<SkCanvas> MakeDOMCanvas(SkDOM* dom) {
    auto svgDevice = SkSVGDevice::Make(SkISize::Make(100, 100),
                                       skstd::make_unique<SkXMLParserWriter>(dom->beginParsing()));
    return svgDevice ? skstd::make_unique<SkCanvas>(svgDevice)
                     : nullptr;
}

#if 0
Using the new system where devices only gets glyphs causes this to fail because the font has no
glyph to unichar data.
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
        if (strcmp(expected, dom.getName(textNode)) != 0) {
            SkDebugf("string fail %s == %s\n", expected, dom.getName(textNode));
        }
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
                if (xpos[i] != SkIntToScalar(expected[i])) {
                    SkDebugf("Bad xs %g == %g\n", xpos[i], SkIntToScalar(expected[i]));
                }
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
    SkFont font;
    SkPoint offset = SkPoint::Make(10, 20);

    {
        auto svgCanvas = MakeDOMCanvas(&dom);
        svgCanvas->drawSimpleText(txt, len, kUTF8_SkTextEncoding, offset.x(), offset.y(),
                                  font, paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 2, expected);

    {
        SkAutoTMalloc<SkScalar> xpos(len);
        for (int i = 0; i < SkToInt(len); ++i) {
            xpos[i] = SkIntToScalar(txt[i]);
        }

        auto svgCanvas = MakeDOMCanvas(&dom);
        auto blob = SkTextBlob::MakeFromPosTextH(txt, len, &xpos[0], offset.y(), font);
        svgCanvas->drawTextBlob(blob, 0, 0, paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 2, expected);

    {
        SkAutoTMalloc<SkPoint> pos(len);
        for (int i = 0; i < SkToInt(len); ++i) {
            pos[i] = SkPoint::Make(SkIntToScalar(txt[i]), -SkIntToScalar(txt[i]));
        }

        SkXMLParserWriter writer(dom.beginParsing());
        auto blob = SkTextBlob::MakeFromPosTextH(txt, len, &pos[0], font);
        svgCanvas->drawTextBlob(blob, 0, 0, paint);
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
#endif


void SetImageShader(SkPaint* paint, int imageWidth, int imageHeight, SkTileMode xTile,
                    SkTileMode yTile) {
    auto surface = SkSurface::MakeRasterN32Premul(imageWidth, imageHeight);
    paint->setShader(surface->makeImageSnapshot()->makeShader(xTile, yTile, nullptr));
}

// Attempt to find the three nodes on which we have expectations:
// the pattern node, the image within that pattern, and the rect which
// uses the pattern as a fill.
// returns false if not all nodes are found.
bool FindImageShaderNodes(skiatest::Reporter* reporter, const SkDOM* dom, const SkDOM::Node* root,
                          const SkDOM::Node** patternOut, const SkDOM::Node** imageOut,
                          const SkDOM::Node** rectOut) {
    if (root == nullptr || dom == nullptr) {
        ERRORF(reporter, "root element not found");
        return false;
    }


    const SkDOM::Node* rect = dom->getFirstChild(root, "rect");
    if (rect == nullptr) {
        ERRORF(reporter, "rect not found");
        return false;
    }
    *rectOut = rect;

    const SkDOM::Node* defs = dom->getFirstChild(root, "defs");
    if (defs == nullptr) {
        ERRORF(reporter, "defs not found");
        return false;
    }

    const SkDOM::Node* pattern = dom->getFirstChild(defs, "pattern");
    if (pattern == nullptr) {
        ERRORF(reporter, "pattern not found");
        return false;
    }
    *patternOut = pattern;

    const SkDOM::Node* image = dom->getFirstChild(pattern, "image");
    if (image == nullptr) {
        ERRORF(reporter, "image not found");
        return false;
    }
    *imageOut = image;

    return true;
}

void ImageShaderTestSetup(SkDOM* dom, SkPaint* paint, int imageWidth, int imageHeight,
                          int rectWidth, int rectHeight, SkTileMode xTile, SkTileMode yTile) {
    SetImageShader(paint, imageWidth, imageHeight, xTile, yTile);
    auto svgCanvas = MakeDOMCanvas(dom);

    SkRect bounds{0, 0, SkIntToScalar(rectWidth), SkIntToScalar(rectHeight)};
    svgCanvas->drawRect(bounds, *paint);
}


DEF_TEST(SVGDevice_image_shader_norepeat, reporter) {
    SkDOM dom;
    SkPaint paint;
    int imageWidth = 3, imageHeight = 3;
    int rectWidth = 10, rectHeight = 10;
    ImageShaderTestSetup(&dom, &paint, imageWidth, imageHeight, rectWidth, rectHeight,
                         SkTileMode::kClamp, SkTileMode::kClamp);

    const SkDOM::Node* root = dom.finishParsing();

    const SkDOM::Node *patternNode, *imageNode, *rectNode;
    bool structureAppropriate =
            FindImageShaderNodes(reporter, &dom, root, &patternNode, &imageNode, &rectNode);
    REPORTER_ASSERT(reporter, structureAppropriate);

    // the image should always maintain its size.
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "width")) == imageWidth);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "height")) == imageHeight);

    // making the pattern as large as the container prevents
    // it from repeating.
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(patternNode, "width"), "100%") == 0);
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(patternNode, "height"), "100%") == 0);
}

DEF_TEST(SVGDevice_image_shader_tilex, reporter) {
    SkDOM dom;
    SkPaint paint;
    int imageWidth = 3, imageHeight = 3;
    int rectWidth = 10, rectHeight = 10;
    ImageShaderTestSetup(&dom, &paint, imageWidth, imageHeight, rectWidth, rectHeight,
                         SkTileMode::kRepeat, SkTileMode::kClamp);

    const SkDOM::Node* root = dom.finishParsing();
    const SkDOM::Node* innerSvg = dom.getFirstChild(root, "svg");
    if (innerSvg == nullptr) {
        ERRORF(reporter, "inner svg element not found");
        return;
    }

    const SkDOM::Node *patternNode, *imageNode, *rectNode;
    bool structureAppropriate =
            FindImageShaderNodes(reporter, &dom, innerSvg, &patternNode, &imageNode, &rectNode);
    REPORTER_ASSERT(reporter, structureAppropriate);

    // the imageNode should always maintain its size.
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "width")) == imageWidth);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "height")) == imageHeight);

    // if the patternNode width matches the imageNode width,
    // it will repeat in along the x axis.
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(patternNode, "width")) == imageWidth);
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(patternNode, "height"), "100%") == 0);
}

DEF_TEST(SVGDevice_image_shader_tiley, reporter) {
    SkDOM dom;
    SkPaint paint;
    int imageNodeWidth = 3, imageNodeHeight = 3;
    int rectNodeWidth = 10, rectNodeHeight = 10;
    ImageShaderTestSetup(&dom, &paint, imageNodeWidth, imageNodeHeight, rectNodeWidth,
                         rectNodeHeight, SkTileMode::kClamp, SkTileMode::kRepeat);

    const SkDOM::Node* root = dom.finishParsing();
    const SkDOM::Node* innerSvg = dom.getFirstChild(root, "svg");
    if (innerSvg == nullptr) {
        ERRORF(reporter, "inner svg element not found");
        return;
    }

    const SkDOM::Node *patternNode, *imageNode, *rectNode;
    bool structureAppropriate =
            FindImageShaderNodes(reporter, &dom, innerSvg, &patternNode, &imageNode, &rectNode);
    REPORTER_ASSERT(reporter, structureAppropriate);

    // the imageNode should always maintain its size.
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "width")) == imageNodeWidth);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "height")) == imageNodeHeight);

    // making the patternNode as large as the container prevents
    // it from repeating.
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(patternNode, "width"), "100%") == 0);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(patternNode, "height")) == imageNodeHeight);
}

DEF_TEST(SVGDevice_image_shader_tileboth, reporter) {
    SkDOM dom;
    SkPaint paint;
    int imageWidth = 3, imageHeight = 3;
    int rectWidth = 10, rectHeight = 10;
    ImageShaderTestSetup(&dom, &paint, imageWidth, imageHeight, rectWidth, rectHeight,
                         SkTileMode::kRepeat, SkTileMode::kRepeat);

    const SkDOM::Node* root = dom.finishParsing();

    const SkDOM::Node *patternNode, *imageNode, *rectNode;
    const SkDOM::Node* innerSvg = dom.getFirstChild(root, "svg");
    if (innerSvg == nullptr) {
        ERRORF(reporter, "inner svg element not found");
        return;
    }
    bool structureAppropriate =
            FindImageShaderNodes(reporter, &dom, innerSvg, &patternNode, &imageNode, &rectNode);
    REPORTER_ASSERT(reporter, structureAppropriate);

    // the imageNode should always maintain its size.
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "width")) == imageWidth);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(imageNode, "height")) == imageHeight);

    REPORTER_ASSERT(reporter, atoi(dom.findAttr(patternNode, "width")) == imageWidth);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(patternNode, "height")) == imageHeight);
}

DEF_TEST(SVGDevice_ColorFilters, reporter) {
    SkDOM dom;
    SkPaint paint;
    paint.setColorFilter(SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcIn));
    {
        auto svgCanvas = MakeDOMCanvas(&dom);
        SkRect bounds{0, 0, SkIntToScalar(100), SkIntToScalar(100)};
        svgCanvas->drawRect(bounds, paint);
    }
    const SkDOM::Node* rootElement = dom.finishParsing();
    ABORT_TEST(reporter, !rootElement, "root element not found");

    const SkDOM::Node* filterElement = dom.getFirstChild(rootElement, "filter");
    ABORT_TEST(reporter, !filterElement, "filter element not found");

    const SkDOM::Node* floodElement = dom.getFirstChild(filterElement, "feFlood");
    ABORT_TEST(reporter, !floodElement, "feFlood element not found");

    const SkDOM::Node* compositeElement = dom.getFirstChild(filterElement, "feComposite");
    ABORT_TEST(reporter, !compositeElement, "feComposite element not found");

    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(filterElement, "width"), "100%") == 0);
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(filterElement, "height"), "100%") == 0);

    REPORTER_ASSERT(reporter,
                    strcmp(dom.findAttr(floodElement, "flood-color"), "rgb(255,0,0)") == 0);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(floodElement, "flood-opacity")) == 1);

    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(compositeElement, "in"), "flood") == 0);
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(compositeElement, "operator"), "in") == 0);
}

#endif

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#ifdef SK_XML

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/base/SkTo.h"
#include "include/svg/SkSVGCanvas.h"
#include "include/utils/SkParse.h"
#include "src/shaders/SkImageShader.h"
#include "src/svg/SkSVGDevice.h"
#include "src/xml/SkDOM.h"
#include "src/xml/SkXMLWriter.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string>

using namespace skia_private;

#define ABORT_TEST(r, cond, ...)                                   \
    do {                                                           \
        if (cond) {                                                \
            REPORT_FAILURE(r, #cond, SkStringPrintf(__VA_ARGS__)); \
            return;                                                \
        }                                                          \
    } while (0)


static std::unique_ptr<SkCanvas> MakeDOMCanvas(SkDOM* dom, uint32_t flags = 0) {
    auto svgDevice = SkSVGDevice::Make(SkISize::Make(100, 100),
                                       std::make_unique<SkXMLParserWriter>(dom->beginParsing()),
                                       flags);
    return svgDevice ? std::make_unique<SkCanvas>(svgDevice)
                     : nullptr;
}

namespace {


void check_text_node(skiatest::Reporter* reporter,
                     const SkDOM& dom,
                     const SkDOM::Node* root,
                     const SkPoint& offset,
                     unsigned scalarsPerPos,
                     const char* txt,
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
        int xposCount = textLen;
        REPORTER_ASSERT(reporter, SkParse::Count(x) == xposCount);

        AutoTMalloc<SkScalar> xpos(xposCount);
        SkParse::FindScalars(x, xpos.get(), xposCount);
        if (scalarsPerPos < 1) {
            // For default-positioned text, we cannot make any assumptions regarding
            // the first glyph position when the string has leading whitespace (to be stripped).
            if (txt[0] != ' ' && txt[0] != '\t') {
                REPORTER_ASSERT(reporter, xpos[0] == offset.x());
            }
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

        AutoTMalloc<SkScalar> ypos(yposCount);
        SkParse::FindScalars(y, ypos.get(), yposCount);
        if (scalarsPerPos < 2) {
            REPORTER_ASSERT(reporter, ypos[0] == offset.y());
        } else {
            for (int i = 0; i < yposCount; ++i) {
                REPORTER_ASSERT(reporter, ypos[i] == 150 - SkIntToScalar(expected[i]));
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
    SkFont font = ToolUtils::DefaultPortableFont();
    SkPoint offset = SkPoint::Make(10, 20);

    {
        MakeDOMCanvas(&dom)->drawSimpleText(txt, len, SkTextEncoding::kUTF8,
                                            offset.x(), offset.y(), font, paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 0, txt, expected);

    {
        AutoTMalloc<SkScalar> xpos(len);
        for (int i = 0; i < SkToInt(len); ++i) {
            xpos[i] = SkIntToScalar(txt[i]);
        }

        auto blob = SkTextBlob::MakeFromPosTextH(txt, len, &xpos[0], offset.y(), font);
        MakeDOMCanvas(&dom)->drawTextBlob(blob, 0, 0, paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 1, txt, expected);

    {
        AutoTMalloc<SkPoint> pos(len);
        for (int i = 0; i < SkToInt(len); ++i) {
            pos[i] = SkPoint::Make(SkIntToScalar(txt[i]), 150 - SkIntToScalar(txt[i]));
        }

        auto blob = SkTextBlob::MakeFromPosText(txt, len, &pos[0], font);
        MakeDOMCanvas(&dom)->drawTextBlob(blob, 0, 0, paint);
    }
    check_text_node(reporter, dom, dom.finishParsing(), offset, 2, txt, expected);
}

} // namespace

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
        { "abcd\t  "  , "abcd " }, // simplifies the implementation
        { "\t\t  \t ab \t\t  \t cd \t\t   \t  ", "ab cd " },
    };

    for (unsigned i = 0; i < std::size(tests); ++i) {
        test_whitespace_pos(reporter, tests[i].tst_in, tests[i].tst_out);
    }
}

void SetImageShader(SkPaint* paint, int imageWidth, int imageHeight, SkTileMode xTile,
                    SkTileMode yTile) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(imageWidth, imageHeight));
    paint->setShader(surface->makeImageSnapshot()->makeShader(xTile, yTile, SkSamplingOptions()));
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
                    strcmp(dom.findAttr(floodElement, "flood-color"), "red") == 0);
    REPORTER_ASSERT(reporter, atoi(dom.findAttr(floodElement, "flood-opacity")) == 1);

    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(compositeElement, "in"), "flood") == 0);
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(compositeElement, "operator"), "in") == 0);
}

DEF_TEST(SVGDevice_textpath, reporter) {
    SkDOM dom;
    SkFont font = ToolUtils::DefaultPortableFont();
    SkPaint paint;

    auto check_text = [&](uint32_t flags, bool expect_path) {
        // By default, we emit <text> nodes.
        {
            auto svgCanvas = MakeDOMCanvas(&dom, flags);
            svgCanvas->drawString("foo", 100, 100, font, paint);
        }
        const auto* rootElement = dom.finishParsing();
        REPORTER_ASSERT(reporter, rootElement, "root element not found");
        const auto* textElement = dom.getFirstChild(rootElement, "text");
        REPORTER_ASSERT(reporter, !!textElement == !expect_path, "unexpected text element");
        const auto* pathElement = dom.getFirstChild(rootElement, "path");
        REPORTER_ASSERT(reporter, !!pathElement == expect_path, "unexpected path element");
    };

    // By default, we emit <text> nodes.
    check_text(0, /*expect_path=*/false);

    // With kConvertTextToPaths_Flag, we emit <path> nodes.
    check_text(SkSVGCanvas::kConvertTextToPaths_Flag, /*expect_path=*/true);

    // We also use paths in the presence of path effects.
    SkScalar intervals[] = {10, 5};
    paint.setPathEffect(SkDashPathEffect::Make(intervals, std::size(intervals), 0));
    check_text(0, /*expect_path=*/true);
}

DEF_TEST(SVGDevice_fill_stroke, reporter) {
    struct {
        SkColor        color;
        SkPaint::Style style;
        const char*    expected_fill;
        const char*    expected_stroke;
    } gTests[] = {
        { SK_ColorBLACK, SkPaint::kFill_Style  , nullptr, nullptr },
        { SK_ColorBLACK, SkPaint::kStroke_Style, "none" , "black" },
        { SK_ColorRED  , SkPaint::kFill_Style  , "red"  , nullptr },
        { SK_ColorRED  , SkPaint::kStroke_Style, "none" , "red"   },
    };

    for (const auto& tst : gTests) {
        SkPaint p;
        p.setColor(tst.color);
        p.setStyle(tst.style);

        SkDOM dom;
        {
            MakeDOMCanvas(&dom)->drawRect(SkRect::MakeWH(100, 100), p);
        }

        const auto* root = dom.finishParsing();
        REPORTER_ASSERT(reporter, root, "root element not found");
        const auto* rect = dom.getFirstChild(root, "rect");
        REPORTER_ASSERT(reporter, rect, "rect element not found");
        const auto* fill = dom.findAttr(rect, "fill");
        REPORTER_ASSERT(reporter, !!fill == !!tst.expected_fill);
        if (fill) {
            REPORTER_ASSERT(reporter, strcmp(fill, tst.expected_fill) == 0);
        }
        const auto* stroke = dom.findAttr(rect, "stroke");
        REPORTER_ASSERT(reporter, !!stroke == !!tst.expected_stroke);
        if (stroke) {
            REPORTER_ASSERT(reporter, strcmp(stroke, tst.expected_stroke) == 0);
        }
    }
}

DEF_TEST(SVGDevice_fill_rect_hex, reporter) {
    SkDOM dom;
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    {
        auto svgCanvas = MakeDOMCanvas(&dom);
        SkRect bounds{0, 0, SkIntToScalar(100), SkIntToScalar(100)};
        svgCanvas->drawRect(bounds, paint);
    }
    const SkDOM::Node* rootElement = dom.finishParsing();
    ABORT_TEST(reporter, !rootElement, "root element not found");

    const SkDOM::Node* rectElement = dom.getFirstChild(rootElement, "rect");
    ABORT_TEST(reporter, !rectElement, "rect element not found");
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectElement, "fill"), "blue") == 0);
}

DEF_TEST(SVGDevice_fill_rect_custom_hex, reporter) {
    SkDOM dom;
    {
        SkPaint paint;
        paint.setColor(0xFFAABCDE);
        auto svgCanvas = MakeDOMCanvas(&dom);
        SkRect bounds{0, 0, SkIntToScalar(100), SkIntToScalar(100)};
        svgCanvas->drawRect(bounds, paint);
        paint.setColor(0xFFAABBCC);
        svgCanvas->drawRect(bounds, paint);
        paint.setColor(0xFFAA1123);
        svgCanvas->drawRect(bounds, paint);
    }
    const SkDOM::Node* rootElement = dom.finishParsing();
    ABORT_TEST(reporter, !rootElement, "root element not found");

    // Test 0xAABCDE filled rect.
    const SkDOM::Node* rectElement = dom.getFirstChild(rootElement, "rect");
    ABORT_TEST(reporter, !rectElement, "rect element not found");
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectElement, "fill"), "#AABCDE") == 0);

    // Test 0xAABBCC filled rect.
    rectElement = dom.getNextSibling(rectElement, "rect");
    ABORT_TEST(reporter, !rectElement, "rect element not found");
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectElement, "fill"), "#ABC") == 0);

    // Test 0xFFAA1123 filled rect. Make sure it does not turn into #A123.
    rectElement = dom.getNextSibling(rectElement, "rect");
    ABORT_TEST(reporter, !rectElement, "rect element not found");
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectElement, "fill"), "#AA1123") == 0);
}

DEF_TEST(SVGDevice_fill_stroke_rect_hex, reporter) {
    SkDOM dom;
    {
        auto svgCanvas = MakeDOMCanvas(&dom);
        SkRect bounds{0, 0, SkIntToScalar(100), SkIntToScalar(100)};

        SkPaint paint;
        paint.setColor(0xFF00BBAC);
        svgCanvas->drawRect(bounds, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(0xFF123456);
        paint.setStrokeWidth(1);
        svgCanvas->drawRect(bounds, paint);
    }
    const SkDOM::Node* rootElement = dom.finishParsing();
    ABORT_TEST(reporter, !rootElement, "root element not found");

    const SkDOM::Node* rectNode = dom.getFirstChild(rootElement, "rect");
    ABORT_TEST(reporter, !rectNode, "rect element not found");
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectNode, "fill"), "#00BBAC") == 0);

    rectNode = dom.getNextSibling(rectNode, "rect");
    ABORT_TEST(reporter, !rectNode, "rect element not found");
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectNode, "stroke"), "#123456") == 0);
    REPORTER_ASSERT(reporter, strcmp(dom.findAttr(rectNode, "stroke-width"), "1") == 0);
}

DEF_TEST(SVGDevice_path_effect, reporter) {
    SkDOM dom;

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    paint.setStrokeCap(SkPaint::kRound_Cap);

    // Produces a line of three red dots.
    SkScalar intervals[] = {0, 20};
    sk_sp<SkPathEffect> pathEffect = SkDashPathEffect::Make(intervals, 2, 0);
    paint.setPathEffect(pathEffect);
    SkPoint points[] = {{50, 15}, {100, 15}, {150, 15} };
    {
        auto svgCanvas = MakeDOMCanvas(&dom);
        svgCanvas->drawPoints(SkCanvas::kLines_PointMode, 3, points, paint);
    }
    const auto* rootElement = dom.finishParsing();
    REPORTER_ASSERT(reporter, rootElement, "root element not found");
    const auto* pathElement = dom.getFirstChild(rootElement, "path");
    REPORTER_ASSERT(reporter, pathElement, "path element not found");

    // The SVG path to draw the three dots is a complex list of instructions.
    // To avoid test brittleness, we don't attempt to match the entire path.
    // Instead, we simply confirm there are three (M)ove instructions, one per
    // dot.  If path effects were not being honored, we would expect only one
    // Move instruction, to the starting position, before drawing a continuous
    // straight line.
    const auto* d = dom.findAttr(pathElement, "d");
    int mCount = 0;
    const char* pos;
    for (pos = d; *pos != '\0'; pos++) {
      mCount += (*pos == 'M') ? 1 : 0;
    }
    REPORTER_ASSERT(reporter, mCount == 3);
}

DEF_TEST(SVGDevice_relative_path_encoding, reporter) {
    SkDOM dom;
    {
        auto svgCanvas = MakeDOMCanvas(&dom, SkSVGCanvas::kRelativePathEncoding_Flag);
        SkPath path;
        path.moveTo(100, 50);
        path.lineTo(200, 50);
        path.lineTo(200, 150);
        path.close();

        svgCanvas->drawPath(path, SkPaint());
    }

    const auto* rootElement = dom.finishParsing();
    REPORTER_ASSERT(reporter, rootElement, "root element not found");
    const auto* pathElement = dom.getFirstChild(rootElement, "path");
    REPORTER_ASSERT(reporter, pathElement, "path element not found");
    const auto* d = dom.findAttr(pathElement, "d");
    REPORTER_ASSERT(reporter, !strcmp(d, "m100 50l100 0l0 100l-100 -100Z"));
}

DEF_TEST(SVGDevice_color_shader, reporter) {
    SkDOM dom;
    {
        auto svgCanvas = MakeDOMCanvas(&dom);

        SkPaint paint;
        paint.setShader(SkShaders::Color(0xffffff00));

        svgCanvas->drawCircle(100, 100, 100, paint);
    }

    const auto* rootElement = dom.finishParsing();
    REPORTER_ASSERT(reporter, rootElement, "root element not found");
    const auto* ellipseElement = dom.getFirstChild(rootElement, "ellipse");
    REPORTER_ASSERT(reporter, ellipseElement, "ellipse element not found");
    const auto* fill = dom.findAttr(ellipseElement, "fill");
    REPORTER_ASSERT(reporter, fill, "fill attribute not found");
    REPORTER_ASSERT(reporter, !strcmp(fill, "yellow"));
}

DEF_TEST(SVGDevice_parse_minmax, reporter) {
    auto check = [&](int64_t n, bool expected) {
        const auto str = std::to_string(n);

        int val;
        REPORTER_ASSERT(reporter, SkToBool(SkParse::FindS32(str.c_str(), &val)) == expected);
        if (expected) {
            REPORTER_ASSERT(reporter, val == n);
        }
    };

    check(std::numeric_limits<int>::max(), true);
    check(std::numeric_limits<int>::min(), true);
    check(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1, false);
    check(static_cast<int64_t>(std::numeric_limits<int>::min()) - 1, false);
}

#endif

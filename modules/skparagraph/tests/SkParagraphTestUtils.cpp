/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkTArray.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTSort.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstring>

DEFINE_string(paragraph_fonts, "", "subdirectory of //resources for fonts to use for these tests");
DEFINE_bool(run_paragraph_tests_needing_system_fonts,
            true,
            "Some tests are finicky and need certain system fonts. "
            "Set this to false to skip those.");

namespace skia::textlayout::test {

bool equal(const char* base, TextRange a, const char* b) {
    return std::strncmp(b, base + a.start, a.width()) == 0;
}

std::u16string mirror(const std::string& text) {
    std::u16string result;
    result += u"\u202E";
    for (auto i = text.size(); i > 0; --i) {
        result += text[i - 1];
    }
    //result += u"\u202C";
    return result;
}

std::u16string straight(const std::string& text) {
    std::u16string result;
    result += u"\u202D";
    for (auto ch : text) {
        result += ch;
    }
    return result;
}

sk_sp<SkUnicode> get_unicode() {
    auto factory = SkShapers::BestAvailable();
    return sk_ref_sp<SkUnicode>(factory->getUnicode());
}

static const std::vector<sk_sp<SkTypeface>>& getTypefaces() {
    static std::vector<sk_sp<SkTypeface>> typefaces = []() -> std::vector<sk_sp<SkTypeface>> {
        if (FLAGS_paragraph_fonts.size() == 0) {
            return {};
        }
        skia_private::TArray<SkString> paths;
        {
            SkString fontResources = GetResourcePath(FLAGS_paragraph_fonts[0]);
            const char* fontDir = fontResources.c_str();
            SkOSFile::Iter iter(fontDir);
            SkString path;
            while (iter.next(&path)) {
                if ((false)) {
                    SkDebugf("Found font file: %s\n", path.c_str());
                }
                SkString fullPath;
                fullPath.printf("%s/%s", fontDir, path.c_str());
                paths.emplace_back(fullPath);
            }
            if (paths.size()) {
                SkTQSort(paths.begin(), paths.end(), [](const SkString& a, const SkString& b) {
                    return strcmp(a.c_str(), b.c_str()) < 0;
                });
            }
        }

        sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
        std::vector<sk_sp<SkTypeface>> typefaces;
        bool fontsFound = false;
        for (auto&& path : paths) {
            if ((false)) {
                SkDebugf("Reading font: %s\n", path.c_str());
            }
            auto stream = SkStream::MakeFromFile(path.c_str());
            SkASSERTF(stream, "%s not readable", path.c_str());
            sk_sp<SkTypeface> typeface = mgr->makeFromStream(std::move(stream), {});
            // Without --nativeFonts, DM will use the portable test font manager which does
            // not know how to read in fonts from bytes.
            if (typeface) {
                if ((false)) {
                    SkString familyName;
                    typeface->getFamilyName(&familyName);
                    SkDebugf("Creating: %s size: %zu\n",
                             familyName.c_str(),
                             typeface->openExistingStream(nullptr)->getLength());
                }
                if (path.endsWith("Roboto-Italic.ttf")) {
                    fontsFound = true;
                }
                typefaces.emplace_back(std::move(typeface));
            } else {
                SkDEBUGF("%s was not turned into a Typeface. Did you set --nativeFonts?\n",
                         path.c_str());
            }
        }
        SkASSERTF_RELEASE(typefaces.size(),
                          "--paragraph_fonts set but no fonts found."
                          "Did you set --nativeFonts?");
        SkASSERTF_RELEASE(fontsFound, "--paragraph_fonts set but Roboto-Italic.ttf not found");
        return typefaces;
    }();
    return typefaces;
}

ResourceFontCollection::ResourceFontCollection(bool testOnly)
        : fFontsFound(false), fResolvedFonts(0), fFontProvider(sk_make_sp<TypefaceFontProvider>()) {
    const std::vector<sk_sp<SkTypeface>>& typefaces = getTypefaces();
    fFontsFound = !typefaces.empty();
    for (auto&& typeface : typefaces) {
        fFontProvider->registerTypeface(typeface);
    }

    if (testOnly) {
        this->setTestFontManager(std::move(fFontProvider));
    } else {
        this->setAssetFontManager(std::move(fFontProvider));
    }
    this->disableFontFallback();
}

TestCanvas::TestCanvas(const char* testName) : name(testName) {
    bits.allocN32Pixels(TestCanvasWidth, TestCanvasHeight);
    canvas = new SkCanvas(bits);
    canvas->clear(SK_ColorWHITE);
}

TestCanvas::~TestCanvas() {
    SkString tmpDir = skiatest::GetTmpDir();
    if (!tmpDir.isEmpty()) {
        SkString path = SkOSPath::Join(tmpDir.c_str(), name);
        SkFILEWStream file(path.c_str());
        if (!SkPngEncoder::Encode(&file, bits.pixmap(), {})) {
            SkDebugf("Cannot write a picture %s\n", name);
        }
    }
    delete canvas;
}

void TestCanvas::drawRects(SkColor color, std::vector<TextBox>& result, bool fill) {

    SkPaint paint;
    if (!fill) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
    }
    paint.setColor(color);
    for (auto& r : result) {
        canvas->drawRect(r.rect, paint);
    }
}

void TestCanvas::drawLine(SkColor color, SkRect rect, bool vertical) {

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);
    paint.setColor(color);
    if (vertical) {
        canvas->drawLine(rect.fLeft, rect.fTop, rect.fLeft, rect.fBottom, paint);
    } else {
        canvas->drawLine(rect.fLeft, rect.fTop, rect.fRight, rect.fTop, paint);
    }
}

void TestCanvas::drawLines(SkColor color, std::vector<TextBox>& result) {

    for (auto& r : result) {
        drawLine(color, r.rect);
    }
}

}  // namespace skia::textlayout::test

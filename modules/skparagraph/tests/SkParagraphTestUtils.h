/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkParagraphTestUtils_DEFINED
#define SkParagraphTestUtils_DEFINED

// SkParagraph tests live in topic files next to this header (SkParagraph<Topic>Test.cpp), so
// parallel CLs no longer all append to one file. Add a test to the matching file or start a new
// one: list it in tests/BUILD.bazel, run make -C bazel generate_gni, and keep the SkParagraph_
// test name prefix so dm --match SkParagraph runs it

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/FontToolUtils.h"

#include <string>
#include <vector>

class SkCanvas;

// What DECLARE_string and DECLARE_bool expand to; the macros end without ';', and then
// clang-format joins them into one line and indents the namespace below even with
// "clang-format off" around them
extern CommandLineFlags::StringArray FLAGS_paragraph_fonts;
extern bool FLAGS_run_paragraph_tests_needing_system_fonts;

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

// Skip tests which do not find the fonts, unless the user set --paragraph_fonts in which case
// we should make a loud error.
#define SKIP_IF_FONTS_NOT_FOUND(r, fontCollection)            \
    if (!fontCollection->fontsFound()) {                      \
        if (FLAGS_paragraph_fonts.size() != 0) {              \
            ERRORF(r, "SkParagraphTests Fonts not found!\n"); \
        }                                                     \
        return;                                               \
    }

#define NEED_SYSTEM_FONTS(fontCollection)                            \
    if (!FLAGS_run_paragraph_tests_needing_system_fonts) {           \
        return;                                                      \
    }                                                                \
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr()); \
    fontCollection->enableFontFallback();

// A named namespace instead of an anonymous one: helpers are defined once in
// SkParagraphTestUtils.cpp, and get_unicode() must not clash with the global one in gm/
namespace skia::textlayout::test {

inline constexpr SkScalar EPSILON1000 = 0.001f;
inline constexpr SkScalar EPSILON100 = 0.01f;
inline constexpr SkScalar EPSILON50 = 0.02f;
inline constexpr SkScalar EPSILON20 = 0.05f;
inline constexpr SkScalar EPSILON10 = 0.1f;
inline constexpr SkScalar EPSILON5 = 0.20f;
inline constexpr SkScalar EPSILON2 = 0.50f;

// True if b starts with the UTF-8 bytes of base in range a
bool equal(const char* base, TextRange a, const char* b);

// ASCII text reversed after RLO (U+202E), as UTF-16
std::u16string mirror(const std::string& text);

// ASCII text after LRO (U+202D), as UTF-16
std::u16string straight(const std::string& text);

// SkUnicode from SkShapers::BestAvailable(), the build's default backend
sk_sp<SkUnicode> get_unicode();

// Loads the fonts from --paragraph_fonts once per process; disables font fallback
class ResourceFontCollection : public FontCollection {
public:
    ResourceFontCollection(bool testOnly = false);

    size_t resolvedFonts() const { return fResolvedFonts; }

    // TODO: temp solution until we check in fonts
    bool fontsFound() const { return fFontsFound; }

private:
    bool fFontsFound;
    size_t fResolvedFonts;
    sk_sp<TypefaceFontProvider> fFontProvider;
};

// Raster canvas that writes testName as a PNG into skiatest::GetTmpDir() when destroyed
class TestCanvas {
public:
    TestCanvas(const char* testName);
    ~TestCanvas();

    void drawRects(SkColor color, std::vector<TextBox>& result, bool fill = false);
    void drawLine(SkColor color, SkRect rect, bool vertical = true);
    // Draws the left edge of each box as a vertical line
    void drawLines(SkColor color, std::vector<TextBox>& result);

    SkCanvas* get() { return canvas; }

private:
    SkBitmap bits;
    SkCanvas* canvas;
    const char* name;
};

}  // namespace skia::textlayout::test

#endif  // SkParagraphTestUtils_DEFINED

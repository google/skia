/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUtils.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/sfnt/SkOTUtils.h"
#include "tools/fonts/TestTypeface.h"

#include <utility>

namespace {

#include "tools/fonts/test_font_monospace.inc"
#include "tools/fonts/test_font_sans_serif.inc"
#include "tools/fonts/test_font_serif.inc"

#include "tools/fonts/test_font_index.inc"

}  // namespace

class SkDescriptor;

const TestTypeface::List& TestTypeface::Typefaces() {
    static List list = []() -> List {
        TestTypeface::List list;
        for (const auto& sub : gSubFonts) {
            List::Family* existingFamily = nullptr;
            for (auto& family : list.families) {
                if (strcmp(family.name, sub.fFamilyName) == 0) {
                    existingFamily = &family;
                    break;
                }
            }
            if (!existingFamily) {
                existingFamily = &list.families.emplace_back();
                existingFamily->name = sub.fFamilyName;
            }

            auto font = sk_make_sp<SkTestFont>(sub.fFont);
            sk_sp<SkTypeface> typeface(new TestTypeface(std::move(font), sub.fStyle));
            bool isDefault = (&sub - gSubFonts == gDefaultFontIndex);
            existingFamily->faces.emplace_back(
                List::Family::Face{std::move(typeface), sub.fStyleName, isDefault});
        }
        return list;
    }();
    return list;
}

SkTestFont::SkTestFont(const SkTestFontData& fontData)
        : INHERITED()
        , fCharCodes(fontData.fCharCodes)
        , fCharCodesCount(fontData.fCharCodes ? fontData.fCharCodesCount : 0)
        , fWidths(fontData.fWidths)
        , fMetrics(fontData.fMetrics)
        , fName(fontData.fName)
        , fPaths(nullptr) {
    init(fontData.fPoints, fontData.fVerbs);
}

SkTestFont::~SkTestFont() {
    delete[] fPaths;
}

SkGlyphID SkTestFont::glyphForUnichar(SkUnichar charCode) const {
    for (size_t index = 0; index < fCharCodesCount; ++index) {
        if (fCharCodes[index] == charCode) {
            return SkTo<SkGlyphID>(index);
        }
    }
    return 0;
}

void SkTestFont::init(const SkScalar* pts, const unsigned char* verbs) {
    fPaths = new SkPath[fCharCodesCount];
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        SkPathBuilder b;
        SkPath::Verb verb;
        while ((verb = (SkPath::Verb)*verbs++) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    b.moveTo(pts[0], pts[1]);
                    pts += 2;
                    break;
                case SkPath::kLine_Verb:
                    b.lineTo(pts[0], pts[1]);
                    pts += 2;
                    break;
                case SkPath::kQuad_Verb:
                    b.quadTo(pts[0], pts[1], pts[2], pts[3]);
                    pts += 4;
                    break;
                case SkPath::kCubic_Verb:
                    b.cubicTo(pts[0], pts[1], pts[2], pts[3], pts[4], pts[5]);
                    pts += 6;
                    break;
                case SkPath::kClose_Verb:
                    b.close();
                    break;
                default:
                    SK_ABORT("bad verb");
            }
        }
        fPaths[index] = b.detach();
    }
}

TestTypeface::TestTypeface(sk_sp<SkTestFont> testFont, const SkFontStyle& style)
        : SkTypeface(style, false), fTestFont(std::move(testFont)) {}

SkVector TestTypeface::getAdvance(SkGlyphID glyphID) const {
    glyphID = glyphID < fTestFont->fCharCodesCount ? glyphID : 0;

    // TODO(benjaminwagner): Update users to use floats.
    return {SkFixedToFloat(fTestFont->fWidths[glyphID]), 0};
}

void TestTypeface::getFontMetrics(SkFontMetrics* metrics) { *metrics = fTestFont->fMetrics; }

SkPath TestTypeface::getPath(SkGlyphID glyphID) {
    glyphID = glyphID < fTestFont->fCharCodesCount ? glyphID : 0;
    return fTestFont->fPaths[glyphID];
}

void TestTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

void TestTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    unsigned glyphCount = fTestFont->fCharCodesCount;
    for (unsigned gid = 0; gid < glyphCount; ++gid) {
        glyphToUnicode[gid] = SkTo<SkUnichar>(fTestFont->fCharCodes[gid]);
    }
}

std::unique_ptr<SkAdvancedTypefaceMetrics> TestTypeface::onGetAdvancedMetrics() const {  // pdf only
    std::unique_ptr<SkAdvancedTypefaceMetrics>info(new SkAdvancedTypefaceMetrics);
    info->fPostScriptName.set(fTestFont->fName);
    return info;
}

static constexpr const char gHeaderString[] = "SkTestTypeface01";
static constexpr const size_t kHeaderSize = sizeof(gHeaderString);

std::unique_ptr<SkStreamAsset> TestTypeface::onOpenStream(int* ttcIndex) const {
    SkDynamicMemoryWStream wstream;
    wstream.write(gHeaderString, kHeaderSize);

    SkString name;
    this->getFamilyName(&name);
    SkFontStyle style = this->fontStyle();

    wstream.writePackedUInt(name.size());
    wstream.write(name.c_str(), name.size());
    wstream.writeScalar(style.weight());
    wstream.writeScalar(style.width());
    wstream.writePackedUInt(style.slant());

    *ttcIndex = 0;
    return wstream.detachAsStream();
}

sk_sp<SkTypeface> TestTypeface::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                               const SkFontArguments&) {
    char header[kHeaderSize];
    if (stream->read(header, kHeaderSize) != kHeaderSize ||
        0 != memcmp(header, gHeaderString, kHeaderSize))
    {
        return nullptr;
    }

    size_t familyNameSize;
    SkString familyName;
    if (!stream->readPackedUInt(&familyNameSize)) { return nullptr; }
    familyName.resize(familyNameSize);
    if (!stream->read(familyName.data(), familyNameSize)) { return nullptr; }

    SkScalar weight;
    SkScalar width;
    size_t slant;
    if (!stream->readScalar(&weight)) { return nullptr; }
    if (!stream->readScalar(&width)) { return nullptr; }
    if (!stream->readPackedUInt(&slant)) { return nullptr; }
    SkFontStyle style(weight, width, (SkFontStyle::Slant)slant);

    auto&& list = TestTypeface::Typefaces();
    for (auto&& family : list.families) {
        if (familyName.equals(family.name)) {
            for (auto&& face : family.faces) {
                if (face.typeface->fontStyle() == style) {
                    return face.typeface;
                }
            }
        }
    }
    return nullptr;
}

void TestTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    desc->setFamilyName(fTestFont->fName);
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(FactoryId);
    *serialize = true;
}

TestTypeface::Register::Register() {
    SkTypeface::Register(TestTypeface::FactoryId, &TestTypeface::MakeFromStream);
}
static TestTypeface::Register registerer;

void TestTypeface::onCharsToGlyphs(const SkUnichar* uni, int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; ++i) {
        glyphs[i] = fTestFont->glyphForUnichar(uni[i]);
    }
}

void TestTypeface::onGetFamilyName(SkString* familyName) const { *familyName = fTestFont->fName; }

bool TestTypeface::onGetPostScriptName(SkString*) const { return false; }

SkTypeface::LocalizedStrings* TestTypeface::onCreateFamilyNameIterator() const {
    SkString familyName(fTestFont->fName);
    SkString language("und");  // undetermined
    return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
}

class SkTestScalerContext : public SkScalerContext {
public:
    SkTestScalerContext(sk_sp<TestTypeface>           face,
                        const SkScalerContextEffects& effects,
                        const SkDescriptor*           desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

protected:
    TestTypeface* getTestTypeface() const {
        return static_cast<TestTypeface*>(this->getTypeface());
    }

    GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
        GlyphMetrics mx(glyph.maskFormat());

        auto advance = this->getTestTypeface()->getAdvance(glyph.getGlyphID());

        mx.advance = fMatrix.mapXY(advance.fX, advance.fY);
        return mx;

        // Always generates from paths, so SkScalerContext::makeGlyph will figure the bounds.
    }

    void generateImage(const SkGlyph&, void*) override {
        SK_ABORT("Should have generated from path.");
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        *path = this->getTestTypeface()->getPath(glyph.getGlyphID()).makeTransform(fMatrix);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        this->getTestTypeface()->getFontMetrics(metrics);
        SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> TestTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkTestScalerContext>(
            sk_ref_sp(const_cast<TestTypeface*>(this)), effects, desc);
}

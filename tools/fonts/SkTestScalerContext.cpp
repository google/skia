/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDescriptor.h"
#include "SkFontDescriptor.h"
#include "SkGlyph.h"
#include "SkMakeUnique.h"
#include "SkMask.h"
#include "SkOTUtils.h"
#include "SkPaintPriv.h"
#include "SkScalerContext.h"
#include "SkSVGDOM.h"
#include "SkTestScalerContext.h"
#include "SkTypefaceCache.h"
#include "SkUtils.h"

SkTestFont::SkTestFont(const SkTestFontData& fontData)
    : INHERITED()
    , fCharCodes(fontData.fCharCodes)
    , fCharCodesCount(fontData.fCharCodes ? fontData.fCharCodesCount : 0)
    , fWidths(fontData.fWidths)
    , fMetrics(fontData.fMetrics)
    , fName(fontData.fName)
    , fPaths(nullptr)
{
    init(fontData.fPoints, fontData.fVerbs);
}

SkTestFont::~SkTestFont() {
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        delete fPaths[index];
    }
    delete[] fPaths;
}

int SkTestFont::codeToIndex(SkUnichar charCode) const {
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        if (fCharCodes[index] == (unsigned) charCode) {
            return (int) index;
        }
    }
    return 0;
}

void SkTestFont::init(const SkScalar* pts, const unsigned char* verbs) {
    fPaths = new SkPath* [fCharCodesCount];
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        SkPath* path = new SkPath;
        SkPath::Verb verb;
        while ((verb = (SkPath::Verb) *verbs++) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    path->moveTo(pts[0], pts[1]);
                    pts += 2;
                    break;
                case SkPath::kLine_Verb:
                    path->lineTo(pts[0], pts[1]);
                    pts += 2;
                    break;
                case SkPath::kQuad_Verb:
                    path->quadTo(pts[0], pts[1], pts[2], pts[3]);
                    pts += 4;
                    break;
                case SkPath::kCubic_Verb:
                    path->cubicTo(pts[0], pts[1], pts[2], pts[3], pts[4], pts[5]);
                    pts += 6;
                    break;
                case SkPath::kClose_Verb:
                    path->close();
                    break;
                default:
                    SkDEBUGFAIL("bad verb");
                    return;
            }
        }
        // This should make SkPath::getBounds() queries threadsafe.
        path->updateBoundsCache();
        fPaths[index] = path;
    }
}

SkTestTypeface::SkTestTypeface(sk_sp<SkTestFont> testFont, const SkFontStyle& style)
    : SkTypeface(style, false)
    , fTestFont(std::move(testFont)) {
}

void SkTestTypeface::getAdvance(SkGlyph* glyph) {
    // TODO(benjaminwagner): Update users to use floats.
    glyph->fAdvanceX = SkFixedToFloat(fTestFont->fWidths[glyph->getGlyphID()]);
    glyph->fAdvanceY = 0;
}

void SkTestTypeface::getFontMetrics(SkPaint::FontMetrics* metrics) {
    *metrics = fTestFont->fMetrics;
}

void SkTestTypeface::getMetrics(SkGlyph* glyph) {
    SkGlyphID glyphID = glyph->getGlyphID();
    glyphID = glyphID < fTestFont->fCharCodesCount ? glyphID : 0;

    // TODO(benjaminwagner): Update users to use floats.
    glyph->fAdvanceX = SkFixedToFloat(fTestFont->fWidths[glyphID]);
    glyph->fAdvanceY = 0;
}

void SkTestTypeface::getPath(SkGlyphID glyphID, SkPath* path) {
    glyphID = glyphID < fTestFont->fCharCodesCount ? glyphID : 0;
    *path = *fTestFont->fPaths[glyphID];
}

void SkTestTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkPaint::kNo_Hinting);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTestTypeface::onGetAdvancedMetrics() const { // pdf only
    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);
    info->fFontName.set(fTestFont->fName);
    int glyphCount = this->onCountGlyphs();

    SkTDArray<SkUnichar>& toUnicode = info->fGlyphToUnicode;
    toUnicode.setCount(glyphCount);
    SkASSERT(glyphCount == SkToInt(fTestFont->fCharCodesCount));
    for (int gid = 0; gid < glyphCount; ++gid) {
        toUnicode[gid] = SkToS32(fTestFont->fCharCodes[gid]);
    }
    return info;
}

void SkTestTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    desc->setFamilyName(fTestFont->fName);
    desc->setStyle(this->fontStyle());
    *isLocal = false;
}

int SkTestTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                    uint16_t glyphs[], int glyphCount) const {
    auto utf8  = (const      char*)chars;
    auto utf16 = (const  uint16_t*)chars;
    auto utf32 = (const SkUnichar*)chars;

    for (int i = 0; i < glyphCount; i++) {
        SkUnichar ch;
        switch (encoding) {
            case kUTF8_Encoding:  ch =  SkUTF8_NextUnichar(&utf8 ); break;
            case kUTF16_Encoding: ch = SkUTF16_NextUnichar(&utf16); break;
            case kUTF32_Encoding: ch =                    *utf32++; break;
        }
        if (glyphs) {
            glyphs[i] = fTestFont->codeToIndex(ch);
        }
    }
    return glyphCount;
}

void SkTestTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = fTestFont->fName;
}

SkTypeface::LocalizedStrings* SkTestTypeface::onCreateFamilyNameIterator() const {
    SkString familyName(fTestFont->fName);
    SkString language("und"); //undetermined
    return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
}

class SkTestScalerContext : public SkScalerContext {
public:
    SkTestScalerContext(sk_sp<SkTestTypeface> face, const SkScalerContextEffects& effects,
                        const SkDescriptor* desc)
        : SkScalerContext(std::move(face), effects, desc)
    {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

protected:
    SkTestTypeface* getTestTypeface() const {
        return static_cast<SkTestTypeface*>(this->getTypeface());
    }

    unsigned generateGlyphCount() override {
        return this->getTestTypeface()->onCountGlyphs();
    }

    uint16_t generateCharToGlyph(SkUnichar uni) override {
        uint16_t glyph;
        (void) this->getTestTypeface()->onCharsToGlyphs((const void *) &uni,
                                                        SkTypeface::kUTF32_Encoding, &glyph, 1);
        return glyph;
    }

    void generateAdvance(SkGlyph* glyph) override {
        this->getTestTypeface()->getAdvance(glyph);

        const SkVector advance = fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                                               SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
    }

    void generateMetrics(SkGlyph* glyph) override {
        this->getTestTypeface()->getMetrics(glyph);

        const SkVector advance = fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                                               SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);

        SkPath path;
        this->getTestTypeface()->getPath(glyph->getGlyphID(), &path);
        path.transform(fMatrix);

        SkRect storage;
        const SkPaint paint;
        const SkRect& newBounds = paint.doComputeFastBounds(path.getBounds(),
                                                            &storage,
                                                            SkPaint::kFill_Style);
        SkIRect ibounds;
        newBounds.roundOut(&ibounds);
        glyph->fLeft = ibounds.fLeft;
        glyph->fTop = ibounds.fTop;
        glyph->fWidth = ibounds.width();
        glyph->fHeight = ibounds.height();
    }

    void generateImage(const SkGlyph& glyph) override {
        SkPath path;
        this->getTestTypeface()->getPath(glyph.getGlyphID(), &path);

        SkBitmap bm;
        bm.installPixels(SkImageInfo::MakeN32Premul(glyph.fWidth, glyph.fHeight),
                            glyph.fImage, glyph.rowBytes());
        bm.eraseColor(0);

        SkCanvas canvas(bm);
        canvas.translate(-SkIntToScalar(glyph.fLeft),
                            -SkIntToScalar(glyph.fTop));
        canvas.concat(fMatrix);
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas.drawPath(path, paint);
    }

    void generatePath(SkGlyphID glyph, SkPath* path) override {
        this->getTestTypeface()->getPath(glyph, path);
        path->transform(fMatrix);
    }

    void generateFontMetrics(SkPaint::FontMetrics* metrics) override {
        this->getTestTypeface()->getFontMetrics(metrics);
        SkPaintPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix         fMatrix;
};

SkScalerContext* SkTestTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return new SkTestScalerContext(sk_ref_sp(const_cast<SkTestTypeface*>(this)), effects, desc);
}

///////////////////////////////////////////////////////////////////////

#include "Resources.h"

SkSVGTestTypeface::SkSVGTestTypeface(const char* name,
                                     int upem,
                                     const SkPaint::FontMetrics& fontMetrics,
                                     const SkSVGTestTypefaceGlyphData* data, int dataCount,
                                     const SkFontStyle& style)
    : SkTypeface(style, false)
    , fName(name)
    , fUpem(upem)
    , fFontMetrics(fontMetrics)
    , fGlyphs(dataCount)
{
    for (int i = 0; i < dataCount; ++i) {
        const SkSVGTestTypefaceGlyphData& datum = data[i];
        std::unique_ptr<SkStreamAsset> stream = GetResourceAsStream(datum.fSvgResourcePath);
        fCMap.set(datum.fUnicode, i);
        if (!stream) {
            fGlyphs.emplace_back(nullptr, datum);
            continue;
        }
        sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromStream(*stream.get());
        if (!svg) {
            fGlyphs.emplace_back(nullptr, datum);
            continue;
        }

        const SkSize& sz = svg->containerSize();
        if (sz.isEmpty()) {
            fGlyphs.emplace_back(nullptr, datum);
            continue;
        }

        fGlyphs.emplace_back(std::move(svg), datum);
    }
}

SkSVGTestTypeface::~SkSVGTestTypeface() {}

SkSVGTestTypeface::Glyph::Glyph(sk_sp<SkSVGDOM> svg, const SkSVGTestTypefaceGlyphData& data)
    : fSvg(std::move(svg)), fOrigin(data.fOrigin), fAdvance(data.fAdvance) {}
SkSVGTestTypeface::Glyph::~Glyph() {}

void SkSVGTestTypeface::getAdvance(SkGlyph* glyph) const {
    glyph->fAdvanceX = fGlyphs[glyph->getGlyphID()].fAdvance;
    glyph->fAdvanceY = 0;
}

void SkSVGTestTypeface::getFontMetrics(SkPaint::FontMetrics* metrics) const {
    *metrics = fFontMetrics;
}

void SkSVGTestTypeface::getMetrics(SkGlyph* glyph) const {
    SkGlyphID glyphID = glyph->getGlyphID();
    glyphID = glyphID < fGlyphs.count() ? glyphID : 0;

    glyph->fAdvanceX = fGlyphs[glyphID].fAdvance;
    glyph->fAdvanceY = 0;
}

void SkSVGTestTypeface::getPath(SkGlyphID glyphID, SkPath* path) const {
    path->reset();
}

void SkSVGTestTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkPaint::kNo_Hinting);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkSVGTestTypeface::onGetAdvancedMetrics() const { // pdf only
    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);
    info->fFontName.set(fName);
    int glyphCount = this->onCountGlyphs();

    SkTDArray<SkUnichar>& toUnicode = info->fGlyphToUnicode;
    toUnicode.setCount(glyphCount);
    SkASSERT(glyphCount == SkToInt(fGlyphs.count()));
    fCMap.foreach([&toUnicode](const SkUnichar& c, const SkGlyphID& g) {
        toUnicode[g] = c;
    });
    return info;
}

void SkSVGTestTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    desc->setFamilyName(fName.c_str());
    desc->setStyle(this->fontStyle());
    *isLocal = false;
}

int SkSVGTestTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                    uint16_t glyphs[], int glyphCount) const {
    auto utf8  = (const      char*)chars;
    auto utf16 = (const  uint16_t*)chars;
    auto utf32 = (const SkUnichar*)chars;

    for (int i = 0; i < glyphCount; i++) {
        SkUnichar ch;
        switch (encoding) {
            case kUTF8_Encoding:  ch =  SkUTF8_NextUnichar(&utf8 ); break;
            case kUTF16_Encoding: ch = SkUTF16_NextUnichar(&utf16); break;
            case kUTF32_Encoding: ch =                    *utf32++; break;
        }
        if (glyphs) {
            SkGlyphID* g = fCMap.find(ch);
            glyphs[i] = g ? *g : 0;
        }
    }
    return glyphCount;
}

void SkSVGTestTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = fName;
}

SkTypeface::LocalizedStrings* SkSVGTestTypeface::onCreateFamilyNameIterator() const {
    SkString familyName(fName);
    SkString language("und"); //undetermined
    return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
}

class SkSVGTestScalerContext : public SkScalerContext {
public:
    SkSVGTestScalerContext(sk_sp<SkSVGTestTypeface> face, const SkScalerContextEffects& effects,
                           const SkDescriptor* desc)
        : SkScalerContext(std::move(face), effects, desc)
    {
        fRec.getSingleMatrix(&fMatrix);
        SkScalar upem = this->getSVGTestTypeface()->fUpem;
        fMatrix.preScale(1.f/upem, 1.f/upem);
    }

protected:
    SkSVGTestTypeface* getSVGTestTypeface() const {
        return static_cast<SkSVGTestTypeface*>(this->getTypeface());
    }

    unsigned generateGlyphCount() override {
        return this->getSVGTestTypeface()->onCountGlyphs();
    }

    uint16_t generateCharToGlyph(SkUnichar uni) override {
        uint16_t glyph;
        (void) this->getSVGTestTypeface()->onCharsToGlyphs((const void *) &uni,
                                                        SkTypeface::kUTF32_Encoding, &glyph, 1);
        return glyph;
    }

    void generateAdvance(SkGlyph* glyph) override {
        this->getSVGTestTypeface()->getAdvance(glyph);

        const SkVector advance = fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                                               SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
    }

    void generateMetrics(SkGlyph* glyph) override {
        this->getSVGTestTypeface()->getMetrics(glyph);

        const SkVector advance = fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                                               SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);

        SkSVGTestTypeface::Glyph& glyphData = this->getSVGTestTypeface()->fGlyphs[glyph->getGlyphID()];
        SkSVGDOM& svg = *glyphData.fSvg.get();
        SkSize containerSize = svg.containerSize();
        SkRect newBounds = SkRect::MakeXYWH(-glyphData.fOrigin.fX, -glyphData.fOrigin.fY, containerSize.fWidth, containerSize.fHeight);
        fMatrix.mapRect(&newBounds);

        SkIRect ibounds;
        newBounds.roundOut(&ibounds);
        glyph->fMaskFormat = SkMask::kARGB32_Format;
        glyph->fLeft = ibounds.fLeft;
        glyph->fTop = ibounds.fTop;
        glyph->fWidth = ibounds.width();
        glyph->fHeight = ibounds.height();
    }

    void generateImage(const SkGlyph& glyph) override {
        SkBitmap bm;
        bm.installPixels(SkImageInfo::MakeN32Premul(glyph.fWidth, glyph.fHeight),
                         glyph.fImage, glyph.rowBytes());
        bm.eraseColor(0);

        SkCanvas canvas(bm);
        canvas.concat(fMatrix);
        SkSVGDOM& svg = *this->getSVGTestTypeface()->fGlyphs[glyph.getGlyphID()].fSvg.get();
        svg.render(&canvas);
    }

    void generatePath(SkGlyphID glyph, SkPath* path) override {
        this->getSVGTestTypeface()->getPath(glyph, path);
        path->transform(fMatrix);
    }

    void generateFontMetrics(SkPaint::FontMetrics* metrics) override {
        this->getSVGTestTypeface()->getFontMetrics(metrics);
        SkPaintPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix         fMatrix;
};

SkScalerContext* SkSVGTestTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return new SkSVGTestScalerContext(sk_ref_sp(const_cast<SkSVGTestTypeface*>(this)), effects, desc);
}

constexpr const static SkSVGTestTypefaceGlyphData gGlyphs[] = {
    {"fonts/svg/smile.svg", {0,800}, 1000, 0x0}, // .notdef
    {"fonts/svg/smile.svg", {0,800}, 1000, 0x1F600},
};

sk_sp<SkSVGTestTypeface> SkSVGTestTypeface::Default() {
    SkPaint::FontMetrics metrics;
    metrics.fFlags = SkPaint::FontMetrics::kUnderlineThicknessIsValid_Flag |
                        SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag  |
                        SkPaint::FontMetrics::kStrikeoutThicknessIsValid_Flag |
                        SkPaint::FontMetrics::kStrikeoutPositionIsValid_Flag;
    metrics.fTop = -800;
    metrics.fAscent = -800;
    metrics.fDescent = 200;
    metrics.fBottom = 200;
    metrics.fLeading = 100;
    metrics.fAvgCharWidth = 1000;
    metrics.fMaxCharWidth = 1000;
    metrics.fXMin = 0;
    metrics.fXMax = 1000;
    metrics.fXHeight = 500;
    metrics.fCapHeight = 700;
    metrics.fUnderlineThickness = 40;
    metrics.fUnderlinePosition = 20;
    metrics.fStrikeoutThickness = 20;
    metrics.fStrikeoutPosition = -400;
    return sk_make_sp<SkSVGTestTypeface>("Emoji", 1000, metrics, gGlyphs, SK_ARRAY_COUNT(gGlyphs),
                                         SkFontStyle::Normal());
}

#include "SkSurface.h"

void SkSVGTestTypeface::exportTtxCommon(SkWStream* out) const {
    out->writeText("  <GlyphOrder>\n");
    //for (const Glyph& g : fGlyphs) {
    for (int i = 0; i < fGlyphs.count(); ++i) {
        out->writeText("    <GlyphID name=\"glyf");
        out->writeHexAsText(i, 4);
        out->writeText("\"/>\n");
    }
    out->writeText("  </GlyphOrder>\n");

    out->writeText("  <head>\n");
    out->writeText("    <tableVersion value=\"1.0\"/>\n");
    out->writeText("    <fontRevision value=\"1.0\"/>\n");
    out->writeText("    <checkSumAdjustment value=\"0xa9c3274\"/>\n");
    out->writeText("    <magicNumber value=\"0x5f0f3cf5\"/>\n");
    out->writeText("    <flags value=\"00000000 00011011\"/>\n");
    out->writeText("    <unitsPerEm value=\"");
        out->writeDecAsText(fUpem);
        out->writeText("\"/>\n");
    out->writeText("    <created value=\"Thu Feb 15 12:55:49 2018\"/>\n");
    out->writeText("    <modified value=\"Thu Feb 15 12:55:49 2018\"/>\n");
    // TODO: not recalculated for bitmap fonts?
    out->writeText("    <xMin value=\"");
        out->writeScalarAsText(fFontMetrics.fXMin);
        out->writeText("\"/>\n");
    out->writeText("    <yMin value=\"");
        out->writeScalarAsText(-fFontMetrics.fBottom);
        out->writeText("\"/>\n");
    out->writeText("    <xMax value=\"");
        out->writeScalarAsText(fFontMetrics.fXMax);
        out->writeText("\"/>\n");
    out->writeText("    <yMax value=\"");
        out->writeScalarAsText(-fFontMetrics.fTop);
        out->writeText("\"/>\n");

    char macStyle[16] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
    if (this->fontStyle().weight() >= SkFontStyle::Bold().weight()) {
        macStyle[0xF - 0x0] = '1'; // Bold
    }
    switch (this->fontStyle().slant()) {
        case SkFontStyle::kUpright_Slant:
            break;
        case SkFontStyle::kItalic_Slant:
            macStyle[0xF - 0x1] = '1'; // Italic
            break;
        case SkFontStyle::kOblique_Slant:
            macStyle[0xF - 0x1] = '1'; // Italic
            break;
        default:
            SK_ABORT("Unknown slant.");
    }
    if (this->fontStyle().width() <= SkFontStyle::kCondensed_Width) {
        macStyle[0xF - 0x5] = '1'; // Condensed
    } else if (this->fontStyle().width() >= SkFontStyle::kExpanded_Width) {
        macStyle[0xF - 0x6] = '1'; // Extended
    }
    out->writeText("    <macStyle value=\"");
        out->write(macStyle, 8);
        out->writeText(" ");
        out->write(macStyle + 8, 8);
        out->writeText("\"/>\n");
    out->writeText("    <lowestRecPPEM value=\"8\"/>\n");
    out->writeText("    <fontDirectionHint value=\"2\"/>\n");
    out->writeText("    <indexToLocFormat value=\"0\"/>\n");
    out->writeText("    <glyphDataFormat value=\"0\"/>\n");
    out->writeText("  </head>\n");

    out->writeText("  <hhea>\n");
    out->writeText("    <tableVersion value=\"0x00010000\"/>\n");
    out->writeText("    <ascent value=\"");
        out->writeDecAsText(-fFontMetrics.fAscent);
        out->writeText("\"/>\n");
    out->writeText("    <descent value=\"");
        out->writeDecAsText(-fFontMetrics.fDescent);
        out->writeText("\"/>\n");
    out->writeText("    <lineGap value=\"");
        out->writeDecAsText(fFontMetrics.fLeading);
        out->writeText("\"/>\n");
    out->writeText("    <advanceWidthMax value=\"0\"/>\n");
    out->writeText("    <minLeftSideBearing value=\"0\"/>\n");
    out->writeText("    <minRightSideBearing value=\"0\"/>\n");
    out->writeText("    <xMaxExtent value=\"");
        out->writeScalarAsText(fFontMetrics.fXMax - fFontMetrics.fXMin);
        out->writeText("\"/>\n");
    out->writeText("    <caretSlopeRise value=\"1\"/>\n");
    out->writeText("    <caretSlopeRun value=\"0\"/>\n");
    out->writeText("    <caretOffset value=\"0\"/>\n");
    out->writeText("    <reserved0 value=\"0\"/>\n");
    out->writeText("    <reserved1 value=\"0\"/>\n");
    out->writeText("    <reserved2 value=\"0\"/>\n");
    out->writeText("    <reserved3 value=\"0\"/>\n");
    out->writeText("    <metricDataFormat value=\"0\"/>\n");
    out->writeText("    <numberOfHMetrics value=\"0\"/>\n");
    out->writeText("  </hhea>\n");

    // This whole table is going to be re-calculated, but we have to write it out anyway.
    out->writeText("  <maxp>\n");
    out->writeText("    <tableVersion value=\"0x10000\"/>\n");
    out->writeText("    <numGlyphs value=\"2\"/>\n");
    out->writeText("    <maxPoints value=\"4\"/>\n");
    out->writeText("    <maxContours value=\"1\"/>\n");
    out->writeText("    <maxCompositePoints value=\"0\"/>\n");
    out->writeText("    <maxCompositeContours value=\"0\"/>\n");
    out->writeText("    <maxZones value=\"1\"/>\n");
    out->writeText("    <maxTwilightPoints value=\"0\"/>\n");
    out->writeText("    <maxStorage value=\"0\"/>\n");
    out->writeText("    <maxFunctionDefs value=\"10\"/>\n");
    out->writeText("    <maxInstructionDefs value=\"0\"/>\n");
    out->writeText("    <maxStackElements value=\"512\"/>\n");
    out->writeText("    <maxSizeOfInstructions value=\"24\"/>\n");
    out->writeText("    <maxComponentElements value=\"0\"/>\n");
    out->writeText("    <maxComponentDepth value=\"0\"/>\n");
    out->writeText("  </maxp>\n");

    out->writeText("  <OS_2>\n");
    out->writeText("    <version value=\"4\"/>\n");
    out->writeText("    <xAvgCharWidth value=\"");
        out->writeScalarAsText(fFontMetrics.fAvgCharWidth);
        out->writeText("\"/>\n");
    out->writeText("    <usWeightClass value=\"");
        out->writeDecAsText(this->fontStyle().weight());
        out->writeText("\"/>\n");
    out->writeText("    <usWidthClass value=\"");
        out->writeDecAsText(this->fontStyle().width());
        out->writeText("\"/>\n");
    out->writeText("    <fsType value=\"00000000 00000000\"/>\n");
    out->writeText("    <ySubscriptXSize value=\"665\"/>\n");
    out->writeText("    <ySubscriptYSize value=\"716\"/>\n");
    out->writeText("    <ySubscriptXOffset value=\"0\"/>\n");
    out->writeText("    <ySubscriptYOffset value=\"143\"/>\n");
    out->writeText("    <ySuperscriptXSize value=\"665\"/>\n");
    out->writeText("    <ySuperscriptYSize value=\"716\"/>\n");
    out->writeText("    <ySuperscriptXOffset value=\"0\"/>\n");
    out->writeText("    <ySuperscriptYOffset value=\"491\"/>\n");
    out->writeText("    <yStrikeoutSize value=\"");
        out->writeScalarAsText(fFontMetrics.fStrikeoutThickness);
        out->writeText("\"/>\n");
    out->writeText("    <yStrikeoutPosition value=\"");
        out->writeScalarAsText(-fFontMetrics.fStrikeoutPosition);
        out->writeText("\"/>\n");
    out->writeText("    <sFamilyClass value=\"0\"/>\n");
    out->writeText("    <panose>\n");
    out->writeText("      <bFamilyType value=\"0\"/>\n");
    out->writeText("      <bSerifStyle value=\"0\"/>\n");
    out->writeText("      <bWeight value=\"0\"/>\n");
    out->writeText("      <bProportion value=\"0\"/>\n");
    out->writeText("      <bContrast value=\"0\"/>\n");
    out->writeText("      <bStrokeVariation value=\"0\"/>\n");
    out->writeText("      <bArmStyle value=\"0\"/>\n");
    out->writeText("      <bLetterForm value=\"0\"/>\n");
    out->writeText("      <bMidline value=\"0\"/>\n");
    out->writeText("      <bXHeight value=\"0\"/>\n");
    out->writeText("    </panose>\n");
    out->writeText("    <ulUnicodeRange1 value=\"00000000 00000000 00000000 00000001\"/>\n");
    out->writeText("    <ulUnicodeRange2 value=\"00010000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <ulUnicodeRange3 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <ulUnicodeRange4 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <achVendID value=\"Skia\"/>\n");
    char fsSelection[16] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
    fsSelection[0xF - 0x7] = '1'; // Use typo metrics
    if (this->fontStyle().weight() >= SkFontStyle::Bold().weight()) {
        fsSelection[0xF - 0x5] = '1'; // Bold
    }
    switch (this->fontStyle().slant()) {
        case SkFontStyle::kUpright_Slant:
            if (this->fontStyle().weight() < SkFontStyle::Bold().weight()) {
                fsSelection[0xF - 0x6] = '1'; // Not bold or italic, is regular
            }
            break;
        case SkFontStyle::kItalic_Slant:
            fsSelection[0xF - 0x0] = '1'; // Italic
            break;
        case SkFontStyle::kOblique_Slant:
            fsSelection[0xF - 0x0] = '1'; // Italic
            fsSelection[0xF - 0x9] = '1'; // Oblique
            break;
        default:
            SK_ABORT("Unknown slant.");
    }
    out->writeText("    <fsSelection value=\"");
        out->write(fsSelection, 8);
        out->writeText(" ");
        out->write(fsSelection + 8, 8);
        out->writeText("\"/>\n");
    out->writeText("    <usFirstCharIndex value=\"0\"/>\n");
    out->writeText("    <usLastCharIndex value=\"0\"/>\n");
    out->writeText("    <sTypoAscender value=\"");
        out->writeScalarAsText(-fFontMetrics.fAscent);
        out->writeText("\"/>\n");
    out->writeText("    <sTypoDescender value=\"");
        out->writeScalarAsText(-fFontMetrics.fDescent);
        out->writeText("\"/>\n");
    out->writeText("    <sTypoLineGap value=\"");
        out->writeScalarAsText(fFontMetrics.fLeading);
        out->writeText("\"/>\n");
    out->writeText("    <usWinAscent value=\"");
        out->writeScalarAsText(-fFontMetrics.fAscent);
        out->writeText("\"/>\n");
    out->writeText("    <usWinDescent value=\"");
        out->writeScalarAsText(fFontMetrics.fDescent);
        out->writeText("\"/>\n");
    out->writeText("    <ulCodePageRange1 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <ulCodePageRange2 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <sxHeight value=\"");
        out->writeScalarAsText(fFontMetrics.fXHeight);
        out->writeText("\"/>\n");
    out->writeText("    <sCapHeight value=\"");
        out->writeScalarAsText(fFontMetrics.fCapHeight);
        out->writeText("\"/>\n");
    out->writeText("    <usDefaultChar value=\"0\"/>\n");
    out->writeText("    <usBreakChar value=\"32\"/>\n");
    out->writeText("    <usMaxContext value=\"0\"/>\n");
    out->writeText("  </OS_2>\n");

    out->writeText("  <hmtx>\n");
    for (int i = 0; i < fGlyphs.count(); ++i) {
        out->writeText("    <mtx name=\"glyf");
        out->writeHexAsText(i, 4);
        out->writeText("\" width=\"");
        out->writeDecAsText(fGlyphs[i].fAdvance);
        out->writeText("\" lsb=\"");
        out->writeDecAsText(fGlyphs[i].fOrigin.fX);
        out->writeText("\"/>\n");
    }
    out->writeText("  </hmtx>\n");

    out->writeText("  <cmap>\n");
    out->writeText("    <tableVersion version=\"0\"/>\n");
    out->writeText("    <cmap_format_12 platformID=\"3\" platEncID=\"1\" format=\"12\" reserved=\"0\" length=\"1\" language=\"0\" nGroups=\"0\">\n");
    fCMap.foreach([&out](const SkUnichar& c, const SkGlyphID& g) {
        out->writeText("      <map code=\"0x");
        out->writeHexAsText(c, 4);
        out->writeText("\" name=\"glyf");
        out->writeHexAsText(g, 4);
        out->writeText("\"/>\n");
    });
    out->writeText("    </cmap_format_12>\n");
    out->writeText("  </cmap>\n");

    out->writeText("  <name>\n");
    out->writeText("    <namerecord nameID=\"1\" platformID=\"3\" platEncID=\"1\" langID=\"0x409\">\n");
    out->writeText("      ");
      out->writeText(fName.c_str());
      out->writeText("CBDT\n");
    out->writeText("    </namerecord>\n");
    out->writeText("    <namerecord nameID=\"2\" platformID=\"3\" platEncID=\"1\" langID=\"0x409\">\n");
    out->writeText("      Regular\n");
    out->writeText("    </namerecord>\n");
    out->writeText("  </name>\n");

    out->writeText("  <post>\n");
    out->writeText("    <formatType value=\"3.0\"/>\n");
    out->writeText("    <italicAngle value=\"0.0\"/>\n");
    out->writeText("    <underlinePosition value=\"");
        out->writeScalarAsText(fFontMetrics.fUnderlinePosition);
        out->writeText("\"/>\n");
    out->writeText("    <underlineThickness value=\"");
        out->writeScalarAsText(fFontMetrics.fUnderlineThickness);
        out->writeText("\"/>\n");
    out->writeText("    <isFixedPitch value=\"0\"/>\n");
    out->writeText("    <minMemType42 value=\"0\"/>\n");
    out->writeText("    <maxMemType42 value=\"0\"/>\n");
    out->writeText("    <minMemType1 value=\"0\"/>\n");
    out->writeText("    <maxMemType1 value=\"0\"/>\n");
    out->writeText("  </post>\n");
}

void SkSVGTestTypeface::exportTtxCbdt(SkWStream* out) const {
    out->writeText("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    out->writeText("<ttFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.19\">\n");

    this->exportTtxCommon(out);

    int strikeSizes[3] = { 16, 64, 128 };

    SkPaint paint;
    paint.setTypeface(sk_ref_sp(const_cast<SkSVGTestTypeface*>(this)));
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    out->writeText("  <CBDT>\n");
    out->writeText("    <header version=\"2.0\"/>\n");
    for (size_t strikeIndex = 0; strikeIndex < SK_ARRAY_COUNT(strikeSizes); ++strikeIndex) {
        paint.setTextSize(strikeSizes[strikeIndex]);
        out->writeText("    <strikedata index=\"");
            out->writeDecAsText(strikeIndex);
            out->writeText("\">\n");
        for (int i = 0; i < fGlyphs.count(); ++i) {
            out->writeText("      <cbdt_bitmap_format_17 name=\"glyf");
                out->writeHexAsText(i, 4);
                out->writeText("\">\n");

            SkGlyphID gid = i;
            SkScalar advance;
            SkRect bounds;
            paint.getTextWidths(&gid, sizeof(gid), &advance, &bounds);
            SkImageInfo image_info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());
            sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
            SkASSERT(surface);
            SkCanvas* canvas = surface->getCanvas();
            canvas->clear(0);
            SkPixmap pix;
            surface->peekPixels(&pix);
            canvas->drawText(&gid, sizeof(gid), -bounds.fLeft, -bounds.fTop, paint);
            canvas->flush();
            sk_sp<SkImage> image = surface->makeImageSnapshot();
            sk_sp<SkData> data = image->encodeToData(SkEncodedImageFormat::kPNG, 100);

            out->writeText("        <SmallGlyphMetrics>\n");
            out->writeText("          <height value=\"");
                out->writeDecAsText(image->height());
                out->writeText("\"/>\n");
            out->writeText("          <width value=\"");
                out->writeDecAsText(image->width());
                out->writeText("\"/>\n");
            out->writeText("          <BearingX value=\"");
                out->writeDecAsText(-bounds.fLeft);
                out->writeText("\"/>\n");
            out->writeText("          <BearingY value=\"");
                out->writeScalarAsText(-bounds.fTop);
                out->writeText("\"/>\n");
            out->writeText("          <Advance value=\"");
                out->writeScalarAsText(advance);
                out->writeText("\"/>\n");
            out->writeText("        </SmallGlyphMetrics>\n");
            out->writeText("        <rawimagedata>");
            uint8_t const * bytes = data->bytes();
            for (size_t i = 0; i < data->size(); ++i) {
                if ((i % 0x10) == 0x0) {
                    out->writeText("\n          ");
                } else if (((i - 1) % 0x4) == 0x3) {
                    out->writeText(" ");
                }
                out->writeHexAsText(bytes[i], 2);
            }
            out->writeText("\n");
            out->writeText("        </rawimagedata>\n");
            out->writeText("      </cbdt_bitmap_format_17>\n");
        }
        out->writeText("    </strikedata>\n");
    }
    out->writeText("  </CBDT>\n");

    SkPaint::FontMetrics fm;
    out->writeText("  <CBLC>\n");
    out->writeText("    <header version=\"2.0\"/>\n");
    for (size_t strikeIndex = 0; strikeIndex < SK_ARRAY_COUNT(strikeSizes); ++strikeIndex) {
        paint.setTextSize(strikeSizes[strikeIndex]);
        paint.getFontMetrics(&fm);
        out->writeText("    <strike index=\"");
            out->writeDecAsText(strikeIndex);
            out->writeText("\">\n");
        out->writeText("      <bitmapSizeTable>\n");
        out->writeText("        <sbitLineMetrics direction=\"hori\">\n");
        out->writeText("          <ascender value=\"");
            out->writeScalarAsText(-fm.fTop);
            out->writeText("\"/>\n");
        out->writeText("          <descender value=\"");
            out->writeScalarAsText(-fm.fBottom);
            out->writeText("\"/>\n");
        out->writeText("          <widthMax value=\"");
            out->writeScalarAsText(fm.fXMax - fm.fXMin);
            out->writeText("\"/>\n");
        out->writeText("          <caretSlopeNumerator value=\"0\"/>\n");
        out->writeText("          <caretSlopeDenominator value=\"0\"/>\n");
        out->writeText("          <caretOffset value=\"0\"/>\n");
        out->writeText("          <minOriginSB value=\"0\"/>\n");
        out->writeText("          <minAdvanceSB value=\"0\"/>\n");
        out->writeText("          <maxBeforeBL value=\"0\"/>\n");
        out->writeText("          <minAfterBL value=\"0\"/>\n");
        out->writeText("          <pad1 value=\"0\"/>\n");
        out->writeText("          <pad2 value=\"0\"/>\n");
        out->writeText("        </sbitLineMetrics>\n");
        out->writeText("        <sbitLineMetrics direction=\"vert\">\n");
        out->writeText("          <ascender value=\"");
            out->writeScalarAsText(-fm.fTop);
            out->writeText("\"/>\n");
        out->writeText("          <descender value=\"");
            out->writeScalarAsText(-fm.fBottom);
            out->writeText("\"/>\n");
        out->writeText("          <widthMax value=\"");
            out->writeScalarAsText(fm.fXMax - fm.fXMin);
            out->writeText("\"/>\n");
        out->writeText("          <caretSlopeNumerator value=\"0\"/>\n");
        out->writeText("          <caretSlopeDenominator value=\"0\"/>\n");
        out->writeText("          <caretOffset value=\"0\"/>\n");
        out->writeText("          <minOriginSB value=\"0\"/>\n");
        out->writeText("          <minAdvanceSB value=\"0\"/>\n");
        out->writeText("          <maxBeforeBL value=\"0\"/>\n");
        out->writeText("          <minAfterBL value=\"0\"/>\n");
        out->writeText("          <pad1 value=\"0\"/>\n");
        out->writeText("          <pad2 value=\"0\"/>\n");
        out->writeText("        </sbitLineMetrics>\n");
        out->writeText("        <colorRef value=\"0\"/>\n");
        out->writeText("        <startGlyphIndex value=\"1\"/>\n");
        out->writeText("        <endGlyphIndex value=\"1\"/>\n");
        out->writeText("        <ppemX value=\"");
            out->writeDecAsText(strikeSizes[strikeIndex]);
            out->writeText("\"/>\n");
        out->writeText("        <ppemY value=\"");
            out->writeDecAsText(strikeSizes[strikeIndex]);
            out->writeText("\"/>\n");
        out->writeText("        <bitDepth value=\"32\"/>\n");
        out->writeText("        <flags value=\"1\"/>\n");
        out->writeText("      </bitmapSizeTable>\n");
        out->writeText("      <eblc_index_sub_table_1 imageFormat=\"17\" firstGlyphIndex=\"1\" lastGlyphIndex=\"1\">\n");
        for (int i = 0; i < fGlyphs.count(); ++i) {
            out->writeText("        <glyphLoc name=\"glyf");
                out->writeHexAsText(i, 4);
                out->writeText("\"/>\n");
        }
        out->writeText("      </eblc_index_sub_table_1>\n");
        out->writeText("    </strike>\n");
    }
    out->writeText("  </CBLC>\n");

    out->writeText("</ttFont>\n");
}

void SkSVGTestTypeface::exportTtxSbix(SkWStream* out) const {
    out->writeText("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    out->writeText("<ttFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.19\">\n");

    this->exportTtxCommon(out);

    int strikeSizes[3] = { 16, 64, 128 };

    SkPaint paint;
    paint.setTypeface(sk_ref_sp(const_cast<SkSVGTestTypeface*>(this)));
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    out->writeText("  <sbix>\n");
    out->writeText("    <version value=\"1\"/>\n");
    out->writeText("    <flags value=\"00000000 00000001\"/>\n");
    for (size_t strikeIndex = 0; strikeIndex < SK_ARRAY_COUNT(strikeSizes); ++strikeIndex) {
        paint.setTextSize(strikeSizes[strikeIndex]);
        out->writeText("    <strike>\n");
        out->writeText("      <ppem value=\"");
            out->writeDecAsText(strikeSizes[strikeIndex]);
            out->writeText("\"/>\n");
        out->writeText("      <resolution value=\"72\"/>\n");
        for (int i = 0; i < fGlyphs.count(); ++i) {
            SkGlyphID gid = i;
            SkScalar advance;
            SkRect bounds;
            paint.getTextWidths(&gid, sizeof(gid), &advance, &bounds);
            SkImageInfo image_info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());
            sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
            SkASSERT(surface);
            SkCanvas* canvas = surface->getCanvas();
            canvas->clear(0);
            SkPixmap pix;
            surface->peekPixels(&pix);
            canvas->drawText(&gid, sizeof(gid), -bounds.fLeft, -bounds.fTop, paint);
            canvas->flush();
            sk_sp<SkImage> image = surface->makeImageSnapshot();
            sk_sp<SkData> data = image->encodeToData(SkEncodedImageFormat::kPNG, 100);

            out->writeText("      <glyph name=\"glyf");
                out->writeHexAsText(i, 4);
                out->writeText("\" graphicType=\"png \" originOffsetX=\"");
                out->writeDecAsText(-bounds.fLeft);
                out->writeText("\" originOffsetY=\"");
                out->writeScalarAsText(bounds.fBottom);
                out->writeText("\">\n");

            out->writeText("        <hexdata>");
            uint8_t const * bytes = data->bytes();
            for (size_t i = 0; i < data->size(); ++i) {
                if ((i % 0x10) == 0x0) {
                    out->writeText("\n          ");
                } else if (((i - 1) % 0x4) == 0x3) {
                    out->writeText(" ");
                }
                out->writeHexAsText(bytes[i], 2);
            }
            out->writeText("\n");
            out->writeText("        </hexdata>\n");
            out->writeText("      </glyph>\n");
        }
        out->writeText("    </strike>\n");
    }
    out->writeText("  </sbix>\n");
    out->writeText("</ttFont>\n");
}

#include "SkNoDrawCanvas.h"
class SkCOLRCanvas : public SkNoDrawCanvas {
public:
    SkCOLRCanvas(SkIRect realBounds, SkWStream* out)
        : SkNoDrawCanvas(realBounds.width(), realBounds.height())
        , fBounds(realBounds)
        , fOut(out)
        , fGlyphId(0)
    { }
    void onDrawRect(const SkRect&, const SkPaint&) override {}
    void onDrawOval(const SkRect&, const SkPaint&) override {}
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override {}
    void onDrawRRect(const SkRRect&, const SkPaint&) override {}
    void onDrawPath(const SkPath&, const SkPaint&) override {
        // The bounds will be re-calculated anyway.
        fOut->writeText("    <TTGlyph name=\"glyf");
            fOut->writeHexAsText(fGlyphId, 4);
            fOut->writeText("\" xMin=\"");
            fOut->writeDecAsText(ibounds.fLeft);
            fOut->writeText("\" yMin=\"");
            fOut->writeDecAsText(-ibounds.fBottom);
            fOut->writeText("\" xMax=\"");
            fOut->writeDecAsText(ibounds.fRight);
            fOut->writeText("\" yMax=\"");
            fOut->writeDecAsText(-ibounds.fTop);
            fOut->writeText("\"/>\n");


        fOut->writeText("    </TTGlyph>\n");
        ++fGlyphId;
    }
private:
    SkIRect const fBounds;
    SkWStream * const fOut;
    SkGlyphID fGlyphId;
};

void SkSVGTestTypeface::exportTtxColr(SkWStream* out) const {
    out->writeText("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    out->writeText("<ttFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.19\">\n");

    this->exportTtxCommon(out);

    out->writeText("  <glyf>\n");
    for (int i = 0; i < fGlyphs.count(); ++i) {

        SkGlyph glyph;
        glyph.initWithGlyphID(SkPackedGlyphID(i));
        this->getMetrics(&glyph);

        const SkSVGTestTypeface::Glyph& glyphData = this->fGlyphs[glyph.getGlyphID()];
        SkSVGDOM& svg = *glyphData.fSvg.get();
        SkSize containerSize = svg.containerSize();
        SkRect newBounds = SkRect::MakeXYWH(-glyphData.fOrigin.fX, -glyphData.fOrigin.fY, containerSize.fWidth, containerSize.fHeight);

        SkIRect ibounds;
        newBounds.roundOut(&ibounds);
        glyph.fLeft = ibounds.fLeft;
        glyph.fTop = ibounds.fTop;
        glyph.fWidth = ibounds.width();
        glyph.fHeight = ibounds.height();

        SkCOLRCanvas canvas(ibounds, out);
        svg.render(&canvas);
    }
    out->writeText("  </glyf>\n");

  <COLR>
    <version value="0"/>
    <ColorGlyph name="uni2139">
      <layer colorID="8" name="uni2139"/>

  <CPAL>
    <version value="0"/>
    <numPaletteEntries value="44"/>
    <palette index="0">
      <color index="0" value="#3F433FFF"/>

    out->writeText("</ttFont>\n");
}

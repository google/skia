/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTestScalerContext_DEFINED
#define SkTestScalerContext_DEFINED

#include "SkFixed.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"
#include "SkTypeface.h"

class SkTestFont;

struct SkTestFontData {
    const SkScalar* fPoints;
    const unsigned char* fVerbs;
    const unsigned* fCharCodes;
    const size_t fCharCodesCount;
    const SkFixed* fWidths;
    const SkPaint::FontMetrics& fMetrics;
    const char* fName;
    SkFontStyle fStyle;
    sk_sp<SkTestFont> fCachedFont;
};

class SkTestFont : public SkRefCnt {
public:
    SkTestFont(const SkTestFontData& );
    virtual ~SkTestFont();
    int codeToIndex(SkUnichar charCode) const;
    void init(const SkScalar* pts, const unsigned char* verbs);
private:
    const unsigned* fCharCodes;
    const size_t fCharCodesCount;
    const SkFixed* fWidths;
    const SkPaint::FontMetrics& fMetrics;
    const char* fName;
    SkPath** fPaths;
    friend class SkTestTypeface;
    typedef SkRefCnt INHERITED;
};


class SkTestTypeface : public SkTypeface {
public:
    SkTestTypeface(sk_sp<SkTestFont>, const SkFontStyle& style);
    void getAdvance(SkGlyph* glyph);
    void getFontMetrics(SkPaint::FontMetrics* metrics);
    void getMetrics(SkGlyph* glyph);
    void getPath(SkGlyphID glyph, SkPath* path);
protected:
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    SkStreamAsset* onOpenStream(int* ttcIndex) const override {
        return nullptr;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    int onCharsToGlyphs(const void* chars, Encoding encoding,
                        uint16_t glyphs[], int glyphCount) const override;

    int onCountGlyphs() const override {
        return (int) fTestFont->fCharCodesCount;
    }

    int onGetUPEM() const override {
        return 2048;
    }

    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override
    {
        return 0;
    }

    int onGetTableTags(SkFontTableTag tags[]) const override {
        return 0;
    }

    size_t onGetTableData(SkFontTableTag tag, size_t offset,
                          size_t length, void* data) const override {
        return 0;
    }
private:
    sk_sp<SkTestFont> fTestFont;
    friend class SkTestScalerContext;
};

#include "SkTHash.h"
class SkSVGDOM;

struct SkSVGTestTypefaceGlyphData {
    const char* fSvgResourcePath;
    SkPoint fOrigin;
    SkScalar fAdvance;
    SkUnichar fUnicode; //TODO: this limits to 1:1
};

class SkSVGTestTypeface : public SkTypeface {
public:
    SkSVGTestTypeface(const char* name,
                      int fUpem,
                      const SkPaint::FontMetrics& metrics,
                      const SkSVGTestTypefaceGlyphData* data, int dataCount,
                      const SkFontStyle& style);
    ~SkSVGTestTypeface() override;
    void getAdvance(SkGlyph* glyph) const;
    void getFontMetrics(SkPaint::FontMetrics* metrics) const;
    void getMetrics(SkGlyph* glyph) const;
    void getPath(SkGlyphID glyph, SkPath* path) const;

    static sk_sp<SkSVGTestTypeface> Default();
    void exportTtxCbdt(SkWStream*) const;
    void exportTtxSbix(SkWStream*) const;
    void exportTtxColr(SkWStream*) const;
protected:
    void exportTtxCommon(SkWStream*) const;

    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    SkStreamAsset* onOpenStream(int* ttcIndex) const override {
        return nullptr;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    int onCharsToGlyphs(const void* chars, Encoding encoding,
                        uint16_t glyphs[], int glyphCount) const override;

    int onCountGlyphs() const override {
        return fGlyphs.count();
    }

    int onGetUPEM() const override {
        return fUpem;
    }

    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override
    {
        return 0;
    }

    int onGetTableTags(SkFontTableTag tags[]) const override {
        return 0;
    }

    size_t onGetTableData(SkFontTableTag tag, size_t offset,
                          size_t length, void* data) const override {
        return 0;
    }
private:
    struct Glyph {
        Glyph(sk_sp<SkSVGDOM> svg, const SkSVGTestTypefaceGlyphData& data);
        ~Glyph();
        sk_sp<SkSVGDOM> fSvg;
        SkPoint fOrigin;
        SkScalar fAdvance;
    };
    SkString fName;
    int fUpem;
    const SkPaint::FontMetrics fFontMetrics;
    SkTArray<Glyph> fGlyphs;
    SkTHashMap<SkUnichar, SkGlyphID> fCMap;
    friend class SkSVGTestScalerContext;
};

#endif

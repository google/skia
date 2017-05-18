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
    SkTypeface::Style fStyle;
    sk_sp<SkTestFont> fCachedFont;
};

class SkTestFont : public SkRefCnt {
public:
    SkTestFont(const SkTestFontData& );
    virtual ~SkTestFont();
    int codeToIndex(SkUnichar charCode) const;
    void init(const SkScalar* pts, const unsigned char* verbs);
#ifdef SK_DEBUG  // detect missing test font data
    mutable unsigned char fDebugBits[16];
    mutable SkUnichar fDebugOverage[8];
    const char* fDebugName;
    SkFontStyle fDebugStyle;
    const char* debugFontName() const { return fName; }
#endif
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

SkTypeface* CreateTestTypeface(const char* name, SkTypeface::Style style);

#endif

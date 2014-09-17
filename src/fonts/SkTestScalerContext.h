/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTestScalerContext_DEFINED
#define SkTestScalerContext_DEFINED

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
    SkTestFont* fFontCache;
};

class SkTestFont : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkTestFont)

    SkTestFont(const SkTestFontData& );
    virtual ~SkTestFont();
    int codeToIndex(SkUnichar charCode) const;
    void init(const SkScalar* pts, const unsigned char* verbs);
#ifdef SK_DEBUG  // detect missing test font data
    mutable unsigned char fDebugBits[16];
    mutable SkUnichar fDebugOverage[8];
    const char* fDebugName;
    SkTypeface::Style fDebugStyle;
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
    SkTestTypeface(SkTestFont* , SkTypeface::Style style);
    virtual ~SkTestTypeface() {
        SkSafeUnref(fTestFont);
    }
    void getAdvance(SkGlyph* glyph);
    void getFontMetrics(SkPaint::FontMetrics* metrics);
    void getMetrics(SkGlyph* glyph);
    void getPath(const SkGlyph& glyph, SkPath* path);
protected:
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor* desc) const SK_OVERRIDE;
    virtual void onFilterRec(SkScalerContextRec* rec) const SK_OVERRIDE;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                    SkAdvancedTypefaceMetrics::PerGlyphInfo ,
                                    const uint32_t* glyphIDs,
                                    uint32_t glyphIDsCount) const SK_OVERRIDE;

    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        SkASSERT(0);  // don't expect to get here
        return NULL;
    }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const SK_OVERRIDE;

    virtual int onCharsToGlyphs(const void* chars, Encoding encoding,
                                uint16_t glyphs[], int glyphCount) const SK_OVERRIDE;

    virtual int onCountGlyphs() const SK_OVERRIDE {
        return (int) fTestFont->fCharCodesCount;
    }

    virtual int onGetUPEM() const SK_OVERRIDE {
        SkASSERT(0);  // don't expect to get here
        return 1;
    }

    virtual void onGetFamilyName(SkString* familyName) const SK_OVERRIDE;
    virtual SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const SK_OVERRIDE;

    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE {
        return 0;
    }

    virtual size_t onGetTableData(SkFontTableTag tag, size_t offset,
                                  size_t length, void* data) const SK_OVERRIDE {
        return 0;
    }
private:
    SkTestFont* fTestFont;
    friend class SkTestScalerContext;
};

SkTypeface* CreateTestTypeface(const char* name, SkTypeface::Style style);

#endif

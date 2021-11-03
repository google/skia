/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestTypeface_DEFINED
#define TestTypeface_DEFINED

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkFixed.h"

#include <memory>

class SkDescriptor;
class SkFontDescriptor;
class SkGlyph;
class SkPath;
class SkScalerContext;
class SkStreamAsset;
class SkString;
class SkTestFont;
struct SkAdvancedTypefaceMetrics;
struct SkScalerContextEffects;
struct SkScalerContextRec;

struct SkTestFontData {
    const SkScalar*      fPoints;
    const unsigned char* fVerbs;
    const SkUnichar*     fCharCodes;
    const size_t         fCharCodesCount;
    const SkFixed*       fWidths;
    const SkFontMetrics& fMetrics;
    const char*          fName;
    SkFontStyle          fStyle;
};

class SkTestFont : public SkRefCnt {
public:
    SkTestFont(const SkTestFontData&);
    ~SkTestFont() override;
    SkGlyphID glyphForUnichar(SkUnichar charCode) const;
    void      init(const SkScalar* pts, const unsigned char* verbs);

private:
    const SkUnichar*     fCharCodes;
    const size_t         fCharCodesCount;
    const SkFixed*       fWidths;
    const SkFontMetrics& fMetrics;
    const char*          fName;
    SkPath*              fPaths;
    friend class TestTypeface;
    using INHERITED = SkRefCnt;
};

class TestTypeface : public SkTypeface {
public:
    TestTypeface(sk_sp<SkTestFont>, const SkFontStyle& style);
    void getAdvance(SkGlyph* glyph);
    void getFontMetrics(SkFontMetrics* metrics);
    SkPath getPath(SkGlyphID glyph);

protected:
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&,
                                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;

    int onCountGlyphs() const override { return (int)fTestFont->fCharCodesCount; }

    void getPostScriptGlyphNames(SkString*) const override {}

    int onGetUPEM() const override { return 2048; }

    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString*) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    bool onGlyphMaskNeedsCurrentColor() const override { return false; }

    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        return 0;
    }

    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override {
        return 0;
    }

    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }

    size_t onGetTableData(SkFontTableTag tag,
                          size_t         offset,
                          size_t         length,
                          void*          data) const override {
        return 0;
    }

private:
    sk_sp<SkTestFont> fTestFont;
    friend class SkTestScalerContext;
};

#endif

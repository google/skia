/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTestSVGTypeface_DEFINED
#define SkTestSVGTypeface_DEFINED

#include "SkFontArguments.h"
#include "SkFontMetrics.h"
#include "SkMutex.h"
#include "SkPaint.h"
#include "SkPathOps.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkSpan.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkTypes.h"

#include <memory>

class SkDescriptor;
class SkFontDescriptor;
class SkFontStyle;
class SkGlyph;
class SkPath;
class SkScalerContext;
class SkSVGDOM;
class SkWStream;
struct SkAdvancedTypefaceMetrics;
struct SkScalerContextEffects;
struct SkScalerContextRec;

struct SkSVGTestTypefaceGlyphData {
    const char* fSvgResourcePath;
    SkPoint fOrigin; // y-down
    SkScalar fAdvance;
    SkUnichar fUnicode; //TODO: this limits to 1:1
};

class SkTestSVGTypeface : public SkTypeface {
public:
    SkTestSVGTypeface(const char* name,
                      int upem,
                      const SkFontMetrics& metrics,
                      SkSpan<const SkSVGTestTypefaceGlyphData> data,
                      const SkFontStyle& style);
    ~SkTestSVGTypeface() override;
    void getAdvance(SkGlyph* glyph) const;
    void getFontMetrics(SkFontMetrics* metrics) const;

    static sk_sp<SkTestSVGTypeface> Default();
    static sk_sp<SkTestSVGTypeface> Planets();
    void exportTtxCbdt(SkWStream*, SkSpan<unsigned> strikeSizes) const;
    void exportTtxSbix(SkWStream*, SkSpan<unsigned> strikeSizes) const;
    void exportTtxColr(SkWStream*) const;
    virtual bool getPathOp(SkColor, SkPathOp*) const = 0;

    struct GlyfLayerInfo {
        GlyfLayerInfo(int layerColorIndex, SkIRect bounds)
            : fLayerColorIndex(layerColorIndex)
            , fBounds(bounds) {}
        int fLayerColorIndex;
        SkIRect fBounds;
    };
    struct GlyfInfo {
        GlyfInfo() : fBounds(SkIRect::MakeEmpty()) {}
        SkIRect fBounds;
        SkTArray<GlyfLayerInfo> fLayers;
    };
protected:
    void exportTtxCommon(SkWStream*, const char* type, const SkTArray<GlyfInfo>* = nullptr) const;

    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    int onCharsToGlyphs(const void* chars, Encoding encoding,
                        uint16_t glyphs[], int glyphCount) const override;

    int onCountGlyphs() const override {
        return fGlyphCount;
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

    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override
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
        Glyph();
        ~Glyph();
        sk_sp<SkSVGDOM> fSvg;
        SkMutex fSvgMutex;
        SkPoint fOrigin;
        SkScalar fAdvance;
    };
    SkString fName;
    int fUpem;
    const SkFontMetrics fFontMetrics;
    std::unique_ptr<Glyph[]> fGlyphs;
    int fGlyphCount;
    SkTHashMap<SkUnichar, SkGlyphID> fCMap;
    friend class SkTestSVGScalerContext;
};

#endif

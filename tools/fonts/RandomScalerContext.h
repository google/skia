/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RandomScalerContext_DEFINED
#define RandomScalerContext_DEFINED

#include "SkScalerContext.h"
#include "SkTypeface.h"

/*
 * This scaler context is for debug only purposes.  It will 'randomly' but deterministically return
 * LCD / A8 / BW / RBGA masks based off of the Glyph ID
 */

class SkRandomTypeface : public SkTypeface {
public:
    SkRandomTypeface(sk_sp<SkTypeface> proxy, const SkPaint&, bool fakeit);

    SkTypeface*    proxy() const { return fProxy.get(); }
    const SkPaint& paint() const { return fPaint; }

protected:
    SkScalerContext*                           onCreateScalerContext(const SkScalerContextEffects&,
                                                                     const SkDescriptor*) const override;
    void                                       onFilterRec(SkScalerContextRec*) const override;
    void                                       getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    std::unique_ptr<SkStreamAsset>             onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    void              onGetFontDescriptor(SkFontDescriptor*, bool* isLocal) const override;

    int onCharsToGlyphs(const void* chars,
                        Encoding    encoding,
                        uint16_t    glyphs[],
                        int         glyphCount) const override;
    int onCountGlyphs() const override;
    int onGetUPEM() const override;

    void                          onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;

private:
    sk_sp<SkTypeface> fProxy;
    SkPaint           fPaint;
    bool              fFakeIt;
};

#endif

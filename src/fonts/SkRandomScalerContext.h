/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRandomScalerContext_DEFINED
#define SkRandomScalerContext_DEFINED

#include "SkScalerContext.h"
#include "SkTypeface.h"

/*
 * This scaler context is for debug only purposes.  It will 'randomly' but deterministically return
 * LCD / A8 / BW / RBGA masks based off of the Glyph ID
 */

class SkRandomTypeface : public SkTypeface {
public:
    SkRandomTypeface(SkTypeface* proxy, const SkPaint&, bool fakeit);
    virtual ~SkRandomTypeface();

    SkTypeface* proxy() const { return fProxy; }
    const SkPaint& paint() const { return fPaint; }

protected:
    SkScalerContext* onCreateScalerContext(const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
        PerGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const override;
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool* isLocal) const override;

    int onCharsToGlyphs(const void* chars, Encoding encoding,
                        uint16_t glyphs[], int glyphCount) const override;
    int onCountGlyphs() const override;
    int onGetUPEM() const override;

    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset,
                          size_t length, void* data) const override;

private:
    SkTypeface* fProxy;
    SkPaint     fPaint;
    bool        fFakeIt;
};

#endif

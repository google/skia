/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGScalerContext_DEFINED
#define SkGScalerContext_DEFINED

#include "SkScalerContext.h"
#include "SkTypeface.h"

class SkGTypeface : public SkTypeface {
public:
    SkGTypeface(SkTypeface* proxy, const SkPaint&);
    virtual ~SkGTypeface();

    SkTypeface* proxy() const { return fProxy; }
    const SkPaint& paint() const { return fPaint; }

protected:
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const SK_OVERRIDE;
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                    SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                    const uint32_t* glyphIDs,
                                    uint32_t glyphIDsCount) const SK_OVERRIDE;
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool* isLocal) const SK_OVERRIDE;

    virtual int onCountGlyphs() const SK_OVERRIDE;
    virtual int onGetUPEM() const SK_OVERRIDE;

    virtual SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const SK_OVERRIDE;

    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const SK_OVERRIDE;
    virtual SkTypeface* onRefMatchingStyle(Style) const SK_OVERRIDE;

private:
    SkTypeface* fProxy;
    SkPaint     fPaint;
};

#endif

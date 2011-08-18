
/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAdvancedTypefaceMetrics.h"
#include "SkTypeface.h"
#include "SkFontHost.h"

//#define TRACE_LIFECYCLE

#ifdef TRACE_LIFECYCLE
    static int32_t gTypefaceCounter;
#endif

SkTypeface::SkTypeface(Style style, SkFontID fontID, bool isFixedWidth)
    : fUniqueID(fontID), fStyle(style), fIsFixedWidth(isFixedWidth) {
#ifdef TRACE_LIFECYCLE
    SkDebugf("SkTypeface: create  %p fontID %d total %d\n",
             this, fontID, ++gTypefaceCounter);
#endif
}

SkTypeface::~SkTypeface() {
#ifdef TRACE_LIFECYCLE
    SkDebugf("SkTypeface: destroy %p fontID %d total %d\n",
             this, fUniqueID, --gTypefaceCounter);
#endif
}

///////////////////////////////////////////////////////////////////////////////

uint32_t SkTypeface::UniqueID(const SkTypeface* face) {
    if (face) {
        return face->uniqueID();
    }

    // We cache the default fontID, assuming it will not change during a boot
    // The initial value of 0 is fine, since a typeface's uniqueID should not
    // be zero.
    static uint32_t gDefaultFontID;

    if (0 == gDefaultFontID) {
        SkTypeface* defaultFace =
                SkFontHost::CreateTypeface(NULL, NULL, NULL, 0,
                                           SkTypeface::kNormal);
        SkASSERT(defaultFace);
        gDefaultFontID = defaultFace->uniqueID();
        defaultFace->unref();
    }
    return gDefaultFontID;
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb) {
    return SkTypeface::UniqueID(facea) == SkTypeface::UniqueID(faceb);
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkTypeface::CreateFromName(const char name[], Style style) {
    return SkFontHost::CreateTypeface(NULL, name, NULL, 0, style);
}

SkTypeface* SkTypeface::CreateForChars(const void* data, size_t bytelength,
                                       Style s) {
    return SkFontHost::CreateTypeface(NULL, NULL, data, bytelength, s);
}

SkTypeface* SkTypeface::CreateFromTypeface(const SkTypeface* family, Style s) {
    return SkFontHost::CreateTypeface(family, NULL, NULL, 0, s);
}

SkTypeface* SkTypeface::CreateFromStream(SkStream* stream) {
    return SkFontHost::CreateTypefaceFromStream(stream);
}

SkTypeface* SkTypeface::CreateFromFile(const char path[]) {
    return SkFontHost::CreateTypefaceFromFile(path);
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::serialize(SkWStream* stream) const {
    SkFontHost::Serialize(this, stream);
}

SkTypeface* SkTypeface::Deserialize(SkStream* stream) {
    return SkFontHost::Deserialize(stream);
}

SkAdvancedTypefaceMetrics* SkTypeface::getAdvancedTypefaceMetrics(
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const {
    return SkFontHost::GetAdvancedTypefaceMetrics(fUniqueID,
                                                  perGlyphInfo,
                                                  glyphIDs,
                                                  glyphIDsCount);
}

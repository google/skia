/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo) const {
    return SkFontHost::GetAdvancedTypefaceMetrics(fUniqueID, perGlyphInfo);
}


/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFontHost.h"

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                     const char famillyName[],
                                     const void* data, size_t bytelength,
                                     SkTypeface::Style style) {
    SkASSERT(!"SkFontHost::FindTypeface unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream*) {
    SkASSERT(!"SkFontHost::CreateTypeface unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(char const*) {
    SkASSERT(!"SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

// static
SkAdvancedTypefaceMetrics* SkFontHost::GetAdvancedTypefaceMetrics(
        uint32_t fontID,
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo) {
    SkASSERT(!"SkFontHost::GetAdvancedTypefaceMetrics unimplemented");
    return NULL;
}

void SkFontHost::FilterRec(SkScalerContext::Rec* rec) {
}

///////////////////////////////////////////////////////////////////////////////

bool SkFontHost::ValidFontID(uint32_t uniqueID) {
    SkASSERT(!"SkFontHost::ResolveTypeface unimplemented");
    return false;
}

SkStream* SkFontHost::OpenStream(uint32_t uniqueID) {
    SkASSERT(!"SkFontHost::OpenStream unimplemented");
    return NULL;
}

size_t SkFontHost::GetFileName(SkFontID fontID, char path[], size_t length,
                               int32_t* index) {
    SkDebugf("SkFontHost::GetFileName unimplemented\n");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkASSERT(!"SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkASSERT(!"SkFontHost::Deserialize unimplemented");
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc) {
    SkASSERT(!"SkFontHost::CreateScalarContext unimplemented");
    return NULL;
}

SkFontID SkFontHost::NextLogicalFont(SkFontID currFontID, SkFontID origFontID) {
    return 0;
}


///////////////////////////////////////////////////////////////////////////////

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar) {
    return 0;   // nothing to do (change me if you want to limit the font cache)
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint) {
    return 0;
}

void SkFontHost::GetGammaTables(const uint8_t* tables[2]) {
    tables[0] = NULL;   // black gamma (e.g. exp=1.4)
    tables[1] = NULL;   // white gamma (e.g. exp= 1/1.4)
}


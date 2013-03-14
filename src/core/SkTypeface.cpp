
/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAdvancedTypefaceMetrics.h"
#include "SkTypeface.h"
#include "SkFontHost.h"

SK_DEFINE_INST_COUNT(SkTypeface)

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

SkTypeface* SkTypeface::GetDefaultTypeface() {
    // we keep a reference to this guy for all time, since if we return its
    // fontID, the font cache may later on ask to resolve that back into a
    // typeface object.
    static SkTypeface* gDefaultTypeface;

    if (NULL == gDefaultTypeface) {
        gDefaultTypeface =
        SkFontHost::CreateTypeface(NULL, NULL, SkTypeface::kNormal);
    }
    return gDefaultTypeface;
}

SkTypeface* SkTypeface::RefDefault() {
    return SkRef(GetDefaultTypeface());
}

uint32_t SkTypeface::UniqueID(const SkTypeface* face) {
    if (NULL == face) {
        face = GetDefaultTypeface();
    }
    return face->uniqueID();
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb) {
    return SkTypeface::UniqueID(facea) == SkTypeface::UniqueID(faceb);
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkTypeface::CreateFromName(const char name[], Style style) {
    return SkFontHost::CreateTypeface(NULL, name, style);
}

SkTypeface* SkTypeface::CreateFromTypeface(const SkTypeface* family, Style s) {
    if (family && family->style() == s) {
        family->ref();
        return const_cast<SkTypeface*>(family);
    }
    return SkFontHost::CreateTypeface(family, NULL, s);
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

///////////////////////////////////////////////////////////////////////////////

int SkTypeface::countTables() const {
    return SkFontHost::CountTables(fUniqueID);
}

int SkTypeface::getTableTags(SkFontTableTag tags[]) const {
    return SkFontHost::GetTableTags(fUniqueID, tags);
}

size_t SkTypeface::getTableSize(SkFontTableTag tag) const {
    return SkFontHost::GetTableSize(fUniqueID, tag);
}

size_t SkTypeface::getTableData(SkFontTableTag tag, size_t offset, size_t length,
                                void* data) const {
    return SkFontHost::GetTableData(fUniqueID, tag, offset, length, data);
}

SkStream* SkTypeface::openStream(int* ttcIndex) const {
    if (ttcIndex) {
        int32_t ndx = 0;
        (void)SkFontHost::GetFileName(fUniqueID, NULL, 0, &ndx);
        *ttcIndex = (int)ndx;
    }
    return SkFontHost::OpenStream(fUniqueID);
}

int SkTypeface::getUnitsPerEm() const {
    int upem = 0;

#ifdef SK_BUILD_FOR_ANDROID
    upem = SkFontHost::GetUnitsPerEm(fUniqueID);
#else
    SkAdvancedTypefaceMetrics* metrics;
    metrics = SkFontHost::GetAdvancedTypefaceMetrics(fUniqueID,
                                 SkAdvancedTypefaceMetrics::kNo_PerGlyphInfo,
                                 NULL, 0);
    if (metrics) {
        upem = metrics->fEmSize;
        metrics->unref();
    }
#endif
    return upem;
}

// TODO: move this impl into the subclass
SkScalerContext* SkTypeface::onCreateScalerContext(const SkDescriptor* desc) const {
    return SkFontHost::CreateScalerContext(desc);
}

// TODO: move this impl into the subclass
void SkTypeface::onFilterRec(SkScalerContextRec* rec) const {
    SkFontHost::FilterRec(rec, const_cast<SkTypeface*>(this));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkFontDescriptor.h"

int SkTypeface::onGetUPEM() const { return 0; }
int SkTypeface::onGetTableTags(SkFontTableTag tags[]) const { return 0; }
size_t SkTypeface::onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const { return 0; }
void SkTypeface::onGetFontDescriptor(SkFontDescriptor* desc) const {
    desc->setStyle(this->style());
}

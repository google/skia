
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
                                SkAdvancedTypefaceMetrics::PerGlyphInfo info,
                                const uint32_t* glyphIDs,
                                uint32_t glyphIDsCount) const {
    return this->onGetAdvancedTypefaceMetrics(info, glyphIDs, glyphIDsCount);
}

///////////////////////////////////////////////////////////////////////////////

int SkTypeface::countTables() const {
    return this->onGetTableTags(NULL);
}

int SkTypeface::getTableTags(SkFontTableTag tags[]) const {
    return this->onGetTableTags(tags);
}

size_t SkTypeface::getTableSize(SkFontTableTag tag) const {
    return this->onGetTableData(tag, 0, ~0U, NULL);
}

size_t SkTypeface::getTableData(SkFontTableTag tag, size_t offset, size_t length,
                                void* data) const {
    return this->onGetTableData(tag, offset, length, data);
}

SkStream* SkTypeface::openStream(int* ttcIndex) const {
    int ttcIndexStorage;
    if (NULL == ttcIndex) {
        // So our subclasses don't need to check for null param
        ttcIndex = &ttcIndexStorage;
    }
    return this->onOpenStream(ttcIndex);
}

int SkTypeface::getUnitsPerEm() const {
    // should we try to cache this in the base-class?
    return this->onGetUPEM();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkFontDescriptor.h"

int SkTypeface::onGetUPEM() const {
    int upem = 0;

    SkAdvancedTypefaceMetrics* metrics;
    metrics = this->getAdvancedTypefaceMetrics(
                             SkAdvancedTypefaceMetrics::kNo_PerGlyphInfo,
                             NULL, 0);
    if (metrics) {
        upem = metrics->fEmSize;
        metrics->unref();
    }
    return upem;
}

SkStream* SkTypeface::onOpenStream(int* ttcIndex) const {
    // If this has not been overridden, then we just don't know the ttcIndex
    // so we set it to 0
    if (ttcIndex) {
        *ttcIndex = 0;
    }
    return SkFontHost::OpenStream(fUniqueID);
}

void SkTypeface::onGetFontDescriptor(SkFontDescriptor* desc) const {
    desc->setStyle(this->style());
}

#include "SkFontStream.h"
#include "SkStream.h"

int SkTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    int ttcIndex;
    SkAutoTUnref<SkStream> stream(this->openStream(&ttcIndex));
    return stream.get() ? SkFontStream::GetTableTags(stream, ttcIndex, tags) : 0;
}

size_t SkTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                  size_t length, void* data) const {
    int ttcIndex;
    SkAutoTUnref<SkStream> stream(this->openStream(&ttcIndex));
    return stream.get()
        ? SkFontStream::GetTableData(stream, ttcIndex, tag, offset, length, data)
        : 0;
}


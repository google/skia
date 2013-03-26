/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
#include "SkFontStream.h"
#include "SkStream.h"
#include "SkTypeface.h"

SK_DEFINE_INST_COUNT(SkTypeface)

//#define TRACE_LIFECYCLE

#ifdef TRACE_LIFECYCLE
    static int32_t gTypefaceCounter;
#endif

SkTypeface::SkTypeface(Style style, SkFontID fontID, bool isFixedPitch)
    : fUniqueID(fontID), fStyle(style), fIsFixedPitch(isFixedPitch) {
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

void SkTypeface::serialize(SkWStream* wstream) const {
    bool isLocal = false;
    SkFontDescriptor desc(this->style());
    this->onGetFontDescriptor(&desc, &isLocal);

    desc.serialize(wstream);
    if (isLocal) {
        int ttcIndex;   // TODO: write this to the stream?
        SkAutoTUnref<SkStream> rstream(this->openStream(&ttcIndex));
        if (rstream.get()) {
            size_t length = rstream->getLength();
            wstream->writePackedUInt(length);
            wstream->writeStream(rstream, length);
        } else {
            wstream->writePackedUInt(0);
        }
    } else {
        wstream->writePackedUInt(0);
    }
}

SkTypeface* SkTypeface::Deserialize(SkStream* stream) {
    SkFontDescriptor desc(stream);
    size_t length = stream->readPackedUInt();
    if (length > 0) {
        void* addr = sk_malloc_flags(length, 0);
        if (addr) {
            SkAutoTUnref<SkStream> localStream(SkNEW_ARGS(SkMemoryStream,
                                                        (addr, length, false)));
            return SkTypeface::CreateFromStream(localStream.get());
        }
        // failed to allocate, so just skip and create-from-name
        stream->skip(length);
    }

    return SkTypeface::CreateFromName(desc.getFamilyName(), desc.getStyle());
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

SkAdvancedTypefaceMetrics* SkTypeface::getAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo info,
                                const uint32_t* glyphIDs,
                                uint32_t glyphIDsCount) const {
    return this->onGetAdvancedTypefaceMetrics(info, glyphIDs, glyphIDsCount);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
